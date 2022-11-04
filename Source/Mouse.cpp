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

#include <array>
#include <bitset>
#include <concurrent_unordered_map.h>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>


namespace Xidi
{
    namespace Mouse
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Type used to represent the state of a virtual mouse's buttons.
        typedef BitSetEnum<EMouseButton> TButtonState;

        /// Type used to represent the individually-sourced mouse movement contributions.
        /// Operations allowed are read, append, and update.
        typedef concurrency::concurrent_unordered_map<uint32_t, int> TMouseMovementContributions;


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

            /// Individually-sourced mouse movement contributions.
            /// Since mouse movements are always relative, only one state data structure is needed, one per mouse axis.
            std::array<TMouseMovementContributions, (unsigned int)EMouseAxis::Count> mouseMovementContributions;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default constructor.
            inline StateContributionTracker(void)
            {
                ResetButtons();
            }


            // -------- INSTANCE METHODS ----------------------------------- //

            /// Determines if the specified mouse button is marked as having been pressed since the last snapshot.
            /// A mouse button marked pressed can also be marked released. The two are not mutually exclusive.
            /// @param [in] button Identifier of the mouse button of interest.
            /// @return `true` if it is marked pressed, `false` if not.
            inline bool IsMarkedPressed(EMouseButton button) const
            {
                return pressedButtons.contains((unsigned int)button);
            }

            /// Determines if the specified mouse button is marked as having been released since the last snapshot.
            /// A mouse button marked released can also be marked pressed. The two are not mutually exclusive.
            /// @param [in] key Identifier of the mouse button of interest.
            /// @return `true` if it is marked released, `false` if not.
            inline bool IsMarkedReleased(EMouseButton button) const
            {
                return !(notReleasedButtons.contains((unsigned int)button));
            }

            /// Registers a mouse button press contribution.
            /// Has no effect if the mouse button is already marked as being pressed since the last snapshot.
            /// @param [in] button Identifier of the target mouse button.
            inline void MarkPressed(EMouseButton button)
            {
                pressedButtons.insert((unsigned int)button);
            }

            /// Registers a mouse button release contribution.
            /// Has no effect if the mouse button is already marked as being released since the last snapshot.
            /// @param [in] button Identifier of the target mouse button.
            inline void MarkRelease(EMouseButton button)
            {
                notReleasedButtons.erase((unsigned int)button);
            }

            /// Retrieves a read-only reference to all mouse movement contributions on all axes.
            /// @return Read-only reference to the mouse movement contribution tracking data structure.
            inline const std::array<TMouseMovementContributions, (unsigned int)EMouseAxis::Count>& MovementContributions(void)
            {
                return mouseMovementContributions;
            }

            /// Computes the next mouse button snapshot by applying the marked changes to the specified previous snapshot.
            /// Afterwards, resets internal state so no mouse buttons are marked as pressed or released.
            /// @param [in] previousSnapshot Previous snapshot against which to apply the marked changes.
            inline TButtonState ButtonSnapshotRelativeTo(const TButtonState& previousSnapshot)
            {
                // If a mouse button is marked pressed since the last snapshot, then no matter what it is pressed in the next snapshot.
                // Otherwise, a mouse button continues to be pressed if it was pressed in the last snapshot and not released since.
                const TButtonState nextSnapshot = pressedButtons | (previousSnapshot & notReleasedButtons);

                ResetButtons();
                return nextSnapshot;
            }

            /// Computes a mouse button state snapshot using only the marked changes.
            /// Afterwards, resets internal state so no keys are marked as pressed or released.
            inline TButtonState ButtonSnapshot(void)
            {
                return ButtonSnapshotRelativeTo(TButtonState());
            }

            /// Resets all marked button contributions back to empty.
            inline void ResetButtons(void)
            {
                pressedButtons.clear();
                notReleasedButtons.fill();
            }

            /// Resets all movement contributions back to motionless.
            inline void ResetMovementContributions(void)
            {
                for (auto& axisMovementContributions : mouseMovementContributions)
                {
                    for (auto& contribution : axisMovementContributions)
                        contribution.second = 0;
                }
            }

