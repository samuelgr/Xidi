/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file Mouse.h
 *   Declaration of virtual mouse event functionality, which allows physical controller elements
 *   to trigger mouse events.
 **************************************************************************************************/

#pragma once

#include <cstdint>
#include <optional>

namespace Xidi
{
  namespace Mouse
  {
    /// Number of milliseconds to wait between physical mouse update attempts.
    inline constexpr unsigned int kMouseUpdatePeriodMilliseconds = 7;

    /// Maximum number of internal units of mouse motion, which represents extreme motion in the
    /// positive direction. These are converted automatically to proper system units for reporting
    /// mouse motion.
    inline constexpr int kMouseMovementUnitsMax = 10000;

    /// Minimum number of internal units of mouse motion, which represents extreme motion in the
    /// negative direction. These are converted automatically to proper system units for reporting
    /// mouse motion.
    inline constexpr int kMouseMovementUnitsMin = -kMouseMovementUnitsMax;

    /// Number of internal units of mouse motion that represent no motion at all.
    /// These are converted automatically to proper system units for reporting mouse motion.
    inline constexpr int kMouseMovementUnitsNeutral =
        (kMouseMovementUnitsMax + kMouseMovementUnitsMin) / 2;

    /// Enumeration of possible mouse axes.
    enum class EMouseAxis
    {
      /// X axis, horizontal pointer motion.
      X,

      /// Y axis, vertical pointer motion.
      Y,

      /// Horizontal mouse wheel motion, potentially used for horizontal scrolling.
      WheelHorizontal,

      /// Vertical moue wheel motion, often used for scrolling.
      WheelVertical,

      /// Sentinel value, total number of enumerators.
      Count
    };

    /// Enumeration of possible mouse buttons.
    /// See https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-mouseinput for
    /// more information on what buttons exist.
    enum class EMouseButton
    {
      /// Left mouse button.
      Left,

      /// Middle mouse button, for example pressing down on the mouse wheel.
      Middle,

      /// Right mouse button.
      Right,

      /// X1 mouse button, used for example as a "back" button in internet browsers.
      X1,

      /// X2 mouse button, used for example as a "forward" button in internet browsers.
      X2,

      /// Sentinel value, total number of enumerators.
      Count
    };

    /// Submits a mouse button state of pressed.
    /// @param [in] button Mouse button that is affected.
    void SubmitMouseButtonPressedState(EMouseButton button);

    /// Submits a mouse button state of released.
    /// @param [in] button Mouse button that is affected.
    void SubmitMouseButtonReleasedState(EMouseButton button);

    /// Submits a mouse movement.
    /// @param [in] axis Mouse axis that is affected.
    /// @param [in] mouseMovementUnits Number of internal mouse movement units along the target
    /// mouse axis.
    /// @param [in] sourceIdentifier Opaque identifier for the source of the mouse movement event.
    void SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier);

    /// Submits a mouse speed override.
    /// @param [in] mouseSpeedScalingFactor Optional scaling factor. A lack of value means that the
    /// contribution is cleared.
    /// @param [in] sourceIdentifier Opaque identifier for the source of the scaling factor.
    void SubmitMouseSpeedOverride(
        std::optional<unsigned int> mouseSpeedScalingFactor, uint32_t sourceIdentifier);
  } // namespace Mouse
} // namespace Xidi
