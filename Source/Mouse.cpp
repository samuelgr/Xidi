/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file Mouse.cpp
 *   Implementation of virtual mouse event functionality, which allows physical controller
 *   elements to trigger mouse events.
 **************************************************************************************************/

#include "Mouse.h"

#include <concurrent_unordered_map.h>

#include <array>
#include <bitset>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <vector>

#include <Infra/Core/Message.h>
#include <Infra/Core/Mutex.h>

#include "ApiBitSet.h"
#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Strings.h"

namespace Xidi
{
  namespace Mouse
  {
    /// Type used to represent the state of a virtual mouse's buttons.
    using TButtonState = BitSetEnum<EMouseButton>;

    /// Type used to represent individually-sourced mouse movement contributions. Operations allowed
    /// are read, append, and update.
    using TMouseMovementContributions = concurrency::concurrent_unordered_map<uint32_t, int>;

    /// Type used to represent individually-sourced mouse speed contributions, which override the
    /// global default mouse speed scaling factor. Only one contribution is actually effective at
    /// any given time, and there is no guarantee as to which it will be if multiple exist.
    /// Operations allowed are read, append, and update.
    using TMouseSpeedContributions =
        concurrency::concurrent_unordered_map<uint32_t, std::optional<unsigned int>>;

    /// Tracks mouse state contributions and generates mouse state snapshots.
    class StateContributionTracker
    {
    public:

      inline StateContributionTracker(void)
      {
        ResetButtons();
      }

      /// Determines if the specified mouse button is marked as having been pressed since the last
      /// snapshot. A mouse button marked pressed can also be marked released. The two are not
      /// mutually exclusive.
      /// @param [in] button Identifier of the mouse button of interest.
      /// @return `true` if it is marked pressed, `false` if not.
      inline bool IsMarkedPressed(EMouseButton button) const
      {
        return pressedButtons.contains((unsigned int)button);
      }

      /// Determines if the specified mouse button is marked as having been released since the last
      /// snapshot. A mouse button marked released can also be marked pressed. The two are not
      /// mutually exclusive.
      /// @param [in] key Identifier of the mouse button of interest.
      /// @return `true` if it is marked released, `false` if not.
      inline bool IsMarkedReleased(EMouseButton button) const
      {
        return !(notReleasedButtons.contains((unsigned int)button));
      }

      /// Locks this object for ensuring proper concurrency control of mouse button operations.
      /// The returned lock object is scoped and, as a result, will automatically unlock upon its
      /// destruction.
      /// @return Scoped lock object that has acquired this object's concurrency control mutex.
      inline std::unique_lock<Infra::Mutex> LockButtonState(void)
      {
        return std::unique_lock(mouseButtonStateGuard);
      }

      /// Registers a mouse button press contribution.
      /// Has no effect if the mouse button is already marked as being pressed since the last
      /// snapshot.
      /// @param [in] button Identifier of the target mouse button.
      inline void MarkPressed(EMouseButton button)
      {
        pressedButtons.insert((unsigned int)button);
      }

      /// Registers a mouse button release contribution.
      /// Has no effect if the mouse button is already marked as being released since the last
      /// snapshot.
      /// @param [in] button Identifier of the target mouse button.
      inline void MarkRelease(EMouseButton button)
      {
        notReleasedButtons.erase((unsigned int)button);
      }

      /// Retrieves a read-only reference to all mouse movement contributions on all axes.
      /// @return Read-only reference to the mouse movement contribution tracking data structure.
      inline const std::array<TMouseMovementContributions, (unsigned int)EMouseAxis::Count>&
          MovementContributions(void)
      {
        return mouseMovementContributions;
      }

