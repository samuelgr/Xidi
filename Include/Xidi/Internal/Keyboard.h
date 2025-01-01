/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file Keyboard.h
 *   Declaration of virtual keyboard event functionality, which allows physical controller
 *   elements to trigger key presses and releases.
 **************************************************************************************************/

#pragma once

namespace Xidi
{
  namespace Keyboard
  {
    /// Number of milliseconds to wait between physical keyboard update attempts.
    inline constexpr unsigned int kKeyboardUpdatePeriodMilliseconds = 10;

    /// Number of keyboard keys that exist in total on a virtual keyboard.
    /// Value taken from DirectInput documentation, which indicates keyboard state is reported as an
    /// array of 256 bytes.
    inline constexpr unsigned int kVirtualKeyboardKeyCount = 256;

    /// Underlying type used to identify keyboard keys.
    /// Values themselves are DirectInput keyboard scan codes (see "dinput.h" for the DIK_*
    /// constants).
    using TKeyIdentifier = unsigned int;

    /// Submits a key state of pressed.
    /// @param [in] key Keyboard key that is affected.
    void SubmitKeyPressedState(TKeyIdentifier key);

    /// Submits a key state of released.
    /// @param [in] key Keyboard key that is affected.
    void SubmitKeyReleasedState(TKeyIdentifier key);
  } // namespace Keyboard
} // namespace Xidi
