/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
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


// -------- CLASS METHODS (ACCESSORS) -------------------------------------- //
// See "Globals.h" for documentation.

HINSTANCE Globals::GetInstanceHandle(void)
{
    return gInstanceHandle;
}


// -------- CLASS METHODS (MUTATORS) --------------------------------------- //
// See "Globals.h" for documentation.

void Globals::SetInstanceHandle(HINSTANCE newInstanceHandle)
{
    gInstanceHandle = newInstanceHandle;
}
