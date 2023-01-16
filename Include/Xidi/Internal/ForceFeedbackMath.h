/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ForceFeedbackMath.h
 *   Common mathematical operations used during force feedback calculations.
 *****************************************************************************/

#pragma once

#include "ForceFeedbackTypes.h"

#include <cmath>
#include <numbers>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            /// Precision to which mathematic operations should be rounded.
            /// Helps with avoiding imprecision error when using conversions between degrees and radians, which happens internally.
            /// The value used is equal to 1/64, also known as 2e-6.
            inline constexpr TEffectValue kMathRoundingPrecision = 0.015625;


            // -------- FUNCTIONS ------------------------------------------ //

            /// Converts the supplied angle from hundredths of degrees to radians.
            /// @param [in] Angle to convert.
            /// @return Converted angle in radians.
            constexpr inline TEffectValue AngleDegreeHundredthsToRadians(TEffectValue angle)
            {
                constexpr TEffectValue kConversionFactor = std::numbers::pi_v<TEffectValue> / 18000;
                return angle * kConversionFactor;
            }

            /// Converts the supplied angle from radians to hundredths of degrees.
            /// @param [in] Angle to convert.
            /// @return Converted angle in hundredths of degrees.
            constexpr inline TEffectValue AngleRadiansToDegreeHundredths(TEffectValue angle)
            {
                constexpr TEffectValue kConversionFactor = 18000 / std::numbers::pi_v<TEffectValue>;
                return angle * kConversionFactor;
            }

            /// Rounds the supplied value to the nearest multiple of another supplied value.
            /// @param [in] value Value to be rounded.
            /// @param [in] roundToMultiple Desired multiple of the input to which the input should be rounded.
            /// @return Input rounded to the nearest multiple of the desired value.
            inline TEffectValue NearestMultiple(TEffectValue value, TEffectValue roundToMultiple)
            {
                return std::nearbyint(value / roundToMultiple) * roundToMultiple;
            }

            /// Computes the cosine of the supplied angle, which is measured in hundredths of degrees.
            /// Rounds the result to the nearest multiple of the constant at the top of this file.
            /// @param [in] angle Angle whose cosine is to be computed.
            /// @return Cosine of the input angle.
            inline TEffectValue TrigonometryCosine(TEffectValue angle)
            {
                return NearestMultiple(std::cos(AngleDegreeHundredthsToRadians(angle)), kMathRoundingPrecision);
            }

            /// Computes the sine of the supplied angle, which is measured in hundredths of degrees.
            /// Rounds the result to the nearest multiple of the constant at the top of this file.
            /// @param [in] angle Angle whose sine is to be computed.
            /// @return Sine of the input angle.
            inline TEffectValue TrigonometrySine(TEffectValue angle)
            {
                return NearestMultiple(std::sin(AngleDegreeHundredthsToRadians(angle)), kMathRoundingPrecision);
            }

            /// Computes the inverse tangent of the ratio if the supplied parameters.
            /// Rounds the result to the nearest multiple of the constant at the top of this file.
            /// @param [in] numerator Numerator of the ratio.
            /// @param [in] denominator Denominator of the ratio.
            /// @return Inverse tangent in hundredths of degrees of the numerator divided by the denominator.
            inline TEffectValue TrigonometryArcTanOfRatio(TEffectValue numerator, TEffectValue denominator)
            {
                const TEffectValue kRawAngle = NearestMultiple(AngleRadiansToDegreeHundredths(std::atan2(numerator, denominator)), kMathRoundingPrecision);

                if (kRawAngle < 0)
                    return kRawAngle + 36000;
                else
                    return kRawAngle;
            }
        }
    }
}
