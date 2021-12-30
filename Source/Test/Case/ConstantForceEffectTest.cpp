/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ConstantForceEffectTest.cpp
 *   Unit tests for force feedback effects that produce a force of constant
 *   magnitude.
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

    /// Common gain value used throughout test cases.
    static constexpr TEffectValue kTestEffectGain = 1000;


    // -------- TEST CASES ------------------------------------------------- //
    
    // Creates constant force effects of varying magnitudes and ensures they report the correct magnitude.
    // No other parameters are set.
    TEST_CASE(ConstantForceEffect_ComputeMagnitude_Nominal)
    {
        constexpr TEffectValue kTestMagnitudes[] = {kEffectForceMagnitudeMinimum, kEffectForceMagnitudeZero, kEffectForceMagnitudeMaximum};

        for (const auto kTestMagnitude : kTestMagnitudes)
        {
            ConstantForceEffect effect;
            effect.InitializeDefaultDirection();
            effect.SetDuration(kTestEffectDuration);
            effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

            for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
                TEST_ASSERT(kTestMagnitude == effect.ComputeMagnitude(t));
        }
    }

    // Creates constant force effects of varying magnitudes and ensures they report the correct magnitude.
    // Additionally specifies a gain.
    TEST_CASE(ConstantForceEffect_ComputeMagnitude_Gain)
    {
        constexpr TEffectValue kTestMagnitudes[] = {kEffectForceMagnitudeMinimum, kEffectForceMagnitudeZero, kEffectForceMagnitudeMaximum};

        for (const auto kTestMagnitude : kTestMagnitudes)
        {
            ConstantForceEffect effect;
            effect.InitializeDefaultDirection();
            effect.SetDuration(kTestEffectDuration);
            effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

            effect.SetGain(kTestEffectGain);

            for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
                TEST_ASSERT((kTestMagnitude * kTestEffectGain / kEffectModifierRelativeDenominator) == effect.ComputeMagnitude(t));
        }
    }

    // Creates a constant force effect with a specific magnitude and applies a start delay. Ensures the start delay is handled correctly.
    // No other properties are specified.
    TEST_CASE(ConstantForceEffect_ComputeMagnitude_StartDelay)
    {
        constexpr TEffectValue kTestMagnitude = 2500;
        constexpr TEffectTimeMs kTestStartDelay = 50;

        ConstantForceEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

        effect.SetStartDelay(kTestStartDelay);

        for (TEffectTimeMs t = 0; t < kTestStartDelay; ++t)
            TEST_ASSERT(kEffectForceMagnitudeZero == effect.ComputeMagnitude(t));

        for (TEffectTimeMs t = kTestStartDelay; t < (kTestStartDelay + kTestEffectDuration); ++t)
            TEST_ASSERT(kTestMagnitude == effect.ComputeMagnitude(t));
    }

    // Creates a constant force effect with a positive magnitude and applies an envelope transformation.
    // No other properties are specified.
    TEST_CASE(ConstantForceEffect_ComputeMagnitude_EnvelopePositive)
    {
        constexpr TEffectValue kTestMagnitude = 5000;

        ConstantForceEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

        constexpr SEnvelope kTestEnvelope = {.attackTime = kTestEffectDuration / 10, .attackLevel = 7000, .fadeTime = kTestEffectDuration / 5, .fadeLevel = 1000};
        effect.SetEnvelope(kTestEnvelope);
        TEST_ASSERT(true == effect.GetEnvelope().has_value());
        TEST_ASSERT(kTestEnvelope == effect.GetEnvelope().value());

        constexpr TEffectValue kAttackSlope = (kTestMagnitude - kTestEnvelope.attackLevel) / kTestEnvelope.attackTime;
        constexpr TEffectValue kFadeSlope = (kTestEnvelope.fadeLevel - kTestMagnitude) / kTestEnvelope.fadeTime;
        constexpr TEffectTimeMs kAttackStartTime = 0;
        constexpr TEffectTimeMs kAttackEndTime = kTestEnvelope.attackTime;
        constexpr TEffectTimeMs kFadeStartTime = kTestEffectDuration - kTestEnvelope.fadeTime;
        constexpr TEffectTimeMs kFadeEndTime = kTestEffectDuration;

        for (TEffectTimeMs t = kAttackStartTime; t < kAttackEndTime; ++t)
            TEST_ASSERT((kTestEnvelope.attackLevel + ((t - kAttackStartTime) * kAttackSlope)) == effect.ComputeMagnitude(t));
        
        for (TEffectTimeMs t = kAttackEndTime; t < kFadeStartTime; ++t)
            TEST_ASSERT(kTestMagnitude == effect.ComputeMagnitude(t));

        for (TEffectTimeMs t = kFadeStartTime; t < kFadeEndTime; ++t)
            TEST_ASSERT((kTestMagnitude + ((t - kFadeStartTime) * kFadeSlope)) == effect.ComputeMagnitude(t));
    }

    // Creates a constant force effect with a negative magnitude and applies an envelope transformation.
    // This test is a bit tricky and requires sign manipulation because the envelope is expected to adjust amplitude (i.e. distance from 0) in the case of a constant force.
    // No other properties are specified.
    TEST_CASE(ConstantForceEffect_ComputeMagnitude_EnvelopeNegative)
    {
        constexpr TEffectValue kTestMagnitude = -5000;

        ConstantForceEffect effect;
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

        constexpr SEnvelope kTestEnvelope = {.attackTime = kTestEffectDuration / 10, .attackLevel = 7000, .fadeTime = kTestEffectDuration / 5, .fadeLevel = 1000};
        effect.SetEnvelope(kTestEnvelope);
        TEST_ASSERT(true == effect.GetEnvelope().has_value());
        TEST_ASSERT(kTestEnvelope == effect.GetEnvelope().value());

        constexpr TEffectValue kAttackSlope = (kTestMagnitude + kTestEnvelope.attackLevel) / kTestEnvelope.attackTime;
        constexpr TEffectValue kFadeSlope = (-kTestEnvelope.fadeLevel - kTestMagnitude) / kTestEnvelope.fadeTime;
        constexpr TEffectTimeMs kAttackStartTime = 0;
        constexpr TEffectTimeMs kAttackEndTime = kTestEnvelope.attackTime;
        constexpr TEffectTimeMs kFadeStartTime = kTestEffectDuration - kTestEnvelope.fadeTime;
        constexpr TEffectTimeMs kFadeEndTime = kTestEffectDuration;

        for (TEffectTimeMs t = kAttackStartTime; t < kAttackEndTime; ++t)
            TEST_ASSERT((-kTestEnvelope.attackLevel + ((t - kAttackStartTime) * kAttackSlope)) == effect.ComputeMagnitude(t));

        for (TEffectTimeMs t = kAttackEndTime; t < kFadeStartTime; ++t)
            TEST_ASSERT(kTestMagnitude == effect.ComputeMagnitude(t));

        for (TEffectTimeMs t = kFadeStartTime; t < kFadeEndTime; ++t)
            TEST_ASSERT((kTestMagnitude + ((t - kFadeStartTime) * kFadeSlope)) == effect.ComputeMagnitude(t));
    }

    // Creates a constant force effect and submits invalid type-specific parameters. Verifies that they are rejected.
    TEST_CASE(ConstantForceEffect_InvalidTypeSpecificParameters)
    {
        ConstantForceEffect effect;

        TEST_ASSERT(false == effect.SetTypeSpecificParameters({.magnitude = kEffectForceMagnitudeMinimum  - 1}));
        TEST_ASSERT(false == effect.SetTypeSpecificParameters({.magnitude = kEffectForceMagnitudeMaximum + 1}));
    }

    // Creates a constant force effect and verifies that it reports correct information for whether or not it is completely defined.
    // Duration and type-specific parameters are required. All others are optional.
    TEST_CASE(ConstantForceEffect_IsCompletelyDefined)
    {
        ConstantForceEffect effect;

        TEST_ASSERT(false == effect.IsCompletelyDefined());
        TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
        TEST_ASSERT(true == effect.InitializeDefaultDirection());
        TEST_ASSERT(false == effect.IsCompletelyDefined());
        TEST_ASSERT(true == effect.SetTypeSpecificParameters({.magnitude = 0}));
        TEST_ASSERT(true == effect.IsCompletelyDefined());
    }
}
