/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file ForceFeedbackTypes.h
 *   Declaration of constants and types used for representing force feedback
 *   effects and actuators.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"

#include <array>
#include <cstdint>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Enumerates the different types of supported coordinate systems that can be used to represent force feedback effect directions.
            enum class ECoordinateSystem : uint8_t
            {
                Cartesian,
                Polar,
                Spherical
            };

            /// Enumerates the force feedback actuators present on physical controllers.
            enum class EActuator : uint8_t
            {
                LeftMotor,                                                  ///< Left motor (low-frequency rumble)
                RightMotor,                                                 ///< Right motor (high-frequency rumble)
                LeftImpulseTrigger,                                         ///< Left impulse trigger (embedded in LT)
                RightImpulseTrigger,                                        ///< Right impulse trigger (embedded in RT)
                Count                                                       ///< Sentinel value, total number of enumerators
            };

            /// Enumerates the different methods of computing a physical actuator's power state data given a force feedback magnitude component vector.
            enum class EActuatorMode : uint8_t
            {
                SingleAxis,                                                 ///< Actuator gets its power state data from a single force feedback axis. Direction mapping can be specified.
                MagnitudeProjection,                                        ///< Actuator gets its power state data from a magnitude projection along two axes.
                Count                                                       ///< Sentinel value, total number of enumerators.
            };

            /// Type used for identifying effects.
            typedef uint64_t TEffectIdentifier;

            /// Type used for keeping track of time, in milliseconds, as it relates to force feedback effects.
            typedef uint32_t TEffectTimeMs;

            /// Type used for all values used in internal effect-related computations.
            typedef float TEffectValue;

            /// Type used to represent a force feedback effect value that can be sent to a physical actuator.
            typedef uint16_t TPhysicalActuatorValue;

            /// Represents the magnitude of a force broken down into its per-axis components, one per element per axis associated with the force feedback effect.
            typedef std::array<TEffectValue, (int)EActuator::Count> TMagnitudeComponents;

            /// Represents the magnitude of a force broken down into its per-axis components using a universal ordering scheme of one element per possible virtual controller axis.
            /// Many of the elements in this array will be 0 for virtual controller axes not associated with the force feedback effect.
            /// This is just a reordering of #TMagnitudeComponents in a way that does not depend on the number or types of axes actually associated with the force feedback effect.
            typedef std::array<TEffectValue, (int)EAxis::Count> TOrderedMagnitudeComponents;

            /// Describes a force feedback actuator element on a virtual controller.
            /// A force feedback actuator can be mapped to an axis and a direction mode on that axis.
            /// The information is used to determine what source of information is used to send output to a physical force feedback actuator.
            struct SActuatorElement
            {
                bool isPresent : 1;                                         ///< Whether or not the associated physical force feedback actuator is present in the mapping.
                EActuatorMode mode : 3;                                     ///< Actuator mode, which describes how the actuator should obtain its power state data from a force feedback magnitude component vector.

                union
                {
                    struct
                    {
                        EAxis axis : 3;                                     ///< Source virtual force feedback axis from which the physical actuator should obtain its state data.
                        EAxisDirection direction : 3;                       ///< Direction mode associated with the virtual force feedback axis.
                    } singleAxis;                                           ///< Parameters for single axis mode.

                    struct
                    {
                        EAxis axisFirst : 3;                                ///< First source virtual force feedback axis for the projection.
                        EAxis axisSecond : 3;                               ///< Second source virtual force feedback axis for the projection.
                    } magnitudeProjection;                                  ///< Parameters for magnitude projection mode.
                };

                
            };
            static_assert(sizeof(SActuatorElement) == 2, "Data structure size constraint violation.");
            static_assert((uint8_t)EActuatorMode::Count <= 0b111, "Highest-valued force feedback actuator mode identifier does not fit into 3 bits.");
            static_assert((uint8_t)EAxis::Count <= 0b111, "Highest-valued axis type identifier does not fit into 3 bits.");
            static_assert((uint8_t)EAxisDirection::Count <= 0b111, "Highest-valued axis direction mode does not fit into 3 bits.");

            /// Represents the magnitude of a force as can be sent to physical force feedback actuators.
            /// One element exists per possible physical force feedback actuator.
            /// Field names correspond to the names of enumerators in #EActuator.
            struct SPhysicalActuatorComponents
            {
                TPhysicalActuatorValue leftMotor;
                TPhysicalActuatorValue rightMotor;
                TPhysicalActuatorValue leftImpulseTrigger;
                TPhysicalActuatorValue rightImpulseTrigger;

                /// Simple check for equality.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SPhysicalActuatorComponents& other) const = default;
            };


            // -------- CONSTANTS ------------------------------------------ //

            /// Minimum number of axes to which a force feedback can be applied.
            inline constexpr int kEffectAxesMinimumNumber = 1;

            /// Maximum number of axes to which a force feedback can be applied.
            inline constexpr int kEffectAxesMaximumNumber = (int)EActuator::Count;

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


            // -------- OPERATORS ------------------------------------------ //

            /// Addition-assignment operator for globally-ordered magnitude component vectors.
            /// @param [in] vectorLeft Vector on the left side of the assignment.
            /// @param [in] vectorRight Vector on the right side of the assignment.
            /// @return Reference to the vector on the left side of the assignment, which is updated with the result of the addition.
            constexpr inline TOrderedMagnitudeComponents& operator+=(TOrderedMagnitudeComponents& vectorLeft, const TOrderedMagnitudeComponents& vectorRight)
            {
                for (size_t i = 0; i < vectorLeft.size(); ++i)
                    vectorLeft[i] = vectorLeft[i] + vectorRight[i];

                return vectorLeft;
            }

            /// Addition operator for globally-ordered magnitude component vectors.
            /// @param [in] vectorA First vector to add.
            /// @param [in] vectorB Second vector to add.
            /// @return Sum of the two magnitude component vectors, which is computed using element-by-element addition.
            constexpr inline TOrderedMagnitudeComponents operator+(const TOrderedMagnitudeComponents& vectorA, const TOrderedMagnitudeComponents& vectorB)
            {
                TOrderedMagnitudeComponents vectorResult = vectorA;
                vectorResult += vectorB;
                return vectorResult;
            }
        }
    }
}
