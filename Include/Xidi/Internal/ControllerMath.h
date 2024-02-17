/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
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
      /// Threshold value used to determine if a trigger is considered "pressed" or not as a digital
      /// button.
      inline constexpr uint8_t kTriggerPressedThreshold = (kTriggerValueMax - kTriggerValueMin) / 6;

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
      uint8_t ApplyRawTriggerTransform(
          uint8_t triggerValue, unsigned int deadzonePercent, unsigned int saturationPercent);

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
      constexpr bool IsTriggerPressed(uint8_t triggerValue)
      {
        return (triggerValue >= kTriggerPressedThreshold);
      }
    } // namespace Math
  }   // namespace Controller
} // namespace Xidi
