/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file XInputInterface.cpp
 *   Implementation of the default interface through which all XInput
 *   functionality is accessed.
 *****************************************************************************/

#include "ApiWindows.h"
#include "XInputInterface.h"

#include <xinput.h>


namespace Xidi
{
    // -------- CONCRETE INSTANCE METHODS ---------------------------------- //
    // See "XInputInterface.h" for documentation.

    DWORD XInput::GetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
    {
        return XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
    }

    // --------

    DWORD XInput::GetState(DWORD dwUserIndex, XINPUT_STATE* pState)
    {
        return XInputGetState(dwUserIndex, pState);
    }
}
