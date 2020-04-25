/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper/XInputNative.cpp
 *   Implements a mapper that maps to the default configuration of an
 *   XInput controller when accessed via DirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "XInputController.h"
#include "Mapper/XInputNative.h"

using namespace Xidi;
using namespace Xidi::Mapper;


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Mapper/Base.h" for documentation.

const TInstanceIdx XInputNative::AxisInstanceIndex(REFGUID axisGUID, const TInstanceIdx instanceNumber)
{
    // Only one axis of each type exists in this mapping.
    if (0 == instanceNumber)
    {
        if (GUID_XAxis == axisGUID) return (TInstanceIdx)EAxis::AxisX;
        if (GUID_YAxis == axisGUID) return (TInstanceIdx)EAxis::AxisY;
        if (GUID_ZAxis == axisGUID) return (TInstanceIdx)EAxis::AxisZ;
        if (GUID_RxAxis == axisGUID) return (TInstanceIdx)EAxis::AxisRX;
        if (GUID_RyAxis == axisGUID) return (TInstanceIdx)EAxis::AxisRY;
        if (GUID_RzAxis == axisGUID) return (TInstanceIdx)EAxis::AxisRZ;
    }

    return (TInstanceIdx)-1;
}

// ---------

const TInstanceCount XInputNative::AxisTypeCount(REFGUID axisGUID)
{
    // Only one axis of each type exists in this mapping.
    // See if the first instance of the specified type exists and, if so, indicate as much.
    if (AxisInstanceIndex(axisGUID, 0) >= 0)
        return 1;

    return 0;
}

// ---------

const GUID XInputNative::AxisTypeFromInstanceNumber(const TInstanceIdx instanceNumber)
{
    EAxis axisNumber = (EAxis)instanceNumber;

    switch (axisNumber)
    {
    case EAxis::AxisX:
        return GUID_XAxis;
    case EAxis::AxisY:
        return GUID_YAxis;
    case EAxis::AxisZ:
        return GUID_ZAxis;
    case EAxis::AxisRX:
        return GUID_RxAxis;
    case EAxis::AxisRY:
        return GUID_RyAxis;
    case EAxis::AxisRZ:
        return GUID_RzAxis;
    }

    return GUID_Unknown;
}

// ---------

const TInstance XInputNative::MapXInputElementToDirectInputInstance(EXInputControllerElement element)
{
    switch (element)
    {
    case EXInputControllerElement::StickLeftHorizontal:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisX);

    case EXInputControllerElement::StickLeftVertical:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisY);

    case EXInputControllerElement::StickRightHorizontal:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisRX);

    case EXInputControllerElement::StickRightVertical:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisRY);

    case EXInputControllerElement::TriggerLT:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisZ);

    case EXInputControllerElement::TriggerRT:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeAxis, (TInstanceIdx)EAxis::AxisRZ);

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

// ---------

const TInstanceCount XInputNative::NumInstancesOfType(const EInstanceType type)
{
    TInstanceCount numInstances = 0;

    switch (type)
    {
    case EInstanceType::InstanceTypeAxis:
        numInstances = (TInstanceCount)EAxis::AxisCount;
        break;

    case EInstanceType::InstanceTypePov:
        numInstances = (TInstanceCount)EPov::PovCount;
        break;

    case EInstanceType::InstanceTypeButton:
        numInstances = (TInstanceCount)EButton::ButtonCount;
    }

    return numInstances;
}
