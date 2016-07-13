/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * OldGamepad.cpp
 *      Implements a mapper that maps to the button layout of an older
 *      DirectInput-compatible gamepad.
 *****************************************************************************/

#include "Mapper/OldGamepad.h"

using namespace XinputControllerDirectInput;
using namespace XinputControllerDirectInput::Mapper;


TInstanceIdx OldGamepad::AxisInstanceIndex(REFGUID axisGUID, DWORD instanceNumber)
{
    // Only one axis of each type exists in this mapping.
    if (0 == instanceNumber)
    {
        if (IsEqualGUID(GUID_XAxis, axisGUID)) return (DWORD)EAxis::AxisX;
        if (IsEqualGUID(GUID_YAxis, axisGUID)) return (DWORD)EAxis::AxisY;
        if (IsEqualGUID(GUID_ZAxis, axisGUID)) return (DWORD)EAxis::AxisZ;
        if (IsEqualGUID(GUID_RzAxis, axisGUID)) return (DWORD)EAxis::AxisRZ;
    }

    return (DWORD)EAxis::AxisCount;
}

BOOL OldGamepad::AxisInstanceExists(REFGUID axisGUID, DWORD instanceNumber)
{
    return (EAxis::AxisCount != AxisInstanceIndex(axisGUID, instanceNumber));
}

TInstanceCount OldGamepad::AxisTypeCount(REFGUID axisGUID)
{
    // Only one axis of each type exists in this mapping.
    // See if the first instance of the specified type exists and, if so, indicate as much.
    if (EAxis::AxisCount != AxisInstanceIndex(axisGUID, 0))
        return 1;

    return 0;
}


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Mapper/Base.h" for documentation.

TInstanceCount OldGamepad::NumInstancesOfType(EInstanceType type)
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
