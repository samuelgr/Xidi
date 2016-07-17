/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * OldGamepad.cpp
 *      Implements a mapper that maps to the button layout of an older
 *      DirectInput-compatible gamepad.
 *****************************************************************************/

#include "ApiDirectInput8.h"
#include "Mapper/OldGamepad.h"

using namespace Xidi;
using namespace Xidi::Mapper;


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Mapper/Base.h" for documentation.

const TInstanceIdx OldGamepad::AxisInstanceIndex(REFGUID axisGUID, const TInstanceIdx instanceNumber)
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

const TInstanceCount OldGamepad::AxisTypeCount(REFGUID axisGUID)
{
    // Only one axis of each type exists in this mapping.
    // See if the first instance of the specified type exists and, if so, indicate as much.
    if (EAxis::AxisCount != AxisInstanceIndex(axisGUID, 0))
        return 1;

    return 0;
}

GUID OldGamepad::AxisTypeFromInstanceNumber(TInstanceIdx instanceNumber)
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

const TInstanceCount OldGamepad::NumInstancesOfType(const EInstanceType type)
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
