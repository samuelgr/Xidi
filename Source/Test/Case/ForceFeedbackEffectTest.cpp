/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackEffectTest.cpp
 *   Unit tests for functionality common to all force feedback effects.
 *****************************************************************************/

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "TestCase.h"


namespace XidiTest
{
    using namespace ::Xidi::Controller::ForceFeedback;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Common duration value used throughout test cases.
    static constexpr TEffectTimeMs kTestEffectDuration = 1000;

    /// Common start delay value used throughout test cases.
    static constexpr TEffectTimeMs kTestEffectStartDelay = 500;

    /// Common sample period value used throughout test cases.
    static constexpr TEffectTimeMs kTestEffectSamplePeriod = 10;

    /// Common gain value used throughout test cases.
    static constexpr TEffectValue kTestEffectGain = 1000;

    /// Common trivial envelope used throughout test cases.
    /// This envelope has no effect. Magnitudes should remain completely unchanged.
    static constexpr SEnvelope kTestTrivialEnvelope = {.attackTime = 0, .attackLevel = 1000, .fadeTime = 0, .fadeLevel = 2500};


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Simple test force feedback effect that returns the raw time as its computed magnitude.
    class TestEffect : public Effect
    {
    public:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //

        TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override
        {
            return (TEffectValue)rawTime;
        }
    };


    // -------- TEST CASES ------------------------------------------------- //

