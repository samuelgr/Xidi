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

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "ControllerTypes.h"

#include <bitset>
#include <type_traits>


namespace Xidi
{
    namespace Keyboard
    {
        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of keyboard keys that exist in total on a virtual keyboard.
        /// Value taken from DirectInput documentation, which indicates keyboard state is reported as an array of 256 bytes.
        static constexpr WORD kVirtualKeyboardKeyCount = 256;


        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Underlying type used to identify keyboard keys.
        /// Values themselves are DirectInput keyboard scan codes (see "dinput.h" for the DIK_* constants).
        typedef std::remove_const_t<decltype(kVirtualKeyboardKeyCount)> TKeyIdentifier;

        /// Enumerates the possible transitions of keyboard key states.
        /// Primarily for internal use, but exposed for testing.
        enum class EKeyTransition
        {
            NoChange,                                                       ///< No change in key state.
            KeyWasPressed,                                                  ///< Key was previously not pressed and is now pressed.
            KeyWasReleased                                                  ///< Key was previously pressed and now is no longer pressed.
        };

        /// Holds a single key's state and offers simple ways of comparing states.
        /// Keeps track of separate contributions from multiple controllers separated by identifier.
        /// A key is considered "pressed" if any individual contribution from a controller says that the key is pressed.
        /// Primarily for internal use, but exposed for testing.
        class KeyState
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Individual contributions to controller state, one element per possible controller.
            std::bitset<Controller::kPhysicalControllerCount> controllerContributions;


        public:
            // -------- INSTANCE METHODS ----------------------------------- //

            /// Retrieves and returns the current pressed state of this keyboard key.
            /// @return `true` if the key is pressed, `false` if not.
            inline bool IsPressed(void) const
            {
                return controllerContributions.any();
            }

            /// Computes the transition that took place from a previous keyboard key state to this one.
            /// @return Corresponding enumerator that represents a state change that may have occurred.
            inline EKeyTransition GetTransitionFrom(const KeyState& previousState) const
            {
                if (previousState.IsPressed() == IsPressed())
                    return EKeyTransition::NoChange;
                else
                    return ((true == IsPressed()) ? EKeyTransition::KeyWasPressed : EKeyTransition::KeyWasReleased);
            }

            /// Registers a key press contribution from the specified controller.
            /// Has no effect if the key is already pressed by that controller.
            /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key press.
            inline void Press(Controller::TControllerIdentifier controllerIdentifier)
            {
                if (controllerIdentifier < controllerContributions.size())
                    controllerContributions.set(controllerIdentifier);
            }

            /// Registers a key release contribution from the specified controller.
            /// Has no effect if the key is not already pressed by that controller.
            /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key release.
            inline void Release(Controller::TControllerIdentifier controllerIdentifier)
            {
                if (controllerIdentifier < controllerContributions.size())
                    controllerContributions.reset(controllerIdentifier);
            }
        };
    }
}
