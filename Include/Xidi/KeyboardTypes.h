/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file KeyboardTypes.h
 *   Declaration of constants and types used for representing virtual
 *   keyboards and the keys they contain.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


namespace Xidi
{
    namespace Keyboard
    {
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Underlying type used to identify keyboard keys.
        /// Values themselves are DirectInput keyboard scan codes (see "dinput.h" for the DIK_* constants).
        typedef WORD TKeyIdentifier;


        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of keyboard keys that exist in total on a virtual keyboard.
        /// Value taken from DirectInput documentation, which indicates keyboard state is reported as an array of 256 bytes.
        static constexpr TKeyIdentifier kVirtualKeyboardKeyCount = 256;
    }
}
