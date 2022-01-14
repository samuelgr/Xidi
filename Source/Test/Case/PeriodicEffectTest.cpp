/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file PeriodicEffectTest.cpp
 *   Unit tests for force feedback effects that produce a force that follows
 *   a periodic waveform.
 *****************************************************************************/

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "MockForceFeedbackEffect.h"
#include "TestCase.h"

#include <memory>


namespace XidiTest
{
    using namespace ::Xidi::Controller::ForceFeedback;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Common amplitude value used throughout test cases.
    static constexpr TEffectValue kTestEffectAmplitude = 5000;


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies that a periodic effect can correctly compute its phase for various raw time inputs.
    // This is the nominal case in which no phase offset is present.
    TEST_CASE(PeriodicEffect_ComputePhase_Nominal)
    {
        MockPeriodicEffect effect;
        TEST_ASSERT(true == effect.SetTypeSpecificParameters({.period = MockPeriodicEffect::kDegreeHundredthsPerCycle}));

        for (TEffectTimeMs t = 0; t <= (10 * MockPeriodicEffect::kDegreeHundredthsPerCycle); t += 100)
        {
            const TEffectValue kExpectedPhase = (TEffectValue)(t % MockPeriodicEffect::kDegreeHundredthsPerCycle);
            const TEffectValue kActualPhase = (TEffectValue)effect.ComputePhase(t);

            TEST_ASSERT(kActualPhase == kExpectedPhase);
        }
    }

    // Verifies that a periodic effect can correctly compute its phase for various raw time inputs.
    // Various different phase offset values are tried.
    TEST_CASE(PeriodicEffect_ComputePhase_PhaseOffset)
    {
        constexpr unsigned int kTestPhaseOffsets[] = {MockPeriodicEffect::kDegreeHundredthsPerCycle / 4, MockPeriodicEffect::kDegreeHundredthsPerCycle / 2, (MockPeriodicEffect::kDegreeHundredthsPerCycle * 3) / 4};

        MockPeriodicEffect effect;

        for (const auto kTestPhaseOffset : kTestPhaseOffsets)
        {
            TEST_ASSERT(true == effect.SetTypeSpecificParameters({.phase = (TEffectValue)kTestPhaseOffset, .period = MockPeriodicEffect::kDegreeHundredthsPerCycle}));

            for (TEffectTimeMs t = 0; t <= MockPeriodicEffect::kDegreeHundredthsPerCycle; t += 100)
            {
                const TEffectValue kExpectedPhase = (TEffectValue)((t + kTestPhaseOffset) % MockPeriodicEffect::kDegreeHundredthsPerCycle);
                const TEffectValue kActualPhase = (TEffectValue)effect.ComputePhase(t);

                TEST_ASSERT(kActualPhase == kExpectedPhase);
            }
        }
    }

    // Verifies that a periodic effect correctly applies a waveform offset when computing its magnitude.
    TEST_CASE(PeriodicEffect_ComputeMagnitude_WithOffset)
    {
        constexpr TEffectTimeMs kTestEffectDuration = MockPeriodicEffect::kDegreeHundredthsPerCycle;
        constexpr TEffectTimeMs kTestEffectEvaluationTime = kTestEffectDuration / 2;
        constexpr TEffectValue kTestEffectOffsets[] = {-1000, -100, 100, 1000};

        MockPeriodicEffect effect;
        TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
     
        for (const auto kTestEffectOffset : kTestEffectOffsets)
        {
            TEST_ASSERT(true == effect.SetTypeSpecificParameters({.amplitude = kTestEffectAmplitude, .offset = kTestEffectOffset, .period = kTestEffectDuration}));

            const TEffectValue kExpectedMagnitude = kTestEffectOffset + (kTestEffectAmplitude * effect.WaveformAmplitude(kTestEffectEvaluationTime));
            const TEffectValue kActualMagnitude = effect.ComputeMagnitude(kTestEffectEvaluationTime);
            TEST_ASSERT(kActualMagnitude == kExpectedMagnitude);
        }
    }

    // Verifies that a periodic effect correctly applies a waveform offset and envelope when computing its magnitude.
    TEST_CASE(PeriodicEffect_ComputeMagnitude_WithOffsetAndEnvelope)
    {
        constexpr TEffectTimeMs kTestEffectDuration = MockPeriodicEffect::kDegreeHundredthsPerCycle;
        constexpr TEffectTimeMs kTestEffectEvaluationTime = kTestEffectDuration / 2;
        constexpr SEnvelope kTestEffectEnvelope = {.attackTime = MockPeriodicEffect::kDegreeHundredthsPerCycle, .attackLevel = 0};
        constexpr TEffectValue kTestEffectOffsets[] = {-1000, -100, 100, 1000};

        // The envelope defined above has the effect of cutting the amplitude of the effect proportionally to its progress through its duration.
        constexpr TEffectValue kTestEffectEnvelopeMultiplier = ((TEffectValue)kTestEffectEvaluationTime / (TEffectValue)kTestEffectDuration);

        MockPeriodicEffect effect;
        TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
        TEST_ASSERT(true == effect.SetEnvelope(kTestEffectEnvelope));

        for (const auto kTestEffectOffset : kTestEffectOffsets)
        {
            TEST_ASSERT(true == effect.SetTypeSpecificParameters({.amplitude = kTestEffectAmplitude, .offset = kTestEffectOffset, .period = kTestEffectDuration}));

            const TEffectValue kExpectedMagnitude = kTestEffectOffset + (kTestEffectAmplitude * kTestEffectEnvelopeMultiplier * effect.WaveformAmplitude(kTestEffectEvaluationTime));
            const TEffectValue kActualMagnitude = effect.ComputeMagnitude(kTestEffectEvaluationTime);
            TEST_ASSERT(kActualMagnitude == kExpectedMagnitude);
        }
    }
}
