/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file ButtonMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual button.
 **************************************************************************************************/

#include "TestCase.h"

#include <cstdint>
#include <memory>
#include <optional>

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  // Creates one button mapper for each possible virtual button and verifies that each correctly
  // identifies its target virtual controller element.
  TEST_CASE(ButtonMapper_GetTargetElement_Nominal)
  {
    for (int i = 0; i < (int)EButton::Count; ++i)
    {
      const ButtonMapper mapper((EButton)i);
      TEST_ASSERT(1 == mapper.GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(0);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(EElementType::Button == targetElement.type);
      TEST_ASSERT(i == (int)targetElement.button);
    }
  }

  // Creates and then clones one button mapper for each possible virtual button and verifies that
  // each correctly identifies its target virtual controller element.
  TEST_CASE(ButtonMapper_GetTargetElement_Clone)
  {
    for (int i = 0; i < (int)EButton::Count; ++i)
    {
      const ButtonMapper mapperOriginal((EButton)i);
      const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
      TEST_ASSERT(nullptr != dynamic_cast<ButtonMapper*>(mapperClone.get()));
      TEST_ASSERT(1 == mapperClone->GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement =
          mapperClone->GetTargetElementAt(0);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(EElementType::Button == targetElement.type);
      TEST_ASSERT(i == (int)targetElement.button);
    }
  }

  // Verifies the nominal behavior in which a button mapper is asked to contribute some arbitrary
  // analog value to a button. Expected behavior is the button is pressed at the extreme analog
  // values and not pressed towards neutral, but the exact transition thresholds are not defined.
  // Sweeps the entire range of possible analog values.
  TEST_CASE(ButtonMapper_ContributeFromAnalogValue_Nominal)
  {
    constexpr EButton kTargetButton = EButton::B1;

    // Expected sequence, based on an analog value sweep, is pressed, not pressed, and finally
    // pressed. The final two values are the same as a way of simplifying the implementation thus
    // disabling a final transition and triggering a test failure.
    constexpr bool kExpectedButtonSequence[] = {true, false, true, true};
    int currentSequenceIndex = 0;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      const ButtonMapper mapper(kTargetButton);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0].button[(int)kTargetButton] =
          kExpectedButtonSequence[currentSequenceIndex];
      possibleExpectedStates[1].button[(int)kTargetButton] =
          kExpectedButtonSequence[currentSequenceIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentSequenceIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Out-of-sequence value %d produced by a button mapper with analog input %d.",
            (int)actualState.button[(int)kTargetButton],
            analogValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedButtonSequence) - 1) - 1);
  }

  // Verifies correct behavior when multiple button mappers all contribute to the same virtual
  // button with neutral analog values as input. The aggregated contribution should always be that
  // the button is not pressed, since no mapper sees any analog value away from neutral.
  TEST_CASE(ButtonMapper_ContributeFromAnalogValue_AllNeutral)
  {
    constexpr EButton kTargetButton = EButton::B2;

    constexpr ButtonMapper mappers[] = {
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = false;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappers)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueNeutral);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies correct behavior when multiple button mappers all contribute to the same virtual
  // button with an extreme analog value as input. The aggregated contribution should always be that
  // the button is pressed.
  TEST_CASE(ButtonMapper_ContributeFromAnalogValue_ConstructiveInterference)
  {
    constexpr EButton kTargetButton = EButton::B3;

    constexpr ButtonMapper mappers[] = {
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = true;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappers)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueMax);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies correct behavior when multiple button mappers all contribute to the same virtual
  // button but the net analog value sum equals the neutral position. For button mappers this does
  // not matter and the expected output is still that the button is pressed.
  TEST_CASE(ButtonMapper_ContributeFromAnalogValue_DestructiveInterference)
  {
    constexpr EButton kTargetButton = EButton::B4;

    constexpr ButtonMapper mappersPositive[] = {
        ButtonMapper(kTargetButton), ButtonMapper(kTargetButton), ButtonMapper(kTargetButton)};

    constexpr ButtonMapper mappersNegative[_countof(mappersPositive)] = {
        ButtonMapper(kTargetButton), ButtonMapper(kTargetButton), ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = true;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappersPositive)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueMax);
    for (auto& mapper : mappersNegative)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueMin);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies the nominal behavior in which a button mapper is asked to contribute some arbitrary
  // button press state to a button.
  TEST_CASE(ButtonMapper_ContributeFromButtonValue_Nominal)
  {
    constexpr EButton kTargetButton = EButton::B5;
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr ButtonMapper mapper(kTargetButton);

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState.button[(int)kTargetButton] = buttonIsPressed;

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Verifies correct behavior when multiple button mapper contributions occur to the same virtual
  // button and all button mappers receive the same input state. As long as one button mapper
  // receives an input of "pressed" then the virtual button should also be pressed.
  TEST_CASE(ButtonMapper_ContributeFromButtonValue_SameButtonSameInput)
  {
    constexpr EButton kTargetButton = EButton::B6;
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr ButtonMapper mappers[] = {
          ButtonMapper(kTargetButton),
          ButtonMapper(kTargetButton),
          ButtonMapper(kTargetButton),
          ButtonMapper(kTargetButton),
          ButtonMapper(kTargetButton)};

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState.button[(int)kTargetButton] = buttonIsPressed;

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      for (auto& mapper : mappers)
        mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Verifies correct behavior when multiple button mapper contributions occur to the same virtual
  // button but mappers receive different input state. As long as one button mapper receives an
  // input of "pressed" then the virtual button should also be pressed.
  TEST_CASE(ButtonMapper_ContributeFromAnalogValue_SameButtonDifferentInput)
  {
    constexpr EButton kTargetButton = EButton::B7;

    constexpr ButtonMapper mappersPressed[] = {
        ButtonMapper(kTargetButton), ButtonMapper(kTargetButton), ButtonMapper(kTargetButton)};

    constexpr ButtonMapper mappersNotPressed[] = {
        ButtonMapper(kTargetButton), ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = true;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappersPressed)
      mapper.ContributeFromButtonValue(actualState, true);
    for (auto& mapper : mappersNotPressed)
      mapper.ContributeFromButtonValue(actualState, false);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies the nominal behavior in which a button mapper is asked to contribute a trigger value
  // to a button. Expected behavior is the button is not pressed at the start and becomes pressed
  // once the trigger value hits a threshold, but the exact transition point is not defined. Sweeps
  // the entire range of possible trigger values.
  TEST_CASE(ButtonMapper_ContributeFromTriggerValue_Nominal)
  {
    constexpr EButton kTargetButton = EButton::B8;

    // Expected sequence, based on a trigger value sweep, is not pressed followed by pressed.
    // The final two values are the same as a way of simplifying the implementation thus disabling a
    // final transition and triggering a test failure.
    constexpr bool kExpectedButtonSequence[] = {false, true, true};
    int currentSequenceIndex = 0;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      const ButtonMapper mapper(kTargetButton);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0].button[(int)kTargetButton] =
          kExpectedButtonSequence[currentSequenceIndex];
      possibleExpectedStates[1].button[(int)kTargetButton] =
          kExpectedButtonSequence[currentSequenceIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentSequenceIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Out-of-sequence value %d produced by a button mapper with trigger input %d.",
            (int)actualState.button[(int)kTargetButton],
            triggerValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedButtonSequence) - 1) - 1);
  }

  // Verifies correct behavior when multiple button mappers all contribute to the same virtual
  // button with minimum trigger values as input. The aggregated contribution should always be that
  // the button is not pressed, since no mapper sees any trigger value that could possibly have
  // exceeded the threshold.
  TEST_CASE(ButtonMapper_ContributeFromTriggerValue_NonePressed)
  {
    constexpr EButton kTargetButton = EButton::B9;

    constexpr ButtonMapper mappers[] = {
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = false;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappers)
      mapper.ContributeFromTriggerValue(actualState, kTriggerValueMin);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies correct behavior when multiple button mappers all contribute to the same virtual
  // button with maximum trigger values. The aggregated contribution should always be that the
  // button is pressed.
  TEST_CASE(ButtonMapper_ContributeFromTriggerValue_AllPressed)
  {
    constexpr EButton kTargetButton = EButton::B10;

    constexpr ButtonMapper mappers[] = {
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton),
        ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = true;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappers)
      mapper.ContributeFromTriggerValue(actualState, kTriggerValueMax);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies correct behavior when multiple button mappers all contribute to the same virtual
  // button and only some are considered pressed based on the input trigger value. For button
  // mappers this does not matter and the expected output is still that the button is pressed.
  TEST_CASE(ButtonMapper_ContributeFromTriggerValue_SomePressed)
  {
    constexpr EButton kTargetButton = EButton::B11;

    constexpr ButtonMapper mappersPressed[] = {
        ButtonMapper(kTargetButton), ButtonMapper(kTargetButton)};

    constexpr ButtonMapper mappersNotPressed[] = {
        ButtonMapper(kTargetButton), ButtonMapper(kTargetButton), ButtonMapper(kTargetButton)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState.button[(int)kTargetButton] = true;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappersPressed)
      mapper.ContributeFromTriggerValue(actualState, kTriggerValueMax);
    for (auto& mapper : mappersNotPressed)
      mapper.ContributeFromTriggerValue(actualState, kTriggerValueMin);

    TEST_ASSERT(actualState == expectedState);
  }
} // namespace XidiTest
