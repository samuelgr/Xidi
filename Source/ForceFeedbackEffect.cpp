/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ForceFeedbackEffect.cpp
 *   Implementation of internal force feedback effect computations.
 **************************************************************************************************/

#include "ForceFeedbackEffect.h"

#include <atomic>
#include <memory>
#include <optional>

#include "ForceFeedbackMath.h"

namespace Xidi
{
  namespace Controller
  {
    namespace ForceFeedback
    {
      /// Holds the next available value for a force feedback effect identifier.
      static std::atomic<TEffectIdentifier> nextEffectIdentifier = 0;

      Effect::Effect(void) : id(nextEffectIdentifier++), commonParameters() {}

      bool ConstantForceEffect::AreTypeSpecificParametersValid(
          const SConstantForceParameters& newTypeSpecificParameters) const
      {
        return (
            (newTypeSpecificParameters.magnitude >= kEffectForceMagnitudeMinimum) &&
            (newTypeSpecificParameters.magnitude <= kEffectForceMagnitudeMaximum));
      }

      bool PeriodicEffect::AreTypeSpecificParametersValid(
          const SPeriodicParameters& newTypeSpecificParameters) const
      {
        if ((newTypeSpecificParameters.amplitude < 0) ||
            (newTypeSpecificParameters.amplitude > kEffectForceMagnitudeMaximum))
          return false;

        if ((newTypeSpecificParameters.offset < kEffectForceMagnitudeMinimum) ||
            (newTypeSpecificParameters.offset > kEffectForceMagnitudeMaximum))
          return false;

        if ((newTypeSpecificParameters.phase < kEffectAngleMinimum) ||
            (newTypeSpecificParameters.phase > kEffectAngleMaximum))
          return false;

        if (newTypeSpecificParameters.period < 1) return false;

        return true;
      }

      bool RampForceEffect::AreTypeSpecificParametersValid(
          const SRampForceParameters& newTypeSpecificParameters) const
      {
        if ((newTypeSpecificParameters.magnitudeStart < kEffectForceMagnitudeMinimum) ||
            (newTypeSpecificParameters.magnitudeStart > kEffectForceMagnitudeMaximum))
          return false;

        if ((newTypeSpecificParameters.magnitudeEnd < kEffectForceMagnitudeMinimum) ||
            (newTypeSpecificParameters.magnitudeEnd > kEffectForceMagnitudeMaximum))
          return false;

        return true;
      }

      void ConstantForceEffect::CheckAndFixTypeSpecificParameters(
          SConstantForceParameters& newTypeSpecificParameters) const
      {
        if (newTypeSpecificParameters.magnitude < kEffectForceMagnitudeMinimum)
          newTypeSpecificParameters.magnitude = kEffectForceMagnitudeMinimum;
        else if (newTypeSpecificParameters.magnitude > kEffectForceMagnitudeMaximum)
          newTypeSpecificParameters.magnitude = kEffectForceMagnitudeMaximum;
      }

      std::unique_ptr<Effect> ConstantForceEffect::Clone(void) const
      {
        return std::make_unique<ConstantForceEffect>(*this);
      }

      std::unique_ptr<Effect> RampForceEffect::Clone(void) const
      {
        return std::make_unique<RampForceEffect>(*this);
      }

      std::unique_ptr<Effect> SawtoothDownEffect::Clone(void) const
      {
        return std::make_unique<SawtoothDownEffect>(*this);
      }

      std::unique_ptr<Effect> SawtoothUpEffect::Clone(void) const
      {
        return std::make_unique<SawtoothUpEffect>(*this);
      }

      std::unique_ptr<Effect> SineWaveEffect::Clone(void) const
      {
        return std::make_unique<SineWaveEffect>(*this);
      }

      std::unique_ptr<Effect> SquareWaveEffect::Clone(void) const
      {
        return std::make_unique<SquareWaveEffect>(*this);
      }

      std::unique_ptr<Effect> TriangleWaveEffect::Clone(void) const
      {
        return std::make_unique<TriangleWaveEffect>(*this);
      }

      TEffectValue PeriodicEffect::ComputePhase(TEffectTimeMs rawTime) const
      {
        const TEffectValue rawTimeInPeriods =
            (TEffectValue)rawTime / (TEffectValue)GetTypeSpecificParameters().value().period;

        TEffectValue currentPhase = std::round(
            ((rawTimeInPeriods - floorf(rawTimeInPeriods)) * 36000) +
            GetTypeSpecificParameters().value().phase);
        if (currentPhase >= 36000) currentPhase -= 36000;

        return currentPhase;
      }

      TEffectValue ConstantForceEffect::ComputeRawMagnitude(TEffectTimeMs rawTime) const
      {
        const TEffectValue magnitude = GetTypeSpecificParameters().value().magnitude;

        if (magnitude >= 0)
          return ApplyEnvelope(rawTime, magnitude);
        else
          return -ApplyEnvelope(rawTime, -magnitude);
      }

