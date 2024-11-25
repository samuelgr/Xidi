/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file CompoundMapperTest.cpp
 *   Unit tests for controller element mappers that forward input received to multiple underlying
 *   element mappers.
 **************************************************************************************************/

#include "TestCase.h"

#include <array>
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

  // Creates one CompoundMapper with an array of underlying element mappers present, some of which
  // are null. Verifies correct reporting of the target elements.
  TEST_CASE(CompoundMapper_GetTargetElement_Nominal)
  {
    constexpr SElementIdentifier kUnderlyingElements[] = {
        {.type = EElementType::Button, .button = EButton::B2},
        {.type = EElementType::Button, .button = EButton::B10},
        {.type = EElementType::Axis, .axis = EAxis::X},
        {.type = EElementType::Axis, .axis = EAxis::X},
        {.type = EElementType::Pov}
    };

    const CompoundMapper mapper(
        {nullptr,
         std::make_unique<ButtonMapper>(EButton::B2),
         nullptr,
         std::make_unique<ButtonMapper>(EButton::B10),
         nullptr,
         std::make_unique<SplitMapper>(
             std::make_unique<AxisMapper>(EAxis::X, EAxisDirection::Positive),
             std::make_unique<AxisMapper>(EAxis::X, EAxisDirection::Negative)),
         nullptr,
         std::make_unique<InvertMapper>(std::make_unique<PovMapper>(EPovDirection::Up))});
    TEST_ASSERT(_countof(kUnderlyingElements) == mapper.GetTargetElementCount());

    for (int i = 0; i < _countof(kUnderlyingElements); ++i)
    {
      const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(i);
      TEST_ASSERT(true == maybeTargetElement.has_value());

      const SElementIdentifier targetElement = maybeTargetElement.value();
      TEST_ASSERT(kUnderlyingElements[i] == targetElement);
    }
  }

  // Creates and then clones one CompoundMapper with an array of underlying element mappers present,
  // some of which are null. Verifies correct reporting of the target elements.
  TEST_CASE(CompoundMapper_GetTargetElement_Clone)
  {
    constexpr SElementIdentifier kUnderlyingElements[] = {
        {.type = EElementType::Button, .button = EButton::B2 },
        {.type = EElementType::Button, .button = EButton::B10},
        {.type = EElementType::Axis,   .axis = EAxis::X      },
        {.type = EElementType::Axis,   .axis = EAxis::X      }
    };

    const CompoundMapper mapperOriginal(
        {nullptr,
         std::make_unique<ButtonMapper>(EButton::B2),
         nullptr,
         std::make_unique<ButtonMapper>(EButton::B10),
         nullptr,
         std::make_unique<SplitMapper>(
             std::make_unique<AxisMapper>(EAxis::X, EAxisDirection::Positive),
             std::make_unique<AxisMapper>(EAxis::X, EAxisDirection::Negative))});
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

  // Verifies correct routing of contributions from an analog source to all underlying element
  // mappers.
  TEST_CASE(CompoundMapper_Route_Analog)
  {
    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      constexpr int kExpectedContributionCount = CompoundMapper::kMaxUnderlyingElementMappers;
      static_assert(
          kExpectedContributionCount <= CompoundMapper::kMaxUnderlyingElementMappers,
          "Too many underlying element mappers are present.");

      int actualContributionCount = 0;

      CompoundMapper::TElementMappers mockElementMappers;
      for (int i = 0; i < kExpectedContributionCount; ++i)
        mockElementMappers[i] = std::make_unique<MockElementMapper>(
            MockElementMapper::EExpectedSource::Analog,
            (int16_t)analogValue,
            &actualContributionCount);

      const CompoundMapper mapper(std::move(mockElementMappers));

      mapper.ContributeFromAnalogValue(unusedControllerState, (int16_t)analogValue);
      TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }
  }

  // Verifies correct routing of contributions from a button source to all underlying element
  // mappers.
  TEST_CASE(CompoundMapper_Route_Button)
  {
    constexpr bool kButtonValues[] = {false, true};

    for (bool buttonValue : kButtonValues)
    {
      constexpr int kExpectedContributionCount = CompoundMapper::kMaxUnderlyingElementMappers;
      static_assert(
          kExpectedContributionCount <= CompoundMapper::kMaxUnderlyingElementMappers,
          "Too many underlying element mappers are present.");

      int actualContributionCount = 0;

      CompoundMapper::TElementMappers mockElementMappers;
      for (int i = 0; i < kExpectedContributionCount; ++i)
        mockElementMappers[i] = std::make_unique<MockElementMapper>(
            MockElementMapper::EExpectedSource::Button, buttonValue, &actualContributionCount);

      const CompoundMapper mapper(std::move(mockElementMappers));

      mapper.ContributeFromButtonValue(unusedControllerState, buttonValue);
      TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }
  }

  // Verifies correct routing of contributions from a trigger source to all underlying element
  // mappers.
  TEST_CASE(CompoundMapper_Route_Trigger)
  {
    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      constexpr int kExpectedContributionCount = CompoundMapper::kMaxUnderlyingElementMappers;
      static_assert(
          kExpectedContributionCount <= CompoundMapper::kMaxUnderlyingElementMappers,
          "Too many underlying element mappers are present.");

      int actualContributionCount = 0;

      CompoundMapper::TElementMappers mockElementMappers;
      for (int i = 0; i < kExpectedContributionCount; ++i)
        mockElementMappers[i] = std::make_unique<MockElementMapper>(
            MockElementMapper::EExpectedSource::Trigger,
            (uint8_t)triggerValue,
            &actualContributionCount);

      const CompoundMapper mapper(std::move(mockElementMappers));

      mapper.ContributeFromTriggerValue(unusedControllerState, (uint8_t)triggerValue);
      TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }
  }

  // Verifies correct routing of neutral contributions to all underlying element mappers.
  TEST_CASE(CompoundMapper_Route_Neutral)
  {
    constexpr int kExpectedContributionCount = CompoundMapper::kMaxUnderlyingElementMappers;
    static_assert(
        kExpectedContributionCount <= CompoundMapper::kMaxUnderlyingElementMappers,
        "Too many underlying element mappers are present.");

    int actualContributionCount = 0;

    CompoundMapper::TElementMappers mockElementMappers;
    for (int i = 0; i < kExpectedContributionCount; ++i)
      mockElementMappers[i] = std::make_unique<MockElementMapper>(
          MockElementMapper::EExpectedSource::Neutral, false, &actualContributionCount);

    const CompoundMapper mapper(std::move(mockElementMappers));

    mapper.ContributeNeutral(unusedControllerState);
    TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
  }
} // namespace XidiTest
