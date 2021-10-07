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

#include "PhysicalController.h"

#include <bitset>
#include <cstdint>
#include <limits>


namespace Xidi
{
    namespace Keyboard
    {
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Underlying type used to identify virtual keys.
        typedef uint8_t TVirtualKey;


        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of virtual keys that exist in total on a virtual keyboard.
        static constexpr int kVirtualKeyCount = std::numeric_limits<TVirtualKey>::max();


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Submits a key state of pressed from a particular virtual controller.
        /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key press state.
        /// @param [in] virtualKey Virtual key that is affected.
        void SubmitKeyPressedState(Controller::TControllerIdentifier controllerIdentifier, TVirtualKey virtualKey);

        /// Submits a key state of released from a particular virtual controller.
        /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key release state.
        /// @param [in] virtualKey Virtual key that is affected.
        void SubmitKeyReleasedState(Controller::TControllerIdentifier controllerIdentifier, TVirtualKey virtualKey);
    }
}
