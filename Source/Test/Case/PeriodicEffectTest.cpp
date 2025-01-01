/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file PeriodicEffectTest.cpp
 *   Unit tests for force feedback effects that produce a force that follows a periodic waveform.
 **************************************************************************************************/

#include <Infra/Test/TestCase.h>

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackMath.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "MockForceFeedbackEffect.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller::ForceFeedback;

  /// Common amplitude value used throughout test cases.
  static constexpr TEffectValue kTestEffectAmplitude = 5000;

  /// Test data record for waveform amplitude tests.
  struct SWaveformAmplitudeTestData
  {
    /// Input provided to the effect object, expressed as a phase in degree hundredths.
    TEffectValue inputPhase;

    /// Expected output from the waveform amplitude method.
    TEffectValue expectedWaveformAmplitude;
  };

  /// Checks if two force feedback effect values are effectively equal, subject to very minor
  /// imprecision.
  /// @param [in] valueA First of the two values to compare.
  /// @param [in] valueB Second of the two values to compare.
  /// @return `true` if the two values are essentially considered equivalent, `false` otherwise.
  static bool TEffectValueEqual(TEffectValue valueA, TEffectValue valueB)
  {
    return ((std::max(valueA, valueB) - std::min(valueA, valueB)) < 0.00001);
  }

  // Verifies that a periodic effect can correctly compute its phase for various raw time inputs.
  // This is the nominal case in which no phase offset is present.
  TEST_CASE(PeriodicEffect_ComputePhase_Nominal)
  {
    MockPeriodicEffect effect;
    TEST_ASSERT(
        true ==
        effect.SetTypeSpecificParameters(
            {.period = MockPeriodicEffect::kDegreeHundredthsPerCycle}));

    for (TEffectTimeMs t = 0; t <= (10 * MockPeriodicEffect::kDegreeHundredthsPerCycle); t += 100)
    {
      const TEffectValue expectedPhase =
          (TEffectValue)(t % MockPeriodicEffect::kDegreeHundredthsPerCycle);
      const TEffectValue actualPhase = (TEffectValue)effect.ComputePhase(t);

      TEST_ASSERT(actualPhase == expectedPhase);
    }
  }

  // Verifies that a periodic effect can correctly compute its phase for various raw time inputs.
  // Various different phase offset values are tried.
  TEST_CASE(PeriodicEffect_ComputePhase_PhaseOffset)
  {
    constexpr unsigned int kTestPhaseOffsets[] = {
        MockPeriodicEffect::kDegreeHundredthsPerCycle / 4,
        MockPeriodicEffect::kDegreeHundredthsPerCycle / 2,
        (MockPeriodicEffect::kDegreeHundredthsPerCycle * 3) / 4};

    MockPeriodicEffect effect;

    for (const auto testPhaseOffset : kTestPhaseOffsets)
    {
      TEST_ASSERT(
          true ==
          effect.SetTypeSpecificParameters(
              {.phase = (TEffectValue)testPhaseOffset,
               .period = MockPeriodicEffect::kDegreeHundredthsPerCycle}));

      for (TEffectTimeMs t = 0; t <= MockPeriodicEffect::kDegreeHundredthsPerCycle; t += 100)
      {
        const TEffectValue expectedPhase =
            (TEffectValue)((t + testPhaseOffset) % MockPeriodicEffect::kDegreeHundredthsPerCycle);
        const TEffectValue actualPhase = (TEffectValue)effect.ComputePhase(t);

        TEST_ASSERT(actualPhase == expectedPhase);
      }
    }
  }

  // Verifies that a periodic effect correctly applies a waveform offset when computing its
  // magnitude.
  TEST_CASE(PeriodicEffect_ComputeMagnitude_WithOffset)
  {
    constexpr TEffectTimeMs kTestEffectDuration = MockPeriodicEffect::kDegreeHundredthsPerCycle;
    constexpr TEffectTimeMs kTestEffectEvaluationTime = kTestEffectDuration / 2;
    constexpr TEffectValue kTestEffectOffsets[] = {-1000, -100, 100, 1000};

    MockPeriodicEffect effect;
    TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));

    for (const auto testEffectOffset : kTestEffectOffsets)
    {
      TEST_ASSERT(
          true ==
          effect.SetTypeSpecificParameters(
              {.amplitude = kTestEffectAmplitude,
               .offset = testEffectOffset,
               .period = kTestEffectDuration}));

      const TEffectValue expectedMagnitude = testEffectOffset +
          (kTestEffectAmplitude * effect.WaveformAmplitude(kTestEffectEvaluationTime));
      const TEffectValue actualMagnitude = effect.ComputeMagnitude(kTestEffectEvaluationTime);
      TEST_ASSERT(actualMagnitude == expectedMagnitude);
    }
  }

  // Verifies that a periodic effect correctly applies a waveform offset and envelope when computing
  // its magnitude.
  TEST_CASE(PeriodicEffect_ComputeMagnitude_WithOffsetAndEnvelope)
  {
    constexpr TEffectTimeMs kTestEffectDuration = MockPeriodicEffect::kDegreeHundredthsPerCycle;
    constexpr TEffectTimeMs kTestEffectEvaluationTime = kTestEffectDuration / 2;
    constexpr SEnvelope kTestEffectEnvelope = {
        .attackTime = MockPeriodicEffect::kDegreeHundredthsPerCycle, .attackLevel = 0};
    constexpr TEffectValue kTestEffectOffsets[] = {-1000, -100, 100, 1000};

    // The envelope defined above has the effect of cutting the amplitude of the effect
    // proportionally to its progress through its duration.
    constexpr TEffectValue kTestEffectEnvelopeMultiplier =
        ((TEffectValue)kTestEffectEvaluationTime / (TEffectValue)kTestEffectDuration);

    MockPeriodicEffect effect;
    TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
    TEST_ASSERT(true == effect.SetEnvelope(kTestEffectEnvelope));

    for (const auto testEffectOffset : kTestEffectOffsets)
    {
      TEST_ASSERT(
          true ==
          effect.SetTypeSpecificParameters(
              {.amplitude = kTestEffectAmplitude,
               .offset = testEffectOffset,
               .period = kTestEffectDuration}));

      const TEffectValue expectedMagnitude = testEffectOffset +
          (kTestEffectAmplitude * kTestEffectEnvelopeMultiplier *
           effect.WaveformAmplitude(kTestEffectEvaluationTime));
      const TEffectValue actualMagnitude = effect.ComputeMagnitude(kTestEffectEvaluationTime);
      TEST_ASSERT(actualMagnitude == expectedMagnitude);
    }
  }

  // Verifies correct waveform amplitude computations for various points in the waveform cycle.
  // This test case is for sawtooth down effects.
  TEST_CASE(PeriodicEffect_WaveformAmplitude_SawtoothDown)
  {
    constexpr SWaveformAmplitudeTestData kTestData[] = {
        {.inputPhase = 0, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 4500, .expectedWaveformAmplitude = 0.75},
        {.inputPhase = 9000, .expectedWaveformAmplitude = 0.5},
        {.inputPhase = 13500, .expectedWaveformAmplitude = 0.25},
        {.inputPhase = 18000, .expectedWaveformAmplitude = 0.0},
        {.inputPhase = 22500, .expectedWaveformAmplitude = -0.25},
        {.inputPhase = 27000, .expectedWaveformAmplitude = -0.5},
        {.inputPhase = 31500, .expectedWaveformAmplitude = -0.75},
        {.inputPhase = 36000, .expectedWaveformAmplitude = -1.0}};

    SawtoothDownEffect effect;

    for (const auto testData : kTestData)
    {
      const TEffectValue actualWaveformAmplitude = effect.WaveformAmplitude(testData.inputPhase);
      TEST_ASSERT(
          true == TEffectValueEqual(actualWaveformAmplitude, testData.expectedWaveformAmplitude));
    }
  }

  // Verifies correct waveform amplitude computations for various points in the waveform cycle.
  // This test case is for sawtooth up effects.
  TEST_CASE(PeriodicEffect_WaveformAmplitude_SawtoothUp)
  {
    constexpr SWaveformAmplitudeTestData kTestData[] = {
        {.inputPhase = 0, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 4500, .expectedWaveformAmplitude = -0.75},
        {.inputPhase = 9000, .expectedWaveformAmplitude = -0.5},
        {.inputPhase = 13500, .expectedWaveformAmplitude = -0.25},
        {.inputPhase = 18000, .expectedWaveformAmplitude = 0.0},
        {.inputPhase = 22500, .expectedWaveformAmplitude = 0.25},
        {.inputPhase = 27000, .expectedWaveformAmplitude = 0.5},
        {.inputPhase = 31500, .expectedWaveformAmplitude = 0.75},
        {.inputPhase = 36000, .expectedWaveformAmplitude = 1.0}};

    SawtoothUpEffect effect;

    for (const auto testData : kTestData)
    {
      const TEffectValue actualWaveformAmplitude = effect.WaveformAmplitude(testData.inputPhase);
      TEST_ASSERT(
          true == TEffectValueEqual(actualWaveformAmplitude, testData.expectedWaveformAmplitude));
    }
  }

  // Verifies correct waveform amplitude computations for various points in the waveform cycle.
  // This test case is for sine wave effects.
  TEST_CASE(PeriodicEffect_WaveformAmplitude_SineWave)
  {
    const TEffectValue kSin45 = TrigonometrySine(4500);

    const SWaveformAmplitudeTestData kTestData[] = {
        {.inputPhase = 0, .expectedWaveformAmplitude = 0},
        {.inputPhase = 4500, .expectedWaveformAmplitude = kSin45},
        {.inputPhase = 9000, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 13500, .expectedWaveformAmplitude = kSin45},
        {.inputPhase = 18000, .expectedWaveformAmplitude = 0},
        {.inputPhase = 22500, .expectedWaveformAmplitude = -kSin45},
        {.inputPhase = 27000, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 31500, .expectedWaveformAmplitude = -kSin45},
        {.inputPhase = 36000, .expectedWaveformAmplitude = 0}};

    SineWaveEffect effect;

    for (const auto testData : kTestData)
    {
      const TEffectValue actualWaveformAmplitude = effect.WaveformAmplitude(testData.inputPhase);
      TEST_ASSERT(
          true == TEffectValueEqual(actualWaveformAmplitude, testData.expectedWaveformAmplitude));
    }
  }

  // Verifies correct waveform amplitude computations for various points in the waveform cycle.
  // This test case is for square wave effects.
  TEST_CASE(PeriodicEffect_WaveformAmplitude_SquareWave)
  {
    constexpr SWaveformAmplitudeTestData kTestData[] = {
        {.inputPhase = 0, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 4500, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 9000, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 13500, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 18000, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 22500, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 27000, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 31500, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 35999, .expectedWaveformAmplitude = -1.0},
    };

    SquareWaveEffect effect;

    for (const auto testData : kTestData)
    {
      const TEffectValue actualWaveformAmplitude = effect.WaveformAmplitude(testData.inputPhase);
      TEST_ASSERT(
          true == TEffectValueEqual(actualWaveformAmplitude, testData.expectedWaveformAmplitude));
    }
  }

  // Verifies correct waveform amplitude computations for various points in the waveform cycle.
  // This test case is for triangle wave effects.
  TEST_CASE(PeriodicEffect_WaveformAmplitude_TriangleWave)
  {
    constexpr SWaveformAmplitudeTestData kTestData[] = {
        {.inputPhase = 0, .expectedWaveformAmplitude = 1.0},
        {.inputPhase = 4500, .expectedWaveformAmplitude = 0.5},
        {.inputPhase = 9000, .expectedWaveformAmplitude = 0.0},
        {.inputPhase = 13500, .expectedWaveformAmplitude = -0.5},
        {.inputPhase = 18000, .expectedWaveformAmplitude = -1.0},
        {.inputPhase = 22500, .expectedWaveformAmplitude = -0.5},
        {.inputPhase = 27000, .expectedWaveformAmplitude = 0.0},
        {.inputPhase = 31500, .expectedWaveformAmplitude = 0.5},
        {.inputPhase = 36000, .expectedWaveformAmplitude = 1.0},
    };

    TriangleWaveEffect effect;

    for (const auto testData : kTestData)
    {
      const TEffectValue actualWaveformAmplitude = effect.WaveformAmplitude(testData.inputPhase);
      TEST_ASSERT(
          true == TEffectValueEqual(actualWaveformAmplitude, testData.expectedWaveformAmplitude));
    }
  }
} // namespace XidiTest
