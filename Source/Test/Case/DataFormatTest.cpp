/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file DataFormatTest.cpp
 *   Unit tests for functionality related to interacting with DirectInput
 *   applications using their own specified data formats.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "DataFormat.h"
#include "TestCase.h"


namespace XidiTest
{
    using namespace ::Xidi;


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies that POV direction values are correctly produced from controller states.
    // Tests all possible combinations of individual POV direction states.
    TEST_CASE(DataFormat_PovDirectionFromControllerState)
    {
        constexpr struct
        {
            bool povUp;
            bool povDown;
            bool povLeft;
            bool povRight;
            DataFormat::EPovValue expectedPovValue;
        } kPovTestData[] = {
            {.povUp = false, .povDown = false, .povLeft = false, .povRight = false, .expectedPovValue = DataFormat::EPovValue::Center},
            {.povUp = false, .povDown = false, .povLeft = false, .povRight = true,  .expectedPovValue = DataFormat::EPovValue::E},
            {.povUp = false, .povDown = false, .povLeft = true,  .povRight = false, .expectedPovValue = DataFormat::EPovValue::W},
            {.povUp = false, .povDown = false, .povLeft = true,  .povRight = true,  .expectedPovValue = DataFormat::EPovValue::Center},
            {.povUp = false, .povDown = true,  .povLeft = false, .povRight = false, .expectedPovValue = DataFormat::EPovValue::S},
            {.povUp = false, .povDown = true,  .povLeft = false, .povRight = true,  .expectedPovValue = DataFormat::EPovValue::SE},
            {.povUp = false, .povDown = true,  .povLeft = true,  .povRight = false, .expectedPovValue = DataFormat::EPovValue::SW},
            {.povUp = false, .povDown = true,  .povLeft = true,  .povRight = true,  .expectedPovValue = DataFormat::EPovValue::S},
            {.povUp = true,  .povDown = false, .povLeft = false, .povRight = false, .expectedPovValue = DataFormat::EPovValue::N},
            {.povUp = true,  .povDown = false, .povLeft = false, .povRight = true,  .expectedPovValue = DataFormat::EPovValue::NE},
            {.povUp = true,  .povDown = false, .povLeft = true,  .povRight = false, .expectedPovValue = DataFormat::EPovValue::NW},
            {.povUp = true,  .povDown = false, .povLeft = true,  .povRight = true,  .expectedPovValue = DataFormat::EPovValue::N},
            {.povUp = true,  .povDown = true,  .povLeft = false, .povRight = false, .expectedPovValue = DataFormat::EPovValue::Center},
            {.povUp = true,  .povDown = true,  .povLeft = false, .povRight = true,  .expectedPovValue = DataFormat::EPovValue::E},
            {.povUp = true,  .povDown = true,  .povLeft = true,  .povRight = false, .expectedPovValue = DataFormat::EPovValue::W},
            {.povUp = true,  .povDown = true,  .povLeft = true,  .povRight = true,  .expectedPovValue = DataFormat::EPovValue::Center},
        };

        int numFailingInputs = 0;
        for (const auto& povTestData : kPovTestData)
        {
            Controller::SState controllerState;
            controllerState.povDirection[(int)Controller::EPovDirection::Up] = povTestData.povUp;
            controllerState.povDirection[(int)Controller::EPovDirection::Down] = povTestData.povDown;
            controllerState.povDirection[(int)Controller::EPovDirection::Left] = povTestData.povLeft;
            controllerState.povDirection[(int)Controller::EPovDirection::Right] = povTestData.povRight;

            const DataFormat::EPovValue kExpectedPovValue = povTestData.expectedPovValue;
            const DataFormat::EPovValue kActualPovValue = DataFormat::PovDirectionFromControllerState(controllerState);

            if (kActualPovValue != kExpectedPovValue)
            {
                PrintFormatted(L"Wrong POV direction for states up=%s down=%s left=%s right=%s (expected %d, got %d).", ((true == povTestData.povUp) ? L"true" : L"false"), ((true == povTestData.povDown) ? L"true" : L"false"), ((true == povTestData.povLeft) ? L"true" : L"false"), ((true == povTestData.povRight) ? L"true" : L"false"), (int)kExpectedPovValue, (int)kActualPovValue);
                numFailingInputs += 1;
            }
        }

        TEST_ASSERT(0 == numFailingInputs);
    }
}