      /// Computes the next mouse button snapshot by applying the marked changes to the specified
      /// previous snapshot. Afterwards, resets internal state so no mouse buttons are marked as
      /// pressed or released.
      /// @param [in] previousSnapshot Previous snapshot against which to apply the marked changes.
      inline TButtonState ButtonSnapshotRelativeTo(const TButtonState& previousSnapshot)
      {
        // If a mouse button is marked pressed since the last snapshot, then no matter what it is
        // pressed in the next snapshot. Otherwise, a mouse button continues to be pressed if it was
        // pressed in the last snapshot and not released since.
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

      /// Determines the effective mouse speed scaling factor override, given all of the
      /// contributions. If multiple different contributions are present then there is no guarantee
      /// as to which is effective.
      /// @return Mouse speed scaling factor override, if a temporary override is in effect.
      inline std::optional<unsigned int> GetMouseSpeedScalingFactorOverride(void)
      {
        for (const auto& maybeMouseSpeedContribution : mouseSpeedContributions)
        {
          if (maybeMouseSpeedContribution.second.has_value())
            return *maybeMouseSpeedContribution.second;
        }

        return std::nullopt;
      }

      /// Resets all marked button contributions back to empty.
      inline void ResetButtons(void)
      {
        pressedButtons.clear();
        notReleasedButtons.fill();
      }

      /// Submits a mouse movement. Either inserts a contribution from a new source or updates the
      /// contribution from an existing source.
      /// @param [in] axis Mouse axis that is affected.
      /// @param [in] mouseMovementUnits Number of internal mouse movement units along the target
      /// mouse axis.
      /// @param [in] sourceIdentifier Opaque identifier for the source of the mouse movement event.
      inline void SubmitMouseMovement(
          EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier)
      {
        mouseMovementContributions[(unsigned int)axis][sourceIdentifier] = mouseMovementUnits;
      }

      /// Submits a mouse speed override. Either inserts a contribution from a new source or updates
      /// the contribution from an existing source.
      /// @param [in] mouseSpeedScalingFactor Scaling factor to submit, or empty to clear the
      /// submission from the specified source.
      /// @param [in] sourceIdentifier Opaque identifier for the source of the mouse speed
      /// contribution.
      inline void SubmitMouseSpeedOverride(
          std::optional<unsigned int> mouseSpeedScalingFactor, uint32_t sourceIdentifier)
      {
        mouseSpeedContributions[sourceIdentifier] = mouseSpeedScalingFactor;
      }

    private:

      /// Set of buttons marked "pressed" since the last snapshot.
      TButtonState pressedButtons;

      /// Inverted set of buttons marked "released" since the last snapshot.
      /// Buttons present in this set have not been marked released since the last snapshot.
      TButtonState notReleasedButtons;

      /// For ensuring proper concurrency control of accesses to the virtual mouse button state
      /// represented by this object.
      Infra::Mutex mouseButtonStateGuard;

      /// Individually-sourced mouse movement contributions, one per mouse axis. Since mouse
      /// movements are always relative, only one state data structure is needed for each mouse
      /// axis.
      std::array<TMouseMovementContributions, (unsigned int)EMouseAxis::Count>
          mouseMovementContributions;

      /// Individually-sourced mouse speed contributions. Holds override contributions.
      TMouseSpeedContributions mouseSpeedContributions;
    };

    /// Manages a thread that continuously runs and updates the physical mouse state from virtual
    /// mouse state. Wraps the thread handle to ensure safe termination and clean-up.
    class MouseUpdateThread
    {
    public:

      inline MouseUpdateThread(StateContributionTracker& mouseTracker)
          : mouseUpdateThread(), mouseUpdateStop(), mouseTracker(mouseTracker)
      {}

      /// Safely exits the keyboard update thread if it is started.
      ~MouseUpdateThread(void)
      {
        Exit();
      }

      /// Terminates the mouse update thread.
      inline void Exit(void)
      {
        if (true == mouseUpdateThread.joinable())
        {
          mouseUpdateStop.request_stop();
          mouseUpdateThread.join();
        }
      }

      /// Starts running the mouse update thread.
      inline void Start(void)
      {
        if ((false == mouseUpdateThread.joinable()) && (false == mouseUpdateStop.stop_requested()))
        {
          mouseUpdateThread =
              std::thread(UpdatePhysicalMouseState, &mouseTracker, mouseUpdateStop.get_token());
        }
      }

    private:

