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
#include "ControllerElementMapper.h"
#include "ControllerMapper.h"
#include "ControllerTypes.h"

#include <set>
#include <string_view>
#include <unordered_map>
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
            std::unordered_map<std::wstring_view, const Mapper*> knownMappers;

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
                    switch (elements.all[i]->GetTargetElementType())
                    {
                    case EElementType::Axis:
                        if (elements.all[i]->GetTargetElementIndex() < (int)EAxis::Count)
                            axesPresent.insert((EAxis)elements.all[i]->GetTargetElementIndex());
                        break;

                    case EElementType::Button:
                        if (elements.all[i]->GetTargetElementIndex() < (int)EButton::Count)
                        {
                            const int button = elements.all[i]->GetTargetElementIndex();
                            if (button > highestButtonSeen)
                                highestButtonSeen = button;
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


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //
        // See "ControllerMapper.h" for documentation.

        Mapper::Mapper(const std::wstring_view name, const SElementMap& elements) : capabilities(DeriveCapabilitiesFromElementMap(elements)), elements(elements), name(name)
        {
            if (false == name.empty())
                MapperRegistry::GetInstance().RegisterMapper(name, this);
        }

        // --------

        Mapper::Mapper(const SElementMap& elements) : Mapper(L"", elements)
        {
            // Nothing to do here.
        }

        // --------

        Mapper::~Mapper(void)
        {
            for (int i = 0; i < _countof(elements.all); ++i)
            {
                if (nullptr != elements.all[i])
                    delete elements.all[i];
            }
        }


        // -------- CLASS METHODS ------------------------------------------ //
        // See "ControllerMapper.h" for documentation.

        const Mapper* Mapper::GetByName(std::wstring_view mapperName)
        {
            return MapperRegistry::GetInstance().GetMapper(mapperName);
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "ControllerMapper.h" for documentation.

        void Mapper::MapXInputState(SState* controllerState, const XINPUT_GAMEPAD& xinputState) const
        {
            ZeroMemory(controllerState, sizeof(*controllerState));

            if (nullptr != elements.named.stickLeftX) elements.named.stickLeftX->ContributeFromAnalogValue(controllerState, xinputState.sThumbLX);
            if (nullptr != elements.named.stickLeftY) elements.named.stickLeftY->ContributeFromAnalogValue(controllerState, xinputState.sThumbLY);

            if (nullptr != elements.named.stickRightX) elements.named.stickRightX->ContributeFromAnalogValue(controllerState, xinputState.sThumbRX);
            if (nullptr != elements.named.stickRightY) elements.named.stickRightY->ContributeFromAnalogValue(controllerState, xinputState.sThumbRY);

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
                .stickLeftX = new AxisMapper(EAxis::X),
                .stickLeftY = new AxisMapper(EAxis::Y),
                .stickRightX = new AxisMapper(EAxis::Z),
                .stickRightY = new AxisMapper(EAxis::RotZ),
                .dpadUp = new PovMapper(EPov::Up),
                .dpadDown = new PovMapper(EPov::Down),
                .dpadLeft = new PovMapper(EPov::Left),
                .dpadRight = new PovMapper(EPov::Right),
                .triggerLT = new ButtonMapper(EButton::B7),
                .triggerRT = new ButtonMapper(EButton::B8),
                .buttonA = new ButtonMapper(EButton::B1),
                .buttonB = new ButtonMapper(EButton::B2),
                .buttonX = new ButtonMapper(EButton::B3),
                .buttonY = new ButtonMapper(EButton::B4),
                .buttonLB = new ButtonMapper(EButton::B5),
                .buttonRB = new ButtonMapper(EButton::B6),
                .buttonBack = new ButtonMapper(EButton::B9),
                .buttonStart = new ButtonMapper(EButton::B10),
                .buttonLS = new ButtonMapper(EButton::B11),
                .buttonRS = new ButtonMapper(EButton::B12)
            }),
            Mapper(L"DigitalGamepad", {
                .stickLeftX = new DigitalAxisMapper(EAxis::X),
                .stickLeftY = new DigitalAxisMapper(EAxis::Y),
                .stickRightX = new DigitalAxisMapper(EAxis::Z),
                .stickRightY = new DigitalAxisMapper(EAxis::RotZ),
                .dpadUp = new DigitalAxisMapper(EAxis::Y, AxisMapper::EDirection::Negative),
                .dpadDown = new DigitalAxisMapper(EAxis::Y, AxisMapper::EDirection::Positive),
                .dpadLeft = new DigitalAxisMapper(EAxis::X, AxisMapper::EDirection::Negative),
                .dpadRight = new DigitalAxisMapper(EAxis::X, AxisMapper::EDirection::Positive),
                .triggerLT = new ButtonMapper(EButton::B7),
                .triggerRT = new ButtonMapper(EButton::B8),
                .buttonA = new ButtonMapper(EButton::B1),
                .buttonB = new ButtonMapper(EButton::B2),
                .buttonX = new ButtonMapper(EButton::B3),
                .buttonY = new ButtonMapper(EButton::B4),
                .buttonLB = new ButtonMapper(EButton::B5),
                .buttonRB = new ButtonMapper(EButton::B6),
                .buttonBack = new ButtonMapper(EButton::B9),
                .buttonStart = new ButtonMapper(EButton::B10),
                .buttonLS = new ButtonMapper(EButton::B11),
                .buttonRS = new ButtonMapper(EButton::B12)
            }),
            Mapper(L"ExtendedGamepad", {
                .stickLeftX = new AxisMapper(EAxis::X),
                .stickLeftY = new AxisMapper(EAxis::Y),
                .stickRightX = new AxisMapper(EAxis::Z),
                .stickRightY = new AxisMapper(EAxis::RotZ),
                .dpadUp = new PovMapper(EPov::Up),
                .dpadDown = new PovMapper(EPov::Down),
                .dpadLeft = new PovMapper(EPov::Left),
                .dpadRight = new PovMapper(EPov::Right),
                .triggerLT = new AxisMapper(EAxis::RotX),
                .triggerRT = new AxisMapper(EAxis::RotY),
                .buttonA = new ButtonMapper(EButton::B1),
                .buttonB = new ButtonMapper(EButton::B2),
                .buttonX = new ButtonMapper(EButton::B3),
                .buttonY = new ButtonMapper(EButton::B4),
                .buttonLB = new ButtonMapper(EButton::B5),
                .buttonRB = new ButtonMapper(EButton::B6),
                .buttonBack = new ButtonMapper(EButton::B7),
                .buttonStart = new ButtonMapper(EButton::B8),
                .buttonLS = new ButtonMapper(EButton::B9),
                .buttonRS = new ButtonMapper(EButton::B10)
            }),
            Mapper(L"XInputNative", {
                .stickLeftX = new AxisMapper(EAxis::X),
                .stickLeftY = new AxisMapper(EAxis::Y),
                .stickRightX = new AxisMapper(EAxis::RotX),
                .stickRightY = new AxisMapper(EAxis::RotY),
                .dpadUp = new PovMapper(EPov::Up),
                .dpadDown = new PovMapper(EPov::Down),
                .dpadLeft = new PovMapper(EPov::Left),
                .dpadRight = new PovMapper(EPov::Right),
                .triggerLT = new AxisMapper(EAxis::Z),
                .triggerRT = new AxisMapper(EAxis::RotZ),
                .buttonA = new ButtonMapper(EButton::B1),
                .buttonB = new ButtonMapper(EButton::B2),
                .buttonX = new ButtonMapper(EButton::B3),
                .buttonY = new ButtonMapper(EButton::B4),
                .buttonLB = new ButtonMapper(EButton::B5),
                .buttonRB = new ButtonMapper(EButton::B6),
                .buttonBack = new ButtonMapper(EButton::B7),
                .buttonStart = new ButtonMapper(EButton::B8),
                .buttonLS = new ButtonMapper(EButton::B9),
                .buttonRS = new ButtonMapper(EButton::B10)
            }),
            Mapper(L"XInputSharedTriggers", {
                .stickLeftX = new AxisMapper(EAxis::X),
                .stickLeftY = new AxisMapper(EAxis::Y),
                .stickRightX = new AxisMapper(EAxis::RotX),
                .stickRightY = new AxisMapper(EAxis::RotY),
                .dpadUp = new PovMapper(EPov::Up),
                .dpadDown = new PovMapper(EPov::Down),
                .dpadLeft = new PovMapper(EPov::Left),
                .dpadRight = new PovMapper(EPov::Right),
                .triggerLT = new AxisMapper(EAxis::Z, AxisMapper::EDirection::Positive),
                .triggerRT = new AxisMapper(EAxis::Z, AxisMapper::EDirection::Negative),
                .buttonA = new ButtonMapper(EButton::B1),
                .buttonB = new ButtonMapper(EButton::B2),
                .buttonX = new ButtonMapper(EButton::B3),
                .buttonY = new ButtonMapper(EButton::B4),
                .buttonLB = new ButtonMapper(EButton::B5),
                .buttonRB = new ButtonMapper(EButton::B6),
                .buttonBack = new ButtonMapper(EButton::B7),
                .buttonStart = new ButtonMapper(EButton::B8),
                .buttonLS = new ButtonMapper(EButton::B9),
                .buttonRS = new ButtonMapper(EButton::B10)
            })
        };
    }
}
