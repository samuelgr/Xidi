/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackEffect.cpp
 *   Implementation of internal force feedback effect computations.
 *****************************************************************************/

#include "ForceFeedbackEffect.h"

#include <optional>


namespace Xidi
{
    namespace ForceFeedback
    {
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //
        // See "ForceFeedbackEffect.h" for documentation.

        bool ConstantForceEffect::AreTypeSpecificParametersValid(const SConstantForceParameters& newTypeSpecificParameters) const
        {
            return ((newTypeSpecificParameters.magnitude >= kEffectForceMagnitudeMinimum) && (newTypeSpecificParameters.magnitude <= kEffectForceMagnitudeMaximum));
        }

        // --------

        TEffectValue ConstantForceEffect::ComputeRawMagnitude(TEffectTimeMs rawTime) const
        {
            const TEffectValue magnitude = GetTypeSpecificParameters().value().magnitude;

            if (magnitude >= 0)
                return ApplyEnvelope(rawTime, magnitude);
            else
                return -ApplyEnvelope(rawTime, -magnitude);
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "ForceFeedbackEffect.h" for documentation.

        TEffectValue Effect::ApplyEnvelope(TEffectTimeMs rawTime, TEffectValue sustainLevel) const
        {
            if (false == commonParameters.envelope.has_value())
                return sustainLevel;

            const SEffectEnvelope& envelope = commonParameters.envelope.value();

            if (rawTime < envelope.attackTime)
            {
                const TEffectTimeMs envelopeTime = rawTime;
                const TEffectValue envelopeSlope = (sustainLevel - envelope.attackLevel) / envelope.attackTime;
                return envelope.attackLevel + (envelopeSlope * envelopeTime);
            }
            else if (rawTime > commonParameters.duration.value() - envelope.fadeTime)
            {
                const TEffectTimeMs envelopeTime = rawTime - (commonParameters.duration.value() - envelope.fadeTime);
                const TEffectValue envelopeSlope = (envelope.fadeLevel - sustainLevel) / envelope.fadeTime;
                return sustainLevel + (envelopeSlope * envelopeTime);
            }

            return sustainLevel;
        }

        // --------

        TEffectValue Effect::ComputeMagnitude(TEffectTimeMs time) const
        {
            if ((time < commonParameters.startDelay) || (time >= (commonParameters.startDelay + commonParameters.duration.value_or(0))))
                return kEffectForceMagnitudeZero;

            const TEffectTimeMs playbackTime = time - commonParameters.startDelay;
            const TEffectTimeMs rawTime = playbackTime - (playbackTime % samplePeriodForComputations);
            return ComputeRawMagnitude(rawTime) * gainFraction;
        }
    }
}