      /// Generates a mouse input event descriptor for a state change in a mouse button.
      /// @param [in] button Identifier of the target mouse button.
      /// @param [in] newStateIsPressed New state of the target mouse button after transitioning,
      /// `true` for pressed or `false` for unpressed.
      /// @return Filled out mouse input event descriptor.
      static MOUSEINPUT MouseInputEventForButtonTransition(
          EMouseButton button, bool newStateIsPressed)
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
            // Vertical mouse wheel needs inversion to be semantically consistent with up being
            // negative and down being positive for other mouse and game controller axes.
            // Ordinarily, positive is "forward, away from the user" (up) and negative is "backward,
            // towards the user" (down). See
            // https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-mouseinput for
            // more information.
            return {.mouseData = (DWORD)(-mouseMovementPixels), .dwFlags = MOUSEEVENTF_WHEEL};

          default:
            return {};
        }
      }

      /// Converts internal mouse movement units to pixels.
      /// @param [in] mouseMovementUnits Number of internal mouse movement units to be converted.
      /// @param [in] mouseSpeedScalingFactorOverride Mouse speed scaling factor override in effect.
      /// If not present, then the configured mouse speed scaling factor is used.
      /// @return Appropriate number of pixels represented by the mouse movement units.
      static int MouseMovementUnitsToPixels(
          int mouseMovementUnits, std::optional<unsigned int> mouseSpeedScalingFactorOverride)
      {
        static const int kDefaultSpeedScalingFactor = static_cast<const int>(
            Globals::GetConfigurationData()
                [Strings::kStrConfigurationSectionProperties]
                [Strings::kStrConfigurationSettingPropertiesMouseSpeedScalingFactorPercent]
                    .ValueOr(100));

        constexpr double kMillisecondsPerSecond = 1000.0;
        constexpr double kPollingPeriodsPerSecond =
            (kMillisecondsPerSecond / (double)kMouseUpdatePeriodMilliseconds);

        const double speedScalingFactor =
            static_cast<double>(
                mouseSpeedScalingFactorOverride.value_or(kDefaultSpeedScalingFactor)) /
            100.0;

        const double fastestPixelsPerSecond = 2000.0 * speedScalingFactor;
        const double fastestPixelsPerPollingPeriod =
            fastestPixelsPerSecond / kPollingPeriodsPerSecond;
        const double conversionScalingFactor = fastestPixelsPerPollingPeriod /
            ((kMouseMovementUnitsMax - kMouseMovementUnitsMin) / 2.0);

        return static_cast<int>(
            static_cast<double>(mouseMovementUnits - kMouseMovementUnitsNeutral) *
            conversionScalingFactor);
      }

