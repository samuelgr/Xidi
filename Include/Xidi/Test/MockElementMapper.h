/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file MockElementMapper.h
 *   Mock element mapper interface that can be used for tests.
 **************************************************************************************************/

#pragma once

#include "TestCase.h"

#include <memory>
#include <optional>
#include <vector>

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  /// Mock version of an element mapper, used for testing purposes to ensure that values read from a
  /// controller are correctly routed.
  class MockElementMapper : public IElementMapper
  {
  public:

    /// Enumerates possible expected sources of input values from an XInput controller.
    /// Specifies which of the `Contribute` methods is expected to be invoked.
    enum class EExpectedSource
    {
      None,
      Analog,
      Button,
      Trigger,
      Neutral
    };

    /// Holds an expected input value, one for each allowed type.
    union UExpectedValue
    {
      int16_t analog;
      bool button;
      uint8_t trigger;

      constexpr UExpectedValue(int16_t analog) : analog(analog) {}

      constexpr UExpectedValue(bool button) : button(button) {}

      constexpr UExpectedValue(uint8_t trigger) : trigger(trigger) {}
    };

    /// Can be used as a default constructor for tests that do not exercise controller capabilities.
    inline MockElementMapper(
        std::optional<EExpectedSource> maybeExpectedSource = std::nullopt,
        std::optional<UExpectedValue> maybeExpectedValue = std::nullopt,
        int* contributionCounter = nullptr,
        std::vector<SElementIdentifier>&& fakeTargetElements = {SElementIdentifier()},
        std::optional<uint32_t> expectedSourceIdentifier = std::nullopt)
        : maybeExpectedSource(maybeExpectedSource),
          maybeExpectedValue(maybeExpectedValue),
          contributionCounter(contributionCounter),
          fakeTargetElements(std::move(fakeTargetElements)),
          expectedSourceIdentifier(expectedSourceIdentifier)
    {}

    /// For simpler tests that expect no contributions but require only a single target element.
    inline MockElementMapper(SElementIdentifier fakeTargetElement)
        : MockElementMapper(EExpectedSource::None, false, nullptr, {fakeTargetElement})
    {}

    /// Retrieves and returns the opaque source identifier that has been associated with this
    /// element mapper, if one has been set.
    /// @return Source identifier associated with this element mapper if one has been set.
    inline std::optional<uint32_t> GetSourceIdentifier(void) const
    {
      return expectedSourceIdentifier;
    }

    // IElementMapper
    std::unique_ptr<IElementMapper> Clone(void) const override
    {
      return std::make_unique<MockElementMapper>(*this);
    }

    void ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const override
    {
      if (EExpectedSource::Analog != maybeExpectedSource.value_or(EExpectedSource::Analog))
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong value source (expected enumerator %d, got Analog).",
            (int)maybeExpectedSource.value());

      if (maybeExpectedValue.value_or(analogValue).analog != analogValue)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong analog value (expected %d, got %d).",
            (int)maybeExpectedValue.value().analog,
            (int)analogValue);

      if (false == expectedSourceIdentifier.has_value())
        const_cast<MockElementMapper*>(this)->expectedSourceIdentifier = sourceIdentifier;
      else if (expectedSourceIdentifier.value() != sourceIdentifier)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong source identifier for analog contribution (expected %u, got %u).",
            (unsigned int)expectedSourceIdentifier.value(),
            (unsigned int)sourceIdentifier);

      if (nullptr != contributionCounter) *contributionCounter += 1;
    }

    void ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const override
    {
      if (EExpectedSource::Button != maybeExpectedSource.value_or(EExpectedSource::Button))
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong value source (expected enumerator %d, got Button).",
            (int)maybeExpectedSource.value());

      if (maybeExpectedValue.value_or(buttonPressed).button != buttonPressed)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong button value (expected %s, got %s).",
            (true == maybeExpectedValue.value().button ? L"'true (pressed)'"
                                                       : L"'false (not pressed)'"),
            (true == buttonPressed ? L"'true (pressed)'" : L"'false (not pressed)'"));

      if (false == expectedSourceIdentifier.has_value())
        const_cast<MockElementMapper*>(this)->expectedSourceIdentifier = sourceIdentifier;
      else if (expectedSourceIdentifier.value() != sourceIdentifier)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong source identifier for button contribution (expected %u, got %u).",
            (unsigned int)expectedSourceIdentifier.value(),
            (unsigned int)sourceIdentifier);

      if (nullptr != contributionCounter) *contributionCounter += 1;
    }

    void ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const override
    {
      if (EExpectedSource::Trigger != maybeExpectedSource.value_or(EExpectedSource::Trigger))
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong value source (expected enumerator %d, got Trigger).",
            (int)maybeExpectedSource.value());

      if (maybeExpectedValue.value_or(triggerValue).trigger != triggerValue)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong trigger value (expected %d, got %d).",
            (int)maybeExpectedValue.value().trigger,
            (int)triggerValue);

      if (false == expectedSourceIdentifier.has_value())
        const_cast<MockElementMapper*>(this)->expectedSourceIdentifier = sourceIdentifier;
      else if (expectedSourceIdentifier.value() != sourceIdentifier)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong source identifier for trigger contribution (expected %u, got %u).",
            (unsigned int)expectedSourceIdentifier.value(),
            (unsigned int)sourceIdentifier);

      if (nullptr != contributionCounter) *contributionCounter += 1;
    }

    void ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const override
    {
      // Neutral contributions are non-destructive.
      // Some element mappers will forward these to sub-element mappers, so unless explicitly
      // testing for neutral contributions they can largely be ignored. The exception is if no
      // contributions whatsoever are expected, in which case any contributions are errors.

      if (EExpectedSource::None == maybeExpectedSource)
        TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected None, got Neutral).");

      if (false == expectedSourceIdentifier.has_value())
        const_cast<MockElementMapper*>(this)->expectedSourceIdentifier = sourceIdentifier;
      else if (expectedSourceIdentifier.value() != sourceIdentifier)
        TEST_FAILED_BECAUSE(
            L"MockElementMapper: wrong source identifier for neutral contribution (expected %u, got %u).",
            (unsigned int)expectedSourceIdentifier.value(),
            (unsigned int)sourceIdentifier);

      if (EExpectedSource::Neutral == maybeExpectedSource)
      {
        if (nullptr != contributionCounter) *contributionCounter += 1;
      }
    }

    int GetTargetElementCount(void) const override
    {
      return (int)fakeTargetElements.size();
    }

    std::optional<SElementIdentifier> GetTargetElementAt(int index) const override
    {
      if ((size_t)index < fakeTargetElements.size()) return fakeTargetElements[index];

      return std::nullopt;
    }

  private:

    /// Specifies the expected source of an input value.
    /// Causes a test to fail if the wrong `ContributeFrom` method is invoked on this object.
    /// Can be empty if not testing this functionality.
    const std::optional<EExpectedSource> maybeExpectedSource;

    /// Specifies the expected input value, one for each allowed type.
    /// Which member is valid is determined by the value of #maybeExpectedSource.
    /// Can be empty if not testing this functionality.
    const std::optional<UExpectedValue> maybeExpectedValue;

    /// Holds the address of a counter that is incremented by 1 whenever this element mapper is
    /// asked for a contribution.
    int* const contributionCounter;

    /// Holds the fake list of target elements.
    const std::vector<SElementIdentifier> fakeTargetElements;

    /// Holds the expected source identifier.
    /// Set either at construction time or the first time this element mapper is asked for a
    /// contribution.
    std::optional<uint32_t> expectedSourceIdentifier;
  };
} // namespace XidiTest
