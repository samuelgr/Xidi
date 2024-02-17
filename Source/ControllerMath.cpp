/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
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

      uint8_t ApplyRawTriggerTransform(
          uint8_t triggerValue, unsigned int deadzonePercent, unsigned int saturationPercent)
      {
        if ((0 == deadzonePercent) && (100 == saturationPercent)) return triggerValue;

        const uint8_t deadzoneCutoff =
            (uint8_t)((((unsigned int)kTriggerValueMax - (unsigned int)kTriggerValueMin) * deadzonePercent) / 100);
        if (triggerValue <= deadzoneCutoff) return kTriggerValueMin;

        const uint8_t saturationCutoff =
            (uint8_t)((((unsigned int)kTriggerValueMax - (unsigned int)kTriggerValueMin) * saturationPercent) / 100);
        if (triggerValue >= saturationCutoff) return kTriggerValueMax;

        const float transformedTriggerBase = (float)triggerValue - (float)deadzoneCutoff;
        const float transformationScaleFactor = ((float)(kTriggerValueMax - kTriggerValueMin)) /
            ((float)(saturationCutoff - deadzoneCutoff));

        return kTriggerValueMin + (uint8_t)(transformedTriggerBase * transformationScaleFactor);
      }
    } // namespace Math
  }   // namespace Controller
} // namespace Xidi
