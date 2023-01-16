/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file Keyboard.cpp
 *   Implementation of virtual keyboard event functionality, which allows
 *   physical controller elements to trigger key presses and releases.
 *****************************************************************************/

#include "ApiBitSet.h"
#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Keyboard.h"
#include "Message.h"

#include <bitset>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>


namespace Xidi
{
    namespace Keyboard
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Type used to represent the state of an entire virtual keyboard.
        typedef BitSet<kVirtualKeyboardKeyCount> TState;

        /// Tracks "pressed" and "released" key state contributions and generates keyboard state snapshots.
        class StateContributionTracker
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Set of keys marked "pressed" since the last snapshot.
            TState pressedKeys;

            /// Inverted set of keys marked "released" since the last snapshot.
            /// Keys present in this set have not been marked released since the last snapshot.
            TState notReleasedKeys;

            /// For ensuring proper concurrency control of accesses to the virtual keyboard state represented by this object.
            std::mutex keyboardStateGuard;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default constructor.
            inline StateContributionTracker(void)
            {
                Reset();
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Determines if the specified key is marked as having been pressed since the last snapshot.
            /// A key marked pressed can also be marked released. The two are not mutually exclusive.
            /// @param [in] key Identifier of the keyboard key of interest.
            /// @return `true` if it is marked pressed, `false` if not.
            constexpr inline bool IsMarkedPressed(TKeyIdentifier key) const
            {
                return pressedKeys.contains(key);
            }

            /// Determines if the specified key is marked as having been released since the last snapshot.
            /// A key marked released can also be marked pressed. The two are not mutually exclusive.
            /// @param [in] key Identifier of the keyboard key of interest.
            /// @return `true` if it is marked released, `false` if not.
            constexpr inline bool IsMarkedReleased(TKeyIdentifier key) const
            {
                return !(notReleasedKeys.contains(key));
            }

            /// Locks this object for ensuring proper concurrency control.
            /// The returned lock object is scoped and, as a result, will automatically unlock upon its destruction.
            /// @return Scoped lock object that has acquired this object's concurrency control mutex.
            inline std::unique_lock<std::mutex> Lock(void)
            {
                return std::unique_lock(keyboardStateGuard);
            }

            /// Registers a key press contribution.
            /// Has no effect if the key is already marked as being pressed since the last snapshot.
            /// @param [in] key Identifier of the target keyboard key.
            constexpr inline void MarkPressed(TKeyIdentifier key)
            {
                pressedKeys.insert(key);
            }

            /// Registers a key release contribution.
            /// Has no effect if the key is already marked as being released since the last snapshot.
            /// @param [in] key Identifier of the target keyboard key.
            constexpr inline void MarkRelease(TKeyIdentifier key)
            {
                notReleasedKeys.erase(key);
            }

            /// Computes the next keyboard snapshot by applying the marked changes to the specified previous snapshot.
            /// Afterwards, resets internal state so no keys are marked as pressed or released.
            /// @param [in] previousSnapshot Previous snapshot against which to apply the marked changes.
            constexpr inline TState SnapshotRelativeTo(const TState& previousSnapshot)
            {
                // If a key is marked pressed since the last snapshot, then no matter what it is pressed in the next snapshot.
                // Otherwise, a key continues to be pressed if it was pressed in the last snapshot and not released since.
                const TState nextSnapshot = pressedKeys | (previousSnapshot & notReleasedKeys);

                Reset();
                return nextSnapshot;
            }

            /// Computes a keyboard state snapshot using only the marked changes.
            /// Afterwards, resets internal state so no keys are marked as pressed or released.
            constexpr inline TState Snapshot(void)
            {
                return SnapshotRelativeTo(TState());
            }

            /// Resets all marked contributions back to empty.
            constexpr inline void Reset(void)
            {
                pressedKeys.clear();
                notReleasedKeys.fill();
            }
        };

        /// Manages a thread that continuously runs and updates the physical keyboard state from virtual keyboard state.
        /// Wraps the thread handle to ensure safe termination and clean-up.
        class KeyboardUpdateThread
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Handle for the update thread itself.
            std::thread keyboardUpdateThread;

            /// Stop indicator to be used for exiting the keyboard update thread when needed.
            std::stop_source keyboardUpdateStop;

            /// Keyboard contribution tracker object that should be used during the updates.
            StateContributionTracker& keyboardTracker;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Initialization constructor.
            /// Requires a keyboard state contribution tracker object.
            inline KeyboardUpdateThread(StateContributionTracker& keyboardTracker) : keyboardUpdateThread(), keyboardUpdateStop(), keyboardTracker(keyboardTracker)
            {
                // Nothing to do here.
            }

            /// Default destructor.
            /// Safely exits the keyboard update thread if it is started.
            ~KeyboardUpdateThread(void)
            {
                Exit();
            }