      TEffectValue PeriodicEffect::ComputeRawMagnitude(TEffectTimeMs rawTime) const
      {
        const TEffectValue modifiedAmplitude =
            ApplyEnvelope(rawTime, GetTypeSpecificParameters().value().amplitude);
        const TEffectValue rawMagnitude =
            (modifiedAmplitude * WaveformAmplitude(ComputePhase(rawTime))) +
            GetTypeSpecificParameters().value().offset;

        return std::min(
            kEffectForceMagnitudeMaximum, std::max(kEffectForceMagnitudeMinimum, rawMagnitude));
      }

      TEffectValue RampForceEffect::ComputeRawMagnitude(TEffectTimeMs rawTime) const
      {
        const SRampForceParameters& rampParameters = GetTypeSpecificParameters().value();
        const TEffectValue slope =
            (rampParameters.magnitudeEnd - rampParameters.magnitudeStart) / GetDuration().value();
        const TEffectValue intercept = rampParameters.magnitudeStart;

        const TEffectValue magnitude = ((rawTime * slope) + intercept);

        if (magnitude >= 0)
          return ApplyEnvelope(rawTime, magnitude);
        else
          return -ApplyEnvelope(rawTime, -magnitude);
      }

      TEffectValue SawtoothDownEffect::WaveformAmplitude(TEffectValue phase) const
      {
        // Per DirectInput documentation, sawtooth down waves start at +1 and descend all the way to
        // -1, hitting it at the 360-degree point. See
        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418719%28v=vs.85%29
        // for more information.

        constexpr TEffectValue kSlope = -((TEffectValue)2 / (TEffectValue)36000);
        constexpr TEffectValue kIntercept = 1;

        return (phase * kSlope) + kIntercept;
      }

      TEffectValue SawtoothUpEffect::WaveformAmplitude(TEffectValue phase) const
      {
        // Per DirectInput documentation, sawtooth up waves start at -1 and ascend all the way to
        // +1, hitting it at the 360-degree point. See
        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418719%28v=vs.85%29
        // for more information.

        constexpr TEffectValue kSlope = ((TEffectValue)2 / (TEffectValue)36000);
        constexpr TEffectValue kIntercept = -1;

        return (phase * kSlope) + kIntercept;
      }

      TEffectValue SineWaveEffect::WaveformAmplitude(TEffectValue phase) const
      {
        return TrigonometrySine(phase);
      }

      TEffectValue SquareWaveEffect::WaveformAmplitude(TEffectValue phase) const
      {
        // Per DirectInput documentation, square waves are at +1 for the first half of the cycle and
        // -1 for the second half of the cycle. See
        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418719%28v=vs.85%29
        // for more information.

        if (phase < 18000)
          return 1;
        else
          return -1;
      }

      TEffectValue TriangleWaveEffect::WaveformAmplitude(TEffectValue phase) const
      {
        // Per DirectInput documentation, triangle waves start at +1 and descend to -1, hitting it
        // at the 180-degree point, then ascending back to +1 and hitting it at the 360-degree
        // point. See
        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418719%28v=vs.85%29
        // for more information.

        if (phase < 18000)
        {
          constexpr TEffectValue kSlope = -((TEffectValue)2 / (TEffectValue)18000);
          constexpr TEffectValue kIntercept = 1;

          return (phase * kSlope) + kIntercept;
        }
        else
        {
          constexpr TEffectValue kSlope = ((TEffectValue)2 / (TEffectValue)18000);
          constexpr TEffectValue kIntercept = -1;

          return ((phase - 18000) * kSlope) + kIntercept;
        }
      }

      TEffectValue Effect::ApplyEnvelope(TEffectTimeMs rawTime, TEffectValue sustainLevel) const
      {
        if (false == commonParameters.envelope.has_value()) return sustainLevel;

        const SEnvelope& envelope = commonParameters.envelope.value();

        if (rawTime < envelope.attackTime)
        {
          const TEffectTimeMs envelopeTime = rawTime;
          const TEffectValue envelopeSlope =
              (sustainLevel - envelope.attackLevel) / envelope.attackTime;
          return envelope.attackLevel + (envelopeSlope * envelopeTime);
        }
        else if (rawTime > commonParameters.duration.value() - envelope.fadeTime)
        {
          const TEffectTimeMs envelopeTime =
              rawTime - (commonParameters.duration.value() - envelope.fadeTime);
          const TEffectValue envelopeSlope =
              (envelope.fadeLevel - sustainLevel) / envelope.fadeTime;
          return sustainLevel + (envelopeSlope * envelopeTime);
        }

        return sustainLevel;
      }

      TEffectValue Effect::ComputeMagnitude(TEffectTimeMs time) const
      {
        if (time >= commonParameters.duration.value_or(0)) return kEffectForceMagnitudeZero;

        const TEffectTimeMs rawTime = time - (time % commonParameters.samplePeriodForComputations);
        return ComputeRawMagnitude(rawTime) * commonParameters.gainFraction;
      }
    } // namespace ForceFeedback
  }   // namespace Controller
} // namespace Xidi
