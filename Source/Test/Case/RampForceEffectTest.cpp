/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file RampForceEffectTest.cpp
 *   Unit tests for force feedback effects that produce a force of magnitude
 *   that changes linearly with time.
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

    // Creates a ramp force effect and ensures it report the correct magnitude as a function of time.
    // No other parameters are set.
    TEST_CASE(RampForceEffect_ComputeMagnitude_Nominal)
    {
        constexpr SRampForceParameters kTestMagnitudes = {.magnitudeStart = 0, .magnitudeEnd = kTestEffectDuration};

        RampForceEffect effect;
        effect.InitializeDefaultAssociatedAxes();
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetTypeSpecificParameters(kTestMagnitudes);

        for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
        {
            const TEffectValue expectedMagnitude = (TEffectValue)t;
            const TEffectValue actualMagnitude = effect.ComputeMagnitude(t);
            TEST_ASSERT(actualMagnitude == expectedMagnitude);
        }
    }

    // Creates a ramp force effect with constant positive magnitude and applies an envelope transformation.
    // No other properties are specified.
    TEST_CASE(RampForceEffect_ComputeMagnitude_EnvelopePositive)
    {
        constexpr TEffectValue kTestMagnitude = 5000;

        RampForceEffect effect;
        effect.InitializeDefaultAssociatedAxes();
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetTypeSpecificParameters({.magnitudeStart = kTestMagnitude, .magnitudeEnd = kTestMagnitude});

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

    // Creates a ramp force effect with constant negative magnitude and applies an envelope transformation.
    // No other properties are specified.
    TEST_CASE(RampForceEffect_ComputeMagnitude_EnvelopeNegative)
    {
        constexpr TEffectValue kTestMagnitude = -5000;

        RampForceEffect effect;
        effect.InitializeDefaultAssociatedAxes();
        effect.InitializeDefaultDirection();
        effect.SetDuration(kTestEffectDuration);
        effect.SetTypeSpecificParameters({.magnitudeStart = kTestMagnitude, .magnitudeEnd = kTestMagnitude});

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
}
