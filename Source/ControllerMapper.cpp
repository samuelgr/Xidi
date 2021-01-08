/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ControllerMapper.cpp
 *   Implementation of functionality used to implement mappings of an entire
 *   XInput controller layout to a virtual controller layout.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Configuration.h"
#include "ControllerElementMapper.h"
#include "ControllerMapper.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Message.h"
#include "Strings.h"

#include <map>
#include <mutex>
#include <set>
#include <string_view>
#include <xinput.h>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Holds a mapping from strings to instances of mapper objects.
        /// Implemented as a singleton object and intended for internal use.
        class MapperRegistry
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Implements the registry of known mappers.
            std::map<std::wstring_view, const Mapper*> knownMappers;

            /// Holds the map key that corresponds to the default mapper.
            /// The first type of mapper that is registered becomes the default.
            std::wstring_view defaultMapper;


            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default constructor. Objects cannot be constructed externally.
            MapperRegistry(void) = default;


        public:
            // -------- CLASS METHODS -------------------------------------- //

            /// Returns a reference to the singleton instance of this class.
            /// @return Reference to the singleton instance.
            static MapperRegistry& GetInstance(void)
            {
                static MapperRegistry mapperRegistry;
                return mapperRegistry;
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Dumps all mappers in this registry.
            void DumpRegisteredMappers(void)
            {
                constexpr Message::ESeverity kDumpSeverity = Message::ESeverity::Info;

                if (Message::WillOutputMessageOfSeverity(kDumpSeverity))
                {
                    Message::Output(kDumpSeverity, L"Begin dump of all known mappers.");

                    Message::Output(kDumpSeverity, L"  Default:");
                    Message::OutputFormatted(kDumpSeverity, L"    %s", defaultMapper);

                    Message::Output(kDumpSeverity, L"  All:");
                    
                    for (const auto& knownMapper : knownMappers)
                    {
                        const std::wstring_view kKnownMapperName = knownMapper.first;
                        const SCapabilities& kKnownMapperCapabilities = knownMapper.second->GetCapabilities();
                        Message::OutputFormatted(kDumpSeverity, L"    %-24s { numAxes = %u, numButtons = %u, hasPov = %s }", kKnownMapperName.data(), (unsigned int)kKnownMapperCapabilities.numAxes, (unsigned int)kKnownMapperCapabilities.numButtons, ((true == kKnownMapperCapabilities.hasPov) ? L"true" : L"false"));
                    }

                    Message::Output(kDumpSeverity, L"End dump of all known mappers.");
                }
            }
            
            /// Registers a mapper object with this registry.
            /// @param [in] name Name to associate with the mapper.
            /// @param [in] object Corresponding mapper object.
            void RegisterMapper(std::wstring_view name, const Mapper* object)
            {
                knownMappers[name] = object;
                
                if (true == defaultMapper.empty())
                    defaultMapper = name;
            }

            /// Retrieves a pointer to the mapper object that corresponds to the specified name, if it exists.
            /// @param [in] mapperName Desired mapper name.
            /// @return Pointer to the corresponding mapper object, or `nullptr` if it does not exist in the registry.
            const Mapper* GetMapper(std::wstring_view mapperName)
            {
                if (true == mapperName.empty())
                    mapperName = defaultMapper;

                const auto mapperRecord = knownMappers.find(mapperName);
                if (knownMappers.cend() != mapperRecord)
                    return mapperRecord->second;

                return nullptr;
            }
        };


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Derives the capabilities of the controller that is described by the specified element mappers in aggregate.
        /// Number of axes is determined as the total number of unique axes on the virtual controller to which element mappers contribute.
        /// Number of buttons is determined by looking at the highest button number to which element mappers contribute.
        /// Presence or absence of a POV is determined by whether or not any element mappers contribute to a POV direction, even if not all POV directions have a contribution.
        /// @param [in] elements Per-element controller map.
        /// @return Virtual controller capabilities as derived from the per-element map in aggregate.
        static SCapabilities DeriveCapabilitiesFromElementMap(const Mapper::UElementMap& elements)
        {
            SCapabilities capabilities;
            ZeroMemory(&capabilities, sizeof(capabilities));

            std::set<EAxis> axesPresent;
            int highestButtonSeen = -1;
            bool povPresent = false;

            for (int i = 0; i < _countof(elements.all); ++i)
            {
                if (nullptr != elements.all[i])
                {
                    const SElementIdentifier targetElement = elements.all[i]->GetTargetElement();
                    switch (targetElement.type)
                    {
                    case EElementType::Axis:
                        if ((int)targetElement.axis < (int)EAxis::Count)
                            axesPresent.insert(targetElement.axis);
                        break;

                    case EElementType::Button:
                        if ((int)targetElement.button < (int)EButton::Count)
                        {
                            if ((int)targetElement.button > highestButtonSeen)
                                highestButtonSeen = (int)targetElement.button;
                        }
                        break;

                    case EElementType::Pov:
                        povPresent = true;
                        break;
                    }
                }
            }

            for (auto it = axesPresent.cbegin(); it != axesPresent.cend(); ++it)
                capabilities.AppendAxis(*it);

            capabilities.numButtons = highestButtonSeen + 1;
            capabilities.hasPov = povPresent;

            return capabilities;
        }


        /// Filters (by saturation) analog stick values that might be slightly out of range due to differences between the implemented range and the XInput actual range.
        /// @param [in] analogValue Raw analog value.
        /// @return Filtered analog value, which will most likely be the same as the input.
        static inline int16_t FilterAnalogStickValue(int16_t analogValue)
        {
            if (analogValue > Controller::kAnalogValueMax)
                return Controller::kAnalogValueMax;
            else if (analogValue < Controller::kAnalogValueMin)
                return Controller::kAnalogValueMin;
            else
                return analogValue;
        }

        /// Filters and inverts analog stick values based on presentation differences between XInput and virtual controller needs.
        /// Useful for XInput axes that have opposite polarity as compared to virtual controller axes.
        /// @param [in] analogValue Raw analog value.
        /// @return Filtered and inverted analog value.
        static inline int16_t FilterAndInvertAnalogStickValue(int16_t analogValue)
        {
            return -FilterAnalogStickValue(analogValue);
        }


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //
        // See "ControllerMapper.h" for documentation.

        Mapper::Mapper(const std::wstring_view name, SElementMap&& elements) : elements(std::move(elements)), capabilities(DeriveCapabilitiesFromElementMap(this->elements)), name(name)
        {
            if (false == name.empty())
                MapperRegistry::GetInstance().RegisterMapper(name, this);
        }

        // --------

        Mapper::Mapper(SElementMap&& elements) : Mapper(L"", std::move(elements))
        {
            // Nothing to do here.
        }


        // -------- CLASS METHODS ------------------------------------------ //
        // See "ControllerMapper.h" for documentation.

        void Mapper::DumpRegisteredMappers(void)
        {
            MapperRegistry::GetInstance().DumpRegisteredMappers();
        }

        // --------

        const Mapper* Mapper::GetByName(std::wstring_view mapperName)
        {
            return MapperRegistry::GetInstance().GetMapper(mapperName);
        }

        // --------

        const Mapper* Mapper::GetConfigured(void)
        {
            static const Mapper* configuredMapper = nullptr;
            static std::once_flag configuredMapperFlag;

            std::call_once(configuredMapperFlag, []() -> void {
                const Configuration::Configuration& config = Globals::GetConfiguration();

                if ((true == config.IsDataValid()) && (true == config.GetData().SectionNamePairExists(Strings::kStrConfigurationSectionMapper, Strings::kStrConfigurationSettingMapperType)))
                {
                    const std::wstring_view kConfiguredMapperName = config.GetData()[Strings::kStrConfigurationSectionMapper][Strings::kStrConfigurationSettingMapperType].FirstValue().GetStringValue();

                    Message::OutputFormatted(Message::ESeverity::Info, L"Attempting to locate mapper '%s' specified in the configuration file.", kConfiguredMapperName.data());
                    configuredMapper = GetByName(kConfiguredMapperName);
                }
                
                if (nullptr == configuredMapper)
                {
                    Message::Output(Message::ESeverity::Info, L"Could not locate mapper specified in the configuration file, or no mapper was specified. Using default mapper instead.");
                    configuredMapper = GetDefault();
                }

                if (nullptr == configuredMapper)
                    Message::Output(Message::ESeverity::Error, L"No mappers could be located. Xidi virtual controllers are unable to function.");
            });

            return configuredMapper;
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "ControllerMapper.h" for documentation.

        void Mapper::MapXInputState(SState* controllerState, const XINPUT_GAMEPAD& xinputState) const
        {
            ZeroMemory(controllerState, sizeof(*controllerState));

            // Left and right stick values need to be saturated at the virtual controller range due to a very slight difference between XInput range and virtual controller range.
            // This difference (-32768 extreme negative for XInput vs -32767 extreme negative for Xidi) does not affect functionality when filtered by saturation.
            // Vertical analog axes additionally need to be inverted because XInput presents up as positive and down as negative whereas Xidi needs to do the opposite.
            
            if (nullptr != elements.named.stickLeftX) elements.named.stickLeftX->ContributeFromAnalogValue(controllerState, FilterAnalogStickValue(xinputState.sThumbLX));
            if (nullptr != elements.named.stickLeftY) elements.named.stickLeftY->ContributeFromAnalogValue(controllerState, FilterAndInvertAnalogStickValue(xinputState.sThumbLY));

            if (nullptr != elements.named.stickRightX) elements.named.stickRightX->ContributeFromAnalogValue(controllerState, FilterAnalogStickValue(xinputState.sThumbRX));
            if (nullptr != elements.named.stickRightY) elements.named.stickRightY->ContributeFromAnalogValue(controllerState, FilterAndInvertAnalogStickValue(xinputState.sThumbRY));

            if (nullptr != elements.named.dpadUp) elements.named.dpadUp->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_UP)));
            if (nullptr != elements.named.dpadDown) elements.named.dpadDown->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)));
            if (nullptr != elements.named.dpadLeft) elements.named.dpadLeft->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)));
            if (nullptr != elements.named.dpadRight) elements.named.dpadRight->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)));

            if (nullptr != elements.named.triggerLT) elements.named.triggerLT->ContributeFromTriggerValue(controllerState, xinputState.bLeftTrigger);
            if (nullptr != elements.named.triggerRT) elements.named.triggerRT->ContributeFromTriggerValue(controllerState, xinputState.bRightTrigger);

            if (nullptr != elements.named.buttonA) elements.named.buttonA->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_A)));
            if (nullptr != elements.named.buttonB) elements.named.buttonB->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_B)));
            if (nullptr != elements.named.buttonX) elements.named.buttonX->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_X)));
            if (nullptr != elements.named.buttonY) elements.named.buttonY->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_Y)));

            if (nullptr != elements.named.buttonLB) elements.named.buttonLB->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)));
            if (nullptr != elements.named.buttonRB) elements.named.buttonRB->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)));

            if (nullptr != elements.named.buttonBack) elements.named.buttonBack->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_BACK)));
            if (nullptr != elements.named.buttonStart) elements.named.buttonStart->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_START)));

            if (nullptr != elements.named.buttonLS) elements.named.buttonLS->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)));
            if (nullptr != elements.named.buttonRS) elements.named.buttonRS->ContributeFromButtonValue(controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)));

            // Once all contributions have been committed, saturate all axis values at the extreme ends of the allowed range.
            // Doing this at the end means that intermediate contributions are computed with much more range than the controller is allowed to report, which can increase accuracy when there are multiple interfering mappers to axes.
            for (auto& axisValue : controllerState->axis)
            {
                if (axisValue > kAnalogValueMax)
                    axisValue = kAnalogValueMax;
                else if (axisValue < kAnalogValueMin)
                    axisValue = kAnalogValueMin;
            }
        }


        // -------- MAPPERS ------------------------------------------------ //

        /// Defines all known mapper types, one element per type.
        /// The first element is the default mapper.
        static const Mapper kMappers[] = {
            Mapper(L"StandardGamepad", {
                .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
                .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
                .stickRightX = std::make_unique<AxisMapper>(EAxis::Z),
                .stickRightY = std::make_unique<AxisMapper>(EAxis::RotZ),
                .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
                .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
                .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
                .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
                .triggerLT = std::make_unique<ButtonMapper>(EButton::B7),
                .triggerRT = std::make_unique<ButtonMapper>(EButton::B8),
                .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
                .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
                .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
                .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
                .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
                .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
                .buttonBack = std::make_unique<ButtonMapper>(EButton::B9),
                .buttonStart = std::make_unique<ButtonMapper>(EButton::B10),
                .buttonLS = std::make_unique<ButtonMapper>(EButton::B11),
                .buttonRS = std::make_unique<ButtonMapper>(EButton::B12)
            }),
            Mapper(L"DigitalGamepad", {
                .stickLeftX = std::make_unique<DigitalAxisMapper>(EAxis::X),
                .stickLeftY = std::make_unique<DigitalAxisMapper>(EAxis::Y),
                .stickRightX = std::make_unique<DigitalAxisMapper>(EAxis::Z),
                .stickRightY = std::make_unique<DigitalAxisMapper>(EAxis::RotZ),
                .dpadUp = std::make_unique<DigitalAxisMapper>(EAxis::Y, AxisMapper::EDirection::Negative),
                .dpadDown = std::make_unique<DigitalAxisMapper>(EAxis::Y, AxisMapper::EDirection::Positive),
                .dpadLeft = std::make_unique<DigitalAxisMapper>(EAxis::X, AxisMapper::EDirection::Negative),
                .dpadRight = std::make_unique<DigitalAxisMapper>(EAxis::X, AxisMapper::EDirection::Positive),
                .triggerLT = std::make_unique<ButtonMapper>(EButton::B7),
                .triggerRT = std::make_unique<ButtonMapper>(EButton::B8),
                .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
                .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
                .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
                .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
                .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
                .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
                .buttonBack = std::make_unique<ButtonMapper>(EButton::B9),
                .buttonStart = std::make_unique<ButtonMapper>(EButton::B10),
                .buttonLS = std::make_unique<ButtonMapper>(EButton::B11),
                .buttonRS = std::make_unique<ButtonMapper>(EButton::B12)
            }),
            Mapper(L"ExtendedGamepad", {
                .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
                .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
                .stickRightX = std::make_unique<AxisMapper>(EAxis::Z),
                .stickRightY = std::make_unique<AxisMapper>(EAxis::RotZ),
                .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
                .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
                .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
                .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
                .triggerLT = std::make_unique<AxisMapper>(EAxis::RotX),
                .triggerRT = std::make_unique<AxisMapper>(EAxis::RotY),
                .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
                .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
                .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
                .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
                .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
                .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
                .buttonBack = std::make_unique<ButtonMapper>(EButton::B7),
                .buttonStart = std::make_unique<ButtonMapper>(EButton::B8),
                .buttonLS = std::make_unique<ButtonMapper>(EButton::B9),
                .buttonRS = std::make_unique<ButtonMapper>(EButton::B10)
            }),
            Mapper(L"XInputNative", {
                .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
                .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
                .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
                .stickRightY = std::make_unique<AxisMapper>(EAxis::RotY),
                .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
                .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
                .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
                .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
                .triggerLT = std::make_unique<AxisMapper>(EAxis::Z),
                .triggerRT = std::make_unique<AxisMapper>(EAxis::RotZ),
                .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
                .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
                .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
                .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
                .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
                .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
                .buttonBack = std::make_unique<ButtonMapper>(EButton::B7),
                .buttonStart = std::make_unique<ButtonMapper>(EButton::B8),
                .buttonLS = std::make_unique<ButtonMapper>(EButton::B9),
                .buttonRS = std::make_unique<ButtonMapper>(EButton::B10)
            }),
            Mapper(L"XInputSharedTriggers", {
                .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
                .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
                .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
                .stickRightY = std::make_unique<AxisMapper>(EAxis::RotY),
                .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
                .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
                .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
                .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
                .triggerLT = std::make_unique<AxisMapper>(EAxis::Z, AxisMapper::EDirection::Positive),
                .triggerRT = std::make_unique<AxisMapper>(EAxis::Z, AxisMapper::EDirection::Negative),
                .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
                .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
                .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
                .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
                .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
                .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
                .buttonBack = std::make_unique<ButtonMapper>(EButton::B7),
                .buttonStart = std::make_unique<ButtonMapper>(EButton::B8),
                .buttonLS = std::make_unique<ButtonMapper>(EButton::B9),
                .buttonRS = std::make_unique<ButtonMapper>(EButton::B10)
            })
        };
    }
}
