/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file FakeKeyboard.cpp
 *   Implementation of a fake version of the keyboard event interface, which
 *   does nothing.
 *****************************************************************************/

#include "Keyboard.h"


namespace Xidi
{
    namespace Keyboard
    {
        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Keyboard.h" for documentation.

        void SubmitKeyPressedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key)
        {
            // Nothing to do here.
        }

        // --------

        void SubmitKeyReleasedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key)
        {
            // Nothing to do here.
        }
    }
}
