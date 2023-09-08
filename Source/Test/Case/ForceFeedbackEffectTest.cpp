/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file ForceFeedbackEffectTest.cpp
 *   Unit tests for functionality common to all force feedback effects.
 **************************************************************************************************/

#include "TestCase.h"

#include "ForceFeedbackEffect.h"

#include "ControllerTypes.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "MockForceFeedbackEffect.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller::ForceFeedback;
  using ::Xidi::Controller::EAxis;

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
  static constexpr SEnvelope kTestTrivialEnvelope = {
      .attackTime = 0, .attackLevel = 1000, .fadeTime = 0, .fadeLevel = 2500};

  // Creates a simple test effect with no properties other than duration.
  // Verifies that it returns the correct computed magnitude at all times throughout its duration.
  TEST_CASE(ForceFeedbackEffect_NominalEffect_Magnitude)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);

    for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
      TEST_ASSERT((TEffectValue)t == effect.ComputeMagnitude(t));
  }

  // Creates a simple test effect with no properties other than duration.
  // Verifies that it returns the correct values for all of its common properties.
  TEST_CASE(ForceFeedbackEffect_NominalEffect_Parameters)
  {
    MockEffect effect;
    TEST_ASSERT(false == effect.HasDirection());
    TEST_ASSERT(false == effect.HasDuration());

    effect.InitializeDefaultAssociatedAxes();
    TEST_ASSERT(true == effect.HasAssociatedAxes());
    TEST_ASSERT(false == effect.HasCompleteDirection());

    effect.InitializeDefaultDirection();
    TEST_ASSERT(true == effect.HasDirection());
    TEST_ASSERT(true == effect.HasCompleteDirection());

    effect.SetDuration(kTestEffectDuration);
    TEST_ASSERT(true == effect.HasDuration());
    TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());

    TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
    TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
    TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
    TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
  }

  // Creates a test effect with a start delay.
  // Verifies that it returns the correct computed magnitude at all times throughout its duration.
  // Start delay handling is not implemented by the effect itself and therefore should not affect
  // the output magnitude it produces.
  TEST_CASE(ForceFeedbackEffect_EffectWithStartDelay_Magnitude)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetStartDelay(kTestEffectStartDelay);

    for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
      TEST_ASSERT(t == effect.ComputeMagnitude(t));
  }

  // Creates a test effect with a start delay.
  // Verifies that it returns the correct values for all of its common properties.
  TEST_CASE(ForceFeedbackEffect_EffectWithStartDelay_Parameters)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetStartDelay(kTestEffectStartDelay);

    TEST_ASSERT(true == effect.HasDuration());
    TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());

    TEST_ASSERT(kTestEffectStartDelay == effect.GetStartDelay());
    TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
    TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
    TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
  }

  // Creates a test effect with a sample period.
  // Verifies that it returns the correct computed magnitude at all times throughout its duration.
  TEST_CASE(ForceFeedbackEffect_EffectWithSamplePeriod_Magnitude)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetSamplePeriod(kTestEffectSamplePeriod);

    for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
    {
      // Time input, and hence magnitude output, should only increase in multiples of the specified
      // sample period.
      const TEffectValue expectedMagnitude =
          (TEffectValue)(t / kTestEffectSamplePeriod) * kTestEffectSamplePeriod;
      const TEffectValue actualMagnitude = effect.ComputeMagnitude(t);

      TEST_ASSERT(actualMagnitude == expectedMagnitude);
    }
  }

  // Creates a test effect with a sample period.
  // Verifies that it returns the correct values for all of its common properties.
  TEST_CASE(ForceFeedbackEffect_EffectWithSamplePeriod_Parameters)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetSamplePeriod(kTestEffectSamplePeriod);

    TEST_ASSERT(true == effect.HasDuration());
    TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());

    TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
    TEST_ASSERT(kTestEffectSamplePeriod == effect.GetSamplePeriod());
    TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
    TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
  }

  // Creates a test effect with a gain.
  // Verifies that it returns the correct computed magnitude at all times throughout its duration.
  TEST_CASE(ForceFeedbackEffect_EffectWithGain_Magnitude)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetGain(kTestEffectGain);

    for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
      TEST_ASSERT(
          (TEffectValue)t * (kTestEffectGain / kEffectModifierRelativeDenominator) ==
          effect.ComputeMagnitude(t));
  }

  // Creates a test effect with a gain.
  // Verifies that it returns the correct values for all of its common properties.
  TEST_CASE(ForceFeedbackEffect_EffectWithGain_Parameters)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetGain(kTestEffectGain);

    TEST_ASSERT(true == effect.HasDuration());
    TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());

    TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
    TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
    TEST_ASSERT(kTestEffectGain == effect.GetGain());
    TEST_ASSERT(SCommonParameters::kDefaultEnvelope == effect.GetEnvelope());
  }

  // Creates a test effect with a trivial envelope that has no effect.
  // Verifies that it returns the correct computed magnitude at all times throughout its duration.
  TEST_CASE(ForceFeedbackEffect_EffectWithTrivialEnvelope_Magnitude)
  {
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
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
    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetEnvelope(kTestTrivialEnvelope);

    TEST_ASSERT(true == effect.HasDuration());
    TEST_ASSERT(kTestEffectDuration == effect.GetDuration().value());

    TEST_ASSERT(SCommonParameters::kDefaultStartDelay == effect.GetStartDelay());
    TEST_ASSERT(SCommonParameters::kDefaultSamplePeriod == effect.GetSamplePeriod());
    TEST_ASSERT(SCommonParameters::kDefaultGain == effect.GetGain());
    TEST_ASSERT(kTestTrivialEnvelope == effect.GetEnvelope());
  }

  // Submits a constant sustain level and uses an envelope to turn it into a linear function that
  // increases with time.
  TEST_CASE(ForceFeedbackEffect_ApplyEnvelope_LinearIncrease)
  {
    // Pattern of increase is equivalent to the simple effect tests in which the result of applying
    // the envelope is simply equal to the input time. Both attack and fade times are half the
    // duration, so the entire effect is defined by the envelope. Sustain level is equal to half the
    // duration. Attack region starts at 0 and goes up to the sustain level in half the duration,
    // and the fade region starts at the sustain level and continues up to the entire duration worth
    // of magnitude.
    constexpr SEnvelope kTestEnvelope = {
        .attackTime = kTestEffectDuration / 2,
        .attackLevel = kEffectForceMagnitudeZero,
        .fadeTime = kTestEffectDuration / 2,
        .fadeLevel = kTestEffectDuration};
    constexpr TEffectValue kSustainLevel = kTestEffectDuration / 2;

    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetEnvelope(kTestEnvelope);

    for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
      TEST_ASSERT((TEffectValue)t == effect.ApplyEnvelope(t, kSustainLevel));
  }

  // Submits a constant sustain level and uses an envelope to turn it into a linear function that
  // decreases with time.
  TEST_CASE(ForceFeedbackEffect_ApplyEnvelope_LinearDecrease)
  {
    // Pattern is exactly as in the linear increase case but inverted.
    // Expected result of applying the envelope is to start at the duration and descend down to 0.
    constexpr SEnvelope kTestEnvelope = {
        .attackTime = kTestEffectDuration / 2,
        .attackLevel = kTestEffectDuration,
        .fadeTime = kTestEffectDuration / 2,
        .fadeLevel = kEffectForceMagnitudeZero};
    constexpr TEffectValue kSustainLevel = kTestEffectDuration / 2;

    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetEnvelope(kTestEnvelope);

    for (TEffectTimeMs t = 0; t < kTestEffectDuration; ++t)
      TEST_ASSERT(
          (TEffectValue)(kTestEffectDuration - t) == effect.ApplyEnvelope(t, kSustainLevel));
  }

  // Submits a constant sustain level and uses an envelope to turn it into a piece-wise function
  // with three pieces: linear increase, constant sustain, and linear decrease.
  TEST_CASE(ForceFeedbackEffect_ApplyEnvelope_PiecewiseUpThenDown)
  {
    constexpr SEnvelope kTestEnvelope = {
        .attackTime = kTestEffectDuration / 4,
        .attackLevel = kEffectForceMagnitudeZero,
        .fadeTime = kTestEffectDuration / 4,
        .fadeLevel = kEffectForceMagnitudeZero};
    constexpr TEffectValue kSustainLevel = kTestEffectDuration;

    MockEffect effect;
    effect.InitializeDefaultAssociatedAxes();
    effect.InitializeDefaultDirection();
    effect.SetDuration(kTestEffectDuration);
    effect.SetEnvelope(kTestEnvelope);

    // First region takes one quarter of the total duration.
    // Attack region of the envelope defines a slope from 0 to the sustain level.
    for (TEffectTimeMs t = 0; t < (kTestEffectDuration / 4); ++t)
      TEST_ASSERT((TEffectValue)t * 4 == effect.ApplyEnvelope(t, kSustainLevel));

    // Second region takes one half of the total duration, from 1/4 to 3/4 of the total effect
    // duration. Expected output is just the sustain level as passed in.
    for (TEffectTimeMs t = (kTestEffectDuration / 4); t < ((kTestEffectDuration * 3) / 4); ++t)
      TEST_ASSERT((TEffectValue)kSustainLevel == effect.ApplyEnvelope(t, kSustainLevel));

    // Third region takes one quarter of the total duration.
    // Fade region of the envelope defines a slope from the sustain level to 0.
    for (TEffectTimeMs t = ((kTestEffectDuration * 3) / 4); t < kTestEffectDuration; ++t)
      TEST_ASSERT(
          (TEffectValue)(kTestEffectDuration - t) * 4 == effect.ApplyEnvelope(t, kSustainLevel));
  }

  // Creates an effect and submits invalid parameters. Verifies that they are all rejected.
  TEST_CASE(ForceFeedbackEffect_InvalidParameters)
  {
    MockEffect effect;

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

  // Creates an effect and verifies that it reports correct information for whether or not it is
  // completely defined. Only a duration is required. All other parameters are optional.
  TEST_CASE(ForceFeedbackEffect_IsCompletelyDefined)
  {
    MockEffect effect;

    TEST_ASSERT(false == effect.IsCompletelyDefined());
    TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
    TEST_ASSERT(false == effect.IsCompletelyDefined());
    TEST_ASSERT(true == effect.InitializeDefaultAssociatedAxes());
    TEST_ASSERT(false == effect.IsCompletelyDefined());
    TEST_ASSERT(true == effect.InitializeDefaultDirection());
    TEST_ASSERT(true == effect.IsCompletelyDefined());
  }

  // Creates an effect with direction and associated axes using one axis.
  // Verifies that it can correctly convert a raw (unordered) magnitude component vector into a
  // globally-understood (ordered) magnitude component vector.
  TEST_CASE(ForceFeedbackEffect_OrderMagnitudeComponents)
  {
    constexpr EAxis kTestAxes[] = {
        EAxis::X, EAxis::Y, EAxis::Z, EAxis::RotX, EAxis::RotY, EAxis::RotZ};

    constexpr TMagnitudeComponents kTestMagnitudeComponents = {55, 66, 77, 88};
    constexpr auto kExpectedComponentValue = kTestMagnitudeComponents[0];

    for (const auto testAxis : kTestAxes)
    {
      MockEffect effect;

      const TEffectValue cartesianCoordinates[] = {1};
      TEST_ASSERT(
          true ==
          effect.Direction().SetDirectionUsingCartesian(
              cartesianCoordinates, _countof(cartesianCoordinates)));

      const SAssociatedAxes associatedAxes = {.count = 1, .type = {testAxis}};
      TEST_ASSERT(true == effect.SetAssociatedAxes(associatedAxes));

      TOrderedMagnitudeComponents expectedOrderedMagnitudeComponents = {};
      expectedOrderedMagnitudeComponents[(int)testAxis] = kExpectedComponentValue;

      TOrderedMagnitudeComponents actualOrderedMagnitudeComponents =
          effect.OrderMagnitudeComponents(kTestMagnitudeComponents);
      TEST_ASSERT(actualOrderedMagnitudeComponents == expectedOrderedMagnitudeComponents);
    }
  }

  // Verifies that a cloned effect is equivalent to its origin effect.
  TEST_CASE(ForceFeedbackEffect_Clone)
  {
    MockEffect effect;
    TEST_ASSERT(true == effect.SetAssociatedAxes({.count = 2, .type = {EAxis::Z, EAxis::RotZ}}));
    TEST_ASSERT(true == effect.SetDuration(123));
    TEST_ASSERT(true == effect.SetStartDelay(456));
    TEST_ASSERT(true == effect.SetSamplePeriod(7890));
    TEST_ASSERT(true == effect.SetGain(5566));
    TEST_ASSERT(
        true ==
        effect.SetEnvelope({.attackTime = 1, .attackLevel = 2, .fadeTime = 3, .fadeLevel = 4}));

    std::unique_ptr<Effect> clonedEffect = effect.Clone();
    TEST_ASSERT(clonedEffect->Identifier() == effect.Identifier());
    TEST_ASSERT(clonedEffect->CommonParameters() == effect.CommonParameters());
  }

  // Verifies that two effect objects with the same identifier can successfully complete a parameter
  // synchronization operation.
  TEST_CASE(ForceFeedbackEffect_SyncParameters_SameIdentifier)
  {
    MockEffect effect;
    std::unique_ptr<Effect> clonedEffect = effect.Clone();

    TEST_ASSERT(true == effect.SetAssociatedAxes({.count = 2, .type = {EAxis::Z, EAxis::RotZ}}));
    TEST_ASSERT(true == effect.SetDuration(123));
    TEST_ASSERT(true == effect.SetStartDelay(456));
    TEST_ASSERT(true == effect.SetSamplePeriod(7890));
    TEST_ASSERT(true == effect.SetGain(5566));
    TEST_ASSERT(
        true ==
        effect.SetEnvelope({.attackTime = 1, .attackLevel = 2, .fadeTime = 3, .fadeLevel = 4}));

    TEST_ASSERT(true == clonedEffect->SyncParametersFrom(effect));
    TEST_ASSERT(clonedEffect->Identifier() == effect.Identifier());
    TEST_ASSERT(clonedEffect->CommonParameters() == effect.CommonParameters());
  }

  // Verifies that two effect objects with different identifiers will not complete a parameter
  // synchronization operation. The failed synchronization operation should result in no changes to
  // the attempted destination effect's parameters.
  TEST_CASE(ForceFeedbackEffect_SyncParameters_DifferentIdentifier)
  {
    MockEffect effect;
    std::unique_ptr<Effect> clonedEffect = effect.Clone();

    MockEffect effect2;
    TEST_ASSERT(true == effect2.SetAssociatedAxes({.count = 2, .type = {EAxis::Z, EAxis::RotZ}}));
    TEST_ASSERT(true == effect2.SetDuration(123));
    TEST_ASSERT(true == effect2.SetStartDelay(456));
    TEST_ASSERT(true == effect2.SetSamplePeriod(7890));
    TEST_ASSERT(true == effect2.SetGain(5566));
    TEST_ASSERT(
        true ==
        effect2.SetEnvelope({.attackTime = 1, .attackLevel = 2, .fadeTime = 3, .fadeLevel = 4}));

    TEST_ASSERT(false == effect.SyncParametersFrom(effect2));
    TEST_ASSERT(clonedEffect->Identifier() == effect.Identifier());
    TEST_ASSERT(clonedEffect->CommonParameters() == effect.CommonParameters());
  }

  // Creates a force effect with type-specific parameters and verifies that it reports correct
  // information for whether or not it is completely defined. Duration and type-specific parameters
  // are required. All others are optional.
  TEST_CASE(ForceFeedbackEffect_TypeSpecificParameters_IsCompletelyDefined)
  {
    MockEffectWithTypeSpecificParameters effect;

    TEST_ASSERT(false == effect.IsCompletelyDefined());
    TEST_ASSERT(true == effect.SetDuration(kTestEffectDuration));
    TEST_ASSERT(true == effect.InitializeDefaultDirection());
    TEST_ASSERT(false == effect.IsCompletelyDefined());
    TEST_ASSERT(true == effect.SetTypeSpecificParameters({.valid = true}));
    TEST_ASSERT(false == effect.IsCompletelyDefined());
    TEST_ASSERT(true == effect.InitializeDefaultAssociatedAxes());
    TEST_ASSERT(true == effect.IsCompletelyDefined());
  }

  // Verifies that a cloned effect is equivalent to its origin effect even in the presence of
  // type-specific parameters.
  TEST_CASE(ForceFeedbackEffect_TypeSpecificParameters_Clone)
  {
    MockEffectWithTypeSpecificParameters effect;
    TEST_ASSERT(
        true ==
        effect.SetEnvelope(
            {.attackTime = 100, .attackLevel = 200, .fadeTime = 300, .fadeLevel = 400}));
    TEST_ASSERT(
        true == effect.SetTypeSpecificParameters({.valid = true, .param1 = 11, .param2 = 234}));

    std::unique_ptr<Effect> clonedEffect = effect.Clone();
    MockEffectWithTypeSpecificParameters* clonedTypedEffect =
        dynamic_cast<MockEffectWithTypeSpecificParameters*>(clonedEffect.get());
    TEST_ASSERT(nullptr != clonedTypedEffect);

    TEST_ASSERT(clonedTypedEffect->Identifier() == effect.Identifier());
    TEST_ASSERT(clonedTypedEffect->CommonParameters() == effect.CommonParameters());
    TEST_ASSERT(
        clonedTypedEffect->GetTypeSpecificParameters() == effect.GetTypeSpecificParameters());
  }

  // Verifies that two effect objects with the same identifier can successfully complete a parameter
  // synchronization operation even in the presence of type-specific parameters.
  TEST_CASE(ForceFeedbackEffect_TypeSpecificParameters_SyncParameters)
  {
    MockEffectWithTypeSpecificParameters effect;
    std::unique_ptr<Effect> clonedEffect = effect.Clone();

    TEST_ASSERT(
        true ==
        effect.SetEnvelope(
            {.attackTime = 100, .attackLevel = 200, .fadeTime = 300, .fadeLevel = 400}));
    TEST_ASSERT(
        true == effect.SetTypeSpecificParameters({.valid = true, .param1 = 11, .param2 = 234}));

    TEST_ASSERT(true == clonedEffect->SyncParametersFrom(effect));
    TEST_ASSERT(clonedEffect->Identifier() == effect.Identifier());
    TEST_ASSERT(clonedEffect->CommonParameters() == effect.CommonParameters());

    MockEffectWithTypeSpecificParameters* clonedTypedEffect =
        dynamic_cast<MockEffectWithTypeSpecificParameters*>(clonedEffect.get());
    TEST_ASSERT(nullptr != clonedTypedEffect);
    TEST_ASSERT(
        clonedTypedEffect->GetTypeSpecificParameters() == effect.GetTypeSpecificParameters());
  }

  // Creates a force effect and submits invalid type-specific parameters. Verifies that they are
  // rejected.
  TEST_CASE(ForceFeedbackEffect_TypeSpecificParameters_Invalid)
  {
    MockEffectWithTypeSpecificParameters effect;
    TEST_ASSERT(false == effect.SetTypeSpecificParameters({.valid = false}));
  }
} // namespace XidiTest