        private:
            // -------- CLASS METHODS -------------------------------------- //

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
            /// @param [in] keyboardTracker Pointer to the keyboard state contribution tracker object to use for updates.
            /// @param [in] keyboardUpdateStopToken Stop token used to indicate that this method should terminate.
            static void UpdatePhysicalKeyboardState(StateContributionTracker* keyboardTracker, std::stop_token keyboardUpdateStopToken)
            {
                std::vector<INPUT> keyboardEvents;
                keyboardEvents.reserve(kVirtualKeyboardKeyCount);

                TState previousKeyboardState;

                while (true)
                {
                    Sleep(kKeyboardUpdatePeriodMilliseconds);

                    const bool kHaveInputFocus = Globals::DoesCurrentProcessHaveInputFocus();
                    const bool kTerminationRequested = keyboardUpdateStopToken.stop_requested();

                    do {
                        auto lock = keyboardTracker->Lock();

                        TState nextKeyboardState = keyboardTracker->SnapshotRelativeTo(previousKeyboardState);

                        // If the current process does not have input focus or this thread is exiting then all pressed keys should be submitted to the system as released.
                        if ((false == kHaveInputFocus) || (true == kTerminationRequested))
                            nextKeyboardState.clear();

                        const TState transitionedKeys = nextKeyboardState ^ previousKeyboardState;

                        for (auto transitionedKeyIter : transitionedKeys)
                        {
                            const int transitionedKey = (int)transitionedKeyIter;

                            if (nextKeyboardState.contains(transitionedKey))
                            {
                                // Key with a transition is present in the next snapshot. This means it was pressed.
                                keyboardEvents.emplace_back(INPUT({.type = INPUT_KEYBOARD, .ki = {.wScan = KeyboardEventScanCode(transitionedKey), .dwFlags = KeyboardEventFlags(transitionedKey)}}));
                            }
                            else
                            {
                                // Key with a transition is present in the next snapshot. This means it was released.
                                keyboardEvents.emplace_back(INPUT({.type = INPUT_KEYBOARD, .ki = {.wScan = KeyboardEventScanCode(transitionedKey), .dwFlags = KEYEVENTF_KEYUP | KeyboardEventFlags(transitionedKey)}}));
                            }
                        }

                        previousKeyboardState = nextKeyboardState;
                    } while (false);

                    if (keyboardEvents.size() > 0)
                    {
                        SendInput((UINT)keyboardEvents.size(), keyboardEvents.data(), (int)sizeof(INPUT));
                        keyboardEvents.clear();
                    }

                    if (true == kTerminationRequested)
                        break;
                }
            }


        public:
            // -------- INSTANCE METHODS ----------------------------------- //

            /// Terminates the keyboard update thread.
            inline void Exit(void)
            {
                if (true == keyboardUpdateThread.joinable())
                {
                    keyboardUpdateStop.request_stop();
                    keyboardUpdateThread.join();
                }
            }

            /// Starts running the keyboard update thread.
            inline void Start(void)
            {
                if ((false == keyboardUpdateThread.joinable()) && (false == keyboardUpdateStop.stop_requested()))
                {
                    keyboardUpdateThread = std::thread(UpdatePhysicalKeyboardState, &keyboardTracker, keyboardUpdateStop.get_token());
                }
            }
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// Holds changes to keyboard state since the last snapshot.
        /// Virtual keyboard state snapshots are maintained by the thread that periodically updates physical keyboard state.
        static StateContributionTracker keyboardTracker;

        /// Singleton object that wraps the keyboard update thread.
        static KeyboardUpdateThread keyboardUpdateThread(keyboardTracker);


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Initializes internal data structures, creates internal threads, and begins periodically checking for keyboard events that need to be submitted.
        /// Idempotent and concurrency-safe.
        static void InitializeAndBeginUpdating(void)
        {
            static std::once_flag initFlag;
            std::call_once(initFlag, []() -> void
                {
                    keyboardUpdateThread.Start();
                    Message::OutputFormatted(Message::ESeverity::Info, L"Initialized the keyboard event thread. Desired update period is %u ms.", kKeyboardUpdatePeriodMilliseconds);
                }
            );
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Keyboard.h" for documentation.

        void SubmitKeyPressedState(TKeyIdentifier key)
        {
            InitializeAndBeginUpdating();

            if (false == keyboardTracker.IsMarkedPressed(key))
            {
                auto lock = keyboardTracker.Lock();
                keyboardTracker.MarkPressed(key);
            }
        }

        // --------

        void SubmitKeyReleasedState(TKeyIdentifier key)
        {
            InitializeAndBeginUpdating();

            if (false == keyboardTracker.IsMarkedReleased(key))
            {
                auto lock = keyboardTracker.Lock();
                keyboardTracker.MarkRelease(key);
            }
        }
    }
}
