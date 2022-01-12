/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file SplitMapperTest.cpp
 *   Unit tests for controller multi-element mappers that split an XInput
 *   controller elemtn into a positive and a negative mapper based on its
 *   state.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ElementMapper.h"
#include "MockElementMapper.h"
#include "TestCase.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Controller state used for tests that need such an instance but do not care about its contents.
    static SState unusedControllerState;


    // -------- TEST CASES ------------------------------------------------- //

    // Creates one SplitMapper with both positive and negative mappers present.
    // Verifies correct reporting of the target elements from each.
    TEST_CASE(SplitMapper_GetTargetElement_Nominal)
    {
        constexpr SElementIdentifier kUnderlyingElements[] = {{.type = EElementType::Button, .button = EButton::B2}, {.type = EElementType::Button, .button = EButton::B10}};

        const SplitMapper mapper(std::make_unique<MockElementMapper>(kUnderlyingElements[0]), std::make_unique<MockElementMapper>(kUnderlyingElements[1]));
        TEST_ASSERT(_countof(kUnderlyingElements) == mapper.GetTargetElementCount());

        for (int i = 0; i < _countof(kUnderlyingElements); ++i)
        {
            const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(i);
            TEST_ASSERT(true == maybeTargetElement.has_value());

            const SElementIdentifier targetElement = maybeTargetElement.value();
            TEST_ASSERT(kUnderlyingElements[i] == targetElement);
        }
    }

    // Creates and then clones one SplitMapper with both positive and negative mappers present.
    // Verifies correct reporting of the target elements from each.
    TEST_CASE(SplitMapper_GetTargetElement_Clone)
    {
        constexpr SElementIdentifier kUnderlyingElements[] = {{.type = EElementType::Button, .button = EButton::B2}, {.type = EElementType::Button, .button = EButton::B10}};

        const SplitMapper mapperOriginal(std::make_unique<MockElementMapper>(kUnderlyingElements[0]), std::make_unique<MockElementMapper>(kUnderlyingElements[1]));
        const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
        TEST_ASSERT(nullptr != dynamic_cast<SplitMapper*>(mapperClone.get()));
        TEST_ASSERT(_countof(kUnderlyingElements) == mapperClone->GetTargetElementCount());

        for (int i = 0; i < _countof(kUnderlyingElements); ++i)
        {
            const std::optional<SElementIdentifier> maybeTargetElement = mapperClone->GetTargetElementAt(i);
            TEST_ASSERT(true == maybeTargetElement.has_value());

            const SElementIdentifier targetElement = maybeTargetElement.value();
            TEST_ASSERT(kUnderlyingElements[i] == targetElement);
        }
    }

    // Creates SplitMappers with only one mapper present.
    // Verifies correct reporting of the target elements from them.
    TEST_CASE(SplitMapper_GetTargetElement_OneNull)
    {
        constexpr SElementIdentifier kUnderlyingElements[] = {{.type = EElementType::Axis, .axis = EAxis::X}, {.type = EElementType::Axis, .axis = EAxis::RotY}};
        const SplitMapper mapper[] = {SplitMapper(std::make_unique<MockElementMapper>(kUnderlyingElements[0]), nullptr), SplitMapper(nullptr, std::make_unique<MockElementMapper>(kUnderlyingElements[1]))};

        static_assert(_countof(kUnderlyingElements) == _countof(mapper), "Test element array count mismatch.");
        
        for (int i = 0; i < _countof(mapper); ++i)
        {
            TEST_ASSERT(1 == mapper[i].GetTargetElementCount());

            const std::optional<SElementIdentifier> maybeTargetElement = mapper[i].GetTargetElementAt(0);
            TEST_ASSERT(true == maybeTargetElement.has_value());

            const SElementIdentifier targetElement = maybeTargetElement.value();
            TEST_ASSERT(kUnderlyingElements[i] == targetElement);
        }
    }

    // Creates one SplitMapper with no mappers present.
    // Verifies correct reporting of the target elements from it.
    TEST_CASE(SplitMapper_GetTargetElement_BothNull)
    {
        const SplitMapper mapper(nullptr, nullptr);
        TEST_ASSERT(0 == mapper.GetTargetElementCount());
    }

    // Verifies correct routing of analog values between positive and negative mappers when both a positive and a negative mapper are present.
    TEST_CASE(SplitMapper_RouteAnalogValue_Nominal)
    {
        constexpr struct {
            int16_t positive;
            int16_t negative;
        } kTestValues[] = {
            {.positive = kAnalogValueMax,           .negative = kAnalogValueMin},
            {.positive = kAnalogValueMax / 2,       .negative = kAnalogValueMin / 2},
            {.positive = kAnalogValueNeutral + 1,   .negative = kAnalogValueNeutral - 1},
            {.positive = kAnalogValueNeutral,       .negative = kAnalogValueNeutral - 1}
        };
        
        for (int i = 0; i < _countof(kTestValues); ++i)
        {
            int numPositiveContributions = 0;
            int numNegativeContributions = 0;

            const SplitMapper mapper(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kTestValues[i].positive, &numPositiveContributions), std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kTestValues[i].negative, &numNegativeContributions));

            mapper.ContributeFromAnalogValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapper.ContributeFromAnalogValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(1 == numNegativeContributions);
        }
    }

    // Verifies correct routing of analog values between positive and negative mappers when only one mapper is present, either positive or negative.
    TEST_CASE(SplitMapper_RouteAnalogValue_OneNull)
    {
        constexpr struct {
            int16_t positive;
            int16_t negative;
        } kTestValues[] = {
            {.positive = kAnalogValueMax,           .negative = kAnalogValueMin},
            {.positive = kAnalogValueMax / 2,       .negative = kAnalogValueMin / 2},
            {.positive = kAnalogValueNeutral + 1,   .negative = kAnalogValueNeutral - 1},
            {.positive = kAnalogValueNeutral,       .negative = kAnalogValueNeutral - 1}
        };

        for (int i = 0; i < _countof(kTestValues); ++i)
        {
            int numPositiveContributions = 0;
            int numNegativeContributions = 0;

            const SplitMapper mapperPositiveOnly(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kTestValues[i].positive, &numPositiveContributions), nullptr);
            const SplitMapper mapperNegativeOnly(nullptr, std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kTestValues[i].negative, &numNegativeContributions));

            mapperPositiveOnly.ContributeFromAnalogValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(0 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperNegativeOnly.ContributeFromAnalogValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(0 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperPositiveOnly.ContributeFromAnalogValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperNegativeOnly.ContributeFromAnalogValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(1 == numNegativeContributions);
        }
    }

    // Verifies correct routing of button values between positive and negative mappers when both a positive and a negative mapper are present.
    TEST_CASE(SplitMapper_RouteButtonValue_Nominal)
    {
        constexpr struct {
            bool positive;
            bool negative;
        } kTestValues[] = {
            {.positive = true, .negative = false}
        };

        for (int i = 0; i < _countof(kTestValues); ++i)
        {
            int numPositiveContributions = 0;
            int numNegativeContributions = 0;

            const SplitMapper mapper(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, true, &numPositiveContributions), std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, true, &numNegativeContributions));

            mapper.ContributeFromButtonValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapper.ContributeFromButtonValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(1 == numNegativeContributions);
        }
    }

    // Verifies correct routing of button values between positive and negative mappers when only one mapper is present, either positive or negative.
    TEST_CASE(SplitMapper_RouteButtonValue_OneNull)
    {
        constexpr struct {
            bool positive;
            bool negative;
        } kTestValues[] = {
            {.positive = true, .negative = false}
        };

        for (int i = 0; i < _countof(kTestValues); ++i)
        {
            int numPositiveContributions = 0;
            int numNegativeContributions = 0;

            const SplitMapper mapperPositiveOnly(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, true, &numPositiveContributions), nullptr);
            const SplitMapper mapperNegativeOnly(nullptr, std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, true, &numNegativeContributions));

            mapperPositiveOnly.ContributeFromButtonValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(0 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperNegativeOnly.ContributeFromButtonValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(0 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperPositiveOnly.ContributeFromButtonValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperNegativeOnly.ContributeFromButtonValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(1 == numNegativeContributions);
        }
    }

    // Verifies correct routing of trigger values between positive and negative mappers when both a positive and a negative mapper are present.
    TEST_CASE(SplitMapper_RouteTriggerValue_Nominal)
    {
        constexpr struct {
            uint8_t positive;
            uint8_t negative;
        } kTestValues[] = {
            {.positive = kTriggerValueMax,          .negative = kTriggerValueMin},
            {.positive = kTriggerValueMax / 2,      .negative = kTriggerValueMin / 2},
            {.positive = kTriggerValueMid + 1,      .negative = kTriggerValueMid - 1},
            {.positive = kTriggerValueMid,          .negative = kTriggerValueMid - 1}
        };

        for (int i = 0; i < _countof(kTestValues); ++i)
        {
            int numPositiveContributions = 0;
            int numNegativeContributions = 0;

            const SplitMapper mapper(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Trigger, kTestValues[i].positive, &numPositiveContributions), std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Trigger, kTestValues[i].negative, &numNegativeContributions));

            mapper.ContributeFromTriggerValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapper.ContributeFromTriggerValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(1 == numNegativeContributions);
        }
    }

    // Verifies correct routing of trigger values between positive and negative mappers when only one mapper is present, either positive or negative.
    TEST_CASE(SplitMapper_RouteTriggerValue_OneNull)
    {
        constexpr struct {
            uint8_t positive;
            uint8_t negative;
        } kTestValues[] = {
            {.positive = kTriggerValueMax,          .negative = kTriggerValueMin},
            {.positive = kTriggerValueMax / 2,      .negative = kTriggerValueMin / 2},
            {.positive = kTriggerValueMid + 1,      .negative = kTriggerValueMid - 1},
            {.positive = kTriggerValueMid,          .negative = kTriggerValueMid - 1}
        };

        for (int i = 0; i < _countof(kTestValues); ++i)
        {
            int numPositiveContributions = 0;
            int numNegativeContributions = 0;

            const SplitMapper mapperPositiveOnly(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Trigger, kTestValues[i].positive, &numPositiveContributions), nullptr);
            const SplitMapper mapperNegativeOnly(nullptr, std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Trigger, kTestValues[i].negative, &numNegativeContributions));

            mapperPositiveOnly.ContributeFromTriggerValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(0 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperNegativeOnly.ContributeFromTriggerValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(0 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperPositiveOnly.ContributeFromTriggerValue(unusedControllerState, kTestValues[i].positive);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(0 == numNegativeContributions);

            mapperNegativeOnly.ContributeFromTriggerValue(unusedControllerState, kTestValues[i].negative);
            TEST_ASSERT(1 == numPositiveContributions);
            TEST_ASSERT(1 == numNegativeContributions);
        }
    }

    // Verifies correct routing of neutral contributions to all underlying element mappers.
    TEST_CASE(SplitMapper_RouteNeutral)
    {
        constexpr int kExpectedContributionCount = 2;
        int actualContributionCount = 0;

        const SplitMapper mapper(std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Neutral, false, &actualContributionCount), std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Neutral, false, &actualContributionCount));

        mapper.ContributeNeutral(unusedControllerState);
        TEST_ASSERT(actualContributionCount == kExpectedContributionCount);
    }

    // Verifies that two axis mappers contribute the same state to a virtual controller as does one full axis mapper that is not split.
    // This does not represent a particularly useful use case but is still a condition that should be true.
    TEST_CASE(SplitMapper_SplitAxisEquivalence_SingleAxis)
    {
        constexpr EAxis kTargetAxis = EAxis::RotX;

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            constexpr AxisMapper axisMapper(kTargetAxis);
            const SplitMapper splitMapper(std::make_unique<AxisMapper>(kTargetAxis), std::make_unique<AxisMapper>(kTargetAxis));

            axisMapper.ContributeFromAnalogValue(expectedState, (int16_t)analogValue);
            splitMapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies that two axis mappers can successfully be split over multiple axes and contribute the correct value to both.
    // This represents a practical use case of separating an XInput axis in half, sending the negative part to one element mapper and the positive part to another.
    TEST_CASE(SplitMapper_SplitAxisEquivalence_DualAxis)
    {
        constexpr EAxis kTargetAxis = EAxis::RotX;
        constexpr struct {
            EAxis positive = EAxis::Z;
            EAxis negative = EAxis::RotZ;
        } kTargetSplitAxes;

        SState tempStateBuffer;

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            constexpr AxisMapper axisMapper(kTargetAxis);
            const SplitMapper splitMapper(std::make_unique<AxisMapper>(kTargetSplitAxes.positive), std::make_unique<AxisMapper>(kTargetSplitAxes.negative));

            ZeroMemory(&tempStateBuffer, sizeof(tempStateBuffer));
            axisMapper.ContributeFromAnalogValue(tempStateBuffer, (int16_t)analogValue);

            const int32_t kExpectedAxisValue = tempStateBuffer.axis[(int)kTargetAxis];

            ZeroMemory(&tempStateBuffer, sizeof(tempStateBuffer));
            splitMapper.ContributeFromAnalogValue(tempStateBuffer, (int16_t)analogValue);

            const int32_t kActualAxisValue = (analogValue >= kAnalogValueNeutral) ? tempStateBuffer.axis[(int)kTargetSplitAxes.positive] : tempStateBuffer.axis[(int)kTargetSplitAxes.negative];
            const int32_t kSupposedlyUntouchedAxisValue = (analogValue >= kAnalogValueNeutral) ? tempStateBuffer.axis[(int)kTargetSplitAxes.negative] : tempStateBuffer.axis[(int)kTargetSplitAxes.positive];

            TEST_ASSERT(kActualAxisValue == kExpectedAxisValue);
            TEST_ASSERT(0 == kSupposedlyUntouchedAxisValue);
        }
    }
}
