/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XinputControllerIdentification.cpp
 *      Implementation of helpers for identifying controller types.
 *****************************************************************************/

#include "Hashers.h"
#include "XinputControllerIdentification.h"

#include <unordered_map>


using namespace XinputControllerDirectInput;


// -------- LOCALS --------------------------------------------------------- //

// Maps each known controller's product GUID to its controller type.
const std::unordered_map<const GUID, EControllerType> XinputControllerIdentification::knownControllers = {
    
    // Xbox 360 controller
    { {0x028E045E, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}}, EControllerType::Xbox360 },

    // Xbox One controller
    { {0x02FF045E, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}}, EControllerType::XboxOne },

};


// -------- CLASS METHODS -------------------------------------------------- //
// See "ControllerIdentification.h" for documentation.

BOOL XinputControllerIdentification::IsControllerTypeKnown(REFGUID productGUID)
{
    return (0 != knownControllers.count(productGUID));
}

// ---------

EControllerType XinputControllerIdentification::GetControllerType(REFGUID productGUID)
{
    auto it = knownControllers.find(productGUID);

    if (knownControllers.end() != it)
        return it->second;
    
    return EControllerType::Unknown;
}
