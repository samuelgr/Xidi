/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ControllerIdentification.cpp
 *      Implementation of helpers for identifying controller types.
 *****************************************************************************/

#include "ControllerIdentification.h"

using namespace XboxControllerDirectInput;


// -------- CONSTANTS ------------------------------------------------------ //
// See "ControllerIdentification.h" for documentation.

const GUID ControllerIdentification::guidXbox360Controller = {0x028E045E, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}};
const GUID ControllerIdentification::guidXboxOneController = {0x02FF045E, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}};


// -------- CLASS METHODS -------------------------------------------------- //
// See "ControllerIdentification.h" for documentation.

BOOL ControllerIdentification::isXbox360Controller(const GUID& productGUID)
{
    if (InlineIsEqualGUID(guidXbox360Controller, productGUID))
        return TRUE;
    else
        return FALSE;
}

// ---------

BOOL ControllerIdentification::isXboxOneController(const GUID& productGUID)
{
    if (InlineIsEqualGUID(guidXboxOneController, productGUID))
        return TRUE;
    else
        return FALSE;
}

// ---------

XboxControllerType ControllerIdentification::xboxControllerType(const GUID& productGUID)
{
    if (isXbox360Controller(productGUID))
        return CONTROLLER_XBOX360;

    if (isXboxOneController(productGUID))
        return CONTROLLER_XBOXONE;

    return CONTROLLER_NOTXBOX;
}