            /// Submits a mouse movement.
            /// Either inserts a contribution from a new source or updates the contribution from an existing source.
            /// @param [in] axis Mouse axis that is affected.
            /// @param [in] mouseMovementUnits Number of internal mouse movement units along the target mouse axis.
            /// @param [in] sourceIdentifier Opaque identifier for the source of the mouse movement event.
            inline void SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier)
            {
                mouseMovementContributions[(unsigned int)axis][sourceIdentifier] = mouseMovementUnits;
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

        /// Generates a mouse input event descriptor for a mouse movement event.
        /// @param [in] axis Identifier of the target mouse axis.
        /// @param [in] numPixels Number of pixels of mouse movement along the target axis.
        /// @return Filled out mouse input event descriptor.
        static MOUSEINPUT MouseInputEventForMovement(EMouseAxis axis, int mouseMovementPixels)
        {
            switch (axis)
            {
            case EMouseAxis::X:
                return {.dx = mouseMovementPixels, .dwFlags = MOUSEEVENTF_MOVE};

            case EMouseAxis::Y:
                return {.dy = mouseMovementPixels, .dwFlags = MOUSEEVENTF_MOVE};

            case EMouseAxis::WheelHorizontal:
                return {.mouseData = (DWORD)mouseMovementPixels, .dwFlags = MOUSEEVENTF_HWHEEL};

            case EMouseAxis::WheelVertical:
                return {.mouseData = (DWORD)mouseMovementPixels, .dwFlags = MOUSEEVENTF_WHEEL};

            default:
                return {};
            }
        }

        /// Converts internal mouse movement units to pixels.
        /// @param [in] mouseMovementUnits Number of internal mouse movement units to be converted.
        /// @return Appropriate number of pixels represented by the mouse movement units.
        static int MouseMovementUnitsToPixels(int mouseMovementUnits)
        {
            constexpr int kFastestPixelsPerPollingPeriod = 14;
            constexpr double kConversionScalingFactor = kFastestPixelsPerPollingPeriod / ((kMouseMovementUnitsMax - kMouseMovementUnitsMin) / 2.0);

            return (int)((double)(mouseMovementUnits - kMouseMovementUnitsNeutral) * kConversionScalingFactor);
        }

        /// Periodically checks for changes between the previous and next views of the virtual mouse button states.
        /// On detected state change, generates and submits a mouse input event to the system.
        static void UpdatePhysicalMouseState(void)
        {
            std::vector<INPUT> mouseEvents;
            mouseEvents.reserve((unsigned int)EMouseAxis::Count + (unsigned int)EMouseButton::Count);

            TButtonState previousMouseButtonState;

            while (true)
            {
                Sleep(kMouseUpdatePeriodMilliseconds);

                // Mouse buttons
                do {
                    std::scoped_lock lock(mouseGuard);

                    const TButtonState nextMouseButtonState = mouseTracker.ButtonSnapshotRelativeTo(previousMouseButtonState);
                    const TButtonState transitionedButtons = nextMouseButtonState ^ previousMouseButtonState;

                    for (auto transitionedButtonsIter : transitionedButtons)
                    {
                        const EMouseButton transitionedButton = (EMouseButton)((unsigned int)transitionedButtonsIter);
                        const bool transitionIsFromUnpressedToPressed = (nextMouseButtonState.contains((unsigned int)transitionedButton));
                        
                        mouseEvents.emplace_back(INPUT({.type = INPUT_MOUSE, .mi = MouseInputEventForButtonTransition((EMouseButton)transitionedButton, transitionIsFromUnpressedToPressed)}));
                    }

                    previousMouseButtonState = nextMouseButtonState;
                } while (false);

                // Mouse movement
                do {
                    const std::array<TMouseMovementContributions, (unsigned int)EMouseAxis::Count>& mouseMovementContributions = mouseTracker.MovementContributions();

                    for (size_t axisIndex = 0; axisIndex < mouseMovementContributions.size(); ++axisIndex)
                    {
                        const TMouseMovementContributions& axisMovementContributions = mouseMovementContributions[axisIndex];
                        int axisMovementUnits = 0;

                        for (const auto& contribution : axisMovementContributions)
                            axisMovementUnits += contribution.second;

                        if (kMouseMovementUnitsNeutral != axisMovementUnits)
                        {
                            if (axisMovementUnits > kMouseMovementUnitsMax)
                                axisMovementUnits = kMouseMovementUnitsMax;
                            else if (axisMovementUnits < kMouseMovementUnitsMin)
                                axisMovementUnits = kMouseMovementUnitsMin;

                            const int axisMovementPixels = MouseMovementUnitsToPixels(axisMovementUnits);
                            if (0 != axisMovementPixels)
                                mouseEvents.emplace_back(INPUT({.type = INPUT_MOUSE, .mi = MouseInputEventForMovement((EMouseAxis)axisIndex, axisMovementPixels)}));
                        }
                    }
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

        // --------

        void SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier)
        {
            InitializeAndBeginUpdating();
            mouseTracker.SubmitMouseMovement(axis, mouseMovementUnits, sourceIdentifier);
        }
    }
}
