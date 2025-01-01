/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MapperTest.cpp
 *   Unit tests for entire controller layout mapper objects.
 **************************************************************************************************/

#include "Mapper.h"

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <unordered_set>
#include <utility>

#include <Infra/Test/TestCase.h>

#include "ApiBitSet.h"
#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "ForceFeedbackTypes.h"
#include "MockElementMapper.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;
  using ::Xidi::Controller::ForceFeedback::EActuatorMode;
  using ::Xidi::Controller::ForceFeedback::SPhysicalActuatorComponents;
  using ::Xidi::Controller::ForceFeedback::TEffectValue;
  using ::Xidi::Controller::ForceFeedback::TOrderedMagnitudeComponents;
  using ::Xidi::Controller::ForceFeedback::TPhysicalActuatorValue;

  /// Opaque source identifier used for many mapper tests in this file.
  static constexpr uint32_t kOpaqueSourceIdentifier = 100;

  /// Mapper with a mock element mapper on every possible controller element.
  /// Does not check for specific contributions.
  /// For use as a template in test cases.
  static const Mapper kFullyMockedMapper(
      {.stickLeftX = std::make_unique<MockElementMapper>(),
       .stickLeftY = std::make_unique<MockElementMapper>(),
       .stickRightX = std::make_unique<MockElementMapper>(),
       .stickRightY = std::make_unique<MockElementMapper>(),
       .dpadUp = std::make_unique<MockElementMapper>(),
       .dpadDown = std::make_unique<MockElementMapper>(),
       .dpadLeft = std::make_unique<MockElementMapper>(),
       .dpadRight = std::make_unique<MockElementMapper>(),
       .triggerLT = std::make_unique<MockElementMapper>(),
       .triggerRT = std::make_unique<MockElementMapper>(),
       .buttonA = std::make_unique<MockElementMapper>(),
       .buttonB = std::make_unique<MockElementMapper>(),
       .buttonX = std::make_unique<MockElementMapper>(),
       .buttonY = std::make_unique<MockElementMapper>(),
       .buttonLB = std::make_unique<MockElementMapper>(),
       .buttonRB = std::make_unique<MockElementMapper>(),
       .buttonBack = std::make_unique<MockElementMapper>(),
       .buttonStart = std::make_unique<MockElementMapper>(),
       .buttonLS = std::make_unique<MockElementMapper>(),
       .buttonRS = std::make_unique<MockElementMapper>()});

  /// Creates a button set given a compile-time-constant list of buttons.
  /// @param [in] buttons Initializer list containing all of the desired buttons to be added to the
  /// set.
  /// @return Button set representation of the button list.
  static constexpr std::bitset<(int)EPhysicalButton::Count> ButtonSet(
      std::initializer_list<EPhysicalButton> buttons)
  {
    std::bitset<(int)EPhysicalButton::Count> buttonSet;

    for (auto button : buttons)
      buttonSet[(int)button] = true;

    return buttonSet;
  }

  /// Computes the expected physical actuator value given an input virtual actuator value.
  /// @param [in] virtualValue Virtual actuator value for which a translation to physical is
  /// desired.
  /// @return Translated physical actuator value.
  static TPhysicalActuatorValue ForceFeedbackActuatorValueVirtualToPhysical(
      TEffectValue virtualValue, TEffectValue gain = ForceFeedback::kEffectForceMagnitudeMaximum)
  {
    constexpr double kScalingFactor =
        (std::numeric_limits<TPhysicalActuatorValue>::max() / 10000.0);
    const double gainMultiplier = gain / 10000.0;

    const long physicalValue =
        std::lround(gainMultiplier * std::abs(virtualValue * kScalingFactor));

    if (physicalValue >= std::numeric_limits<TPhysicalActuatorValue>::max())
      return (TPhysicalActuatorValue)std::numeric_limits<TPhysicalActuatorValue>::max();

    return (TPhysicalActuatorValue)physicalValue;
  }

  /// Generates and returns the minimal representation of a virtual controller's capabilities.
  /// @return Minimal capabilities structure.
  static consteval SCapabilities MinimalCapabilities(void)
  {
    SCapabilities minCapabilities = {
        .numButtons = Mapper::kMinNumButtons, .hasPov = Mapper::kIsPovRequired};

    for (auto requiredAxis : Mapper::kRequiredAxes | Mapper::kRequiredForceFeedbackAxes)
      minCapabilities.AppendAxis(
          {.type = (EAxis)((int)requiredAxis),
           .supportsForceFeedback = Mapper::kRequiredForceFeedbackAxes.contains(requiredAxis)});

    return minCapabilities;
  }

  /// Generates a complete expected capabilities structure by accepting a base expected capabilities
  /// from a test case and merging it with the minimum required virtual controller capabilities.
  /// Virtual controllers are required to have at least certain axes and a minimum number of
  /// buttons.
  /// @param [in] baseExpectedCapabilities Expected capabilities from the test case.
  /// @return Expected capabilities from the test case merged with the minimum allowed capabilities.
  static consteval SCapabilities MakeExpectedCapabilities(SCapabilities baseExpectedCapabilities)
  {
    const SCapabilities minCapabilities = MinimalCapabilities();

    SCapabilities expectedCapabilities = {
        .numButtons = std::max(minCapabilities.numButtons, baseExpectedCapabilities.numButtons),
        .hasPov = (minCapabilities.HasPov() || baseExpectedCapabilities.HasPov())};
    for (int i = 0; i < (int)EAxis::Count; ++i)
    {
      const bool supportsForceFeedback =
          (baseExpectedCapabilities.ForceFeedbackIsSupportedForAxis((EAxis)i) ||
           minCapabilities.ForceFeedbackIsSupportedForAxis((EAxis)i));

      if (baseExpectedCapabilities.HasAxis((EAxis)i))
        expectedCapabilities.AppendAxis(
            {.type = baseExpectedCapabilities
                         .axisCapabilities[baseExpectedCapabilities.FindAxis((EAxis)i)]
                         .type,
             .supportsForceFeedback = supportsForceFeedback});
      else if (minCapabilities.HasAxis((EAxis)i))
        expectedCapabilities.AppendAxis(
            minCapabilities.axisCapabilities[minCapabilities.FindAxis((EAxis)i)]);
    }

    return expectedCapabilities;
  }

  // Verifies that opaque source identifiers are always the same for the same controller and
  // controller element, regardless of how the mapping takes place and regardless of which specific
  // mapper object is used.
  TEST_CASE(Mapper_OpaqueSourceIdentifier_SameAcrossMappingAttempts)
  {
    const Mapper kTestMappers[] = {
        kFullyMockedMapper,
        kFullyMockedMapper,
        kFullyMockedMapper,
        kFullyMockedMapper,
        kFullyMockedMapper};

    std::unordered_set<uint32_t> seenSourceIdentifiers;

    for (const auto& testMapper : kTestMappers)
    {
      for (int mappingIter = 0; mappingIter < 10; ++mappingIter)
      {
        // Test will fail if any individual element mapper gets a different opaque source identifier
        // between mapping attempts. If that happens it means that the same controller element on
        // the same controller got a different opaque source identifier, which violates the
        // guarantee about opaque source identifiers.
        testMapper.MapStatePhysicalToVirtual({}, kOpaqueSourceIdentifier);
        testMapper.MapNeutralPhysicalToVirtual(kOpaqueSourceIdentifier);
      }
    }

    // Scanning element-by-element through the element map should show the same opaque source
    // identifier for each element across all the mapper objects.
    for (uint32_t elementMapIdx = 0; elementMapIdx < _countof(Mapper::UElementMap::all);
         ++elementMapIdx)
    {
      const uint32_t expectedSourceIdentifier =
          static_cast<const MockElementMapper*>(
              kTestMappers[0].ElementMap().all[elementMapIdx].get())
              ->GetSourceIdentifier()
              .value();

      for (const auto& testMapper : kTestMappers)
      {
        const uint32_t actualSourceIdentifier =
            static_cast<const MockElementMapper*>(testMapper.ElementMap().all[elementMapIdx].get())
                ->GetSourceIdentifier()
                .value();
        TEST_ASSERT(actualSourceIdentifier == expectedSourceIdentifier);
      }
    }
  }

  /// Verifies that all opaque source identifiers on the same controller but for different elements
  /// are different.
  TEST_CASE(Mapper_OpaqueSourceIdentifier_DifferentAcrossControllerElements)
  {
    const Mapper testMapper(kFullyMockedMapper);
    testMapper.MapNeutralPhysicalToVirtual(kOpaqueSourceIdentifier);

    std::unordered_set<uint32_t> seenSourceIdentifiers;

    const Mapper::UElementMap& testMapperElementMap = testMapper.ElementMap();
    for (uint32_t elementMapIdx = 0; elementMapIdx < _countof(testMapperElementMap.all);
         ++elementMapIdx)
    {
      if (nullptr != testMapperElementMap.all[elementMapIdx])
      {
        const uint32_t sourceIdentifier =
            static_cast<const MockElementMapper*>(testMapperElementMap.all[elementMapIdx].get())
                ->GetSourceIdentifier()
                .value();

        // Every time through this loop there should be a different opaque source identifier.
        // Any duplicates will not cause an insertion into the set, so the number of actual items in
        // the set will be less than expected by the end.
        const bool sourceIdentifierIsUnique = seenSourceIdentifiers.insert(sourceIdentifier).second;
        TEST_ASSERT(true == sourceIdentifierIsUnique);
      }
    }
  }

  /// Verifies that opaque source identifiers are different across different controllers, even if
  /// the controller element is the same.
  TEST_CASE(Mapper_OpaqueSourceIdentifier_DifferentAcrossControllers)
  {
    const struct
    {
      Mapper mapper;
      uint32_t opaqueControllerIdentifier;
    } kTestRecords[] = {
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 0},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 1},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 2},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 3},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 4},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 100},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 2000},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 3033},
        {.mapper = kFullyMockedMapper, .opaqueControllerIdentifier = 456789}};

    std::unordered_set<uint32_t> seenSourceIdentifiers;

    for (const auto& testRecord : kTestRecords)
    {
      // Sets the opaque source identifier within each individual test mapper.
      // Since the opaque controller identifier is different these whould all produce different
      // values.
      testRecord.mapper.MapNeutralPhysicalToVirtual(testRecord.opaqueControllerIdentifier);

      const Mapper::UElementMap& testMapperElementMap = testRecord.mapper.ElementMap();
      for (uint32_t elementMapIdx = 0; elementMapIdx < _countof(testMapperElementMap.all);
           ++elementMapIdx)
      {
        if (nullptr != testMapperElementMap.all[elementMapIdx])
        {
          const uint32_t sourceIdentifier =
              static_cast<const MockElementMapper*>(testMapperElementMap.all[elementMapIdx].get())
                  ->GetSourceIdentifier()
                  .value();

          // Every time through this loop there should be a different opaque source identifier.
          // Any duplicates will not cause an insertion into the set, so the number of actual items
          // in the set will be less than expected by the end.
          const bool sourceIdentifierIsUnique =
              seenSourceIdentifiers.insert(sourceIdentifier).second;
          TEST_ASSERT(true == sourceIdentifierIsUnique);
        }
      }
    }
  }

  // In this context, "route" means that the correct element mapper is invoked with the correct
  // value source (analog for left and right stick axes, trigger for LT and RT, and buttons for all
  // controller buttons including the d-pad).

  // Left stick, horizontal
  TEST_CASE(Mapper_Route_StickLeftX)
  {
    constexpr int16_t kTestValue = 1111;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.stickLeftX = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .stick = {kTestValue, 0, 0, 0}},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Left stick, vertical
  TEST_CASE(Mapper_Route_StickLeftY)
  {
    constexpr int16_t kTestValue = 2233;
    constexpr int16_t kInvertedTestValue = -kTestValue;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.stickLeftY = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Analog, kInvertedTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .stick = {0, kTestValue, 0, 0}},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Right stick, horizontal
  TEST_CASE(Mapper_Route_StickRightX)
  {
    constexpr int16_t kTestValue = 4556;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.stickRightX = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .stick = {0, 0, kTestValue, 0}},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Right stick, vertical
  TEST_CASE(Mapper_Route_StickRightY)
  {
    constexpr int16_t kTestValue = 6789;
    constexpr int16_t kInvertedTestValue = -kTestValue;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.stickRightY = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Analog, kInvertedTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .stick = {0, 0, 0, kTestValue}},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // D-pad up
  TEST_CASE(Mapper_Route_DpadUp)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.dpadUp = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::DpadUp})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // D-pad down
  TEST_CASE(Mapper_Route_DpadDown)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.dpadDown = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .button = ButtonSet({EPhysicalButton::DpadDown})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // D-pad left
  TEST_CASE(Mapper_Route_DpadLeft)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.dpadLeft = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .button = ButtonSet({EPhysicalButton::DpadLeft})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // D-pad right
  TEST_CASE(Mapper_Route_DpadRight)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.dpadRight = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .button = ButtonSet({EPhysicalButton::DpadRight})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Left trigger (LT)
  TEST_CASE(Mapper_Route_TriggerLT)
  {
    constexpr uint8_t kTestValue = 45;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.triggerLT = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Trigger, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .trigger = {kTestValue, 0}},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Right trigger (RT)
  TEST_CASE(Mapper_Route_TriggerRT)
  {
    constexpr uint8_t kTestValue = 167;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.triggerRT = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Trigger, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .trigger = {0, kTestValue}},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // A button
  TEST_CASE(Mapper_Route_ButtonA)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonA = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::A})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // B button
  TEST_CASE(Mapper_Route_ButtonB)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonB = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::B})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // X button
  TEST_CASE(Mapper_Route_ButtonX)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonX = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::X})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Y button
  TEST_CASE(Mapper_Route_ButtonY)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonY = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::Y})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // LB button
  TEST_CASE(Mapper_Route_ButtonLB)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonLB = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::LB})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // RB button
  TEST_CASE(Mapper_Route_ButtonRB)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonRB = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::RB})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Back button
  TEST_CASE(Mapper_Route_ButtonBack)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonBack = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::Back})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Start button
  TEST_CASE(Mapper_Route_ButtonStart)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonStart = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::Start})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // LS button
  TEST_CASE(Mapper_Route_ButtonLS)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonLS = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::LS})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // RS button
  TEST_CASE(Mapper_Route_ButtonRS)
  {
    constexpr bool kTestValue = true;
    int numContributions = 0;

    const Mapper controllerMapper(
        {.buttonRS = std::make_unique<MockElementMapper>(
             MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
    controllerMapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok, .button = ButtonSet({EPhysicalButton::RS})},
        kOpaqueSourceIdentifier);

    TEST_ASSERT(1 == numContributions);
  }

  // Empty mapper.
  // Nothing should be present on the virtual controller.
  TEST_CASE(Mapper_Capabilities_EmptyMapper)
  {
    constexpr SCapabilities expectedCapabilities =
        MakeExpectedCapabilities({.numAxes = 0, .numButtons = 0, .hasPov = false});

    const Mapper mapper({
        // Empty.
    });

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Null mapper.
  // Nothing should be present on the virtual controller.
  TEST_CASE(Mapper_Capabilities_NullMapper)
  {
    constexpr SCapabilities expectedCapabilities =
        MakeExpectedCapabilities({.numAxes = 0, .numButtons = 0, .hasPov = false});

    const Mapper* mapper = Mapper::GetNull();

    const SCapabilities actualCapabilities = mapper->GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper with only buttons, and they are disjoint.
  // Virtual controller should have only buttons, and the number present is based on the highest
  // button to which an element mapper writes.
  TEST_CASE(Mapper_Capabilities_DisjointButtons)
  {
    constexpr SCapabilities expectedCapabilities =
        MakeExpectedCapabilities({.numAxes = 0, .numButtons = 10, .hasPov = false});

    const Mapper mapper(
        {.stickLeftX = std::make_unique<ButtonMapper>(EButton::B2),
         .dpadUp = std::make_unique<ButtonMapper>(EButton::B6),
         .dpadLeft = std::make_unique<ButtonMapper>(EButton::B10),
         .buttonLB = std::make_unique<ButtonMapper>(EButton::B4)});

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper with only buttons, and all mappers write to the same button.
  // Virtual controller should have only buttons, and the number present is based on the button to
  // which all element mappers write.
  TEST_CASE(Mapper_Capabilities_SingleButton)
  {
    constexpr SCapabilities expectedCapabilities =
        MakeExpectedCapabilities({.numAxes = 0, .numButtons = 6, .hasPov = false});

    const Mapper mapper(
        {.stickLeftY = std::make_unique<ButtonMapper>(EButton::B6),
         .dpadDown = std::make_unique<ButtonMapper>(EButton::B6),
         .buttonStart = std::make_unique<ButtonMapper>(EButton::B6)});

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper with only axes.
  // Virtual controller should have only axes based on the axes to which the element mappers write.
  TEST_CASE(Mapper_Capabilities_MultipleAxes)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities = {{.type = EAxis::Y}, {.type = EAxis::RotX}},
         .numAxes = 2,
         .numButtons = 0,
         .hasPov = false});

    const Mapper mapper(
        {.stickRightX = std::make_unique<AxisMapper>(EAxis::Y),
         .dpadDown = std::make_unique<AxisMapper>(EAxis::RotX),
         .buttonStart = std::make_unique<AxisMapper>(EAxis::RotX),
         .buttonRS = std::make_unique<AxisMapper>(EAxis::Y)});

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper with only a POV, and only part of it receives values from mappers.
  // Virtual controller should have only a POV and nothing else.
  TEST_CASE(Mapper_Capabilities_IncompletePov)
  {
    constexpr SCapabilities expectedCapabilities =
        MakeExpectedCapabilities({.numAxes = 0, .numButtons = 0, .hasPov = true});

    const Mapper mapper({.stickRightX = std::make_unique<PovMapper>(EPovDirection::Left)});

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper with only a complete POV.
  // Virtual controller should have only a POV and nothing else.
  TEST_CASE(Mapper_Capabilities_CompletePov)
  {
    constexpr SCapabilities expectedCapabilities =
        MakeExpectedCapabilities({.numAxes = 0, .numButtons = 0, .hasPov = true});

    const Mapper mapper(
        {.stickLeftY = std::make_unique<PovMapper>(EPovDirection::Left),
         .stickRightX = std::make_unique<PovMapper>(EPovDirection::Right),
         .triggerLT = std::make_unique<PovMapper>(EPovDirection::Up),
         .triggerRT = std::make_unique<PovMapper>(EPovDirection::Down),
         .buttonA = std::make_unique<PovMapper>(EPovDirection::Left),
         .buttonY = std::make_unique<PovMapper>(EPovDirection::Left),
         .buttonLS = std::make_unique<PovMapper>(EPovDirection::Up),
         .buttonRS = std::make_unique<PovMapper>(EPovDirection::Down)});

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper with multiple virtual elements all coming from the same XInput controller element using
  // a SplitMapper. Virtual controller should report the presence of all parts to which the
  // SplitMapper contributes.
  TEST_CASE(Mapper_Capabilities_SplitMapper)
  {
    constexpr EAxis kTestAxis = EAxis::Z;
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities = {{.type = kTestAxis}}, .numAxes = 1, .numButtons = 0, .hasPov = true});

    const Mapper mapper(
        {.stickRightY = std::make_unique<SplitMapper>(
             std::make_unique<MockElementMapper>(
                 SElementIdentifier({.type = EElementType::Axis, .axis = kTestAxis})),
             std::make_unique<MockElementMapper>(
                 SElementIdentifier({.type = EElementType::Pov})))});

    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper that is empty except for defining force feedback actuators on an axis using single axis
  // mode. Virtual controller should show that this axis exists but only for force feedback and not
  // for physical controller element input.
  TEST_CASE(Mapper_Capabilities_ForceFeedbackOnly_SingleAxis)
  {
    constexpr EAxis kTestAxis = EAxis::Z;
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities = {{.type = kTestAxis, .supportsForceFeedback = true}},
         .numAxes = 1,
         .numButtons = 0,
         .hasPov = false});

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .rightMotor =
            {.isPresent = true,
             .mode = EActuatorMode::SingleAxis,
             .singleAxis = {.axis = kTestAxis, .direction = EAxisDirection::Negative}},
    };

    const Mapper mapper({}, kTestActuatorMap);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // Mapper that is empty except for defining force feedback actuators on an axis using magnitude
  // projection mode. Virtual controller should show that this axis exists but only for force
  // feedback and not for physical controller element input.
  TEST_CASE(Mapper_Capabilities_ForceFeedbackOnly_MagnitudeProjection)
  {
    constexpr EAxis kTestAxisFirst = EAxis::Z;
    constexpr EAxis kTestAxisSecond = EAxis::RotZ;

    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = kTestAxisFirst, .supportsForceFeedback = true},
              {.type = kTestAxisSecond, .supportsForceFeedback = true}},
         .numAxes = 2,
         .numButtons = 0,
         .hasPov = false});

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .rightMotor =
            {.isPresent = true,
             .mode = EActuatorMode::MagnitudeProjection,
             .magnitudeProjection = {.axisFirst = kTestAxisFirst, .axisSecond = kTestAxisSecond}},
    };

    const Mapper mapper({}, kTestActuatorMap);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // StandardGamepad, a known and documented mapper.
  TEST_CASE(Mapper_Capabilities_StandardGamepad)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotZ}},
         .numAxes = 4,
         .numButtons = 12,
         .hasPov = true});

    const Mapper* const mapper = Mapper::GetByName(L"StandardGamepad");
    TEST_ASSERT(nullptr != mapper);

    const SCapabilities actualCapabilities = mapper->GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // DigitalGamepad, a known and documented mapper.
  TEST_CASE(Mapper_Capabilities_DigitalGamepad)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotZ}},
         .numAxes = 4,
         .numButtons = 12,
         .hasPov = false});

    const Mapper* const mapper = Mapper::GetByName(L"DigitalGamepad");
    TEST_ASSERT(nullptr != mapper);

    const SCapabilities actualCapabilities = mapper->GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // ExtendedGamepad, a known and documented mapper.
  TEST_CASE(Mapper_Capabilities_ExtendedGamepad)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotX},
              {.type = EAxis::RotY},
              {.type = EAxis::RotZ}},
         .numAxes = 6,
         .numButtons = 10,
         .hasPov = true});

    const Mapper* const mapper = Mapper::GetByName(L"ExtendedGamepad");
    TEST_ASSERT(nullptr != mapper);

    const SCapabilities actualCapabilities = mapper->GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // XInputNative, a known and documented mapper.
  TEST_CASE(Mapper_Capabilities_XInputNative)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotX},
              {.type = EAxis::RotY},
              {.type = EAxis::RotZ}},
         .numAxes = 6,
         .numButtons = 10,
         .hasPov = true});

    const Mapper* const mapper = Mapper::GetByName(L"XInputNative");
    TEST_ASSERT(nullptr != mapper);

    const SCapabilities actualCapabilities = mapper->GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // XInputSharedTriggers, a known and documented mapper.
  TEST_CASE(Mapper_Capabilities_XInputSharedTriggers)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotX},
              {.type = EAxis::RotY}},
         .numAxes = 5,
         .numButtons = 10,
         .hasPov = true});

    const Mapper* const mapper = Mapper::GetByName(L"XInputSharedTriggers");
    TEST_ASSERT(nullptr != mapper);

    const SCapabilities actualCapabilities = mapper->GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // This is the same as the Capabilities suite using known mappers but with clones.

  // StandardGamepad, a known and documented mapper.
  // The X and Y axes are removed.
  TEST_CASE(Mapper_Clone_StandardGamepad)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotZ}},
         .numAxes = 4,
         .numButtons = 12,
         .hasPov = true});

    Mapper::UElementMap clonedElementMap = Mapper::GetByName(L"StandardGamepad")->CloneElementMap();
    Mapper::UForceFeedbackActuatorMap clonedForceFeedbackActuatorMap =
        Mapper::GetByName(L"StandardGamepad")->GetForceFeedbackActuatorMap();
    clonedElementMap.named.stickLeftX = nullptr;
    clonedElementMap.named.stickLeftY = nullptr;

    const Mapper mapper(std::move(clonedElementMap.named), clonedForceFeedbackActuatorMap.named);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // DigitalGamepad, a known and documented mapper.
  // The Z and RotZ axes are removed.
  TEST_CASE(Mapper_Clone_DigitalGamepad)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true}},
         .numAxes = 2,
         .numButtons = 12,
         .hasPov = false});

    Mapper::UElementMap clonedElementMap = Mapper::GetByName(L"DigitalGamepad")->CloneElementMap();
    Mapper::UForceFeedbackActuatorMap clonedForceFeedbackActuatorMap =
        Mapper::GetByName(L"DigitalGamepad")->GetForceFeedbackActuatorMap();
    clonedElementMap.named.stickRightX = nullptr;
    clonedElementMap.named.stickRightY = nullptr;

    const Mapper mapper(std::move(clonedElementMap.named), clonedForceFeedbackActuatorMap.named);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // ExtendedGamepad, a known and documented mapper.
  // The RotX and RotY axes are removed.
  TEST_CASE(Mapper_Clone_ExtendedGamepad)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotZ}},
         .numAxes = 4,
         .numButtons = 10,
         .hasPov = true});

    Mapper::UElementMap clonedElementMap = Mapper::GetByName(L"ExtendedGamepad")->CloneElementMap();
    Mapper::UForceFeedbackActuatorMap clonedForceFeedbackActuatorMap =
        Mapper::GetByName(L"ExtendedGamepad")->GetForceFeedbackActuatorMap();
    clonedElementMap.named.triggerLT = nullptr;
    clonedElementMap.named.triggerRT = nullptr;

    const Mapper mapper(std::move(clonedElementMap.named), clonedForceFeedbackActuatorMap.named);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // XInputNative, a known and documented mapper.
  // The POV is removed.
  TEST_CASE(Mapper_Clone_XInputNative)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::Z},
              {.type = EAxis::RotX},
              {.type = EAxis::RotY},
              {.type = EAxis::RotZ}},
         .numAxes = 6,
         .numButtons = 10,
         .hasPov = false});

    Mapper::UElementMap clonedElementMap = Mapper::GetByName(L"XInputNative")->CloneElementMap();
    Mapper::UForceFeedbackActuatorMap clonedForceFeedbackActuatorMap =
        Mapper::GetByName(L"XInputNative")->GetForceFeedbackActuatorMap();
    clonedElementMap.named.dpadUp = nullptr;
    clonedElementMap.named.dpadDown = nullptr;
    clonedElementMap.named.dpadLeft = nullptr;
    clonedElementMap.named.dpadRight = nullptr;

    const Mapper mapper(std::move(clonedElementMap.named), clonedForceFeedbackActuatorMap.named);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // XInputSharedTriggers, a known and documented mapper.
  // The Z axis is removed.
  TEST_CASE(Mapper_Clone_XInputSharedTriggers)
  {
    constexpr SCapabilities expectedCapabilities = MakeExpectedCapabilities(
        {.axisCapabilities =
             {{.type = EAxis::X, .supportsForceFeedback = true},
              {.type = EAxis::Y, .supportsForceFeedback = true},
              {.type = EAxis::RotX},
              {.type = EAxis::RotY}},
         .numAxes = 4,
         .numButtons = 10,
         .hasPov = true});

    Mapper::UElementMap clonedElementMap =
        Mapper::GetByName(L"XInputSharedTriggers")->CloneElementMap();
    Mapper::UForceFeedbackActuatorMap clonedForceFeedbackActuatorMap =
        Mapper::GetByName(L"XInputSharedTriggers")->GetForceFeedbackActuatorMap();
    clonedElementMap.named.triggerLT = nullptr;
    clonedElementMap.named.triggerRT = nullptr;

    const Mapper mapper(std::move(clonedElementMap.named), clonedForceFeedbackActuatorMap.named);
    const SCapabilities actualCapabilities = mapper.GetCapabilities();
    TEST_ASSERT(actualCapabilities == expectedCapabilities);
  }

  // The formula for each test case body is create an expected controller state, obtain a mapper,
  // ask it to write to a controller state, and finally compare expected and actual states.

  // An empty mapper is expected to produce all zeroes in its output controller state, irrespective
  // of the XInput controller's state.
  TEST_CASE(Mapper_State_ZeroOnEmpty)
  {
    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));

    const Mapper mapper({
        // Empty
    });

    SState actualState = mapper.MapStatePhysicalToVirtual({}, kOpaqueSourceIdentifier);
    TEST_ASSERT(actualState == expectedState);

    actualState = mapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .stick = {16383, -16383, -16383, 16383},
         .trigger = {128, 128},
         .button = 32767},
        kOpaqueSourceIdentifier);
    TEST_ASSERT(actualState == expectedState);
  }

  // Even though intermediate contributions may result in analog axis values that exceed the allowed
  // range, mappers are expected to saturate at the allowed range. This test verifies correct
  // saturation in the positive direction.
  TEST_CASE(Mapper_State_AnalogSaturationPositive)
  {
    constexpr int32_t kInvertedInputValue = kAnalogValueMin;
    constexpr int32_t kNonInvertedInputValue = kAnalogValueMax;
    constexpr int32_t expectedOutputValue = kAnalogValueMax;

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[EAxis::X] = expectedOutputValue;

    const Mapper mapper(
        {.stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
         .stickLeftY = std::make_unique<AxisMapper>(EAxis::X),
         .stickRightX = std::make_unique<AxisMapper>(EAxis::X),
         .stickRightY = std::make_unique<AxisMapper>(EAxis::X)});

    SState actualState = mapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .stick =
             {kNonInvertedInputValue,
              kInvertedInputValue,
              kNonInvertedInputValue,
              kInvertedInputValue}},
        kOpaqueSourceIdentifier);
    TEST_ASSERT(actualState == expectedState);
  }

  // Even though intermediate contributions may result in analog axis values that exceed the allowed
  // range, mappers are expected to saturate at the allowed range. This test verifies correct
  // saturation in the negative direction.
  TEST_CASE(Mapper_State_AnalogSaturationNegative)
  {
    constexpr int32_t kInvertedInputValue = kAnalogValueMax;
    constexpr int32_t kNonInvertedInputValue = kAnalogValueMin;
    constexpr int32_t expectedOutputValue = kAnalogValueMin;

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[EAxis::RotX] = expectedOutputValue;

    const Mapper mapper(
        {.stickLeftX = std::make_unique<AxisMapper>(EAxis::RotX),
         .stickLeftY = std::make_unique<AxisMapper>(EAxis::RotX),
         .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
         .stickRightY = std::make_unique<AxisMapper>(EAxis::RotX)});

    SState actualState = mapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .stick =
             {kNonInvertedInputValue,
              kInvertedInputValue,
              kNonInvertedInputValue,
              kInvertedInputValue}},
        kOpaqueSourceIdentifier);
    TEST_ASSERT(actualState == expectedState);
  }

  // Incoming controller data uses a range slightly different from virtual controller range.
  // Furthermore, the vertical axes on analog sticks use opposite polarity from what virtual
  // controllers expect and present. Mappers are expected to ensure values are correctly filtered
  // and inverted to compensate.
  TEST_CASE(Mapper_State_AnalogFilterAndInvert)
  {
    constexpr int32_t kExtremeNegativeInputValue = (int32_t)INT16_MIN;
    constexpr int32_t kNonInvertedExpectedOutputValue = kAnalogValueMin;
    constexpr int32_t kInvertedExpectedOutputValue = kAnalogValueMax;

    SState expectedState;
    ZeroMemory(&expectedState, sizeof(expectedState));
    expectedState[EAxis::X] = kNonInvertedExpectedOutputValue;
    expectedState[EAxis::Y] = kInvertedExpectedOutputValue;
    expectedState[EAxis::RotX] = kNonInvertedExpectedOutputValue;
    expectedState[EAxis::RotY] = kInvertedExpectedOutputValue;

    const Mapper mapper(
        {.stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
         .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
         .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
         .stickRightY = std::make_unique<AxisMapper>(EAxis::RotY)});

    SState actualState = mapper.MapStatePhysicalToVirtual(
        {.deviceStatus = EPhysicalDeviceStatus::Ok,
         .stick =
             {kExtremeNegativeInputValue,
              kExtremeNegativeInputValue,
              kExtremeNegativeInputValue,
              kExtremeNegativeInputValue}},
        kOpaqueSourceIdentifier);
    TEST_ASSERT(actualState == expectedState);
  }

  // Nominal case of some actuators mapped in single axis mode and using axes with the default of
  // both directions.
  TEST_CASE(Mapper_ForceFeedback_Nominal_SingleAxis)
  {
    constexpr TOrderedMagnitudeComponents kTestMagnitudeVector = {
        1111, -2222, 3333, -4444, 5555, -6666};

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .leftMotor =
            {.isPresent = true,
             .mode = EActuatorMode::SingleAxis,
             .singleAxis = {.axis = EAxis::X, .direction = EAxisDirection::Both}},
        .rightMotor =
            {.isPresent = true,
             .mode = EActuatorMode::SingleAxis,
             .singleAxis = {.axis = EAxis::Y, .direction = EAxisDirection::Both}},
        .leftImpulseTrigger = {.isPresent = false},
        .rightImpulseTrigger = {
            .isPresent = true,
            .mode = EActuatorMode::SingleAxis,
            .singleAxis = {.axis = EAxis::RotZ, .direction = EAxisDirection::Both}}};

    const SPhysicalActuatorComponents expectedActuatorComponents = {
        .leftMotor =
            ForceFeedbackActuatorValueVirtualToPhysical(kTestMagnitudeVector[(int)EAxis::X]),
        .rightMotor =
            ForceFeedbackActuatorValueVirtualToPhysical(kTestMagnitudeVector[(int)EAxis::Y]),
        .rightImpulseTrigger =
            ForceFeedbackActuatorValueVirtualToPhysical(kTestMagnitudeVector[(int)EAxis::RotZ])};

    const Mapper mapper({}, kTestActuatorMap);

    const SPhysicalActuatorComponents actualActuatorComponents =
        mapper.MapForceFeedbackVirtualToPhysical(kTestMagnitudeVector);
    TEST_ASSERT(actualActuatorComponents == expectedActuatorComponents);
  }

  // Nominal case of some actuators mapped in magnitude projection mode.
  // To keep the math simple, both X and Y axes have the same magnitude components, and these are
  // the axes used in the magnitude projection.
  TEST_CASE(Mapper_ForceFeedback_Nominal_MagnitudeProjection)
  {
    constexpr TOrderedMagnitudeComponents kTestMagnitudeVector = {
        1111, 1111, 2233, 4455, 6677, 8899};
    const TEffectValue kSqrt2 = std::sqrtf(2);

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .leftMotor =
            {.isPresent = true,
             .mode = EActuatorMode::MagnitudeProjection,
             .magnitudeProjection = {.axisFirst = EAxis::X, .axisSecond = EAxis::Y}},
        .rightMotor =
            {.isPresent = true,
             .mode = EActuatorMode::MagnitudeProjection,
             .magnitudeProjection = {.axisFirst = EAxis::Y, .axisSecond = EAxis::X}},
        .leftImpulseTrigger = {.isPresent = false},
        .rightImpulseTrigger = {.isPresent = false}};

    const SPhysicalActuatorComponents expectedActuatorComponents = {
        .leftMotor = ForceFeedbackActuatorValueVirtualToPhysical(
            kTestMagnitudeVector[(int)EAxis::X] * kSqrt2),
        .rightMotor = ForceFeedbackActuatorValueVirtualToPhysical(
            kTestMagnitudeVector[(int)EAxis::Y] * kSqrt2)};

    const Mapper mapper({}, kTestActuatorMap);

    const SPhysicalActuatorComponents actualActuatorComponents =
        mapper.MapForceFeedbackVirtualToPhysical(kTestMagnitudeVector);
    TEST_ASSERT(actualActuatorComponents == expectedActuatorComponents);
  }

  // Slightly more complex case of some actuators mapped and in all cases using only a single axis
  // direction.
  TEST_CASE(Mapper_ForceFeedback_Unidirectional)
  {
    constexpr TOrderedMagnitudeComponents kTestMagnitudeVector = {
        1111, -2222, 3333, -4444, 5555, -6666};

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .leftMotor =
            {.isPresent = true,
             .mode = EActuatorMode::SingleAxis,
             .singleAxis = {.axis = EAxis::X, .direction = EAxisDirection::Positive}},
        .rightMotor =
            {.isPresent = true,
             .mode = EActuatorMode::SingleAxis,
             .singleAxis = {.axis = EAxis::Y, .direction = EAxisDirection::Positive}},
        .leftImpulseTrigger = {.isPresent = false},
        .rightImpulseTrigger = {
            .isPresent = true,
            .mode = EActuatorMode::SingleAxis,
            .singleAxis = {.axis = EAxis::RotZ, .direction = EAxisDirection::Negative}}};

    const SPhysicalActuatorComponents expectedActuatorComponents = {
        .leftMotor =
            ((kTestMagnitudeVector[(int)EAxis::X] > 0)
                 ? ForceFeedbackActuatorValueVirtualToPhysical(kTestMagnitudeVector[(int)EAxis::X])
                 : (TPhysicalActuatorValue)0),
        .rightMotor =
            ((kTestMagnitudeVector[(int)EAxis::Y] > 0)
                 ? ForceFeedbackActuatorValueVirtualToPhysical(kTestMagnitudeVector[(int)EAxis::Y])
                 : (TPhysicalActuatorValue)0),
        .rightImpulseTrigger =
            ((kTestMagnitudeVector[(int)EAxis::RotZ] < 0)
                 ? ForceFeedbackActuatorValueVirtualToPhysical(
                       kTestMagnitudeVector[(int)EAxis::RotZ])
                 : (TPhysicalActuatorValue)0)};

    const Mapper mapper({}, kTestActuatorMap);

    const SPhysicalActuatorComponents actualActuatorComponents =
        mapper.MapForceFeedbackVirtualToPhysical(kTestMagnitudeVector);
    TEST_ASSERT(actualActuatorComponents == expectedActuatorComponents);
  }

  // Saturation test in which the input magnitude vector is at extreme values and needs to be
  // saturated.
  TEST_CASE(Mapper_ForceFeedback_Saturation)
  {
    constexpr TOrderedMagnitudeComponents kTestMagnitudeVectors[] = {
        {ForceFeedback::kEffectForceMagnitudeMinimum},
        {ForceFeedback::kEffectForceMagnitudeMaximum},
        {ForceFeedback::kEffectForceMagnitudeMinimum * 200},
        {ForceFeedback::kEffectForceMagnitudeMaximum * 200}};

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .leftMotor = {
            .isPresent = true,
            .mode = EActuatorMode::SingleAxis,
            .singleAxis = {.axis = EAxis::X, .direction = EAxisDirection::Both}}};

    const Mapper mapper({}, kTestActuatorMap);

    const SPhysicalActuatorComponents expectedActuatorComponents = {
        .leftMotor = std::numeric_limits<TPhysicalActuatorValue>::max()};

    for (const auto& testMagnitudeVector : kTestMagnitudeVectors)
    {
      const SPhysicalActuatorComponents actualActuatorComponents =
          mapper.MapForceFeedbackVirtualToPhysical(testMagnitudeVector);
      TEST_ASSERT(actualActuatorComponents == expectedActuatorComponents);
    }
  }

  // Gain test in which the input magnitude vector is modified by a gain property.
  TEST_CASE(Mapper_ForceFeedback_Gain)
  {
    constexpr TOrderedMagnitudeComponents kTestMagnitudeVector = {-1000};

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .leftMotor = {
            .isPresent = true,
            .mode = EActuatorMode::SingleAxis,
            .singleAxis = {.axis = EAxis::X, .direction = EAxisDirection::Both}}};

    constexpr TEffectValue kTestGainValues[] = {10000, 7500, 5000, 2500, 1000};

    const Mapper mapper({}, kTestActuatorMap);

    for (const auto testGainValue : kTestGainValues)
    {
      const SPhysicalActuatorComponents expectedActuatorComponents = {
          .leftMotor =
              ForceFeedbackActuatorValueVirtualToPhysical(kTestMagnitudeVector[0], testGainValue)};

      const SPhysicalActuatorComponents actualActuatorComponents =
          mapper.MapForceFeedbackVirtualToPhysical(kTestMagnitudeVector, testGainValue);
      TEST_ASSERT(actualActuatorComponents == expectedActuatorComponents);
    }
  }

  // Simultaneous gain and saturation test in which the input magnitude vector is at extreme values
  // and needs to be saturated while simultaneously being modified by a gain property.
  TEST_CASE(Mapper_ForceFeedback_SaturationAndGain)
  {
    constexpr TOrderedMagnitudeComponents kTestMagnitudeVectors[] = {
        {ForceFeedback::kEffectForceMagnitudeMinimum},
        {ForceFeedback::kEffectForceMagnitudeMaximum},
        {ForceFeedback::kEffectForceMagnitudeMinimum * 200},
        {ForceFeedback::kEffectForceMagnitudeMaximum * 200}};

    constexpr TEffectValue kTestGainValues[] = {5000, 2500, 1000};

    constexpr Mapper::SForceFeedbackActuatorMap kTestActuatorMap = {
        .leftMotor = {
            .isPresent = true,
            .mode = EActuatorMode::SingleAxis,
            .singleAxis = {.axis = EAxis::X, .direction = EAxisDirection::Both}}};

    const Mapper mapper({}, kTestActuatorMap);

    for (const auto testGainValue : kTestGainValues)
    {
      const TEffectValue testGainMultiplier = testGainValue / 10000;
      const TEffectValue expectedActuatorValue =
          (TEffectValue)std::numeric_limits<TPhysicalActuatorValue>::max() * testGainMultiplier;

      const SPhysicalActuatorComponents expectedActuatorComponents = {
          .leftMotor = (TPhysicalActuatorValue)std::lround(expectedActuatorValue)};

      for (const auto& testMagnitudeVector : kTestMagnitudeVectors)
      {
        const SPhysicalActuatorComponents actualActuatorComponents =
            mapper.MapForceFeedbackVirtualToPhysical(testMagnitudeVector, testGainValue);
        TEST_ASSERT(actualActuatorComponents == expectedActuatorComponents);
      }
    }
  }
} // namespace XidiTest
