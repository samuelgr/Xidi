/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ConstantForceEffectTest.cpp
 *   Unit tests for force feedback effects that produce a force of constant magnitude.
 **************************************************************************************************/

#include "TestCase.h"

#include "ForceFeedbackEffect.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller::ForceFeedback;

  /// Common duration value used throughout test cases.
  static constexpr TEffectTimeMs kTestEffectDuration = 1000;

  /// Common gain value used throughout test cases.
  static constexpr TEffectValue kTestEffectGain = 1000;

  // Creates constant force effects of varying magnitudes and ensures they report the correct
  // magnitude. No other parameters are set.
  TEST_CASE(ConstantForceEffect_ComputeMagnitude_Nominal)
  {
    constexpr TEffectValue kTestMagnitudes[] = {
        kEffectForceMagnitudeMinimum, kEffectForceMagnitudeZero, kEffectForceMagnitudeMaximum};

    for (const auto testMagnitude : kTestMagnitudes)
    {
      ConstantForceEffect effect;
      effect.InitializeDefaultAssociatedAxes();
      effect.InitializeDefaultDirection();
      effect.SetDuration(kTestEffectDuration);
      effect.SetTypeSpecificParameters({.magnitude = testMagnitude});

      for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
        TEST_ASSERT(testMagnitude == effect.ComputeMagnitude(t));
    }
  }

  // Creates constant force effects of varying magnitudes and ensures they report the correct
  // magnitude. Additionally specifies a gain.
  TEST_CASE(ConstantForceEffect_ComputeMagnitude_Gain)
  {
    constexpr TEffectValue kTestMagnitudes[] = {
        kEffectForceMagnitudeMinimum, kEffectForceMagnitudeZero, kEffectForceMagnitudeMaximum};

    for (const auto testMagnitude : kTestMagnitudes)
    {
      ConstantForceEffect effect;
      effect.InitializeDefaultAssociatedAxes();
      effect.InitializeDefaultDirection();
      effect.SetDuration(kTestEffectDuration);
      effect.SetTypeSpecificParameters({.magnitude = testMagnitude});

      effect.SetGain(kTestEffectGain);

      for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
        TEST_ASSERT(
            (testMagnitude * kTestEffectGain / kEffectModifierRelativeDenominator) ==
            effect.ComputeMagnitude(t));
    }
  }

  // Creates a constant force effect with a positive magnitude and applies an envelope
  // transformation. No other properties are specified.
  TEST_CASE(ConstantForceEffect_ComputeMagnitude_EnvelopePositive)
  {
    constexpr TEffectValue kTestMagnitude = 5000;

    ConstantForceEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

    constexpr SEnvelope kTestEnvelope = {
        .attackTime = kTestEffectDuration / 10,
        .attackLevel = 7000,
        .fadeTime = kTestEffectDuration / 5,
        .fadeLevel = 1000};
    effect.SetEnvelope(kTestEnvelope);
    TEST_ASSERT(true == effect.GetEnvelope().has_value());
    TEST_ASSERT(kTestEnvelope == effect.GetEnvelope().value());

    constexpr TEffectValue kAttackSlope =
        (kTestMagnitude - kTestEnvelope.attackLevel) / kTestEnvelope.attackTime;
    constexpr TEffectValue kFadeSlope =
        (kTestEnvelope.fadeLevel - kTestMagnitude) / kTestEnvelope.fadeTime;
    constexpr TEffectTimeMs kAttackStartTime = 0;
    constexpr TEffectTimeMs kAttackEndTime = kTestEnvelope.attackTime;
    constexpr TEffectTimeMs kFadeStartTime = kTestEffectDuration - kTestEnvelope.fadeTime;
    constexpr TEffectTimeMs kFadeEndTime = kTestEffectDuration;

    for (TEffectTimeMs t = kAttackStartTime; t < kAttackEndTime; ++t)
      TEST_ASSERT(
          (kTestEnvelope.attackLevel + ((t - kAttackStartTime) * kAttackSlope)) ==
          effect.ComputeMagnitude(t));

    for (TEffectTimeMs t = kAttackEndTime; t < kFadeStartTime; ++t)
      TEST_ASSERT(kTestMagnitude == effect.ComputeMagnitude(t));

    for (TEffectTimeMs t = kFadeStartTime; t < kFadeEndTime; ++t)
      TEST_ASSERT(
          (kTestMagnitude + ((t - kFadeStartTime) * kFadeSlope)) == effect.ComputeMagnitude(t));
  }

  // Creates a constant force effect with a negative magnitude and applies an envelope
  // transformation. This test is a bit tricky and requires sign manipulation because the envelope
  // is expected to adjust amplitude (i.e. distance from 0) in the case of a constant force. No
  // other properties are specified.
  TEST_CASE(ConstantForceEffect_ComputeMagnitude_EnvelopeNegative)
  {
    constexpr TEffectValue kTestMagnitude = -5000;

    ConstantForceEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetTypeSpecificParameters({.magnitude = kTestMagnitude});

    constexpr SEnvelope kTestEnvelope = {
        .attackTime = kTestEffectDuration / 10,
        .attackLevel = 7000,
        .fadeTime = kTestEffectDuration / 5,
        .fadeLevel = 1000};
    effect.SetEnvelope(kTestEnvelope);
    TEST_ASSERT(true == effect.GetEnvelope().has_value());
    TEST_ASSERT(kTestEnvelope == effect.GetEnvelope().value());

    constexpr TEffectValue kAttackSlope =
        (kTestMagnitude + kTestEnvelope.attackLevel) / kTestEnvelope.attackTime;
    constexpr TEffectValue kFadeSlope =
        (-kTestEnvelope.fadeLevel - kTestMagnitude) / kTestEnvelope.fadeTime;
    constexpr TEffectTimeMs kAttackStartTime = 0;
    constexpr TEffectTimeMs kAttackEndTime = kTestEnvelope.attackTime;
    constexpr TEffectTimeMs kFadeStartTime = kTestEffectDuration - kTestEnvelope.fadeTime;
    constexpr TEffectTimeMs kFadeEndTime = kTestEffectDuration;

    for (TEffectTimeMs t = kAttackStartTime; t < kAttackEndTime; ++t)
      TEST_ASSERT(
          (-kTestEnvelope.attackLevel + ((t - kAttackStartTime) * kAttackSlope)) ==
          effect.ComputeMagnitude(t));

    for (TEffectTimeMs t = kAttackEndTime; t < kFadeStartTime; ++t)
      TEST_ASSERT(kTestMagnitude == effect.ComputeMagnitude(t));

    for (TEffectTimeMs t = kFadeStartTime; t < kFadeEndTime; ++t)
      TEST_ASSERT(
          (kTestMagnitude + ((t - kFadeStartTime) * kFadeSlope)) == effect.ComputeMagnitude(t));
  }

  // Verifies that out-of-bounds magnitudes are accepted and saturated at the extreme ends of the
  // supported range.
  TEST_CASE(ConstantForceEffect_SetMagnitude_CheckAndFixTypeSpecificParameters)
  {
    constexpr TEffectValue kInputMagnitudes[] = {
        (3 * kEffectForceMagnitudeMinimum),
        (2 * kEffectForceMagnitudeMinimum),
        (kEffectForceMagnitudeMinimum - 1),
        (kEffectForceMagnitudeMaximum + 1),
        (2 * kEffectForceMagnitudeMaximum),
        (3 * kEffectForceMagnitudeMaximum)};
    constexpr TEffectValue kExpectedMagnitudes[] = {
        kEffectForceMagnitudeMinimum,
        kEffectForceMagnitudeMinimum,
        kEffectForceMagnitudeMinimum,
        kEffectForceMagnitudeMaximum,
        kEffectForceMagnitudeMaximum,
        kEffectForceMagnitudeMaximum};
    static_assert(
        _countof(kInputMagnitudes) == _countof(kExpectedMagnitudes),
        "Input and expected output magnitudes must have the same count.");

    for (int i = 0; i < _countof(kInputMagnitudes); ++i)
    {
      ConstantForceEffect effect;
      TEST_ASSERT(true == effect.SetTypeSpecificParameters({.magnitude = kInputMagnitudes[i]}));

      const auto maybeActualMagnitude = effect.GetTypeSpecificParameters();
      TEST_ASSERT(true == maybeActualMagnitude.has_value());

      const TEffectValue actualMagnitude = maybeActualMagnitude.value().magnitude;
      TEST_ASSERT(actualMagnitude == kExpectedMagnitudes[i]);
    }
  }
} // namespace XidiTest
