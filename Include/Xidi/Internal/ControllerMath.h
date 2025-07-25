/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ControllerMath.h
 *   Declaration and partial implementation of common mathematical operations for interpreting
 *   and transforming controller data.
 **************************************************************************************************/

#pragma once

#include <cstdint>

#include "ControllerTypes.h"

namespace Xidi
{
  namespace Controller
  {
    namespace Math
    {
      /// Represents the two-dimensional coordinates of an analog stick position, both X and Y,
      /// which correspond to physical axis directions. In both cases 0 is the center point of the
      /// axis, with the signs used to indicate direction. The actual mapping of sign to physical
      /// direction is immaterial because the math implemented by this module is sign-agnostic.
      /// Range of motion on each axis is -32768 to +32767.
      struct SAnalogStickCoordinates
      {
        int16_t x;
        int16_t y;

        constexpr bool operator==(const SAnalogStickCoordinates& other) const = default;
      };

      /// Threshold value used to determine if a trigger is considered "pressed" or not as a digital
      /// button.
      inline constexpr int16_t kTriggerPressedThreshold = (kTriggerValueMax - kTriggerValueMin) / 6;

      /// Threshold negative direction value used to determine if an analog stick is considered
      /// "pressed" or not as a digital button.
      inline constexpr int16_t kAnalogPressedThresholdNegative =
          (kAnalogValueNeutral + kAnalogValueMin) / 3;

      /// Threshold positive direction value used to determine if an analog stick is considered
      /// "pressed" or not as a digital button.
      inline constexpr int16_t kAnalogPressedThresholdPositive =
          (kAnalogValueNeutral + kAnalogValueMax) / 3;

      /// Applies deadzone and saturation transformations to a raw analog value.
      /// @param [in] analogValue Analog value for which a deadzone should be applied.
      /// @param [in] deadzoneHudnredthsOfPercent Hundredths of a percent of the analog range for
      /// which the deadzone should be applied.
      int16_t ApplyRawAnalogTransform(
          int16_t analogValue, unsigned int deadzonePercent, unsigned int saturationPercent);

      /// Applies deadzone and saturation transformations to a raw trigger value.
      /// @param [in] analogValue Analog value for which a deadzone should be applied.
      /// @param [in] deadzoneHudnredthsOfPercent Hundredths of a percent of the analog range for
      /// which the deadzone should be applied.
      int16_t ApplyRawTriggerTransform(
          int16_t triggerValue, unsigned int deadzonePercent, unsigned int saturationPercent);

      /// Determines if an analog reading is considered "pressed" as a digital button in the
      /// negative direction.
      /// @param [in] analogValue Analog reading from the XInput controller.
      /// @return `true` if the virtual button is considered pressed, `false` otherwise.
      constexpr bool IsAnalogPressedNegative(int16_t analogValue)
      {
        return (analogValue <= kAnalogPressedThresholdNegative);
      }

      /// Determines if an analog reading is considered "pressed" as a digital button in the
      /// positive direction.
      /// @param [in] analogValue Analog reading from the XInput controller.
      /// @return `true` if the virtual button is considered pressed, `false` otherwise.
      constexpr bool IsAnalogPressedPositive(int16_t analogValue)
      {
        return (analogValue >= kAnalogPressedThresholdPositive);
      }

      /// Determines if an analog reading is considered "pressed" as a digital button.
      /// @param [in] analogValue Analog reading from the XInput controller.
      /// @return `true` if the virtual button is considered pressed, `false` otherwise.
      constexpr bool IsAnalogPressed(int16_t analogValue)
      {
        return (IsAnalogPressedNegative(analogValue) || IsAnalogPressedPositive(analogValue));
      }

      /// Determines if a trigger reading is considered "pressed" as a digital button.
      /// @param [in] triggerValue Trigger reading from the XInput controller.
      /// @return `true` if the virtual button is considered pressed, `false` otherwise.
      constexpr bool IsTriggerPressed(int16_t triggerValue)
      {
        return (triggerValue >= kTriggerPressedThreshold);
      }

      /// Applies a correction to convert the coordinates of an analog stick reading
      /// from circle to square. On many controllers, the analog stick range of motion follows a
      /// circular pattern, but sometimes the application expects a square (for example, diagonals
      /// that hit extreme coordinate values on both axes simultaneously, which is not possible with
      /// a circular range of motion).
      /// @param [in] circleCoords Physical coordinates read from the analog stick, assumed to be on
      /// a circular range of motion.
      /// @param [in] amountFraction Value between 0.0 and 1.0 that determines the amount of
      /// transformation to apply.
      /// @return Replacement analog stick coordinates after the transformation is applied from a
      /// circular range of motion to a square range of motion.
      SAnalogStickCoordinates TransformCoordinatesCircleToSquare(
          SAnalogStickCoordinates cirleCoords, double amountFraction);
    } // namespace Math
  }   // namespace Controller
} // namespace Xidi
