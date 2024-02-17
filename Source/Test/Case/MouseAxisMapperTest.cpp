/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file MouseAxisMapperTest.cpp
 *   Unit tests for controller element mappers that contribute movement of a virtual mouse along
 *   a virtual mouse axis.
 **************************************************************************************************/

#include "TestCase.h"

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "MockMouse.h"
#include "Mouse.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;
  using ::Xidi::Mouse::EMouseAxis;

  /// Opaque source identifier used for many tests in this file.
  static constexpr uint32_t kOpaqueSourceIdentifier = 1234;

  // Creates one mouse axis mapper for various possible mouse axes and verifies two things.
  // First, verifies that it does not map to any virtual controller element.
  // Second, verifies that it correctly identifies its target mouse axis.
  TEST_CASE(MouseAxisMapper_GetTargetElement_Nominal)
  {
    constexpr EMouseAxis kTestMouseAxes[] = {
        EMouseAxis::X, EMouseAxis::Y, EMouseAxis::WheelHorizontal, EMouseAxis::WheelVertical};

    for (auto mouseAxis : kTestMouseAxes)
    {
      const MouseAxisMapper mapper(mouseAxis);
      TEST_ASSERT(0 == mapper.GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(0);
      TEST_ASSERT(false == maybeTargetElement.has_value());

      TEST_ASSERT(mapper.GetAxis() == mouseAxis);
    }
  }

  // Creates and then clones one mouse axis mapper for various possible mouse axes and verifies two
  // things. First, verifies that it does not map to any virtual controller element. Second,
  // verifies that it correctly identifies its target mouse axis.
  TEST_CASE(MouseAxisMapper_GetTargetElement_Clone)
  {
    constexpr EMouseAxis kTestMouseAxes[] = {
        EMouseAxis::X, EMouseAxis::Y, EMouseAxis::WheelHorizontal, EMouseAxis::WheelVertical};

    for (auto mouseAxis : kTestMouseAxes)
    {
      const MouseAxisMapper mapperOriginal(mouseAxis);
      const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
      TEST_ASSERT(nullptr != dynamic_cast<MouseAxisMapper*>(mapperClone.get()));
      TEST_ASSERT(0 == mapperClone->GetTargetElementCount());

      const std::optional<SElementIdentifier> maybeTargetElement =
          mapperClone->GetTargetElementAt(0);
      TEST_ASSERT(false == maybeTargetElement.has_value());

      TEST_ASSERT(dynamic_cast<MouseAxisMapper*>(mapperClone.get())->GetAxis() == mouseAxis);
    }
  }

  // Verifies the nominal behavior in which a mouse axis mapper is asked to contribute some
  // arbitrary analog value to a mouse movement axis. Sweeps the entire range of possible analog
  // values.
  TEST_CASE(MouseAxisMapper_ContributeFromAnalogValue_Nominal_EntireAxis)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::Y;
    constexpr MouseAxisMapper mapper(kTargetAxis);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    int lastMouseMovementContribution = INT_MIN;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      SState unusedState;
      mapper.ContributeFromAnalogValue(unusedState, (int16_t)analogValue, kOpaqueSourceIdentifier);

      // Verify that a contribution was registered.
      std::optional<int> maybeMouseMovementContribution =
          mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
      TEST_ASSERT(true == maybeMouseMovementContribution.has_value());

      // Verify that contributions increase monotonically with increasing input analog value.
      int mouseMovementContribution = maybeMouseMovementContribution.value();
      TEST_ASSERT(mouseMovementContribution >= lastMouseMovementContribution);
      lastMouseMovementContribution = mouseMovementContribution;

      // Verify the extremities and the midpoint.
      switch (analogValue)
      {
        case kAnalogValueMin:
          TEST_ASSERT(kMouseMovementUnitsMin == mouseMovementContribution);
          break;

        case kAnalogValueNeutral:
          TEST_ASSERT(kMouseMovementUnitsNeutral == mouseMovementContribution);
          break;

        case kAnalogValueMax:
          TEST_ASSERT(kMouseMovementUnitsMax == mouseMovementContribution);
          break;

        default:
          break;
      }
    }
  }

  // Same as above, but for a half axis in the positive direction.
  TEST_CASE(MouseAxisMapper_ContributeFromAnalogValue_Nominal_HalfAxisPositive)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::X;
    constexpr MouseAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    int lastMouseMovementContribution = INT_MIN;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      SState unusedState;
      mapper.ContributeFromAnalogValue(unusedState, (int16_t)analogValue, kOpaqueSourceIdentifier);

      // Verify that a contribution was registered.
      std::optional<int> maybeMouseMovementContribution =
          mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
      TEST_ASSERT(true == maybeMouseMovementContribution.has_value());

      // Verify that contributions increase monotonically with increasing input analog value.
      int mouseMovementContribution = maybeMouseMovementContribution.value();
      TEST_ASSERT(mouseMovementContribution >= lastMouseMovementContribution);
      lastMouseMovementContribution = mouseMovementContribution;

      // Verify the extremities and the midpoint.
      switch (analogValue)
      {
        case kAnalogValueMin:
          TEST_ASSERT(kMouseMovementUnitsNeutral == mouseMovementContribution);
          break;

        case kAnalogValueNeutral:
          TEST_ASSERT(
              ((kMouseMovementUnitsNeutral + kMouseMovementUnitsMax) / 2) ==
              mouseMovementContribution);
          break;

        case kAnalogValueMax:
          TEST_ASSERT(kMouseMovementUnitsMax == mouseMovementContribution);
          break;

        default:
          break;
      }
    }
  }

  // Same as above, but for a half axis in the negative direction.
  TEST_CASE(MouseAxisMapper_ContributeFromAnalogValue_Nominal_HalfAxisNegative)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::X;
    constexpr MouseAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    int lastMouseMovementContribution = INT_MIN;

    for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
    {
      SState unusedState;
      mapper.ContributeFromAnalogValue(unusedState, (int16_t)analogValue, kOpaqueSourceIdentifier);

      // Verify that a contribution was registered.
      std::optional<int> maybeMouseMovementContribution =
          mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
      TEST_ASSERT(true == maybeMouseMovementContribution.has_value());

      // Verify that contributions increase monotonically with increasing input analog value.
      int mouseMovementContribution = maybeMouseMovementContribution.value();
      TEST_ASSERT(mouseMovementContribution >= lastMouseMovementContribution);
      lastMouseMovementContribution = mouseMovementContribution;

      // Verify the extremities and the midpoint.
      switch (analogValue)
      {
        case kAnalogValueMin:
          TEST_ASSERT(kMouseMovementUnitsMin == mouseMovementContribution);
          break;

        case kAnalogValueNeutral:
          TEST_ASSERT(
              ((kMouseMovementUnitsMin + kMouseMovementUnitsNeutral) / 2) ==
              mouseMovementContribution);
          break;

        case kAnalogValueMax:
          TEST_ASSERT(kMouseMovementUnitsNeutral == mouseMovementContribution);
          break;

        default:
          break;
      }
    }
  }

  // Verifies the nominal behavior in which a mouse axis mapper is asked to contribute some
  // arbitrary button press state to a mouse movement axis.
  TEST_CASE(MouseAxisMapper_ContributeFromButtonValue_Nominal_EntireAxis)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::WheelHorizontal;
    constexpr MouseAxisMapper mapper(kTargetAxis);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    SState unusedState;
    std::optional<int> maybeMouseMovementContribution;

    // Verify that motion is in the negative direction when the button is not pressed.
    mapper.ContributeFromButtonValue(unusedState, false, kOpaqueSourceIdentifier);
    maybeMouseMovementContribution =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
    TEST_ASSERT(true == maybeMouseMovementContribution.has_value());
    TEST_ASSERT(maybeMouseMovementContribution.value() < kMouseMovementUnitsNeutral);
    const int positiveMovementOffset =
        maybeMouseMovementContribution.value() - kMouseMovementUnitsNeutral;

    // Verify that motion is in the positive direction when the button is pressed.
    mapper.ContributeFromButtonValue(unusedState, true, kOpaqueSourceIdentifier);
    maybeMouseMovementContribution =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
    TEST_ASSERT(true == maybeMouseMovementContribution.has_value());
    TEST_ASSERT(maybeMouseMovementContribution.value() > kMouseMovementUnitsNeutral);
    const int negativeMovementOffset =
        kMouseMovementUnitsNeutral - maybeMouseMovementContribution.value();

    // Verify that motions in both directions are equal in magnitude in response to a button press.
    TEST_ASSERT(positiveMovementOffset == negativeMovementOffset);
  }

  // Same as above, but for a half axis in the positive direction.
  TEST_CASE(MouseAxisMapper_ContributeFromButtonValue_Nominal_HalfAxisPositive)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::WheelVertical;
    constexpr MouseAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    SState unusedState;
    std::optional<int> maybeMouseMovementContribution;

    // Verify that there is no motion at all when the button is not pressed.
    mapper.ContributeFromButtonValue(unusedState, false, kOpaqueSourceIdentifier);
    maybeMouseMovementContribution =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
    TEST_ASSERT(true == maybeMouseMovementContribution.has_value());
    TEST_ASSERT(maybeMouseMovementContribution.value() == kMouseMovementUnitsNeutral);

    // Verify that motion is in the positive direction when the button is pressed.
    mapper.ContributeFromButtonValue(unusedState, true, kOpaqueSourceIdentifier);
    maybeMouseMovementContribution =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
    TEST_ASSERT(true == maybeMouseMovementContribution.has_value());
    TEST_ASSERT(maybeMouseMovementContribution.value() > kMouseMovementUnitsNeutral);
  }

  // Same as above, but for a half axis in the negative direction.
  TEST_CASE(MouseAxisMapper_ContributeFromButtonValue_Nominal_HalfAxisNegative)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::X;
    constexpr MouseAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    SState unusedState;
    std::optional<int> maybeMouseMovementContribution;

    // Verify that there is no motion at all when the button is not pressed.
    mapper.ContributeFromButtonValue(unusedState, false, kOpaqueSourceIdentifier);
    maybeMouseMovementContribution =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
    TEST_ASSERT(true == maybeMouseMovementContribution.has_value());
    TEST_ASSERT(maybeMouseMovementContribution.value() == kMouseMovementUnitsNeutral);

    // Verify that motion is in the negative direction when the button is pressed.
    mapper.ContributeFromButtonValue(unusedState, true, kOpaqueSourceIdentifier);
    maybeMouseMovementContribution =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
    TEST_ASSERT(true == maybeMouseMovementContribution.has_value());
    TEST_ASSERT(maybeMouseMovementContribution.value() < kMouseMovementUnitsNeutral);
  }

  // Verifies that a mouse axis mapper produces the same magnitude of motion in both directions when
  // in half-axis mode.
  TEST_CASE(MouseAxisMapper_ContributeFromButtonValue_HalfAxisEqualMagnitude)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::Y;
    constexpr MouseAxisMapper mapperPositive(kTargetAxis, EAxisDirection::Positive);
    constexpr MouseAxisMapper mapperNegative(kTargetAxis, EAxisDirection::Negative);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    SState unusedState;
    std::optional<int> maybeMouseMovementContribution;

    mapperPositive.ContributeFromButtonValue(unusedState, true, kOpaqueSourceIdentifier);
    const int positiveMagnitude =
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier).value() -
        kMouseMovementUnitsNeutral;

    mapperNegative.ContributeFromButtonValue(unusedState, true, kOpaqueSourceIdentifier);
    const int negativeMagnitude = kMouseMovementUnitsNeutral -
        mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier).value();

    // Verify that the magnitudes in each direction are the same.
    TEST_ASSERT(positiveMagnitude == negativeMagnitude);
  }

  // Verifies the nominal behavior in which a mouse axis mapper is asked to contribute some
  // arbitrary trigger value to a mouse movement axis. Sweeps the entire range of possible trigger
  // values.
  TEST_CASE(MouseAxisMapper_ContributeFromTriggerValue_Nominal_EntireAxis)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::WheelHorizontal;
    constexpr MouseAxisMapper mapper(kTargetAxis);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    int lastMouseMovementContribution = INT_MIN;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      SState unusedState;
      mapper.ContributeFromTriggerValue(
          unusedState, (uint8_t)triggerValue, kOpaqueSourceIdentifier);

      // Verify that a contribution was registered.
      std::optional<int> maybeMouseMovementContribution =
          mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
      TEST_ASSERT(true == maybeMouseMovementContribution.has_value());

      // Verify that contributions increase monotonically with increasing input analog value.
      int mouseMovementContribution = maybeMouseMovementContribution.value();
      TEST_ASSERT(mouseMovementContribution >= lastMouseMovementContribution);
      lastMouseMovementContribution = mouseMovementContribution;

      // Verify the extremities. Triggers do not have an explicitly-defined midpoint.
      switch (triggerValue)
      {
        case kTriggerValueMin:
          TEST_ASSERT(kMouseMovementUnitsMin == mouseMovementContribution);
          break;

        case kTriggerValueMax:
          TEST_ASSERT(kMouseMovementUnitsMax == mouseMovementContribution);
          break;

        default:
          break;
      }
    }
  }

  // Same as above, but for a half axis in the positive direction.
  TEST_CASE(MouseAxisMapper_ContributeFromTriggerValue_Nominal_HalfAxisPositive)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::WheelHorizontal;
    constexpr MouseAxisMapper mapper(kTargetAxis, EAxisDirection::Positive);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    int lastMouseMovementContribution = INT_MIN;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      SState unusedState;
      mapper.ContributeFromTriggerValue(
          unusedState, (uint8_t)triggerValue, kOpaqueSourceIdentifier);

      // Verify that a contribution was registered.
      std::optional<int> maybeMouseMovementContribution =
          mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
      TEST_ASSERT(true == maybeMouseMovementContribution.has_value());

      // Verify that contributions increase monotonically with increasing input analog value.
      int mouseMovementContribution = maybeMouseMovementContribution.value();
      TEST_ASSERT(mouseMovementContribution >= lastMouseMovementContribution);
      lastMouseMovementContribution = mouseMovementContribution;

      // Verify the extremities. Triggers do not have an explicitly-defined midpoint.
      switch (triggerValue)
      {
        case kTriggerValueMin:
          TEST_ASSERT(kMouseMovementUnitsNeutral == mouseMovementContribution);
          break;

        case kTriggerValueMax:
          TEST_ASSERT(kMouseMovementUnitsMax == mouseMovementContribution);
          break;

        default:
          break;
      }
    }
  }

  // Same as above, but for a half axis in the negative direction.
  TEST_CASE(MouseAxisMapper_ContributeFromTriggerValue_Nominal_HalfAxisNegative)
  {
    constexpr EMouseAxis kTargetAxis = EMouseAxis::WheelHorizontal;
    constexpr MouseAxisMapper mapper(kTargetAxis, EAxisDirection::Negative);

    MockMouse mockMouse;
    mockMouse.BeginCapture();

    int lastMouseMovementContribution = INT_MAX;

    for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
    {
      SState unusedState;
      mapper.ContributeFromTriggerValue(
          unusedState, (uint8_t)triggerValue, kOpaqueSourceIdentifier);

      // Verify that a contribution was registered.
      std::optional<int> maybeMouseMovementContribution =
          mockMouse.GetMovementContributionFromSource(kTargetAxis, kOpaqueSourceIdentifier);
      TEST_ASSERT(true == maybeMouseMovementContribution.has_value());

      // Verify that contributions decrease monotonically with increasing input analog value.
      // Triggers assigned to half-axes are special cases. All other test cases check for monotonic
      // increase.
      int mouseMovementContribution = maybeMouseMovementContribution.value();
      TEST_ASSERT(mouseMovementContribution <= lastMouseMovementContribution);
      lastMouseMovementContribution = mouseMovementContribution;

      // Verify the extremities. Triggers do not have an explicitly-defined midpoint.
      switch (triggerValue)
      {
        case kTriggerValueMin:
          TEST_ASSERT(kMouseMovementUnitsNeutral == mouseMovementContribution);
          break;

        case kTriggerValueMax:
          TEST_ASSERT(kMouseMovementUnitsMin == mouseMovementContribution);
          break;

        default:
          break;
      }
    }
  }
} // namespace XidiTest
