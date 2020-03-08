/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper/ExtendedGamepad.cpp
 *   Implements a mapper that maps to the button layout of an older
 *   DirectInput-compatible gamepad, except with triggers as axes.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "XInputController.h"
#include "Mapper/ExtendedGamepad.h"

using namespace Xidi;
using namespace Xidi::Mapper;


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Mapper/Base.h" for documentation.

const TInstance ExtendedGamepad::MapXInputElementToDirectInputInstance(EXInputControllerElement element)
{
    switch (element)
    {
    case EXInputControllerElement::StickLeftHorizontal:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisX);

    case EXInputControllerElement::StickLeftVertical:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisY);

    case EXInputControllerElement::StickRightHorizontal:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisZ);

    case EXInputControllerElement::StickRightVertical:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisRZ);

    case EXInputControllerElement::TriggerLT:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisRX);

    case EXInputControllerElement::TriggerRT:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisRY);

    case EXInputControllerElement::Dpad:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypePov, (TInstanceIdx)EPov::PovDpad);

    case EXInputControllerElement::ButtonA:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonA);

    case EXInputControllerElement::ButtonB:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonB);

    case EXInputControllerElement::ButtonX:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonX);

    case EXInputControllerElement::ButtonY:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonY);

    case EXInputControllerElement::ButtonLB:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonLB);

    case EXInputControllerElement::ButtonRB:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonRB);

    case EXInputControllerElement::ButtonBack:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonBack);

    case EXInputControllerElement::ButtonStart:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonStart);

    case EXInputControllerElement::ButtonLeftStick:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonLeftStick);

    case EXInputControllerElement::ButtonRightStick:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonRightStick);
    }

    return (TInstance)-1;
}
