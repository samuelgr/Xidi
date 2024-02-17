/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ForceFeedbackTypes.h
 *   Declaration of constants and types used for representing force feedback
 *   effects and actuators.
 **************************************************************************************************/

#pragma once

#include <array>
#include <cstdint>

#include "ControllerTypes.h"

namespace Xidi
{
  namespace Controller
  {
    namespace ForceFeedback
    {
      /// Enumerates the different types of supported coordinate systems that can be used to
      /// represent force feedback effect directions.
      enum class ECoordinateSystem : uint8_t
      {
        Cartesian,
        Polar,
        Spherical
      };

      /// Enumerates the force feedback actuators present on physical controllers.
      enum class EActuator : uint8_t
      {
        /// Left motor (low-frequency rumble)
        LeftMotor,

        /// Right motor (high-frequency rumble)
        RightMotor,

        /// Left impulse trigger (embedded in LT)
        LeftImpulseTrigger,

        /// Right impulse trigger (embedded in RT)
        RightImpulseTrigger,

        /// Sentinel value, total number of enumerators
        Count
      };

      /// Enumerates the different methods of computing a physical actuator's power state data given
      /// a force feedback magnitude component vector.
      enum class EActuatorMode : uint8_t
      {
        /// Actuator gets its power state data from a single force feedback axis. Direction mapping
        /// can be specified.
        SingleAxis,

        /// Actuator gets its power state data from a magnitude projection along two axes.
        MagnitudeProjection,

        /// Sentinel value, total number of enumerators.
        Count
      };

      /// Type used for identifying effects.
      using TEffectIdentifier = uint64_t;

      /// Type used for keeping track of time, in milliseconds, as it relates to force feedback
      /// effects.
      using TEffectTimeMs = uint32_t;

      /// Type used for all values used in internal effect-related computations.
      using TEffectValue = float;

      /// Type used to represent a force feedback effect value that can be sent to a physical
      /// actuator.
      using TPhysicalActuatorValue = uint16_t;

      /// Represents the magnitude of a force broken down into its per-axis components, one per
      /// element per axis associated with the force feedback effect.
      using TMagnitudeComponents = std::array<TEffectValue, static_cast<int>(EActuator::Count)>;

      /// Represents the magnitude of a force broken down into its per-axis components using a
      /// universal ordering scheme of one element per possible virtual controller axis. Many of the
      /// elements in this array will be 0 for virtual controller axes not associated with the force
      /// feedback effect. This is just a reordering of #TMagnitudeComponents in a way that does not
      /// depend on the number or types of axes actually associated with the force feedback effect.
      using TOrderedMagnitudeComponents = std::array<TEffectValue, static_cast<int>(EAxis::Count)>;

      /// Describes a force feedback actuator element on a virtual controller.
      /// A force feedback actuator can be mapped to an axis and a direction mode on that axis.
      /// The information is used to determine what source of information is used to send output to
      /// a physical force feedback actuator.
      struct SActuatorElement
      {
        /// Whether or not the associated physical force feedback actuator is present in the
        /// mapping.
        bool isPresent : 1;

        /// Actuator mode, which describes how the actuator should obtain its power state data from
        /// a force feedback magnitude component vector.
        EActuatorMode mode : 3;

        union
        {
          struct
          {
            /// Source virtual force feedback axis from which the physical actuator should obtain
            /// its state data.
            EAxis axis : 3;

            /// Direction mode associated with the virtual force feedback axis.
            EAxisDirection direction : 3;
          } singleAxis;

          struct
          {
            /// First source virtual force feedback axis for the projection.
            EAxis axisFirst : 3;

            /// Second source virtual force feedback axis for the projection.
            EAxis axisSecond : 3;
          } magnitudeProjection;
        };

        constexpr bool operator==(const SActuatorElement& other) const
        {
          if (other.isPresent != isPresent) return false;

          if (other.mode != mode) return false;

          switch (mode)
          {
            case EActuatorMode::SingleAxis:
              return (
                  (other.singleAxis.axis == singleAxis.axis) &&
                  (other.singleAxis.direction == singleAxis.direction));

            case EActuatorMode::MagnitudeProjection:
              return (
                  (other.magnitudeProjection.axisFirst == magnitudeProjection.axisFirst) &&
                  (other.magnitudeProjection.axisSecond == magnitudeProjection.axisSecond));

            default:
              return true;
          }
        }
      };

      static_assert(sizeof(SActuatorElement) == 2, "Data structure size constraint violation.");
      static_assert(
          (uint8_t)EActuatorMode::Count <= 0b111,
          "Highest-valued force feedback actuator mode identifier does not fit into 3 bits.");
      static_assert(
          (uint8_t)EAxis::Count <= 0b111,
          "Highest-valued axis type identifier does not fit into 3 bits.");
      static_assert(
          (uint8_t)EAxisDirection::Count <= 0b111,
          "Highest-valued axis direction mode does not fit into 3 bits.");

      /// Represents the magnitude of a force as can be sent to physical force feedback actuators.
      /// One element exists per possible physical force feedback actuator. Field names correspond
      /// to the names of enumerators in #EActuator.
      struct SPhysicalActuatorComponents
      {
        TPhysicalActuatorValue leftMotor;
        TPhysicalActuatorValue rightMotor;
        TPhysicalActuatorValue leftImpulseTrigger;
        TPhysicalActuatorValue rightImpulseTrigger;

        constexpr bool operator==(const SPhysicalActuatorComponents& other) const = default;
      };

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
      inline constexpr TEffectValue kEffectModifierRelativeDenominator =
          (kEffectModifierMaximum - kEffectModifierMinimum);

      /// Minimum value for an effect's output magnitude.
      /// This value is intended to signify full device strength in the negative direction.
      inline constexpr TEffectValue kEffectForceMagnitudeMinimum = -10000;

      /// Maximum value for an effect's output magnitude.
      /// This value is intended to signify full device strength in the positive direction.
      inline constexpr TEffectValue kEffectForceMagnitudeMaximum = 10000;

      /// Zero value for an effect's output magnitude.
      /// This value is intended to signify that there is no force generated at all.
      inline constexpr TEffectValue kEffectForceMagnitudeZero = 0;

      /// Vector addition is performed element-by-element.
      constexpr TOrderedMagnitudeComponents& operator+=(
          TOrderedMagnitudeComponents& vectorLeft, const TOrderedMagnitudeComponents& vectorRight)
      {
        for (size_t i = 0; i < vectorLeft.size(); ++i)
          vectorLeft[i] = vectorLeft[i] + vectorRight[i];

        return vectorLeft;
      }

      constexpr TOrderedMagnitudeComponents operator+(
          const TOrderedMagnitudeComponents& vectorA, const TOrderedMagnitudeComponents& vectorB)
      {
        TOrderedMagnitudeComponents vectorResult = vectorA;
        vectorResult += vectorB;
        return vectorResult;
      }
    } // namespace ForceFeedback
  }   // namespace Controller
} // namespace Xidi
