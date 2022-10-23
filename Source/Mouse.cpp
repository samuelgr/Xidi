/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file Mouse.cpp
 *   Implementation of virtual mouse event functionality, which allows
 *   physical controller elements to trigger mouse events.
 *****************************************************************************/

#include "ApiBitSet.h"
#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Mouse.h"
#include "Message.h"

#include <bitset>
#include <mutex>
#include <thread>
#include <vector>


namespace Xidi
{
    namespace Mouse
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Type used to represent the state of a virtual mouse's buttons.
        typedef BitSet<(unsigned int)EMouseButton::Count> TButtonState;

        /// Tracks mouse state contributions and generates mouse state snapshots.
        class StateContributionTracker
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Set of buttons marked "pressed" since the last snapshot.
            TButtonState pressedButtons;

            /// Inverted set of buttons marked "released" since the last snapshot.
            /// Buttons present in this set have not been marked released since the last snapshot.
            TButtonState notReleasedButtons;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default constructor.
            constexpr inline StateContributionTracker(void)
            {
                Reset();
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Determines if the specified mouse button is marked as having been pressed since the last snapshot.
            /// A mouse button marked pressed can also be marked released. The two are not mutually exclusive.
            /// @param [in] button Identifier of the mouse button of interest.
            /// @return `true` if it is marked pressed, `false` if not.
            constexpr inline bool IsMarkedPressed(EMouseButton button) const
            {
                return pressedButtons.contains((unsigned int)button);
            }

            /// Determines if the specified mouse button is marked as having been released since the last snapshot.
            /// A mouse button marked released can also be marked pressed. The two are not mutually exclusive.
            /// @param [in] key Identifier of the mouse button of interest.
            /// @return `true` if it is marked released, `false` if not.
            constexpr inline bool IsMarkedReleased(EMouseButton button) const
            {
                return !(notReleasedButtons.contains((unsigned int)button));
            }

            /// Registers a mouse button press contribution.
            /// Has no effect if the mouse button is already marked as being pressed since the last snapshot.
            /// @param [in] button Identifier of the target mouse button.
            constexpr inline void MarkPressed(EMouseButton button)
            {
                pressedButtons.insert((unsigned int)button);
            }

            /// Registers a mouse button release contribution.
            /// Has no effect if the mouse button is already marked as being released since the last snapshot.
            /// @param [in] button Identifier of the target mouse button.
            constexpr inline void MarkRelease(EMouseButton button)
            {
                notReleasedButtons.erase((unsigned int)button);
            }

            /// Computes the next mouse snapshot by applying the marked changes to the specified previous snapshot.
            /// Afterwards, resets internal state so no mouse buttons are marked as pressed or released.
            /// @param [in] previousSnapshot Previous snapshot against which to apply the marked changes.
            constexpr inline TButtonState SnapshotRelativeTo(const TButtonState& previousSnapshot)
            {
                // If a mouse button is marked pressed since the last snapshot, then no matter what it is pressed in the next snapshot.
                // Otherwise, a mouse button continues to be pressed if it was pressed in the last snapshot and not released since.
                const TButtonState nextSnapshot = pressedButtons | (previousSnapshot & notReleasedButtons);

                Reset();
                return nextSnapshot;
            }

            /// Computes a mouse state snapshot using only the marked changes.
            /// Afterwards, resets internal state so no keys are marked as pressed or released.
            constexpr inline TButtonState Snapshot(void)
            {
                return SnapshotRelativeTo(TButtonState());
            }

            /// Resets all marked contributions back to empty.
            constexpr inline void Reset(void)
            {
                pressedButtons.clear();
                notReleasedButtons.fill();
            }
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// For ensuring proper concurrency control of accesses to the virtual mice.
        static std::mutex mouseGuard;

        /// Holds changes to mouse state since the last snapshot.
        /// Virtual mouse state snapshots are maintained by the thread that periodically updates physical mouse state.
        static StateContributionTracker mouseTracker;


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Generates a mouse input event descriptor for a state change in a mouse button.
        /// @param [in] button Identifier of the target mouse button.
        /// @param [in] newStateIsPressed New state of the target mouse button after transitioning, `true` for pressed or `false` for unpressed.
        /// @return Filled out mouse input event descriptor.
        static MOUSEINPUT MouseInputEventForButtonTransition(EMouseButton button, bool newStateIsPressed)
        {
            switch (button)
            {
            case EMouseButton::Left:
                if (true == newStateIsPressed)
                    return {.mouseData = 0, .dwFlags = MOUSEEVENTF_LEFTDOWN};
                else
                    return {.mouseData = 0, .dwFlags = MOUSEEVENTF_LEFTUP};

            case EMouseButton::Middle:
                if (true == newStateIsPressed)
                    return {.mouseData = 0, .dwFlags = MOUSEEVENTF_MIDDLEDOWN};
                else
                    return {.mouseData = 0, .dwFlags = MOUSEEVENTF_MIDDLEUP};

            case EMouseButton::Right:
                if (true == newStateIsPressed)
                    return {.mouseData = 0, .dwFlags = MOUSEEVENTF_RIGHTDOWN};
                else
                    return {.mouseData = 0, .dwFlags = MOUSEEVENTF_RIGHTUP};

            case EMouseButton::X1:
                if (true == newStateIsPressed)
                    return {.mouseData = XBUTTON1, .dwFlags = MOUSEEVENTF_XDOWN};
                else
                    return {.mouseData = XBUTTON1, .dwFlags = MOUSEEVENTF_XUP};

            case EMouseButton::X2:
                if (true == newStateIsPressed)
                    return {.mouseData = XBUTTON2, .dwFlags = MOUSEEVENTF_XDOWN};
                else
                    return {.mouseData = XBUTTON2, .dwFlags = MOUSEEVENTF_XUP};

            default:
                return {};
            }
        }

        /// Periodically checks for changes between the previous and next views of the virtual mouse button states.
        /// On detected state change, generates and submits a mouse input event to the system.
        static void UpdatePhysicalMouseState(void)
        {
            std::vector<INPUT> mouseEvents;
            mouseEvents.reserve((unsigned int)EMouseButton::Count);

            TButtonState previousMouseState;

            while (true)
            {
                Sleep(kMouseUpdatePeriodMilliseconds);

                do {
                    std::scoped_lock lock(mouseGuard);

                    const TButtonState nextMouseState = mouseTracker.SnapshotRelativeTo(previousMouseState);
                    const TButtonState transitionedButtons = nextMouseState ^ previousMouseState;

                    for (auto transitionedButtonsIter : transitionedButtons)
                    {
                        const EMouseButton transitionedButton = (EMouseButton)((unsigned int)transitionedButtonsIter);
                        const bool transitionIsFromUnpressedToPressed = (nextMouseState.contains((unsigned int)transitionedButton));
                        
                        mouseEvents.emplace_back(INPUT({.type = INPUT_MOUSE, .mi = MouseInputEventForButtonTransition((EMouseButton)transitionedButton, transitionIsFromUnpressedToPressed)}));
                    }

                    previousMouseState = nextMouseState;
                } while (false);

                if (mouseEvents.size() > 0)
                {
                    //if (true == Globals::DoesCurrentProcessHaveInputFocus())
                        SendInput((UINT)mouseEvents.size(), mouseEvents.data(), (int)sizeof(INPUT));
                }

                mouseEvents.clear();
            }
        }

        /// Initializes internal data structures, creates internal threads, and begins periodically checking for mouse events that need to be submitted.
        /// Idempotent and concurrency-safe.
        static void InitializeAndBeginUpdating(void)
        {
            static std::once_flag initFlag;
            std::call_once(initFlag, []() -> void
                {
                    std::thread(UpdatePhysicalMouseState).detach();
                    Message::OutputFormatted(Message::ESeverity::Info, L"Initialized the mouse event thread. Desired update period is %u ms.", kMouseUpdatePeriodMilliseconds);
                }
            );
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Mouse.h" for documentation.

        void SubmitMouseButtonPressedState(EMouseButton button)
        {
            InitializeAndBeginUpdating();

            if (false == mouseTracker.IsMarkedPressed(button))
            {
                std::scoped_lock lock(mouseGuard);
                mouseTracker.MarkPressed(button);
            }
        }

        // --------

        void SubmitMouseButtonReleasedState(EMouseButton button)
        {
            InitializeAndBeginUpdating();

            if (false == mouseTracker.IsMarkedReleased(button))
            {
                std::scoped_lock lock(mouseGuard);
                mouseTracker.MarkRelease(button);
            }
        }
    }
}
