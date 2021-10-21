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
        static inline bool DoesCurrentProcessHaveInputFocus(void)
        {
            DWORD foregroundProcess = 0;
            GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);

            return (Globals::GetCurrentProcessId() == foregroundProcess);
        }
        
        /// Generates the proper flags indicating how the scan code should be interpreted for the given keyboard key.
        /// @param [in] key Keyboard key identifier.
        /// @return Flags indicating how the scan code corresponding to the identified key should be interpreted.
        static inline DWORD KeyboardEventFlags(TKeyIdentifier key)
        {
            // Any key identifiers higher than the maximum 7-bit value are "extended" keys and need to be flagged as such.
            if (key > 0x7f)
                return (KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY);
            else
                return KEYEVENTF_SCANCODE;
        }

        /// Generates the proper 16-bit scan code for the given keyboard key.
        /// @param [in] key Keyboard key identifier.
        /// @return Proper 16-bit scan code to use for the key.
        static inline WORD KeyboardEventScanCode(TKeyIdentifier key)
        {
            // Only the bottom 7 bits of the key identifier are used.
            // Any key identifiers higher than the maximum 7-bit value are "extended" keys for which a prefix of 0xe0 is needed in the full 16-bit quantity.
            if (key > 0x7f)
                return ((0xe0 << 8) | (key & 0x7f));
            else
                return key;
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
                            keyboardEvents.emplace_back(INPUT({.type = INPUT_KEYBOARD, .ki = {.wScan = KeyboardEventScanCode(key), .dwFlags = KeyboardEventFlags(key)}}));
                            break;

                        case EKeyTransition::KeyWasReleased:
                            keyboardEvents.emplace_back(INPUT({.type = INPUT_KEYBOARD, .ki = {.wScan = KeyboardEventScanCode(key), .dwFlags = KeyboardEventFlags(key) | KEYEVENTF_KEYUP}}));
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
                        SendInput((UINT)keyboardEvents.size(), keyboardEvents.data(), (int)sizeof(INPUT));
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

            if ((key < kVirtualKeyboardKeyCount) && (false == nextVirtualKeyboardState[key].IsPressedBy(controllerIdentifier)))
            {
                std::scoped_lock lock(keyboardGuard);
                nextVirtualKeyboardState[key].Press(controllerIdentifier);
            }
        }

        // --------

        void SubmitKeyReleasedState(Controller::TControllerIdentifier controllerIdentifier, TKeyIdentifier key)
        {
            InitializeAndBeginUpdating();

            if ((key < kVirtualKeyboardKeyCount) && (true == nextVirtualKeyboardState[key].IsPressedBy(controllerIdentifier)))
            {
                std::scoped_lock lock(keyboardGuard);
                nextVirtualKeyboardState[key].Release(controllerIdentifier);
            }
        }
    }
}
