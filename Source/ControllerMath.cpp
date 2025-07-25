/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ControllerMath.cpp
 *   Partial implementation of common mathematical operations for interpreting
 *   and transforming controller data.
 **************************************************************************************************/

#include "ControllerMath.h"

#include <cmath>

#include "ControllerTypes.h"

namespace Xidi
{
  namespace Controller
  {
    namespace Math
    {
      int16_t ApplyRawAnalogTransform(
          int16_t analogValue, unsigned int deadzonePercent, unsigned int saturationPercent)
      {
        if ((0 == deadzonePercent) && (100 == saturationPercent)) return analogValue;

        const int16_t deadzoneCutoff =
            ((kAnalogValueMax - kAnalogValueNeutral) * deadzonePercent) / 100;
        if (std::abs(analogValue) <= deadzoneCutoff) return kAnalogValueNeutral;

        const int16_t saturationCutoff =
            ((kAnalogValueMax - kAnalogValueNeutral) * saturationPercent) / 100;
        ;
        if (std::abs(analogValue) >= saturationCutoff)
          return ((analogValue >= 0) ? kAnalogValueMax : kAnalogValueMin);

        const double transformedAnalogBase =
            ((analogValue >= 0) ? ((double)analogValue - (double)deadzoneCutoff)
                                : ((double)analogValue + (double)deadzoneCutoff));
        const double transformationScaleFactor = ((double)(kAnalogValueMax - kAnalogValueNeutral)) /
            ((double)(saturationCutoff - deadzoneCutoff));

        return kAnalogValueNeutral + (int16_t)(transformedAnalogBase * transformationScaleFactor);
      }

      int16_t ApplyRawTriggerTransform(
          int16_t triggerValue, unsigned int deadzonePercent, unsigned int saturationPercent)
      {
        if ((0 == deadzonePercent) && (100 == saturationPercent)) return triggerValue;

        const int16_t deadzoneCutoff =
            (int16_t)((((unsigned int)kTriggerValueMax - (unsigned int)kTriggerValueMin) *
                       deadzonePercent) /
                      100);
        if (triggerValue <= deadzoneCutoff) return kTriggerValueMin;

        const int16_t saturationCutoff =
            (int16_t)((((unsigned int)kTriggerValueMax - (unsigned int)kTriggerValueMin) *
                       saturationPercent) /
                      100);
        if (triggerValue >= saturationCutoff) return kTriggerValueMax;

        const float transformedTriggerBase = (float)triggerValue - (float)deadzoneCutoff;
        const float transformationScaleFactor = ((float)(kTriggerValueMax - kTriggerValueMin)) /
            ((float)(saturationCutoff - deadzoneCutoff));

        return kTriggerValueMin + (int16_t)(transformedTriggerBase * transformationScaleFactor);
      }

      SAnalogStickCoordinates TransformCoordinatesCircleToSquare(
          SAnalogStickCoordinates circleCoords, double amountFraction)
      {
        if (0.0 == amountFraction) return circleCoords;

        const double x = static_cast<double>(circleCoords.x);
        const double y = static_cast<double>(circleCoords.y);

        const double radius = std::min(
            static_cast<double>(std::numeric_limits<int16_t>::max()), std::sqrt((x * x) + (y * y)));
        const double multiplier = radius / static_cast<double>(std::max(std::abs(x), std::abs(y)));
        const double weightedMultiplier =
            ((1.0 == amountFraction) ? multiplier : std::pow(multiplier, amountFraction));

        return SAnalogStickCoordinates{
            .x = static_cast<int16_t>(x * weightedMultiplier),
            .y = static_cast<int16_t>(y * weightedMultiplier),
        };
      }
    } // namespace Math
  }   // namespace Controller
} // namespace Xidi
