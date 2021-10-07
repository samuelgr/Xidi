/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Keyboard.cpp
 *   Implementation of virtual keyboard event functionality, which allows
 *   physical controller element to trigger key presses and releases.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Keyboard.h"
#include "KeyboardTypes.h"
#include "Message.h"

#include <bitset>
#include <mutex>
#include <thread>
#include <vector>


namespace Xidi
{
    namespace Keyboard
    {
        // -------- INTERNAL CONSTANTS ------------------------------------- //

        /// Number of milliseconds to wait between physical keyboard update attempts.
        static constexpr DWORD kUpdatePeriodMilliseconds = 10;

        
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Enumerates the possible transitions of keyboard key states.
        enum class EKeyTransition
        {
            NoChange,                                                       ///< No change in key state.
            KeyWasPressed,                                                  ///< Key was previously not pressed and is now pressed.
            KeyWasReleased                                                  ///< Key was previously pressed and now is no longer pressed.
        };

        /// Holds a single key's state and offers simple ways of comparing states.
        /// Keeps track of separate contributions from multiple controllers separated by identifier.
        /// A key is considered "pressed" if any individual contribution from a controller says that the key is pressed.
        class KeyState
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Individual contributions to controller state, one element per possible controller.
            std::bitset<Controller::kPhysicalControllerCount> controllerContributions;


        public:
            // -------- OPERATORS ------------------------------------------ //

            /// Simple equality check to detect key state differences.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const KeyState& other) const
            {
                return (other.IsPressed() == IsPressed());
            }


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
                if (previousState == *this)
                    return EKeyTransition::NoChange;
                else
                    return ((true == IsPressed()) ? EKeyTransition::KeyWasPressed : EKeyTransition::KeyWasReleased);
            }

            /// Registers a key press contribution from the specified controller.
            /// Has no effect if the key is already pressed by that controller.
            /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key press.
            inline void Press(Controller::TControllerIdentifier controllerIdentifier)
            {
                controllerContributions.set(controllerIdentifier);
            }

            /// Registers a key release contribution from the specified controller.
            /// Has no effect if the key is not already pressed by that controller.
            /// @param [in] controllerIdentifier Identifier of the controller that is the source of the key release.
            inline void Release(Controller::TControllerIdentifier controllerIdentifier)
            {
                controllerContributions.reset(controllerIdentifier);
            }
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// For ensuring proper concurrency control of accesses to the virtual keyboards.
        static std::mutex keyboardGuard;

        /// Background thread for updating the physical keyboard status by reading the virtual keyboard status snapshots.
        static std::thread physicalKeyboardUpdateThread;
        
        /// Old view of all the keyboard key states in the virtual keyboard.
        /// Used to detect transitions.
        static KeyState previousVirtualKeyboardState[kVirtualKeyboardKeyCount];

        /// Upcoming view of all the keyboard key states in the virtual keyboard.
        /// Submissions are written to this view and compared with the previous to detect transitions.
        static KeyState nextVirtualKeyboardState[kVirtualKeyboardKeyCount];


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Determines if this process has input focus based on whether or not a window it owns is at the foreground.
        /// @return `true` if so, `false` if not.
        inline bool DoesCurrentProcessHaveInputFocus(void)
        {
            DWORD foregroundProcess = 0;
            GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);

            return (Globals::GetCurrentProcessId() == foregroundProcess);
        }
        
        /// Periodically checks for changes between the previous and next views of the virtual keyboard key states.
        /// On detected state change, generates and submits a keyboard input event to the system.
        static void UpdatePhysicalKeyboardState(void)
        {
            std::vector<INPUT> keyboardEvents;
            keyboardEvents.reserve(kVirtualKeyboardKeyCount);

            while (true)
            {
                Sleep(kUpdatePeriodMilliseconds);

                do
                {
                    std::scoped_lock lock(keyboardGuard);

                    for (TKeyIdentifier key = 0; key < kVirtualKeyboardKeyCount; ++key)
                    {
                        switch (nextVirtualKeyboardState[key].GetTransitionFrom(previousVirtualKeyboardState[key]))
                        {
                        case EKeyTransition::KeyWasPressed:
                            keyboardEvents.emplace_back(INPUT({.type = INPUT_KEYBOARD, .ki = {.wScan = key, .dwFlags = KEYEVENTF_SCANCODE}}));
                            break;

                        case EKeyTransition::KeyWasReleased:
                            keyboardEvents.emplace_back(INPUT({.type = INPUT_KEYBOARD, .ki = {.wScan = key, .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP}}));
                            break;

                        default:
                            break;
                        }

                        previousVirtualKeyboardState[key] = nextVirtualKeyboardState[key];
                    }
                } while (false);

                if (keyboardEvents.size() > 0)
                {
                    if (true == DoesCurrentProcessHaveInputFocus())
                        SendInput(keyboardEvents.size(), keyboardEvents.data(), sizeof(INPUT));
                }

                keyboardEvents.clear();
            }
        }

        /// Initializes internal data structures, creates internal threads, and begins periodically checking for keyboard events that need to be submitted.
        /// Idempotent and concurrency-safe.
        static void InitializeAndBeginUpdating(void)
        {
            static std::once_flag initFlag;
            std::call_once(initFlag, []() -> void
                {
                    physicalKeyboardUpdateThread = std::thread(UpdatePhysicalKeyboardState);
                    Message::OutputFormatted(Message::ESeverity::Info, L"Initialized the keyboard event thread. Desired update period is %u ms.", kUpdatePeriodMilliseconds);
                }
            );
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Keyboard.h" for documentation.

        void SubmitKeyPressedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key)
        {
            InitializeAndBeginUpdating();

            std::scoped_lock lock(keyboardGuard);
            nextVirtualKeyboardState[key].Press(controllerIdentifier);
        }

        // --------

        void SubmitKeyReleasedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key)
        {
            InitializeAndBeginUpdating();

            std::scoped_lock lock(keyboardGuard);
            nextVirtualKeyboardState[key].Release(controllerIdentifier);
        }
    }
}
