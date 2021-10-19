/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Mapper.cpp
 *   Implementation of functionality used to implement mappings of an entire
 *   XInput controller layout to a virtual controller layout.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Globals.h"
#include "Mapper.h"
#include "Message.h"
#include "Strings.h"

#ifndef XIDI_SKIP_CONFIG
#include "Configuration.h"
#endif

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
                        const SCapabilities kKnownMapperCapabilities = knownMapper.second->GetCapabilities();
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
                if (true == name.empty())
                {
                    Message::Output(Message::ESeverity::Error, L"Internal error due to attempting to register a mapper without a name.");
                    return;
                }

                knownMappers[name] = object;

                if (true == defaultMapper.empty())
                    defaultMapper = name;
            }

            /// Unregisters a mapper object from this registry, if the registration details provided match the contents of the registry.
            /// @param [in] name Name associated with the mapper.
            /// @param [in] object Corresponding mapper object.
            void UnregisterMapper(std::wstring_view name, const Mapper* object)
            {
                if (true == name.empty())
                {
                    Message::Output(Message::ESeverity::Error, L"Internal error due to attempting to unregister a mapper without a name.");
                    return;
                }

                if (defaultMapper == name)
                {
                    Message::OutputFormatted(Message::ESeverity::Error, L"Internal error due to attempting to unregister the default mapper %s.", name.data());
                    return;
                }

                if (false == knownMappers.contains(name))
                {
                    Message::OutputFormatted(Message::ESeverity::Error, L"Internal error due to attempting to unregister unknown mapper %s.", name.data());
                    return;
                }

                if (object != knownMappers.at(name))
                {
                    Message::OutputFormatted(Message::ESeverity::Error, L"Internal error due to object mismatch while attempting to unregister mapper %s.", name.data());
                    return;
                }

                knownMappers.erase(name);
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
                    for (int j = 0; j < elements.all[i]->GetTargetElementCount(); ++j)
                    {
                        const std::optional<SElementIdentifier> maybeTargetElement = elements.all[i]->GetTargetElementAt(j);
                        if (false == maybeTargetElement.has_value())
                            continue;

                        const SElementIdentifier targetElement = maybeTargetElement.value();
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
        // See "Mapper.h" for documentation.

        Mapper::UElementMap::UElementMap(const UElementMap& other) : named()
        {
            for (int i = 0; i < _countof(all); ++i)
            {
                if (nullptr != other.all[i])
                    all[i] = other.all[i]->Clone();
            }
        }

        // --------

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

        // --------

        Mapper::~Mapper(void)
        {
            if (false == name.empty())
            {
                MapperRegistry::GetInstance().UnregisterMapper(name, this);
                Message::OutputFormatted(Message::ESeverity::Warning, L"Unregistered and destroyed mapper %s.", name.data());
            }
        }


        // -------- OPERATORS ---------------------------------------------- //
        // See "Mapper.h" for documentation.

        Mapper::UElementMap& Mapper::UElementMap::operator=(const UElementMap& other)
        {
            for (int i = 0; i < _countof(all); ++i)
            {
                if (nullptr == other.all[i])
                    all[i] = nullptr;
                else
                    all[i] = other.all[i]->Clone();
            }

            return *this;
        }

        // --------

        Mapper::UElementMap& Mapper::UElementMap::operator=(UElementMap&& other)
        {
            for (int i = 0; i < _countof(all); ++i)
                all[i] = std::move(other.all[i]);

            return *this;
        }


        // -------- CLASS METHODS ------------------------------------------ //
        // See "Mapper.h" for documentation.

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

        const Mapper* Mapper::GetConfigured(TControllerIdentifier controllerIdentifier)
        {
            static const Mapper* configuredMapper[kPhysicalControllerCount];
            static std::once_flag configuredMapperFlag;

            std::call_once(configuredMapperFlag, []() -> void
                {
#ifndef XIDI_SKIP_CONFIG
                    const Configuration::ConfigurationFile& config = Globals::GetConfiguration();

                    if ((true == config.IsDataValid()) && (true == config.GetData().SectionExists(Strings::kStrConfigurationSectionMapper)))
                    {
                        // Mapper section exists in the configuration file.
                        // If the controller-independent type setting exists, it will be used as the fallback default, otherwise the default mapper will be used for this purpose.
                        // If any per-controller type settings exist, they take precedence.
                        const auto& mapperConfigData = config.GetData()[Strings::kStrConfigurationSectionMapper];

                        const Mapper* fallbackMapper = nullptr;
                        if (true == mapperConfigData.NameExists(Strings::kStrConfigurationSettingMapperType))
                        {
                            std::wstring_view fallbackMapperName = mapperConfigData[Strings::kStrConfigurationSettingMapperType].FirstValue().GetStringValue();
                            fallbackMapper = GetByName(fallbackMapperName);

                            if (nullptr == fallbackMapper)
                                Message::OutputFormatted(Message::ESeverity::Warning, L"Could not locate mapper '%s' specified in the configuration file as the default.", fallbackMapperName.data());
                        }

                        if (nullptr == fallbackMapper)
                        {
                            fallbackMapper = GetDefault();

                            if (nullptr == fallbackMapper)
                            {
                                Message::Output(Message::ESeverity::Error, L"Internal error due to inability to locate the default mapper.");
                                fallbackMapper = GetNull();
                            }
                        }
                        
                        for (TControllerIdentifier i = 0; i < _countof(configuredMapper); ++i)
                        {
                            if (true == mapperConfigData.NameExists(Strings::MapperTypeConfigurationNameString(i)))
                            {
                                std::wstring_view configuredMapperName = mapperConfigData[Strings::MapperTypeConfigurationNameString(i)].FirstValue().GetStringValue();
                                configuredMapper[i] = GetByName(configuredMapperName.data());

                                if (nullptr == configuredMapper[i])
                                {
                                    Message::OutputFormatted(Message::ESeverity::Warning, L"Could not locate mapper '%s' specified in the configuration file for controller %u.", configuredMapperName.data(), (unsigned int)(1 + i));
                                    configuredMapper[i] = fallbackMapper;
                                }
                            }
                            else
                            {
                                configuredMapper[i] = fallbackMapper;
                            }
                        }
                    }
                    else
#endif
                    {
                        // Mapper section does not exist in the configuration file, or configuration file code was skipped entirely.
                        const Mapper* defaultMapper = GetDefault();
                        if (nullptr == defaultMapper)
                        {
                            Message::Output(Message::ESeverity::Error, L"Internal error due to inability to locate the default mapper. Virtual controllers will not function.");
                            defaultMapper = GetNull();
                        }

                        for (TControllerIdentifier i = 0; i < _countof(configuredMapper); ++i)
                            configuredMapper[i] = defaultMapper;
                    }

                    Message::Output(Message::ESeverity::Info, L"Mappers assigned to controllers...");
                    for (TControllerIdentifier i = 0; i < _countof(configuredMapper); ++i)
                        Message::OutputFormatted(Message::ESeverity::Info, L"    [%u]: %s", (unsigned int)(1 + i), configuredMapper[i]->GetName().data());
                }
            );

            if (controllerIdentifier >= _countof(configuredMapper))
            {
                Message::OutputFormatted(Message::ESeverity::Error, L"Internal error due to requesting a mapper for out-of-bounds controller %u.", (unsigned int)(1 + controllerIdentifier));
                return GetNull();
            }

            return configuredMapper[controllerIdentifier];
        }

        // --------

        const Mapper* Mapper::GetNull(void)
        {
            static const Mapper kNullMapper({});
            return &kNullMapper;
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "Mapper.h" for documentation.

        void Mapper::MapXInputState(TControllerIdentifier controllerIdentifier, SState& controllerState, XINPUT_GAMEPAD xinputState) const
        {
            ZeroMemory(&controllerState, sizeof(controllerState));

            // Left and right stick values need to be saturated at the virtual controller range due to a very slight difference between XInput range and virtual controller range.
            // This difference (-32768 extreme negative for XInput vs -32767 extreme negative for Xidi) does not affect functionality when filtered by saturation.
            // Vertical analog axes additionally need to be inverted because XInput presents up as positive and down as negative whereas Xidi needs to do the opposite.
            
            if (nullptr != elements.named.stickLeftX) elements.named.stickLeftX->ContributeFromAnalogValue(controllerIdentifier, controllerState, FilterAnalogStickValue(xinputState.sThumbLX));
            if (nullptr != elements.named.stickLeftY) elements.named.stickLeftY->ContributeFromAnalogValue(controllerIdentifier, controllerState, FilterAndInvertAnalogStickValue(xinputState.sThumbLY));

            if (nullptr != elements.named.stickRightX) elements.named.stickRightX->ContributeFromAnalogValue(controllerIdentifier, controllerState, FilterAnalogStickValue(xinputState.sThumbRX));
            if (nullptr != elements.named.stickRightY) elements.named.stickRightY->ContributeFromAnalogValue(controllerIdentifier, controllerState, FilterAndInvertAnalogStickValue(xinputState.sThumbRY));

            if (nullptr != elements.named.dpadUp) elements.named.dpadUp->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_UP)));
            if (nullptr != elements.named.dpadDown) elements.named.dpadDown->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)));
            if (nullptr != elements.named.dpadLeft) elements.named.dpadLeft->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)));
            if (nullptr != elements.named.dpadRight) elements.named.dpadRight->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)));

            if (nullptr != elements.named.triggerLT) elements.named.triggerLT->ContributeFromTriggerValue(controllerIdentifier, controllerState, xinputState.bLeftTrigger);
            if (nullptr != elements.named.triggerRT) elements.named.triggerRT->ContributeFromTriggerValue(controllerIdentifier, controllerState, xinputState.bRightTrigger);

            if (nullptr != elements.named.buttonA) elements.named.buttonA->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_A)));
            if (nullptr != elements.named.buttonB) elements.named.buttonB->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_B)));
            if (nullptr != elements.named.buttonX) elements.named.buttonX->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_X)));
            if (nullptr != elements.named.buttonY) elements.named.buttonY->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_Y)));

            if (nullptr != elements.named.buttonLB) elements.named.buttonLB->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)));
            if (nullptr != elements.named.buttonRB) elements.named.buttonRB->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)));

            if (nullptr != elements.named.buttonBack) elements.named.buttonBack->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_BACK)));
            if (nullptr != elements.named.buttonStart) elements.named.buttonStart->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_START)));

            if (nullptr != elements.named.buttonLS) elements.named.buttonLS->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)));
            if (nullptr != elements.named.buttonRS) elements.named.buttonRS->ContributeFromButtonValue(controllerIdentifier, controllerState, (0 != (xinputState.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)));

            // Once all contributions have been committed, saturate all axis values at the extreme ends of the allowed range.
            // Doing this at the end means that intermediate contributions are computed with much more range than the controller is allowed to report, which can increase accuracy when there are multiple interfering mappers contributing to axes.
            for (auto& axisValue : controllerState.axis)
            {
                if (axisValue > kAnalogValueMax)
                    axisValue = kAnalogValueMax;
                else if (axisValue < kAnalogValueMin)
                    axisValue = kAnalogValueMin;
            }
        }
    }
}
