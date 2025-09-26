/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MouseSpeedModifierMapperTest.cpp
 *   Unit tests for controller element mappers that modify the mouse movement speed scaling factor.
 **************************************************************************************************/

#include <cstdint>
#include <memory>
#include <optional>

#include <Infra/Test/TestCase.h>

#include "ElementMapper.h"
#include "MockMouse.h"
#include "Mouse.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  /// Mouse speed scaling factor override used for all test cases in this file.
  static constexpr unsigned int kTestMouseSpeedScalingFactorOverride = 33;

  /// Opaque source identifier used for many tests in this file.
  static constexpr uint32_t kOpaqueSourceIdentifier = 5678;

  /// Empty virtual controller state used as a comparison target throughout this file.
  static constexpr SState kEmptyVirtualControllerState = {};

  // Creates one mouse speed modifier mapper and verifies two things. First, verifies that it does
  // not map to any virtual controller element. Second, verifies that it correctly identifies its
  // mouse speed scaling factor override.
  TEST_CASE(MouseSpeedModifierMapper_GetTargetElement_Nominal)
  {
    const MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);
    TEST_ASSERT(0 == mapper.GetTargetElementCount());

    const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(0);
    TEST_ASSERT(false == maybeTargetElement.has_value());

    TEST_ASSERT(mapper.GetMouseSpeedScalingFactor() == kTestMouseSpeedScalingFactorOverride);
  }

  // Creates and then clones one mouse speed modifier mapper and verifies two things. First,
  // verifies that it does not map to any virtual controller element. Second, verifies that it
  // correctly identifies its mouse speed scaling factor override.
  TEST_CASE(MouseSpeedModifierMapper_GetTargetElement_Clone)
  {
    const MouseSpeedModifierMapper mapperOriginal(kTestMouseSpeedScalingFactorOverride);
    const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
    TEST_ASSERT(nullptr != dynamic_cast<MouseSpeedModifierMapper*>(mapperClone.get()));
    TEST_ASSERT(0 == mapperClone->GetTargetElementCount());

    const std::optional<SElementIdentifier> maybeTargetElement = mapperClone->GetTargetElementAt(0);
    TEST_ASSERT(false == maybeTargetElement.has_value());

    TEST_ASSERT(
        dynamic_cast<MouseSpeedModifierMapper*>(mapperClone.get())->GetMouseSpeedScalingFactor() ==
        kTestMouseSpeedScalingFactorOverride);
  }

  // Verifies the nominal behavior in which a mouse speed modifier mapper is asked to contribute
  // some arbitrary analog value. Expected behavior is the mouse speed modifier is applied at the
  // extreme analog values and not applied towards neutral, but the exact transition thresholds are
  // not defined. Sweeps the entire range of possible analog values.
  TEST_CASE(MouseSpeedModifierMapper_ContributeFromAnalogValue_Nominal)
  {
    MockMouse expectedMouseStateNotApplied;
    expectedMouseStateNotApplied.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

    MockMouse expectedMouseStateApplied;
    expectedMouseStateApplied.SubmitMouseSpeedOverride(
        kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);

    // Expected sequence, based on an analog value sweep, is applied, not applied, and finally
    // applied. The final two values are the same as a way of simplifying the implementation thus
    // disabling a final transition and triggering a test failure.
    const MockMouse* const kExpectedMouseSequence[] = {
        &expectedMouseStateApplied,
        &expectedMouseStateNotApplied,
        &expectedMouseStateApplied,
        &expectedMouseStateApplied};
    int currentSequenceIndex = 0;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      const MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);
      const MockMouse* possibleExpectedStates[2] = {
          kExpectedMouseSequence[currentSequenceIndex],
          kExpectedMouseSequence[currentSequenceIndex + 1]};

      MockMouse actualState;
      SState actualVirtualControllerState = kEmptyVirtualControllerState;

      actualState.BeginCapture();
      mapper.ContributeFromAnalogValue(
          actualVirtualControllerState, (int16_t)analogValue, kOpaqueSourceIdentifier);
      actualState.EndCapture();

      TEST_ASSERT(actualVirtualControllerState == kEmptyVirtualControllerState);

      if (actualState == *possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == *possibleExpectedStates[1])
      {
        currentSequenceIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Out-of-sequence mouse state produced by a mouse speed modifier mapper with analog input %d.",
            analogValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedMouseSequence) - 1) - 1);
  }

  // Verifies a sequence of contributions of extreme and neutral analog values lead to a mouse
  // speed modifier mapper applying, then unapplying, a mouse speed modifier.
  TEST_CASE(MouseSpeedModifierMapper_ContributeFromAnalogValue_PressUnpressSequence)
  {
    constexpr int16_t kAnalogValuePress = kAnalogValueMax;
    constexpr int16_t kAnalogValueRelease = kAnalogValueNeutral;

    MockMouse expectedMouseStateInitial;

    MockMouse expectedMouseStateNotApplied;
    expectedMouseStateNotApplied.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

    MockMouse expectedMouseStateApplied;
    expectedMouseStateApplied.SubmitMouseSpeedOverride(
        kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);

    MockMouse actualMouseState;
    SState unusedVirtualControllerState;

    const MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);

    actualMouseState.BeginCapture();
    TEST_ASSERT(actualMouseState == expectedMouseStateInitial);
    mapper.ContributeFromAnalogValue(
        unusedVirtualControllerState, kAnalogValuePress, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualMouseState == expectedMouseStateApplied);
    mapper.ContributeFromAnalogValue(
        unusedVirtualControllerState, kAnalogValueRelease, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualMouseState == expectedMouseStateNotApplied);
    actualMouseState.EndCapture();
  }

  // Verifies the nominal behavior in which a mouse speed modifier mapper is asked to contribute
  // some arbitrary button press state.
  TEST_CASE(MouseSpeedModifierMapper_ContributeFromButtonValue_Nominal)
  {
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);

      SState unusedVirtualControllerState;

      MockMouse expectedState;
      if (true == buttonIsPressed)
        expectedState.SubmitMouseSpeedOverride(
            kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);
      else
        expectedState.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

      MockMouse actualState;
      actualState.BeginCapture();
      mapper.ContributeFromButtonValue(
          unusedVirtualControllerState, buttonIsPressed, kOpaqueSourceIdentifier);
      actualState.EndCapture();

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Verifies a sequence of contributions of pressed and unpressed button values lead to a mouse
  // speed modifier mapper applying, then unapplying, a mouse speed modifier.
  TEST_CASE(MouseSpeedModifierMapper_ContributeFromButtonValue_PressUnpressSequence)
  {
    constexpr bool kButtonValuePress = true;
    constexpr bool kButtonValueRelease = false;

    MockMouse expectedMouseStateInitial;

    MockMouse expectedMouseStateNotApplied;
    expectedMouseStateNotApplied.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

    MockMouse expectedMouseStateApplied;
    expectedMouseStateApplied.SubmitMouseSpeedOverride(
        kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);

    MockMouse actualMouseState;
    SState unusedVirtualControllerState;

    const MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);

    actualMouseState.BeginCapture();
    TEST_ASSERT(actualMouseState == expectedMouseStateInitial);
    mapper.ContributeFromButtonValue(
        unusedVirtualControllerState, kButtonValuePress, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualMouseState == expectedMouseStateApplied);
    mapper.ContributeFromButtonValue(
        unusedVirtualControllerState, kButtonValueRelease, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualMouseState == expectedMouseStateNotApplied);
    actualMouseState.EndCapture();
  }

  // Verifies the nominal behavior in which a mouse speed modifier mapper is asked to contribute a
  // trigger value. Expected behavior is the mouse speed modifier is not applied at the start and
  // becomes applied once the trigger value hits a threshold, but the exact transition point is not
  // defined. Sweeps the entire range of possible trigger values.
  TEST_CASE(MouseSpeedModifierMapper_ContributeFromTriggerValue_Nominal)
  {
    MockMouse expectedMouseStateNotApplied;
    expectedMouseStateNotApplied.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

    MockMouse expectedMouseStateApplied;
    expectedMouseStateApplied.SubmitMouseSpeedOverride(
        kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);

    // Expected sequence, based on an analog value sweep, is applied, not applied, and finally
    // applied. The final two values are the same as a way of simplifying the implementation thus
    // disabling a final transition and triggering a test failure.
    const MockMouse* const kExpectedMouseSequence[] = {
        &expectedMouseStateApplied,
        &expectedMouseStateNotApplied,
        &expectedMouseStateApplied,
        &expectedMouseStateApplied};
    int currentSequenceIndex = 0;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      const MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);
      const MockMouse* possibleExpectedStates[2] = {
          kExpectedMouseSequence[currentSequenceIndex],
          kExpectedMouseSequence[currentSequenceIndex + 1]};

      MockMouse actualState;
      SState actualVirtualControllerState = kEmptyVirtualControllerState;

      actualState.BeginCapture();
      mapper.ContributeFromTriggerValue(
          actualVirtualControllerState, (uint8_t)triggerValue, kOpaqueSourceIdentifier);
      actualState.EndCapture();

      TEST_ASSERT(actualVirtualControllerState == kEmptyVirtualControllerState);

      if (actualState == *possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == *possibleExpectedStates[1])
      {
        currentSequenceIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Out-of-sequence mouse state produced by a mouse speed modifier mapper with trigger input %d.",
            triggerValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedMouseSequence) - 1) - 1);
  }

  // Verifies a sequence of contributions of extreme and neutral trigger values lead to a mouse
  // speed override modifier mapper applying, then unapplying, a mouse speed modifier.
  TEST_CASE(MouseSpeedModifierMapper_ContributeFromTriggerValue_PressUnpressSequence)
  {
    constexpr uint8_t kTriggerValuePress = kTriggerValueMax;
    constexpr uint8_t kTriggerValueRelease = kTriggerValueMin;

    MockMouse expectedMouseStateInitial;

    MockMouse expectedMouseStateNotApplied;
    expectedMouseStateNotApplied.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

    MockMouse expectedMouseStateApplied;
    expectedMouseStateApplied.SubmitMouseSpeedOverride(
        kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);

    MockMouse actualMouseState;
    SState unusedVirtualControllerState;

    const MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);

    actualMouseState.BeginCapture();
    TEST_ASSERT(actualMouseState == expectedMouseStateInitial);
    mapper.ContributeFromTriggerValue(
        unusedVirtualControllerState, kTriggerValuePress, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualMouseState == expectedMouseStateApplied);
    mapper.ContributeFromTriggerValue(
        unusedVirtualControllerState, kTriggerValueRelease, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualMouseState == expectedMouseStateNotApplied);
    actualMouseState.EndCapture();
  }

  // Verifies that a mouse speed modifier mapper causes the mouse speed override to be removed when
  // it is asked for a neutral contribution.
  TEST_CASE(MouseSpeedModifierMapper_ContributeNeutral)
  {
    constexpr MouseSpeedModifierMapper mapper(kTestMouseSpeedScalingFactorOverride);

    SState unusedVirtualControllerState;

    MockMouse expectedState;
    expectedState.SubmitMouseSpeedOverride(std::nullopt, kOpaqueSourceIdentifier);

    MockMouse actualState;
    actualState.SubmitMouseSpeedOverride(
        kTestMouseSpeedScalingFactorOverride, kOpaqueSourceIdentifier);

    actualState.BeginCapture();
    mapper.ContributeNeutral(unusedVirtualControllerState, kOpaqueSourceIdentifier);
    actualState.EndCapture();

    TEST_ASSERT(actualState == expectedState);
  }
} // namespace XidiTest
