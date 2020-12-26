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
#include "ControllerElementMapper.h"
#include "ControllerTypes.h"
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
            TEST_ASSERT(EElementType::Axis == mapper.GetTargetElementType());
            TEST_ASSERT(i == mapper.GetTargetElementIndex());
        }

        TEST_PASSED;
    }
    
    // Creates one axis mapper for each of several axes.
    // Verifies that they each contribute the correct value to the expected axis.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_Nominal)
    {
        constexpr AxisMapper mapperX(EAxis::X);
        constexpr AxisMapper mapperY(EAxis::Y);
        constexpr AxisMapper mapperZ(EAxis::Z);
        constexpr AxisMapper mapperRotX(EAxis::RotX);
        constexpr AxisMapper mapperRotY(EAxis::RotY);
        constexpr AxisMapper mapperRotZ(EAxis::RotZ);

        constexpr int16_t kValueX = 110;
        constexpr int16_t kValueY = 2202;
        constexpr int16_t kValueZ = 303;
        constexpr int16_t kValueRotX = -4040;
        constexpr int16_t kValueRotY = -555;
        constexpr int16_t kValueRotZ = -6600;

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::X] = kValueX;
        expectedState.axis[(int)EAxis::Y] = kValueY;
        expectedState.axis[(int)EAxis::Z] = kValueZ;
        expectedState.axis[(int)EAxis::RotX] = kValueRotX;
        expectedState.axis[(int)EAxis::RotY] = kValueRotY;
        expectedState.axis[(int)EAxis::RotZ] = kValueRotZ;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperX.ContributeFromAnalogValue(&actualState, kValueX);
        mapperY.ContributeFromAnalogValue(&actualState, kValueY);
        mapperZ.ContributeFromAnalogValue(&actualState, kValueZ);
        mapperRotX.ContributeFromAnalogValue(&actualState, kValueRotX);
        mapperRotY.ContributeFromAnalogValue(&actualState, kValueRotY);
        mapperRotZ.ContributeFromAnalogValue(&actualState, kValueRotZ);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Verifies correct saturation behavior of an axis mapper when it contributes to an axis.
    // There is a very slight difference in the range of values an XInput controller can report versus what a virtual controller reports.
    // The point of this test is to make sure that axis mappers properly account for that difference.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_Saturation)
    {
        constexpr AxisMapper mapperExtremePositive(EAxis::X);
        constexpr AxisMapper mapperExtremeNegative(EAxis::Y);

        constexpr int16_t kValueExtremePositive = INT16_MAX;
        constexpr int16_t kValueExtremeNegative = INT16_MIN;

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::X] = kAnalogValueMax;
        expectedState.axis[(int)EAxis::Y] = kAnalogValueMin;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperExtremePositive.ContributeFromAnalogValue(&actualState, kValueExtremePositive);
        mapperExtremeNegative.ContributeFromAnalogValue(&actualState, kValueExtremeNegative);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers on the same axis.
    // Verifies that they together correctly aggregate their contributions to the virtual controller axis.
    // It is possible that the result of aggregating all contributing axis mappers exceeds the maximum possible analog axis value.
    // This is acceptable, as it is not the job of the individual axis mapper to ensure overall value consistency on the controller state.
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
        for (int i = 0; i < _countof(mappers); ++i)
            mappers[i].ContributeFromAnalogValue(&actualState, kAnalogValue);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers on the same axis.
    // Verifies that they together correctly aggregate their contributions to the virtual controller axis.
    // In this case, the sum of all contributions is zero.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_DestructiveInterference)
    {
        constexpr int16_t kAnalogValue = 10;
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

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (int i = 0; i < _countof(mappers); ++i)
        {
            mappers[i].ContributeFromAnalogValue(&actualState, kAnalogValue);
            mappers[i].ContributeFromAnalogValue(&actualState, -kAnalogValue);
        }   

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one for each of a selection of axes, each configured as a positive half-axis.
    // Verifies that the analog reading is correctly mapped to the space of a half-axis.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_HalfAxisPositive)
    {
        constexpr AxisMapper mapperX(EAxis::X, AxisMapper::EDirection::Positive);
        constexpr AxisMapper mapperY(EAxis::Y, AxisMapper::EDirection::Positive);
        constexpr AxisMapper mapperZ(EAxis::Z, AxisMapper::EDirection::Positive);

        constexpr int16_t kValueX = kAnalogValueMin;
        constexpr int16_t kValueY = kAnalogValueNeutral;
        constexpr int16_t kValueZ = kAnalogValueMax;

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::X] = kAnalogValueNeutral;
        expectedState.axis[(int)EAxis::Y] = (kAnalogValueNeutral - kAnalogValueMin) >> 1;
        expectedState.axis[(int)EAxis::Z] = kAnalogValueMax;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperX.ContributeFromAnalogValue(&actualState, kValueX);
        mapperY.ContributeFromAnalogValue(&actualState, kValueY);
        mapperZ.ContributeFromAnalogValue(&actualState, kValueZ);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one for each of a selection of axes, each configured as a negative half-axis.
    // Verifies that the analog reading is correctly mapped to the space of a half-axis.
    TEST_CASE(AxisMapper_ContributeFromAnalogValue_HalfAxisNegative)
    {
        constexpr AxisMapper mapperX(EAxis::X, AxisMapper::EDirection::Negative);
        constexpr AxisMapper mapperY(EAxis::Y, AxisMapper::EDirection::Negative);
        constexpr AxisMapper mapperZ(EAxis::Z, AxisMapper::EDirection::Negative);

        constexpr int16_t kValueX = kAnalogValueMin;
        constexpr int16_t kValueY = kAnalogValueNeutral;
        constexpr int16_t kValueZ = kAnalogValueMax;

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::X] = kAnalogValueMin;
        expectedState.axis[(int)EAxis::Y] = (kAnalogValueNeutral + kAnalogValueMin) >> 1;
        expectedState.axis[(int)EAxis::Z] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperX.ContributeFromAnalogValue(&actualState, kValueX);
        mapperY.ContributeFromAnalogValue(&actualState, kValueY);
        mapperZ.ContributeFromAnalogValue(&actualState, kValueZ);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one on each whole axis, and causes values to be contributed from button presses.
    // Verifies that they contribute either extreme negative (not pressed) or extreme positive (pressed).
    TEST_CASE(AxisMapper_ContributeFromButtonValue_WholeAxis)
    {
        constexpr EAxis kTargetAxisPressed = EAxis::X;
        constexpr EAxis kTargetAxisNotPressed = EAxis::Y;

        constexpr AxisMapper mapperPressed(kTargetAxisPressed);
        constexpr AxisMapper mapperNotPressed(kTargetAxisNotPressed);
        
        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxisPressed] = kAnalogValueMax;
        expectedState.axis[(int)kTargetAxisNotPressed] = kAnalogValueMin;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperPressed.ContributeFromButtonValue(&actualState, true);
        mapperNotPressed.ContributeFromButtonValue(&actualState, false);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one on each positive half-axis, and causes values to be contributed from button presses.
    // Verifies that they contribute either neutral (not pressed) or extreme positive (pressed).
    TEST_CASE(AxisMapper_ContributeFromButtonValue_HalfAxisPositive)
    {
        constexpr EAxis kTargetAxisPressed = EAxis::X;
        constexpr EAxis kTargetAxisNotPressed = EAxis::Y;

        constexpr AxisMapper mapperPressed(kTargetAxisPressed, AxisMapper::EDirection::Positive);
        constexpr AxisMapper mapperNotPressed(kTargetAxisNotPressed, AxisMapper::EDirection::Positive);

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxisPressed] = kAnalogValueMax;
        expectedState.axis[(int)kTargetAxisNotPressed] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperPressed.ContributeFromButtonValue(&actualState, true);
        mapperNotPressed.ContributeFromButtonValue(&actualState, false);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one on each negative half-axis, and causes values to be contributed from button presses.
    // Verifies that they contribute either neutral (not pressed) or extreme negative (pressed).
    TEST_CASE(AxisMapper_ContributeFromButtonValue_HalfAxisNegative)
    {
        constexpr EAxis kTargetAxisPressed = EAxis::X;
        constexpr EAxis kTargetAxisNotPressed = EAxis::Y;

        constexpr AxisMapper mapperPressed(kTargetAxisPressed, AxisMapper::EDirection::Negative);
        constexpr AxisMapper mapperNotPressed(kTargetAxisNotPressed, AxisMapper::EDirection::Negative);

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxisPressed] = kAnalogValueMin;
        expectedState.axis[(int)kTargetAxisNotPressed] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperPressed.ContributeFromButtonValue(&actualState, true);
        mapperNotPressed.ContributeFromButtonValue(&actualState, false);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one on each whole axis, and causes values to be contributed from trigger values.
    // Verifies that they contribute either extreme negative (trigger is not pressed at all) or extreme positive (trigger is fully pressed).
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_WholeAxis)
    {
        constexpr EAxis kTargetAxisPressed = EAxis::X;
        constexpr EAxis kTargetAxisNotPressed = EAxis::Y;

        constexpr AxisMapper mapperPressed(kTargetAxisPressed);
        constexpr AxisMapper mapperNotPressed(kTargetAxisNotPressed);

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxisPressed] = kAnalogValueMax;
        expectedState.axis[(int)kTargetAxisNotPressed] = kAnalogValueMin;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperPressed.ContributeFromTriggerValue(&actualState, kTriggerValueMax);
        mapperNotPressed.ContributeFromTriggerValue(&actualState, kTriggerValueMin);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one on each positive half-axis, and causes values to be contributed from trigger values.
    // Verifies that they contribute either neutral (trigger is not pressed at all) or extreme positive (trigger is fully pressed).
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_HalfAxisPositive)
    {
        constexpr EAxis kTargetAxisPressed = EAxis::X;
        constexpr EAxis kTargetAxisNotPressed = EAxis::Y;

        constexpr AxisMapper mapperPressed(kTargetAxisPressed, AxisMapper::EDirection::Positive);
        constexpr AxisMapper mapperNotPressed(kTargetAxisNotPressed, AxisMapper::EDirection::Positive);

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxisPressed] = kAnalogValueMax;
        expectedState.axis[(int)kTargetAxisNotPressed] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperPressed.ContributeFromTriggerValue(&actualState, kTriggerValueMax);
        mapperNotPressed.ContributeFromTriggerValue(&actualState, kTriggerValueMin);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Creates multiple axis mappers, one on each negative half-axis, and causes values to be contributed from trigger values.
    // Verifies that they contribute either neutral (trigger is not pressed at all) or extreme negative (trigger is fully pressed).
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_HalfAxisNegative)
    {
        constexpr EAxis kTargetAxisPressed = EAxis::X;
        constexpr EAxis kTargetAxisNotPressed = EAxis::Y;

        constexpr AxisMapper mapperPressed(kTargetAxisPressed, AxisMapper::EDirection::Negative);
        constexpr AxisMapper mapperNotPressed(kTargetAxisNotPressed, AxisMapper::EDirection::Negative);

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxisPressed] = kAnalogValueMin;
        expectedState.axis[(int)kTargetAxisNotPressed] = kAnalogValueNeutral;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperPressed.ContributeFromTriggerValue(&actualState, kTriggerValueMax);
        mapperNotPressed.ContributeFromTriggerValue(&actualState, kTriggerValueMin);

        TEST_PASSED_IF(actualState == expectedState);
    }

    // Simulates both triggers sharing the Z axis, as is something that occurs in mappers.
    // Exercises a variety of different situations.
    TEST_CASE(AxisMapper_ContributeFromTriggerValue_TriggersSharedAxis)
    {
        constexpr EAxis kTargetAxis = EAxis::Z;
        constexpr AxisMapper mapperLeftTrigger(kTargetAxis, AxisMapper::EDirection::Positive);
        constexpr AxisMapper mapperRightTrigger(kTargetAxis, AxisMapper::EDirection::Negative);

        SState expectedState;
        SState actualState;


        // Scenario 1: neither trigger is pressed at all, axis state should be neutral.
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral;

        ZeroMemory(&actualState, sizeof(actualState));
        mapperLeftTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMin);
        mapperRightTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMin);

        TEST_ASSERT(actualState == expectedState);


        // Scenario 2: both triggers are fully pressed, axis state should be neutral.
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral;

        ZeroMemory(&actualState, sizeof(actualState));
        mapperLeftTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMax);
        mapperRightTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMax);

        TEST_ASSERT(actualState == expectedState);


        // Scenario 3: LT is fully pressed but RT is not pressed at all, axis state should be extreme positive.
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueMax;

        ZeroMemory(&actualState, sizeof(actualState));
        mapperLeftTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMax);
        mapperRightTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMin);

        TEST_ASSERT(actualState == expectedState);


        // Scenario 4: RT is fully pressed but LT is not pressed at all, axis state should be extreme negative.
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueMin;

        ZeroMemory(&actualState, sizeof(actualState));
        mapperLeftTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMin);
        mapperRightTrigger.ContributeFromTriggerValue(&actualState, kTriggerValueMax);

        TEST_ASSERT(actualState == expectedState);


        // Scenario 5: LT and RT are both halfway pressed, axis state should be neutral.
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)kTargetAxis] = kAnalogValueNeutral;

        ZeroMemory(&actualState, sizeof(actualState));
        mapperLeftTrigger.ContributeFromTriggerValue(&actualState, (kTriggerValueMin + kTriggerValueMax) / 2);
        mapperRightTrigger.ContributeFromTriggerValue(&actualState, (kTriggerValueMin + kTriggerValueMax) / 2);

        TEST_ASSERT(actualState == expectedState);


        TEST_PASSED;
    }
}