    // Creates a simple test effect with no properties other than duration.
    // Verifies that it returns the correct computed magnitude at all times throughout its duration.
    TEST_CASE(ForceFeedbackEffect_NominalEffect_Magnitude)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            TEST_ASSERT((TEffectValue)t == effect.ComputeMagnitude(t));
    }

    // Creates a simple test effect with no properties other than duration.
    // Verifies that it returns the correct values for all of its common properties.
    TEST_CASE(ForceFeedbackEffect_NominalEffect_Parameters)
    {
        TestEffect effect;
        TEST_ASSERT(false == effect.HasDirection());
        TEST_ASSERT(false == effect.HasDuration());

        effect.InitializeDefaultDirection();
        TEST_ASSERT(true == effect.HasDirection());

        effect.SetDuration(kTestEffectDuration);
        TEST_ASSERT(true == effect.HasDuration());
        TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());
        TEST_ASSERT(kTestEffectDuration == effect.GetTotalTime());

        TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
        TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
        TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
        TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
    }

    // Creates a test effect with a start delay.
    // Verifies that it returns the correct computed magnitude at all times throughout its duration.
    TEST_CASE(ForceFeedbackEffect_EffectWithStartDelay_Magnitude)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetStartDelay(kTestEffectStartDelay);

        for (TEffectTimeMs t = 0; t < kTestEffectStartDelay; ++t)
            TEST_ASSERT(kEffectForceMagnitudeZero == effect.ComputeMagnitude(t));

        for (TEffectTimeMs t = kTestEffectStartDelay; t < (kTestEffectDuration + kTestEffectStartDelay); ++t)
            TEST_ASSERT(t - kTestEffectStartDelay == effect.ComputeMagnitude(t));
    }

    // Creates a test effect with a start delay.
    // Verifies that it returns the correct values for all of its common properties.
    TEST_CASE(ForceFeedbackEffect_EffectWithStartDelay_Parameters)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetStartDelay(kTestEffectStartDelay);

        TEST_ASSERT(true == effect.HasDuration());
        TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());
        TEST_ASSERT(kTestEffectDuration + kTestEffectStartDelay == effect.GetTotalTime());

        TEST_ASSERT(kTestEffectStartDelay == effect.GetStartDelay());
        TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
        TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
        TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
    }

    // Creates a test effect with a sample period.
    // Verifies that it returns the correct computed magnitude at all times throughout its duration.
    TEST_CASE(ForceFeedbackEffect_EffectWithSamplePeriod_Magnitude)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetSamplePeriod(kTestEffectSamplePeriod);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
        {
            // Time input, and hence magnitude output, should only increase in multiples of the specified sample period.
            const TEffectValue kExpectedMagnitude = (TEffectValue)(t / kTestEffectSamplePeriod) * kTestEffectSamplePeriod;
            const TEffectValue kActualMagnitude = effect.ComputeMagnitude(t);

            TEST_ASSERT(kActualMagnitude == kExpectedMagnitude);
        }
    }

    // Creates a test effect with a sample period.
    // Verifies that it returns the correct values for all of its common properties.
    TEST_CASE(ForceFeedbackEffect_EffectWithSamplePeriod_Parameters)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetSamplePeriod(kTestEffectSamplePeriod);

        TEST_ASSERT(true == effect.HasDuration());
        TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());
        TEST_ASSERT(kTestEffectDuration == effect.GetTotalTime());

        TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
        TEST_ASSERT(kTestEffectSamplePeriod == effect.GetSamplePeriod());
        TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
        TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
    }

    // Creates a test effect with a gain.
    // Verifies that it returns the correct computed magnitude at all times throughout its duration.
    TEST_CASE(ForceFeedbackEffect_EffectWithGain_Magnitude)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetGain(kTestEffectGain);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            TEST_ASSERT((TEffectValue)t * (kTestEffectGain / kEffectModifierRelativeDenominator) == effect.ComputeMagnitude(t));
    }

    // Creates a test effect with a gain.
    // Verifies that it returns the correct values for all of its common properties.
    TEST_CASE(ForceFeedbackEffect_EffectWithGain_Parameters)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetGain(kTestEffectGain);

        TEST_ASSERT(true == effect.HasDuration());
        TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());
        TEST_ASSERT(kTestEffectDuration== effect.GetTotalTime());

        TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
        TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
        TEST_ASSERT(kTestEffectGain == effect.GetGain());
        TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
    }

    // Creates a test effect with a trivial envelope that has no effect.
    // Verifies that it returns the correct computed magnitude at all times throughout its duration.
    TEST_CASE(ForceFeedbackEffect_EffectWithTrivialEnvelope_Magnitude)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetEnvelope(kTestTrivialEnvelope);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            TEST_ASSERT((TEffectValue)t == effect.ComputeMagnitude(t));
    }

    // Creates a test effect with a trivial envelope that has no effect.
    // Verifies that it returns the correct values for all of its common properties.
    TEST_CASE(ForceFeedbackEffect_EffectWithTrivialEnvelope_Parameters)
    {
        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetEnvelope(kTestTrivialEnvelope);

        TEST_ASSERT(true == effect.HasDuration());
        TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());
        TEST_ASSERT(kTestEffectDuration == effect.GetTotalTime());

        TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
        TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
        TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
        TEST_ASSERT(kTestTrivialEnvelope == effect.GetEnvelope());
    }

    // Submits a constant sustain level and uses an envelope to turn it into a linear function that increases with time.
    TEST_CASE(ForceFeedbackEffect_ApplyEnvelope_LinearIncrease)
    {
        // Pattern of increase is equivalent to the simple effect tests in which the result of applying the envelope is simply equal to the input time.
        // Both attack and fade times are half the duration, so the entire effect is defined by the envelope. Sustain level is equal to half the duration.
        // Attack region starts at 0 and goes up to the sustain level in half the duration, and the fade region starts at the sustain level and continues up to the entire duration worth of magnitude.
        constexpr SEnvelope kTestEnvelope = {.attackTime = kTestEffectDuration / 2, .attackLevel = kEffectForceMagnitudeZero, .fadeTime = kTestEffectDuration / 2, .fadeLevel = kTestEffectDuration};
        constexpr TEffectValue kSustainLevel = kTestEffectDuration / 2;

        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetEnvelope(kTestEnvelope);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            TEST_ASSERT((TEffectValue)t == effect.ApplyEnvelope(t, kSustainLevel));
    }

    // Submits a constant sustain level and uses an envelope to turn it into a linear function that decreases with time.
    TEST_CASE(ForceFeedbackEffect_ApplyEnvelope_LinearDecrease)
    {
        // Pattern is exactly as in the linear increase case but inverted.
        // Expected result of applying the envelope is to start at the duration and descend down to 0.
        constexpr SEnvelope kTestEnvelope = {.attackTime = kTestEffectDuration / 2, .attackLevel = kTestEffectDuration, .fadeTime = kTestEffectDuration / 2, .fadeLevel = kEffectForceMagnitudeZero};
        constexpr TEffectValue kSustainLevel = kTestEffectDuration / 2;

        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetEnvelope(kTestEnvelope);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
            TEST_ASSERT((TEffectValue)(kTestEffectDuration - t) == effect.ApplyEnvelope(t, kSustainLevel));
    }

    // Submits a constant sustain level and uses an envelope to turn it into a piece-wise function with three pieces: linear increase, constant sustain, and linear decrease.
    TEST_CASE(ForceFeedbackEffect_ApplyEnvelope_PiecewiseUpThenDown)
    {
        constexpr SEnvelope kTestEnvelope = {.attackTime = kTestEffectDuration / 4, .attackLevel = kEffectForceMagnitudeZero, .fadeTime = kTestEffectDuration / 4, .fadeLevel = kEffectForceMagnitudeZero};
        constexpr TEffectValue kSustainLevel = kTestEffectDuration;

        TestEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetEnvelope(kTestEnvelope);

        // First region takes one quarter of the total duration.
        // Attack region of the envelope defines a slope from 0 to the sustain level.
        for (TEffectTimeMs t = 0; t < (kTestEffectDuration / 4); ++t)
            TEST_ASSERT((TEffectValue)t * 4 == effect.ApplyEnvelope(t, kSustainLevel));

        // Second region takes one half of the total duration, from 1/4 to 3/4 of the total effect duration.
        // Expected output is just the sustain level as passed in.
        for (TEffectTimeMs t = (kTestEffectDuration / 4); t < ((kTestEffectDuration * 3) / 4); ++t)
            TEST_ASSERT((TEffectValue)kSustainLevel == effect.ApplyEnvelope(t, kSustainLevel));

        // Third region takes one quarter of the total duration.
        // Fade region of the envelope defines a slope from the sustain level to 0.
        for (TEffectTimeMs t = ((kTestEffectDuration * 3) / 4); t < kTestEffectDuration; ++t)
            TEST_ASSERT((TEffectValue)(kTestEffectDuration - t) * 4 == effect.ApplyEnvelope(t, kSustainLevel));
    }

    // Creates an effect and submits invalid parameters. Verifies that they are all rejected.
    TEST_CASE(ForceFeedbackEffect_InvalidParameters)
    {
        TestEffect effect;
        
        TEST_ASSERT(false == effect.SetDuration(0));
        TEST_ASSERT(false == effect.HasDuration());

        TEST_ASSERT(false == effect.SetGain(kEffectModifierMinimum - 1));
        TEST_ASSERT(false == effect.SetGain(kEffectModifierMaximum + 1));
        TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());

        TEST_ASSERT(false == effect.SetEnvelope({.attackLevel = kEffectModifierMinimum - 1}));
        TEST_ASSERT(false == effect.SetEnvelope({.attackLevel = kEffectModifierMaximum + 1}));
        TEST_ASSERT(false == effect.SetEnvelope({.fadeLevel = kEffectModifierMinimum - 1}));
        TEST_ASSERT(false == effect.SetEnvelope({.fadeLevel = kEffectModifierMaximum + 1}));
        TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
    }

    // Creates an effect and verifies that it reports correct information for whether or not it is completely defined.
    // Only a duration is required. All other parameters are optional.
    TEST_CASE(ForceFeedbackEffect_IsCompletelyDefined)
    {
        TestEffect effect;

        TEST_ASSERT(false == effect.IsCompletelyDefined());
        TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
        TEST_ASSERT(false == effect.IsCompletelyDefined());
        TEST_ASSERT(true == effect.InitializeDefaultDirection());
        TEST_ASSERT(true == effect.IsCompletelyDefined());
    }
}
