/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * Globals.cpp
 *      Implementation of accessors and mutators for global data items.
 *      Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Globals.h" for documentation.

HINSTANCE Globals::gInstanceHandle = NULL;


// -------- CLASS METHODS -------------------------------------------------- //
// See "Globals.h" for documentation.

HINSTANCE Globals::GetInstanceHandle(void)
{
    return gInstanceHandle;
}

// ---------

void Globals::SetInstanceHandle(HINSTANCE newInstanceHandle)
{
    gInstanceHandle = newInstanceHandle;
}
