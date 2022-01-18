/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MapperDefinitions.cpp
 *   Definitions of all known mapper types.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"


namespace Xidi
{
    namespace Controller
    {
        // -------- MAPPER DEFINITIONS ------------------------------------- //

        /// Defines all known mapper types, one element per type. The first element is the default mapper.
        /// Any field that corresponds to an XInput controller element can be omitted or assigned `nullptr` and the mapper will simply ignore input from that XInput controller element.
        static const Mapper kMappers[] = {

            Mapper(
                L"StandardGamepad",
                {
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
                }
            ),

            Mapper(
                L"DigitalGamepad",
                {
                    .stickLeftX = std::make_unique<DigitalAxisMapper>(EAxis::X),
                    .stickLeftY = std::make_unique<DigitalAxisMapper>(EAxis::Y),
                    .stickRightX = std::make_unique<DigitalAxisMapper>(EAxis::Z),
                    .stickRightY = std::make_unique<DigitalAxisMapper>(EAxis::RotZ),
                    .dpadUp = std::make_unique<DigitalAxisMapper>(EAxis::Y, EAxisDirection::Negative),
                    .dpadDown = std::make_unique<DigitalAxisMapper>(EAxis::Y, EAxisDirection::Positive),
                    .dpadLeft = std::make_unique<DigitalAxisMapper>(EAxis::X, EAxisDirection::Negative),
                    .dpadRight = std::make_unique<DigitalAxisMapper>(EAxis::X, EAxisDirection::Positive),
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
                }
            ),

            Mapper(
                L"ExtendedGamepad",
                {
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
                }
            ),

            Mapper(
                L"XInputNative",
                {
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
                }
            ),

            Mapper(
                L"XInputSharedTriggers",
                {
                    .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
                    .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
                    .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
                    .stickRightY = std::make_unique<AxisMapper>(EAxis::RotY),
                    .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
                    .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
                    .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
                    .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
                    .triggerLT = std::make_unique<AxisMapper>(EAxis::Z, EAxisDirection::Positive),
                    .triggerRT = std::make_unique<AxisMapper>(EAxis::Z, EAxisDirection::Negative),
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
                }
            )
        };
    }
}
