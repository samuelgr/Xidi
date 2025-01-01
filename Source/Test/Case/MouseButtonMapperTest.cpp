/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MouseButtonMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual mouse button.
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
  using ::Xidi::Mouse::EMouseButton;

  /// Mouse button identifier used for all test cases in this file.
  static constexpr EMouseButton kTestMouseButton = EMouseButton::Right;

  /// Empty virtual controller state used as a comparison target throughout this file.
  static constexpr SState kEmptyVirtualControllerState = {};

  // Creates one mouse button mapper for various possible mouse buttons and verifies two things.
  // First, verifies that it does not map to any virtual controller element.
  // Second, verifies that it correctly identifies its target mouse button.
  TEST_CASE(MouseButtonMapper_GetTargetElement_Nominal)
  {
    constexpr EMouseButton kTestButtons[] = {
        EMouseButton::Left,
        EMouseButton::Middle,
        EMouseButton::Right,
        EMouseButton::X1,
        EMouseButton::X2};

    for (auto button : kTestButtons)
    {
      const MouseButtonMapper mapper(button);
      TEST_ASSERT(0 == mapper.GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(0);
      TEST_ASSERT(false == maybeTargetElement.has_value());

      TEST_ASSERT(mapper.GetMouseButton() == button);
    }
  }

  // Creates and then clones one mouse button mapper for various possible mouse buttons and verifies
  // two things. First, verifies that it does not map to any virtual controller element. Second,
  // verifies that it correctly identifies its target mouse button.
  TEST_CASE(MouseButtonMapper_GetTargetElement_Clone)
  {
    constexpr EMouseButton kTestButtons[] = {
        EMouseButton::Left,
        EMouseButton::Middle,
        EMouseButton::Right,
        EMouseButton::X1,
        EMouseButton::X2};

    for (auto button : kTestButtons)
    {
      const MouseButtonMapper mapperOriginal(button);
      const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
      TEST_ASSERT(nullptr != dynamic_cast<MouseButtonMapper*>(mapperClone.get()));
      TEST_ASSERT(0 == mapperClone->GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement =
          mapperClone->GetTargetElementAt(0);
      TEST_ASSERT(false == maybeTargetElement.has_value());

      TEST_ASSERT(dynamic_cast<MouseButtonMapper*>(mapperClone.get())->GetMouseButton() == button);
    }
  }

  // Verifies the nominal behavior in which a mouse button mapper is asked to contribute some
  // arbitrary analog value to a mouse button. Expected behavior is the mouse button is pressed at
  // the extreme analog values and not pressed towards neutral, but the exact transition thresholds
  // are not defined. Sweeps the entire range of possible analog values.
  TEST_CASE(MouseButtonMapper_ContributeFromAnalogValue_Nominal)
  {
    MockMouse expectedMouseStateUnpressed;

    MockMouse expectedMouseStatePressed;
    expectedMouseStatePressed.SubmitMouseButtonPressedState(kTestMouseButton);

    // Expected sequence, based on an analog value sweep, is pressed, not pressed, and finally
    // pressed. The final two values are the same as a way of simplifying the implementation thus
    // disabling a final transition and triggering a test failure.
    const MockMouse* const kExpectedMouseSequence[] = {
        &expectedMouseStatePressed,
        &expectedMouseStateUnpressed,
        &expectedMouseStatePressed,
        &expectedMouseStatePressed};
    int currentSequenceIndex = 0;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      const MouseButtonMapper mapper(kTestMouseButton);
      const MockMouse* possibleExpectedStates[2] = {
          kExpectedMouseSequence[currentSequenceIndex],
          kExpectedMouseSequence[currentSequenceIndex + 1]};

      MockMouse actualState;
      SState actualVirtualControllerState = kEmptyVirtualControllerState;

      actualState.BeginCapture();
      mapper.ContributeFromAnalogValue(actualVirtualControllerState, (int16_t)analogValue);
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
            L"Out-of-sequence mouse state produced by a mouse button mapper with analog input %d.",
            analogValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedMouseSequence) - 1) - 1);
  }

  // Verifies a sequence of contributions of extreme and neutral analog values lead to a mouse
  // button mapper pressing, then unpressing, a mouse button.
  TEST_CASE(MouseButtonMapper_ContributeFromAnalogValue_PressUnpressSequence)
  {
    constexpr int16_t kAnalogValuePress = kAnalogValueMax;
    constexpr int16_t kAnalogValueRelease = kAnalogValueNeutral;

    MockMouse expectedMouseStateUnpressed;

    MockMouse expectedMouseStatePressed;
    expectedMouseStatePressed.SubmitMouseButtonPressedState(kTestMouseButton);

    MockMouse actualMouseState;
    SState unusedVirtualControllerState;

    const MouseButtonMapper mapper(kTestMouseButton);

    actualMouseState.BeginCapture();
    TEST_ASSERT(actualMouseState == expectedMouseStateUnpressed);
    mapper.ContributeFromAnalogValue(unusedVirtualControllerState, kAnalogValuePress);
    TEST_ASSERT(actualMouseState == expectedMouseStatePressed);
    mapper.ContributeFromAnalogValue(unusedVirtualControllerState, kAnalogValueRelease);
    TEST_ASSERT(actualMouseState == expectedMouseStateUnpressed);
    actualMouseState.EndCapture();
  }

  // Verifies the nominal behavior in which a mouse button mapper is asked to contribute some
  // arbitrary button press state to a mouse button.
  TEST_CASE(MouseButtonMapper_ContributeFromButtonValue_Nominal)
  {
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr MouseButtonMapper mapper(kTestMouseButton);

      SState unusedVirtualControllerState;

      MockMouse expectedState;
      if (true == buttonIsPressed) expectedState.SubmitMouseButtonPressedState(kTestMouseButton);

      MockMouse actualState;
      actualState.BeginCapture();
      mapper.ContributeFromButtonValue(unusedVirtualControllerState, buttonIsPressed);
      actualState.EndCapture();

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Verifies a sequence of contributions of pressed and unpressed button values lead to a mouse
  // button mapper pressing, then unpressing, a mouse button.
  TEST_CASE(MouseButtonMapper_ContributeFromButtonValue_PressUnpressSequence)
  {
    constexpr bool kButtonValuePress = true;
    constexpr bool kButtonValueRelease = false;

    MockMouse expectedMouseStateUnpressed;

    MockMouse expectedMouseStatePressed;
    expectedMouseStatePressed.SubmitMouseButtonPressedState(kTestMouseButton);

    MockMouse actualMouseState;
    SState unusedVirtualControllerState;

    const MouseButtonMapper mapper(kTestMouseButton);

    actualMouseState.BeginCapture();
    TEST_ASSERT(actualMouseState == expectedMouseStateUnpressed);
    mapper.ContributeFromButtonValue(unusedVirtualControllerState, kButtonValuePress);
    TEST_ASSERT(actualMouseState == expectedMouseStatePressed);
    mapper.ContributeFromButtonValue(unusedVirtualControllerState, kButtonValueRelease);
    TEST_ASSERT(actualMouseState == expectedMouseStateUnpressed);
    actualMouseState.EndCapture();
  }

  // Verifies the nominal behavior in which a mouse button mapper is asked to contribute a trigger
  // value to a mouse button. Expected behavior is the mouse button is not pressed at the start and
  // becomes pressed once the trigger value hits a threshold, but the exact transition point is not
  // defined. Sweeps the entire range of possible trigger values.
  TEST_CASE(MouseButtonMapper_ContributeFromTriggerValue_Nominal)
  {
    MockMouse expectedMouseStateUnpressed;

    MockMouse expectedMouseStatePressed;
    expectedMouseStatePressed.SubmitMouseButtonPressedState(kTestMouseButton);

    // Expected sequence, based on an analog value sweep, is pressed, not pressed, and finally
    // pressed. The final two values are the same as a way of simplifying the implementation thus
    // disabling a final transition and triggering a test failure.
    const MockMouse* const kExpectedMouseSequence[] = {
        &expectedMouseStatePressed,
        &expectedMouseStateUnpressed,
        &expectedMouseStatePressed,
        &expectedMouseStatePressed};
    int currentSequenceIndex = 0;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      const MouseButtonMapper mapper(kTestMouseButton);
      const MockMouse* possibleExpectedStates[2] = {
          kExpectedMouseSequence[currentSequenceIndex],
          kExpectedMouseSequence[currentSequenceIndex + 1]};

      MockMouse actualState;
      SState actualVirtualControllerState = kEmptyVirtualControllerState;

      actualState.BeginCapture();
      mapper.ContributeFromTriggerValue(actualVirtualControllerState, (uint8_t)triggerValue);
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
            L"Out-of-sequence mouse state produced by a mouse button mapper with trigger input %d.",
            triggerValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedMouseSequence) - 1) - 1);
  }

  // Verifies a sequence of contributions of extreme and neutral trigger values lead to a mouse
  // button mapper pressing, then unpressing, a mouse button.
  TEST_CASE(MouseButtonMapper_ContributeFromTriggerValue_PressUnpressSequence)
  {
    constexpr uint8_t kTriggerValuePress = kTriggerValueMax;
    constexpr uint8_t kTriggerValueRelease = kTriggerValueMin;

    MockMouse expectedMouseStateUnpressed;

    MockMouse expectedMouseStatePressed;
    expectedMouseStatePressed.SubmitMouseButtonPressedState(kTestMouseButton);

    MockMouse actualMouseState;
    SState unusedVirtualControllerState;

    const MouseButtonMapper mapper(kTestMouseButton);

    actualMouseState.BeginCapture();
    TEST_ASSERT(actualMouseState == expectedMouseStateUnpressed);
    mapper.ContributeFromTriggerValue(unusedVirtualControllerState, kTriggerValuePress);
    TEST_ASSERT(actualMouseState == expectedMouseStatePressed);
    mapper.ContributeFromTriggerValue(unusedVirtualControllerState, kTriggerValueRelease);
    TEST_ASSERT(actualMouseState == expectedMouseStateUnpressed);
    actualMouseState.EndCapture();
  }

  // Verifies that a mouse button mapper causes a mouse button to be released when it is asked for a
  // neutral contribution.
  TEST_CASE(MouseButtonMapper_ContributeNeutral)
  {
    constexpr MouseButtonMapper mapper(kTestMouseButton);

    SState unusedVirtualControllerState;

    MockMouse expectedState;

    MockMouse actualState;
    actualState.SubmitMouseButtonPressedState(kTestMouseButton);

    actualState.BeginCapture();
    mapper.ContributeNeutral(unusedVirtualControllerState);
    actualState.EndCapture();

    TEST_ASSERT(actualState == expectedState);
  }
} // namespace XidiTest
