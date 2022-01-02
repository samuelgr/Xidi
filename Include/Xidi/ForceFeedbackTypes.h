/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackTypes.h
 *   Declaration of constants and types used for representing force feedback
 *   effects and actuators.
 *****************************************************************************/

#pragma once

#include <array>
#include <cstdint>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Type used for keeping track of time, in milliseconds, as it relates to force feedback effects.
            typedef uint32_t TEffectTimeMs;

            /// Type used for all values used in internal effect-related computations.
            typedef float TEffectValue;

            /// Represents the magnitude of a force broken down into its per-axis components.
            typedef std::array<TEffectValue, 4> TMagnitudeComponents;

            /// Enumerates the different types of supported coordinate systems that can be used to represent force feedback effect directions.
            enum class ECoordinateSystem : uint8_t
            {
                Cartesian,
                Polar,
                Spherical
            };


            // -------- CONSTANTS ------------------------------------------ //

            /// Minimum number of axes to which a force feedback can be applied.
            inline constexpr int kEffectAxesMinimumNumber = 1;

            /// Maximum number of axes to which a force feedback can be applied.
            inline constexpr int kEffectAxesMaximumNumber = std::tuple_size_v<TMagnitudeComponents>;

            /// Minimum allowed value for an angle. Represents 0 degrees.
            inline constexpr TEffectValue kEffectAngleMinimum = 0;

            /// Maximum allowed value for an angle. Represents 359.99 degrees.
            inline constexpr TEffectValue kEffectAngleMaximum = 35999;

            /// Minimum value for an effect modifier.
            inline constexpr TEffectValue kEffectModifierMinimum = 0;

            /// Maximum value for an effect modifier.
            inline constexpr TEffectValue kEffectModifierMaximum = 10000;

            /// Denominator for relative effect modifiers.
            inline constexpr TEffectValue kEffectModifierRelativeDenominator = (kEffectModifierMaximum - kEffectModifierMinimum);

            /// Minimum value for an effect's output magnitude.
            /// This value is intended to signify full device strength in the negative direction.
            inline constexpr TEffectValue kEffectForceMagnitudeMinimum = -10000;

            /// Maximum value for an effect's output magnitude.
            /// This value is intended to signify full device strength in the positive direction.
            inline constexpr TEffectValue kEffectForceMagnitudeMaximum = 10000;

            /// Zero value for an effect's output magnitude.
            /// This value is intended to signify that there is no force generated at all.
            inline constexpr TEffectValue kEffectForceMagnitudeZero = 0;
        }
    }
}
