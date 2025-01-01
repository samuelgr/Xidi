/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file DigitalAxisMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual axis but without any
 *   analog functionality (i.e. extreme values only).
 **************************************************************************************************/

#include <cstdint>
#include <memory>
#include <optional>

#include <Infra/Test/TestCase.h>

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  // Creates one digital axis mapper for each possible virtual button and verifies that each
  // correctly identifies its target virtual controller element.
  TEST_CASE(DigitalAxisMapper_GetTargetElement_Nominal)
  {
    for (int i = 0; i < (int)EAxis::Count; ++i)
    {
      const DigitalAxisMapper mapper((EAxis)i);
      TEST_ASSERT(1 == mapper.GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(0);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(EElementType::Axis == targetElement.type);
      TEST_ASSERT(i == (int)targetElement.axis);
    }
  }

  // Creates one digital axis mapper for each possible virtual button and verifies that each
  // correctly identifies its target virtual controller element.
  TEST_CASE(DigitalAxisMapper_GetTargetElement_Clone)
  {
    for (int i = 0; i < (int)EAxis::Count; ++i)
    {
      const DigitalAxisMapper mapperOriginal((EAxis)i);
      const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
      TEST_ASSERT(1 == mapperClone->GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement =
          mapperClone->GetTargetElementAt(0);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(EElementType::Axis == targetElement.type);
      TEST_ASSERT(i == (int)targetElement.axis);
    }
  }

  // Verifies the nominal behavior in which a digital axis mapper is asked to contribute some
  // arbitrary analog value to an axis. Sweeps the entire range of possible analog values. The only
  // valid values that can be produced are extreme negative, neutral, and extreme positive, and
  // these values must exist in that order.
  TEST_CASE(DigitalAxisMapper_ContributeFromAnalogValue_Nominal_EntireAxis)
  {
    constexpr EAxis kTargetAxis = EAxis::RotX;

    // Specify the allowed values in the order in which they should appear, but without specifying
    // the analog thresholds at which transitions take place. If the current allowed index value is
    // not seen but the next allowed index is, then a transition occurred and we move onto the next
    // allowed index. The final two values are the same as a way of simplifying the implementation
    // thus disabling a final transition and triggering a test failure.
    constexpr int32_t kAllowedValues[] = {
        kAnalogValueMin, kAnalogValueNeutral, kAnalogValueMax, kAnalogValueMax};
    int currentAllowedIndex = 0;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0][kTargetAxis] = kAllowedValues[currentAllowedIndex];
      possibleExpectedStates[1][kTargetAxis] = kAllowedValues[currentAllowedIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentAllowedIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Unexpected value %d produced by a digital axis mapper with analog input %d.",
            (int)actualState[kTargetAxis],
            analogValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentAllowedIndex == (_countof(kAllowedValues) - 1) - 1);
  }

  // Same as above, but for a half axis in the positive direction.
  // Here, the negative part of the axis is ignored, and the positive part is expected to produce
  // either neutral or extreme positive.
  TEST_CASE(DigitalAxisMapper_ContributeFromAnalogValue_Nominal_HalfAxisPositive)
  {
    constexpr EAxis kTargetAxis = EAxis::RotY;

    // The negative part of the axis should be totally ignored.
    for (int32_t analogValue = kAnalogValueMin; analogValue < kAnalogValueNeutral; ++analogValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState[kTargetAxis] = kAnalogValueNeutral;

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

      TEST_ASSERT(actualState == expectedState);
    }

    // The positive part of the axis should be converted into either neutral or extreme positive.
    // Logic is the same as the whole-axis case.
    constexpr int32_t kAllowedValues[] = {kAnalogValueNeutral, kAnalogValueMax, kAnalogValueMax};
    int currentAllowedIndex = 0;

    for (int32_t analogValue = kAnalogValueNeutral; analogValue <= kAnalogValueMax; ++analogValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0][kTargetAxis] = kAllowedValues[currentAllowedIndex];
      possibleExpectedStates[1][kTargetAxis] = kAllowedValues[currentAllowedIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentAllowedIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Unexpected value %d produced by a digital axis mapper with analog input %d.",
            (int)actualState[kTargetAxis],
            analogValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentAllowedIndex == (_countof(kAllowedValues) - 1) - 1);
  }

  // Same as above, but for a half axis in the negative direction.
  // Here, the positive part of the axis is ignored, and the negative part is expected to produce
  // either extreme negative or neutral.
  TEST_CASE(DigitalAxisMapper_ContributeFromAnalogValue_Nominal_HalfAxisNegative)
  {
    constexpr EAxis kTargetAxis = EAxis::RotZ;

    // The positive part of the axis should be totally ignored.
    for (int32_t analogValue = (1 + kAnalogValueNeutral); analogValue <= kAnalogValueMax;
         ++analogValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState[kTargetAxis] = kAnalogValueNeutral;

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

      TEST_ASSERT(actualState == expectedState);
    }

    // The positive part of the axis should be converted into either neutral or extreme positive.
    // Logic is the same as the whole-axis case.
    constexpr int32_t kAllowedValues[] = {
        kAnalogValueMin, kAnalogValueNeutral, kAnalogValueNeutral};
    int currentAllowedIndex = 0;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueNeutral; ++analogValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0][kTargetAxis] = kAllowedValues[currentAllowedIndex];
      possibleExpectedStates[1][kTargetAxis] = kAllowedValues[currentAllowedIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentAllowedIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Unexpected value %d produced by a digital axis mapper with analog input %d.",
            (int)actualState[kTargetAxis],
            analogValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentAllowedIndex == (_countof(kAllowedValues) - 1) - 1);
  }

  // Verifies correct behavior when multiple digital axis mappers all contribute to the same virtual
  // axis. The aggregated contribution should be the sum of the values contributed by each axis
  // mapper. It is possible and acceptable that the result of aggregating all contributing axis
  // mappers exceeds the maximum possible analog axis value.
  TEST_CASE(DigitalAxisMapper_ContributeFromAnalogValue_ConstructiveInterference)
  {
    constexpr EAxis kTargetAxis = EAxis::RotY;

    constexpr DigitalAxisMapper mappers[] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[kTargetAxis] = kAnalogValueMin * (int32_t)_countof(mappers);

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappers)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueMin);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies correct behavior when multiple digital axis mappers all contribute to the same virtual
  // axis but the net contribution sums to the neutral position.
  TEST_CASE(DigitalAxisMapper_ContributeFromAnalogValue_DestructiveInterference)
  {
    constexpr EAxis kTargetAxis = EAxis::RotY;

    constexpr DigitalAxisMapper mappersPositive[] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    constexpr DigitalAxisMapper mappersNegative[_countof(mappersPositive)] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[kTargetAxis] = kAnalogValueNeutral;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappersPositive)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueMax);
    for (auto& mapper : mappersNegative)
      mapper.ContributeFromAnalogValue(actualState, kAnalogValueMin);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies the nominal behavior in which a digital axis mapper is asked to contribute some
  // arbitrary button press state to an axis.
  TEST_CASE(DigitalAxisMapper_ContributeFromButtonValue_Nominal_EntireAxis)
  {
    constexpr EAxis kTargetAxis = EAxis::X;
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis);

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState[kTargetAxis] = (true == buttonIsPressed ? kAnalogValueMax : kAnalogValueMin);

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Same as above, but for a half axis in the positive direction.
  TEST_CASE(DigitalAxisMapper_ContributeFromButtonValue_Nominal_HalfAxisPositive)
  {
    constexpr EAxis kTargetAxis = EAxis::Y;
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState[kTargetAxis] =
          (true == buttonIsPressed ? kAnalogValueMax : kAnalogValueNeutral);

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Same as above, but for a half axis in the negative direction.
  TEST_CASE(DigitalAxisMapper_ContributeFromButtonValue_Nominal_HalfAxisNegative)
  {
    constexpr EAxis kTargetAxis = EAxis::Y;
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState[kTargetAxis] =
          (true == buttonIsPressed ? kAnalogValueMin : kAnalogValueNeutral);

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Verifies correct behavior when multiple digital axis mappers all contribute to the same virtual
  // axis but sourced by a button state. The aggregated contribution should be the sum of the values
  // contributed by each axis mapper, which themselves should be extreme in one direction or
  // another.
  TEST_CASE(DigitalAxisMapper_ContributeFromButtonValue_ConstructiveInterference)
  {
    constexpr EAxis kTargetAxis = EAxis::Z;
    constexpr bool kButtonStates[] = {false, true};

    for (bool buttonIsPressed : kButtonStates)
    {
      constexpr DigitalAxisMapper mappers[] = {
          DigitalAxisMapper(kTargetAxis),
          DigitalAxisMapper(kTargetAxis),
          DigitalAxisMapper(kTargetAxis),
          DigitalAxisMapper(kTargetAxis),
          DigitalAxisMapper(kTargetAxis),
          DigitalAxisMapper(kTargetAxis)};

      SState expectedState;
      ZeroMemory(&expectedState, sizeof(expectedState));
      expectedState[kTargetAxis] =
          (true == buttonIsPressed ? kAnalogValueMax : kAnalogValueMin) * _countof(mappers);

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      for (auto& mapper : mappers)
        mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

      TEST_ASSERT(actualState == expectedState);
    }
  }

  // Verifies correct behavior when multiple digital axis mappers all contribute to the same virtual
  // axis but sourced by a button state. In this case, the aggregate contribution sums to a net of
  // the neutral position (i.e. there are as many button states "pressed" as "not pressed").
  TEST_CASE(DigitalAxisMapper_ContributeFromButtonValue_DestructiveInterference)
  {
    constexpr EAxis kTargetAxis = EAxis::Z;

    constexpr DigitalAxisMapper mappersPressed[] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    constexpr DigitalAxisMapper mappersNotPressed[_countof(mappersPressed)] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[kTargetAxis] = kAnalogValueNeutral;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappersPressed)
      mapper.ContributeFromButtonValue(actualState, true);
    for (auto& mapper : mappersNotPressed)
      mapper.ContributeFromButtonValue(actualState, false);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies the nominal behavior in which a digital axis mapper is asked to contribute a trigger
  // value to an axis. Sweeps the entire range of possible trigger values. The only valid values
  // that can be produced are extreme negative and extreme positive, and these values must exist in
  // that order.
  TEST_CASE(DigitalAxisMapper_ContributeFromTriggerValue_Nominal_EntireAxis)
  {
    constexpr EAxis kTargetAxis = EAxis::RotX;

    // Same logic applies as in the analog value case, except that there is no neutral value because
    // triggers do not have a neutral centered position.
    constexpr int32_t kAllowedValues[] = {kAnalogValueMin, kAnalogValueMax, kAnalogValueMax};
    int currentAllowedIndex = 0;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0][kTargetAxis] = kAllowedValues[currentAllowedIndex];
      possibleExpectedStates[1][kTargetAxis] = kAllowedValues[currentAllowedIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentAllowedIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Unexpected value %d produced by a digital axis mapper with trigger input %d.",
            (int)actualState[kTargetAxis],
            triggerValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentAllowedIndex == (_countof(kAllowedValues) - 1) - 1);
  }

  // Same as above, but for a half axis in the positive direction.
  // Here, the produced value is either neutral (trigger not pressed) or extreme positive (trigger
  // is pressed).
  TEST_CASE(DigitalAxisMapper_ContributeFromTriggerValue_Nominal_HalfAxisPositive)
  {
    constexpr EAxis kTargetAxis = EAxis::RotY;

    constexpr int32_t kAllowedValues[] = {kAnalogValueNeutral, kAnalogValueMax, kAnalogValueMax};
    int currentAllowedIndex = 0;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0][kTargetAxis] = kAllowedValues[currentAllowedIndex];
      possibleExpectedStates[1][kTargetAxis] = kAllowedValues[currentAllowedIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentAllowedIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Unexpected value %d produced by a digital axis mapper with trigger input %d.",
            (int)actualState[kTargetAxis],
            triggerValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentAllowedIndex == (_countof(kAllowedValues) - 1) - 1);
  }

  // Same as above, but for a half axis in the negative direction.
  // Here, the produced value is either neutral (trigger not pressed) or extreme negative (trigger
  // is pressed).
  TEST_CASE(DigitalAxisMapper_ContributeFromTriggerValue_Nominal_HalfAxisNegative)
  {
    constexpr EAxis kTargetAxis = EAxis::RotZ;

    constexpr int32_t kAllowedValues[] = {kAnalogValueNeutral, kAnalogValueMin, kAnalogValueMin};
    int currentAllowedIndex = 0;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      constexpr DigitalAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

      SState possibleExpectedStates[2];
      ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
      possibleExpectedStates[0][kTargetAxis] = kAllowedValues[currentAllowedIndex];
      possibleExpectedStates[1][kTargetAxis] = kAllowedValues[currentAllowedIndex + 1];

      SState actualState;
      ZeroMemory(&actualState, sizeof(actualState));
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

      if (actualState == possibleExpectedStates[0])
      {
        continue;
      }
      else if (actualState == possibleExpectedStates[1])
      {
        currentAllowedIndex += 1;
        continue;
      }
      else
      {
        TEST_FAILED_BECAUSE(
            L"Unexpected value %d produced by a digital axis mapper with trigger input %d.",
            (int)actualState[kTargetAxis],
            triggerValue);
      }
    }

    // The last value in the allowed values array is a sentinel just for ease of implementation.
    // We do, however, expect that all other values will have been reached.
    TEST_ASSERT(currentAllowedIndex == (_countof(kAllowedValues) - 1) - 1);
  }

  // Verifies correct behavior when multiple digital axis mappers all contribute to the same virtual
  // axis but sourced by a trigger value. The aggregated contribution should be the sum of the
  // values contributed by each axis mapper, which themselves should be extreme positive based on
  // the test parameters.
  TEST_CASE(DigitalAxisMapper_ContributeFromTriggerValue_ConstructiveInterference)
  {
    constexpr EAxis kTargetAxis = EAxis::Z;

    constexpr DigitalAxisMapper mappers[] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[kTargetAxis] = kAnalogValueMax * _countof(mappers);

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappers)
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)kTriggerValueMax);

    TEST_ASSERT(actualState == expectedState);
  }

  // Verifies correct behavior when multiple digital axis mappers all contribute to the same virtual
  // axis but sourced by a trigger value. In this case, the aggregate contribution sums to a net of
  // the neutral position.
  TEST_CASE(DigitalAxisMapper_ContributeFromTriggerValue_DestructiveInterference)
  {
    constexpr EAxis kTargetAxis = EAxis::Z;

    constexpr DigitalAxisMapper mappersPositive[] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    constexpr DigitalAxisMapper mappersNegative[_countof(mappersPositive)] = {
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis),
        DigitalAxisMapper(kTargetAxis)};

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[kTargetAxis] = kAnalogValueNeutral;

    SState actualState;
    ZeroMemory(&actualState, sizeof(actualState));
    for (auto& mapper : mappersPositive)
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)kTriggerValueMax);
    for (auto& mapper : mappersNegative)
      mapper.ContributeFromTriggerValue(actualState, (uint8_t)kTriggerValueMin);

    TEST_ASSERT(actualState == expectedState);
  }
} // namespace XidiTest
