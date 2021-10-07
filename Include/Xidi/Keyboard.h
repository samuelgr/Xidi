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

#include "ControllerTypes.h"
#include "KeyboardTypes.h"


namespace Xidi
{
    namespace Keyboard
    {
        // -------- FUNCTIONS ---------------------------------------------- //

        /// Submits a key state of pressed from a particular identified controller.
        /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key press state.
        /// @param [in] key Keyboard key that is affected.
        void SubmitKeyPressedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key);

        /// Submits a key state of released from a particular identified controller.
        /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key release state.
        /// @param [in] key Keyboard key that is affected.
        void SubmitKeyReleasedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key);
    }
}
