/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Keyboard.h
 *   Declaration of virtual keyboard event functionality, which allows
 *   physical controller element to trigger key presses and releases.
 *****************************************************************************/

#pragma once


namespace Xidi
{
    namespace Keyboard
    {
        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of keyboard keys that exist in total on a virtual keyboard.
        /// Value taken from DirectInput documentation, which indicates keyboard state is reported as an array of 256 bytes.
        inline constexpr unsigned int kVirtualKeyboardKeyCount = 256;


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Underlying type used to identify keyboard keys.
        /// Values themselves are DirectInput keyboard scan codes (see "dinput.h" for the DIK_* constants).
        typedef unsigned int TKeyIdentifier;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Submits a key state of pressed.
        /// @param [in] key Keyboard key that is affected.
        void SubmitKeyPressedState(TKeyIdentifier key);

        /// Submits a key state of released.
        /// @param [in] key Keyboard key that is affected.
        void SubmitKeyReleasedState(TKeyIdentifier key);
    }
}
