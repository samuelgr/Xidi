/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file Mouse.h
 *   Declaration of virtual mouse event functionality, which allows physical
 *   controller elements to trigger mouse events.
 *****************************************************************************/

#pragma once


namespace Xidi
{
    namespace Mouse
    {
        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of milliseconds to wait between physical mouse update attempts.
        inline constexpr unsigned int kMouseUpdatePeriodMilliseconds = 7;


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Underlying type used to identify mouse buttons.
        /// See https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-mouseinput for more information on what buttons exist.
        enum class EMouseButton
        {
            Left,                                                           ///< Left mouse button.
            Middle,                                                         ///< Middle mouse button, for example pressing down on the mouse wheel.
            Right,                                                          ///< Right mouse button.
            X1,                                                             ///< X1 mouse button, used for example as a "back" button in internet browsers.
            X2,                                                             ///< X2 mouse button, used for example as a "forward" button in internet browsers.
            Count                                                           ///< Sentinel value, total number of enumerators.
        };


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Submits a mouse button state of pressed.
        /// @param [in] button Mouse button that is affected.
        void SubmitMouseButtonPressedState(EMouseButton button);

        /// Submits a mouse button state of released.
        /// @param [in] button Mouse button that is affected.
        void SubmitMouseButtonReleasedState(EMouseButton button);
    }
}
