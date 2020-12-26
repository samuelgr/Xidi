/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ButtonMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual
 *   button.
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

    // Creates one button mapper for each possible virtual button and verifies that each correctly identifies its target virtual controller element.
    TEST_CASE(ButtonMapper_GetTargetElement)
    {
        for (int i = 0; i < (int)EButton::Count; ++i)
        {
            const ButtonMapper mapper((EButton)i);
            TEST_ASSERT(EElementType::Button == mapper.GetTargetElementType());
            TEST_ASSERT(i == mapper.GetTargetElementIndex());
        }
    }

    // Creates a few button mappers and verifies that they contribute correctly to a virtual button press.
    // An analog value that is neutral should be unpressed, whereas an analog value at either extreme should be pressed.
    TEST_CASE(ButtonMapper_ContributeFromAnalogValue_Nominal)
    {
        constexpr EButton kTargetButtonUnpressed = EButton::B1;
        constexpr EButton kTargetButtonPressedPositive = EButton::B4;
        constexpr EButton kTargetButtonPressedNegative = EButton::B8;
        
        constexpr ButtonMapper mapperUnpressed(kTargetButtonUnpressed);
        constexpr ButtonMapper mapperPressedPositive(kTargetButtonPressedPositive);
        constexpr ButtonMapper mapperPressedNegative(kTargetButtonPressedNegative);

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.button[(int)kTargetButtonUnpressed] = false;
        expectedState.button[(int)kTargetButtonPressedPositive] = true;
        expectedState.button[(int)kTargetButtonPressedNegative] = true;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        mapperUnpressed.ContributeFromAnalogValue(&actualState, kAnalogValueNeutral);
        mapperPressedPositive.ContributeFromAnalogValue(&actualState, kAnalogValueMax);
        mapperPressedNegative.ContributeFromAnalogValue(&actualState, kAnalogValueMin);

        TEST_ASSERT(actualState == expectedState);
    }
}
