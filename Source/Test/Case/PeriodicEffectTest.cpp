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
#include "ForceFeedbackMath.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "MockForceFeedbackEffect.h"
#include "TestCase.h"


namespace XidiTest
{
    using namespace ::Xidi::Controller::ForceFeedback;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Common amplitude value used throughout test cases.
    static constexpr TEffectValue kTestEffectAmplitude = 5000;


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Test data record for waveform amplitude tests.
    struct SWaveformAmplitudeTestData
    {
        TEffectValue inputPhase;                                            ///< Input provided to the effect object, expressed as a phase in degree hundredths.
        TEffectValue expectedWaveformAmplitude;                             ///< Expected output from the waveform amplitude method.
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Checks if two force feedback effect values are effectively equal, subject to very minor imprecision.
    /// @param [in] valueA First of the two values to compare.
    /// @param [in] valueB Second of the two values to compare.
    /// @return `true` if the two values are essentially considered equivalent, `false` otherwise.
    static bool TEffectValueEqual(TEffectValue valueA, TEffectValue valueB)
    {
        return ((std::max(valueA, valueB) - std::min(valueA, valueB)) < 0.00001);
    }


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

    // Verifies correct waveform amplitude computations for various points in the waveform cycle.
    // This test case is for sawtooth down effects.
    TEST_CASE(PeriodicEffect_WaveformAmplitude_SawtoothDown)
    {
        constexpr SWaveformAmplitudeTestData kTestData[] = {
            {.inputPhase = 0,       .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 4500,    .expectedWaveformAmplitude = 0.75},
            {.inputPhase = 9000,    .expectedWaveformAmplitude = 0.5},
            {.inputPhase = 13500,   .expectedWaveformAmplitude = 0.25},
            {.inputPhase = 18000,   .expectedWaveformAmplitude = 0.0},
            {.inputPhase = 22500,   .expectedWaveformAmplitude = -0.25},
            {.inputPhase = 27000,   .expectedWaveformAmplitude = -0.5},
            {.inputPhase = 31500,   .expectedWaveformAmplitude = -0.75},
            {.inputPhase = 36000,   .expectedWaveformAmplitude = -1.0}
        };

        SawtoothDownEffect effect;

        for (const auto kTest : kTestData)
        {
            const TEffectValue kActualWaveformAmplitude = effect.WaveformAmplitude(kTest.inputPhase);
            TEST_ASSERT(true == TEffectValueEqual(kActualWaveformAmplitude, kTest.expectedWaveformAmplitude));
        }
    }

    // Verifies correct waveform amplitude computations for various points in the waveform cycle.
    // This test case is for sawtooth up effects.
    TEST_CASE(PeriodicEffect_WaveformAmplitude_SawtoothUp)
    {
        constexpr SWaveformAmplitudeTestData kTestData[] = {
            {.inputPhase = 0,       .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 4500,    .expectedWaveformAmplitude = -0.75},
            {.inputPhase = 9000,    .expectedWaveformAmplitude = -0.5},
            {.inputPhase = 13500,   .expectedWaveformAmplitude = -0.25},
            {.inputPhase = 18000,   .expectedWaveformAmplitude = 0.0},
            {.inputPhase = 22500,   .expectedWaveformAmplitude = 0.25},
            {.inputPhase = 27000,   .expectedWaveformAmplitude = 0.5},
            {.inputPhase = 31500,   .expectedWaveformAmplitude = 0.75},
            {.inputPhase = 36000,   .expectedWaveformAmplitude = 1.0}
        };

        SawtoothUpEffect effect;

        for (const auto kTest : kTestData)
        {
            const TEffectValue kActualWaveformAmplitude = effect.WaveformAmplitude(kTest.inputPhase);
            TEST_ASSERT(true == TEffectValueEqual(kActualWaveformAmplitude, kTest.expectedWaveformAmplitude));
        }
    }

    // Verifies correct waveform amplitude computations for various points in the waveform cycle.
    // This test case is for sine wave effects.
    TEST_CASE(PeriodicEffect_WaveformAmplitude_SineWave)
    {
        const TEffectValue kSin45 = TrigonometrySine(4500);

        const SWaveformAmplitudeTestData kTestData[] = {
            {.inputPhase = 0,       .expectedWaveformAmplitude = 0},
            {.inputPhase = 4500,    .expectedWaveformAmplitude = kSin45},
            {.inputPhase = 9000,    .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 13500,   .expectedWaveformAmplitude = kSin45},
            {.inputPhase = 18000,   .expectedWaveformAmplitude = 0},
            {.inputPhase = 22500,   .expectedWaveformAmplitude = -kSin45},
            {.inputPhase = 27000,   .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 31500,   .expectedWaveformAmplitude = -kSin45},
            {.inputPhase = 36000,   .expectedWaveformAmplitude = 0}
        };

        SineWaveEffect effect;

        for (const auto kTest : kTestData)
        {
            const TEffectValue kActualWaveformAmplitude = effect.WaveformAmplitude(kTest.inputPhase);
            TEST_ASSERT(true == TEffectValueEqual(kActualWaveformAmplitude, kTest.expectedWaveformAmplitude));
        }
    }

    // Verifies correct waveform amplitude computations for various points in the waveform cycle.
    // This test case is for square wave effects.
    TEST_CASE(PeriodicEffect_WaveformAmplitude_SquareWave)
    {
        constexpr SWaveformAmplitudeTestData kTestData[] = {
            {.inputPhase = 0,       .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 4500,    .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 9000,    .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 13500,   .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 18000,   .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 22500,   .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 27000,   .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 31500,   .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 35999,   .expectedWaveformAmplitude = -1.0},
        };

        SquareWaveEffect effect;

        for (const auto kTest : kTestData)
        {
            const TEffectValue kActualWaveformAmplitude = effect.WaveformAmplitude(kTest.inputPhase);
            TEST_ASSERT(true == TEffectValueEqual(kActualWaveformAmplitude, kTest.expectedWaveformAmplitude));
        }
    }

    // Verifies correct waveform amplitude computations for various points in the waveform cycle.
    // This test case is for triangle wave effects.
    TEST_CASE(PeriodicEffect_WaveformAmplitude_TriangleWave)
    {
        constexpr SWaveformAmplitudeTestData kTestData[] = {
            {.inputPhase = 0,       .expectedWaveformAmplitude = 1.0},
            {.inputPhase = 4500,    .expectedWaveformAmplitude = 0.5},
            {.inputPhase = 9000,    .expectedWaveformAmplitude = 0.0},
            {.inputPhase = 13500,   .expectedWaveformAmplitude = -0.5},
            {.inputPhase = 18000,   .expectedWaveformAmplitude = -1.0},
            {.inputPhase = 22500,   .expectedWaveformAmplitude = -0.5},
            {.inputPhase = 27000,   .expectedWaveformAmplitude = 0.0},
            {.inputPhase = 31500,   .expectedWaveformAmplitude = 0.5},
            {.inputPhase = 36000,   .expectedWaveformAmplitude = 1.0},
        };

        TriangleWaveEffect effect;

        for (const auto kTest : kTestData)
        {
            const TEffectValue kActualWaveformAmplitude = effect.WaveformAmplitude(kTest.inputPhase);
            TEST_ASSERT(true == TEffectValueEqual(kActualWaveformAmplitude, kTest.expectedWaveformAmplitude));
        }
    }
}
