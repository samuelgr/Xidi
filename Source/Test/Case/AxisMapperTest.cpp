/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file AxisMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual
 *   axis.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "TestCase.h"

#include <cstdint>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- TEST CASES ------------------------------------------------- //

    // Creates one axis mapper for each possible virtual axis and verifies that each correctly identifies its target virtual controller element.
    TEST_CASE(AxisMapper_GetTargetElement)
    {
        for (int i = 0; i < (int)EAxis::Count; ++i)
        {
            const AxisMapper mapper((EAxis)i);
            const SElementIdentifier targetElement = mapper.GetTargetElement();
            TEST_ASSERT(EElementType::Axis == targetElement.type);
            TEST_ASSERT(i == (int)targetElement.axis);
        }
    }

    // Verifies the nominal behavior in which an axis mapper is asked to contribute some arbitrary analog value to an axis.
    // Sweeps the entire range of possible analog values.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_Nominal_EntireAxis)
    {
        constexpr EAxis kTargetAxis = EAxis::RotX;

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            constexpr AxisMapper mapper(kTargetAxis);

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = analogValue;

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but for a half axis in the positive direction.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_Nominal_HalfAxisPositive)
    {
        constexpr EAxis kTargetAxis = EAxis::RotY;
        constexpr double kStepSize = (double)(kAnalogValueMax - kAnalogValueNeutral) / (double)(kAnalogValueMax - kAnalogValueMin);

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            constexpr AxisMapper mapper(kTargetAxis, AxisMapper::EDirection::Positive);
            const double analogValueDisplacement = (double)analogValue - (double)kAnalogValueMin;

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral + (int32_t)(analogValueDisplacement * kStepSize);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but for a half axis in the negative direction.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_Nominal_HalfAxisNegative)
    {
        constexpr EAxis kTargetAxis = EAxis::RotZ;
        constexpr double kStepSize = (double)(kAnalogValueMax - kAnalogValueNeutral) / (double)(kAnalogValueMax - kAnalogValueMin);

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            constexpr AxisMapper mapper(kTargetAxis, AxisMapper::EDirection::Negative);
            const double analogValueDisplacement = (double)analogValue - (double)kAnalogValueMin;

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = kAnalogValueMin + (int32_t)(analogValueDisplacement * kStepSize);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromAnalogValue(actualState, (int32_t)analogValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies correct behavior when multiple axis mappers all contribute to the same virtual axis.
    // The aggregated contribution should be the sum of the values contributed by each axis mapper.
    // It is possible and acceptable that the result of aggregating all contributing axis mappers exceeds the maximum possible analog axis value.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_ConstructiveInterference)
    {
        constexpr int16_t kAnalogValue = 30000;
        constexpr EAxis kTargetAxis = EAxis::RotY;

        constexpr AxisMapper mappers[] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = (int32_t)kAnalogValue * _countof(mappers);

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappers)
            mapper.ContributeFromAnalogValue(actualState, kAnalogValue);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies correct behavior when multiple axis mappers all contribute to the same virtual axis but the net contribution sums to the neutral position.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_DestructiveInterference)
    {
        constexpr int16_t kAnalogValue = 10;
        constexpr EAxis kTargetAxis = EAxis::RotY;

        constexpr AxisMapper mappersPositive[] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        constexpr AxisMapper mappersNegative[_countof(mappersPositive)] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappersPositive)
            mapper.ContributeFromAnalogValue(actualState, kAnalogValue);
        for (auto& mapper : mappersNegative)
            mapper.ContributeFromAnalogValue(actualState, -kAnalogValue);

        TEST_ASSERT(actualState == expectedState);
    }
    
    // Verifies the nominal behavior in which an axis mapper is asked to contribute some arbitrary button press state to an axis.
    TEST_CASE(AxisMapper_ContributeFromButtonValue_Nominal_EntireAxis)
    {
        constexpr EAxis kTargetAxis = EAxis::X;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr AxisMapper mapper(kTargetAxis);

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = (true == buttonIsPressed ? kAnalogValueMax : kAnalogValueMin);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but for a half axis in the positive direction.
    TEST_CASE(AxisMapper_ContributeFromButtonValue_Nominal_HalfAxisPositive)
    {
        constexpr EAxis kTargetAxis = EAxis::Y;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr AxisMapper mapper(kTargetAxis, AxisMapper::EDirection::Positive);

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = (true == buttonIsPressed ? kAnalogValueMax : kAnalogValueNeutral);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but for a half axis in the negative direction.
    TEST_CASE(AxisMapper_ContributeFromButtonValue_Nominal_HalfAxisNegative)
    {
        constexpr EAxis kTargetAxis = EAxis::Y;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr AxisMapper mapper(kTargetAxis, AxisMapper::EDirection::Negative);

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = (true == buttonIsPressed ? kAnalogValueMin : kAnalogValueNeutral);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies correct behavior when multiple axis mappers all contribute to the same virtual axis but sourced by a button state.
    // The aggregated contribution should be the sum of the values contributed by each axis mapper, which themselves should be extreme in one direction or another.
    TEST_CASE(AxisMapper_ContributeFromButtonValue_ConstructiveInterference)
    {
        constexpr EAxis kTargetAxis = EAxis::Z;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr AxisMapper mappers[] = {
                AxisMapper(kTargetAxis),
                AxisMapper(kTargetAxis),
                AxisMapper(kTargetAxis),
                AxisMapper(kTargetAxis),
                AxisMapper(kTargetAxis),
                AxisMapper(kTargetAxis)
            };

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = (true == buttonIsPressed ? kAnalogValueMax : kAnalogValueMin) * _countof(mappers);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            for (auto& mapper : mappers)
                mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies correct behavior when multiple axis mappers all contribute to the same virtual axis but sourced by a button state.
    // In this case, the aggregate contribution sums to a net of the neutral position (i.e. there are as many button states "pressed" as "not pressed").
    TEST_CASE(AxisMapper_ContributeFromButtonValue_DestructiveInterference)
    {
        constexpr EAxis kTargetAxis = EAxis::Z;

        constexpr AxisMapper mappersPressed[] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        constexpr AxisMapper mappersNotPressed[_countof(mappersPressed)] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappersPressed)
            mapper.ContributeFromButtonValue(actualState, true);
        for (auto& mapper : mappersNotPressed)
            mapper.ContributeFromButtonValue(actualState, false);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies the nominal behavior in which an axis mapper is asked to contribute some arbitrary trigger value to an axis.
    // Sweeps the entire range of possible trigger values.
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_Nominal_EntireAxis)
    {
        constexpr EAxis kTargetAxis = EAxis::RotX;
        constexpr double kStepSize = (double)(kAnalogValueMax - kAnalogValueMin) / (double)(kTriggerValueMax - kTriggerValueMin);

        for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
        {
            constexpr AxisMapper mapper(kTargetAxis);
            const double triggerValueDisplacement = (double)triggerValue - (double)kTriggerValueMin;

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = kAnalogValueMin + (int32_t)(triggerValueDisplacement * kStepSize);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but for a half axis in the positive direction.
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_Nominal_HalfAxisPositive)
    {
        constexpr EAxis kTargetAxis = EAxis::RotY;
        constexpr double kStepSize = (double)(kAnalogValueMax - kAnalogValueNeutral) / (double)(kTriggerValueMax - kTriggerValueMin);

        for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
        {
            constexpr AxisMapper mapper(kTargetAxis, AxisMapper::EDirection::Positive);
            const double triggerValueDisplacement = (double)triggerValue - (double)kTriggerValueMin;

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral + (int32_t)(triggerValueDisplacement * kStepSize);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but for a half axis in the negative direction.
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_Nominal_HalfAxisNegative)
    {
        constexpr EAxis kTargetAxis = EAxis::RotZ;
        constexpr double kStepSize = (double)(kAnalogValueNeutral - kAnalogValueMin) / (double)(kTriggerValueMax - kTriggerValueMin);

        for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
        {
            constexpr AxisMapper mapper(kTargetAxis, AxisMapper::EDirection::Negative);
            const double triggerValueDisplacement = (double)triggerValue - (double)kTriggerValueMin;

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral - (int32_t)(triggerValueDisplacement * kStepSize);

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies correct behavior when multiple axis mappers all contribute to the same virtual axis but sourced by a trigger value.
    // The aggregated contribution should be the sum of the values contributed by each axis mapper, which themselves should be extreme positive based on the test parameters.
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_ConstructiveInterference)
    {
        constexpr EAxis kTargetAxis = EAxis::Z;

        constexpr AxisMapper mappers[] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueMax * _countof(mappers);

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappers)
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)kTriggerValueMax);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies correct behavior when multiple axis mappers all contribute to the same virtual axis but sourced by a trigger value.
    // In this case, the aggregate contribution sums to a net of the neutral position.
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_DestructiveInterference)
    {
        constexpr EAxis kTargetAxis = EAxis::Z;

        constexpr AxisMapper mappersPositive[] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        constexpr AxisMapper mappersNegative[_countof(mappersPositive)] = {
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis),
            AxisMapper(kTargetAxis)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappersPositive)
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)kTriggerValueMax);
        for (auto& mapper : mappersNegative)
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)kTriggerValueMin);

        TEST_ASSERT(actualState == expectedState);
    }
}
