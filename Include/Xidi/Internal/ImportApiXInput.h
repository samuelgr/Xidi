/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file ImportApiXInput.h
 *   Declarations of functions for accessing the XInput API imported from the
 *   native XInput library.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <xinput.h>


namespace Xidi
{
    namespace ImportApiXInput
    {
        // -------- FUNCTIONS ---------------------------------------------- //

        /// Dynamically loads the XInput library and sets up all imported function calls.
        void Initialize(void);


        // -------- IMPORTED FUNCTIONS ------------------------------------- //
        // See XInput documentation for more information.

        DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
        DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);
    }
}