      /// Periodically checks for changes between the previous and next views of the virtual mouse
      /// button states. On detected state change, generates and submits a mouse input event to the
      /// system.
      /// @param [in] mouseTracker Pointer to the mouse state contribution tracker object to use for
      /// updates.
      /// @param [in] mouseUpdateStopToken Stop token used to indicate that this method should
      /// terminate.
      static void UpdatePhysicalMouseState(
          StateContributionTracker* mouseTracker, std::stop_token mouseUpdateStopToken)
      {
        std::vector<INPUT> mouseEvents;
        mouseEvents.reserve(
            static_cast<size_t>(EMouseAxis::Count) + static_cast<size_t>(EMouseButton::Count));

        TButtonState previousMouseButtonState;

        while (true)
        {
          Sleep(kMouseUpdatePeriodMilliseconds);

          const bool haveInputFocus = Globals::DoesCurrentProcessHaveInputFocus();
          const bool terminationRequested = mouseUpdateStopToken.stop_requested();

          // Mouse buttons
          {
            auto lock = mouseTracker->LockButtonState();

            TButtonState nextMouseButtonState =
                mouseTracker->ButtonSnapshotRelativeTo(previousMouseButtonState);

            // If the current process does not have input focus or this thread is exiting then all
            // pressed keys should be submitted to the system as released.
            if ((false == haveInputFocus) || (true == terminationRequested))
              nextMouseButtonState.clear();

            const TButtonState transitionedButtons =
                nextMouseButtonState ^ previousMouseButtonState;

            for (auto transitionedButtonsIter : transitionedButtons)
            {
              const EMouseButton transitionedButton =
                  (EMouseButton)((unsigned int)transitionedButtonsIter);
              const bool transitionIsFromUnpressedToPressed =
                  (nextMouseButtonState.contains((unsigned int)transitionedButton));

              mouseEvents.emplace_back(INPUT(
                  {.type = INPUT_MOUSE,
                   .mi = MouseInputEventForButtonTransition(
                       (EMouseButton)transitionedButton, transitionIsFromUnpressedToPressed)}));
            }

            previousMouseButtonState = nextMouseButtonState;
          }

          // Mouse movement
          if ((true == haveInputFocus) && (false == terminationRequested))
          {
            const std::array<TMouseMovementContributions, (unsigned int)EMouseAxis::Count>&
                mouseMovementContributions = mouseTracker->MovementContributions();

            for (size_t axisIndex = 0; axisIndex < mouseMovementContributions.size(); ++axisIndex)
            {
              const TMouseMovementContributions& axisMovementContributions =
                  mouseMovementContributions[axisIndex];
              int axisMovementUnits = 0;

              for (const auto& contribution : axisMovementContributions)
                axisMovementUnits += contribution.second;

              if (kMouseMovementUnitsNeutral != axisMovementUnits)
              {
                if (axisMovementUnits > kMouseMovementUnitsMax)
                  axisMovementUnits = kMouseMovementUnitsMax;
                else if (axisMovementUnits < kMouseMovementUnitsMin)
                  axisMovementUnits = kMouseMovementUnitsMin;

                const int axisMovementPixels = MouseMovementUnitsToPixels(
                    axisMovementUnits, mouseTracker->GetMouseSpeedScalingFactorOverride());
                if (0 != axisMovementPixels)
                  mouseEvents.emplace_back(INPUT(
                      {.type = INPUT_MOUSE,
                       .mi =
                           MouseInputEventForMovement((EMouseAxis)axisIndex, axisMovementPixels)}));
              }
            }
          };

          if (mouseEvents.size() > 0)
          {
            SendInput((UINT)mouseEvents.size(), mouseEvents.data(), (int)sizeof(INPUT));
            mouseEvents.clear();
          }

          if (true == terminationRequested) break;
        }
      }

      /// Handle for the update thread itself.
      std::thread mouseUpdateThread;

      /// Stop indicator to be used for exiting the keyboard update thread when needed.
      std::stop_source mouseUpdateStop;

      /// Mouse state contribution tracker object that should be used during the updates.
      StateContributionTracker& mouseTracker;
    };

    /// Holds changes to mouse state since the last snapshot.
    /// Virtual mouse state snapshots are maintained by the thread that periodically updates
    /// physical mouse state.
    static StateContributionTracker mouseTracker;

    /// Singleton object that wraps the mouse update thread.
    static MouseUpdateThread mouseUpdateThread(mouseTracker);

    /// Initializes internal data structures, creates internal threads, and begins periodically
    /// checking for mouse events that need to be submitted. Idempotent and concurrency-safe.
    static void InitializeAndBeginUpdating(void)
    {
      static std::once_flag initFlag;
      std::call_once(
          initFlag,
          []() -> void
          {
            mouseUpdateThread.Start();
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Info,
                L"Initialized the mouse event thread. Desired update period is %u ms.",
                kMouseUpdatePeriodMilliseconds);
          });
    }

    void SubmitMouseButtonPressedState(EMouseButton button)
    {
      InitializeAndBeginUpdating();

      if (false == mouseTracker.IsMarkedPressed(button))
      {
        auto lock = mouseTracker.LockButtonState();
        mouseTracker.MarkPressed(button);
      }
    }

    void SubmitMouseButtonReleasedState(EMouseButton button)
    {
      InitializeAndBeginUpdating();

      if (false == mouseTracker.IsMarkedReleased(button))
      {
        auto lock = mouseTracker.LockButtonState();
        mouseTracker.MarkRelease(button);
      }
    }

    void SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier)
    {
      InitializeAndBeginUpdating();
      mouseTracker.SubmitMouseMovement(axis, mouseMovementUnits, sourceIdentifier);
    }

    void SubmitMouseSpeedOverride(
        std::optional<unsigned int> mouseSpeedScalingFactor, uint32_t sourceIdentifier)
    {
      InitializeAndBeginUpdating();
      mouseTracker.SubmitMouseSpeedOverride(mouseSpeedScalingFactor, sourceIdentifier);
    }
  } // namespace Mouse
} // namespace Xidi
