/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file InvertMapperTest.cpp
 *   Unit tests for controller element mappers that invert input received and forward the result
 *   to another element mapper.
 **************************************************************************************************/

#include "TestCase.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include "ApiWindows.h"
#include "ElementMapper.h"
#include "MockElementMapper.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  /// Controller state used for tests that need such an instance but do not care about its contents.
  static SState unusedControllerState;

  // Creates one InvertMapper with an underlying compound element mapper present.
  // Verifies correct reporting of the target elements.
  TEST_CASE(InvertMapper_GetTargetElement_Nominal)
  {
    constexpr SElementIdentifier kUnderlyingElements[] = {
        {.type = EElementType::Button, .button = EButton::B2},
        {.type = EElementType::Button, .button = EButton::B10}};

    const InvertMapper mapper(std::make_unique<SplitMapper>(
        std::make_unique<MockElementMapper>(kUnderlyingElements[0]),
        std::make_unique<MockElementMapper>(kUnderlyingElements[1])));
    TEST_ASSERT(_countof(kUnderlyingElements) == mapper.GetTargetElementCount());

    for (int i = 0; i < _countof(kUnderlyingElements); ++i)
    {
      const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(i);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(kUnderlyingElements[i] == targetElement);
    }
  }

  // Creates and then clones one InvertMapper with an underlying compound element mapper present.
  // Verifies correct reporting of the target elements.
  TEST_CASE(InvertMapper_GetTargetElement_Clone)
  {
    constexpr SElementIdentifier kUnderlyingElements[] = {
        {.type = EElementType::Button, .button = EButton::B2},
        {.type = EElementType::Button, .button = EButton::B10}};

    const InvertMapper mapperOriginal(std::make_unique<SplitMapper>(
        std::make_unique<MockElementMapper>(kUnderlyingElements[0]),
        std::make_unique<MockElementMapper>(kUnderlyingElements[1])));
    const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
    TEST_ASSERT(_countof(kUnderlyingElements) == mapperClone->GetTargetElementCount());

    for (int i = 0; i < _countof(kUnderlyingElements); ++i)
    {
      const std::optional<SElementIdentifier> maybeTargetElement =
          mapperClone->GetTargetElementAt(i);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(kUnderlyingElements[i] == targetElement);
    }
  }

  // Creates one InvertMapper with no underlying mapper present.
  // Verifies correct reporting of the target element from it.
  TEST_CASE(InvertMapper_GetTargetElement_UnderlyingNull)
  {
    const InvertMapper mapper(nullptr);
    TEST_ASSERT(0 == mapper.GetTargetElementCount());
  }

  // Verifies that InvertMapper objects correctly invert analog values.
  // Loops through all possible analog values.
  TEST_CASE(InvertMapper_InvertContribution_Analog)
  {
    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      constexpr int kExpectedContributionCount = 1;
      int actualContributionCount = 0;

      const int32_t expectedContributionValue = kAnalogValueMax - (analogValue - kAnalogValueMin);
      const InvertMapper mapper(std::make_unique<MockElementMapper>(
          MockElementMapper::EExpectedSource::Analog,
          (int16_t)expectedContributionValue,
          &actualContributionCount));

      mapper.ContributeFromAnalogValue(unusedControllerState, (int16_t)analogValue);
      TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }
  }

  // Verifies that InvertMapper objects correctly invert button values.
  // Loops through all possible button values.
  TEST_CASE(InvertMapper_InvertContribution_Button)
  {
    constexpr bool kButtonValues[] = {false, true};

    for (bool buttonValue : kButtonValues)
    {
      constexpr int kExpectedContributionCount = 1;
      int actualContributionCount = 0;

      const bool expectedContributionValue = !buttonValue;
      const InvertMapper mapper(std::make_unique<MockElementMapper>(
          MockElementMapper::EExpectedSource::Button,
          expectedContributionValue,
          &actualContributionCount));

      mapper.ContributeFromButtonValue(unusedControllerState, buttonValue);
      TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }
  }

  // Verifies that InvertMapper objects correctly invert trigger values.
  // Loops through all possible trigger values.
  TEST_CASE(InvertMapper_InvertContribution_Trigger)
  {
    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      constexpr int kExpectedContributionCount = 1;
      int actualContributionCount = 0;

      const int32_t expectedContributionValue =
          kTriggerValueMax - ((int32_t)triggerValue - kTriggerValueMin);
      const InvertMapper mapper(std::make_unique<MockElementMapper>(
          MockElementMapper::EExpectedSource::Trigger,
          (uint8_t)expectedContributionValue,
          &actualContributionCount));

      mapper.ContributeFromTriggerValue(unusedControllerState, (uint8_t)triggerValue);
      TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }
  }

  // Verifies that InvertMapper objects correctly forward neutral contributions.
  TEST_CASE(InvertMapper_InvertContribution_Neutral)
  {
    constexpr int kExpectedContributionCount = 1;
    int actualContributionCount = 0;

    const InvertMapper mapper(std::make_unique<MockElementMapper>(
        MockElementMapper::EExpectedSource::Neutral, false, &actualContributionCount));

    mapper.ContributeNeutral(unusedControllerState);
    TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
  }
} // namespace XidiTest
