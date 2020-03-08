/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper/StandardGamepad.cpp
 *   Implements a mapper that maps to the button layout of an older
 *   DirectInput-compatible gamepad.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "XInputController.h"
#include "Mapper/StandardGamepad.h"

using namespace Xidi;
using namespace Xidi::Mapper;


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Mapper/Base.h" for documentation.

const TInstanceIdx StandardGamepad::AxisInstanceIndex(REFGUID axisGUID, const TInstanceIdx instanceNumber)
{
    // Only one axis of each type exists in this mapping.
    if (0 == instanceNumber)
    {
        if (GUID_XAxis == axisGUID) return (TInstanceIdx)EAxis::AxisX;
        if (GUID_YAxis == axisGUID) return (TInstanceIdx)EAxis::AxisY;
        if (GUID_ZAxis == axisGUID) return (TInstanceIdx)EAxis::AxisZ;
        if (GUID_RzAxis == axisGUID) return (TInstanceIdx)EAxis::AxisRZ;
    }

    return (TInstanceIdx)-1;
}

// ---------

const TInstanceCount StandardGamepad::AxisTypeCount(REFGUID axisGUID)
{
    // Only one axis of each type exists in this mapping.
    // See if the first instance of the specified type exists and, if so, indicate as much.
    if (AxisInstanceIndex(axisGUID, 0) >= 0)
        return 1;

    return 0;
}

// ---------

const GUID StandardGamepad::AxisTypeFromInstanceNumber(const TInstanceIdx instanceNumber)
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
    case EAxis::AxisRZ:
        return GUID_RzAxis;
    }

    return GUID_Unknown;
}

// ---------

const TInstance StandardGamepad::MapXInputElementToDirectInputInstance(EXInputControllerElement element)
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
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonL2);
        
    case EXInputControllerElement::TriggerRT:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonR2);
        
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
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonL1);
        
    case EXInputControllerElement::ButtonRB:
        return MakeInstanceIdentifier(EInstanceType::InstanceTypeButton, (TInstanceIdx)EButton::ButtonR1);
        
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

const TInstanceCount StandardGamepad::NumInstancesOfType(const EInstanceType type)
{
    TInstanceCount numInstances = 0;
    
    switch (type)
    {
    case EInstanceType::InstanceTypeAxis:
        numInstances = EAxis::AxisCount;
        break;

    case EInstanceType::InstanceTypePov:
        numInstances = EPov::PovCount;
        break;

    case EInstanceType::InstanceTypeButton:
        numInstances = EButton::ButtonCount;
    }

    return numInstances;
}
