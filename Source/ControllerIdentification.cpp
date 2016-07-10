/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ControllerIdentification.cpp
 *      Implementation of helpers for identifying controller types.
 *****************************************************************************/

#include "ControllerIdentification.h"

using namespace XinputControllerDirectInput;


// -------- CONSTANTS ------------------------------------------------------ //
// See "ControllerIdentification.h" for documentation.

const GUID ControllerIdentification::kGuidXbox360Controller = {0x028E045E, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}};
const GUID ControllerIdentification::kGuidXboxOneController = {0x02FF045E, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}};


// -------- CLASS METHODS -------------------------------------------------- //
// See "ControllerIdentification.h" for documentation.

BOOL ControllerIdentification::IsXbox360Controller(REFGUID productGUID)
{
    if (InlineIsEqualGUID(kGuidXbox360Controller, productGUID))
        return TRUE;
    else
        return FALSE;
}

// ---------

BOOL ControllerIdentification::IsXboxOneController(REFGUID productGUID)
{
    if (InlineIsEqualGUID(kGuidXboxOneController, productGUID))
        return TRUE;
    else
        return FALSE;
}

// ---------

eControllerType ControllerIdentification::GetControllerType(REFGUID productGUID)
{
    if (IsXbox360Controller(productGUID))
        return CONTROLLERTYPE_XBOX360;

    if (IsXboxOneController(productGUID))
        return CONTROLLERTYPE_XBOXONE;

    return CONTROLLERTYPE_UNKNOWN;
}
