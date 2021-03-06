/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file PovMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual
 *   point-of-view hat.
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

    // Creates one POV mapper for each possible virtual POV direction and verifies that each correctly identifies its target virtual controller element.
    // Because all POV mappers contribute to the same virutal POV object (one direction per mapper), the element index is always the same.
    TEST_CASE(PovMapper_GetTargetElement)
    {
        for (int i = 0; i < (int)EPovDirection::Count; ++i)
        {
            const PovMapper mapper((EPovDirection)i);
            const SElementIdentifier targetElement = mapper.GetTargetElement();
            TEST_ASSERT(EElementType::Pov == targetElement.type);
        }
    }

    // Verifies the nominal behavior in which a POV mapper is asked to contribute some arbitrary analog value to a POV hat direction.
    // Expected behavior is the POV direction is pressed at the extreme analog values and not pressed towards neutral, but the exact transition thresholds are not defined.
    // Sweeps the entire range of possible analog values.
    TEST_CASE(PovMapper_ContributeFromAnalogValue_Nominal)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Up;

        // Expected sequence, based on an analog value sweep, not pressed followed by pressed.
        // The final two values are the same as a way of simplifying the implementation thus disabling a final transition and triggering a test failure.
        constexpr bool kExpectedPovSequence[] = {false, true, true};
        int currentSequenceIndex = 0;

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            const PovMapper mapper(kTargetPov);

            SState possibleExpectedStates[2];
            ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
            possibleExpectedStates[0].povDirection.components[(int)kTargetPov] = kExpectedPovSequence[currentSequenceIndex];
            possibleExpectedStates[1].povDirection.components[(int)kTargetPov] = kExpectedPovSequence[currentSequenceIndex + 1];

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

            if (actualState == possibleExpectedStates[0])
            {
                continue;
            }
            else if (actualState == possibleExpectedStates[1])
            {
                currentSequenceIndex += 1;
                continue;
            }
            else
            {
                TEST_FAILED_BECAUSE(L"Out-of-sequence value %d produced by a POV mapper with analog input %d.", (int)actualState.povDirection.components[(int)kTargetPov], analogValue);
            }
        }

        // The last value in the allowed values array is a sentinel just for ease of implementation.
        // We do, however, expect that all other values will have been reached.
        TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedPovSequence) - 1) - 1);
    }

    // Same as above, but with a negative POV direction specified as well.
    TEST_CASE(PovMapper_ContributeFromAnalogValue_NominalBidirectional)
    {
        constexpr EPovDirection kTargetPovPositive = EPovDirection::Up;
        constexpr EPovDirection kTargetPovNegative = EPovDirection::Down;

        // Expected sequence, based on an analog value sweep, is negative pressed, then neither pressed, then positive pressed.
        // The final two values are the same as a way of simplifying the implementation thus disabling a final transition and triggering a test failure.
        constexpr bool kExpectedPovSequencePositive[] = {false, false, true, true};
        constexpr bool kExpectedPovSequenceNegative[] = {true, false, false, false};
        int currentSequenceIndex = 0;

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            const PovMapper mapper(kTargetPovPositive, kTargetPovNegative);

            SState possibleExpectedStates[2];
            ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
            possibleExpectedStates[0].povDirection.components[(int)kTargetPovPositive] = kExpectedPovSequencePositive[currentSequenceIndex];
            possibleExpectedStates[0].povDirection.components[(int)kTargetPovNegative] = kExpectedPovSequenceNegative[currentSequenceIndex];
            possibleExpectedStates[1].povDirection.components[(int)kTargetPovPositive] = kExpectedPovSequencePositive[currentSequenceIndex + 1];
            possibleExpectedStates[1].povDirection.components[(int)kTargetPovNegative] = kExpectedPovSequenceNegative[currentSequenceIndex + 1];

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromAnalogValue(actualState, (int16_t)analogValue);

            if (actualState == possibleExpectedStates[0])
            {
                continue;
            }
            else if (actualState == possibleExpectedStates[1])
            {
                currentSequenceIndex += 1;
                continue;
            }
            else
            {
                TEST_FAILED_BECAUSE(L"Out-of-sequence values produced by a POV mapper with analog input %d.", analogValue);
            }
        }

        // The last value in the allowed values array is a sentinel just for ease of implementation.
        // We do, however, expect that all other values will have been reached.
        TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedPovSequencePositive) - 1) - 1);
    }

    // Verifies correct behavior when multiple POV mappers all contribute to the same virtual POV direction with neutral analog values as input.
    // The aggregated contribution should always be that the POV direction is not pressed, since no mapper sees any analog value away from neutral.
    TEST_CASE(PovMapper_ContributeFromAnalogValue_AllNeutral)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Down;

        constexpr PovMapper mappers[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = false;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappers)
            mapper.ContributeFromAnalogValue(actualState, kAnalogValueNeutral);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies correct behavior when multiple POV mappers all contribute to the same virtual POV direction with an extreme analog value as input.
    // The aggregated contribution should always be that the POV direction is pressed.
    TEST_CASE(PovMapper_ContributeFromAnalogValue_ConstructiveInterference)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Left;

        constexpr PovMapper mappers[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = true;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappers)
            mapper.ContributeFromAnalogValue(actualState, kAnalogValueMax);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies correct behavior when multiple POV mappers all contribute to the same virtual POV direction but the net analog value sum equals the neutral position.
    // For POV mappers this does not matter and the expected output is still that the POV direction is pressed.
    TEST_CASE(PovMapper_ContributeFromAnalogValue_DestructiveInterference)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Right;

        constexpr PovMapper mappersPositive[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        constexpr PovMapper mappersNegative[_countof(mappersPositive)] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = true;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappersPositive)
            mapper.ContributeFromAnalogValue(actualState, kAnalogValueMax);
        for (auto& mapper : mappersNegative)
            mapper.ContributeFromAnalogValue(actualState, kAnalogValueMin);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies the nominal behavior in which a POV mapper is asked to contribute some arbitrary button press state to a POV hat direction.
    TEST_CASE(PovMapper_ContributeFromButtonValue_Nominal)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Up;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr PovMapper mapper(kTargetPov);

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.povDirection.components[(int)kTargetPov] = buttonIsPressed;

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Same as above, but with a negative POV direction specified as well.
    TEST_CASE(PovMapper_ContributeFromButtonValue_NominalBidirectional)
    {
        constexpr EPovDirection kTargetPovPositive = EPovDirection::Up;
        constexpr EPovDirection kTargetPovNegative = EPovDirection::Right;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr PovMapper mapper(kTargetPovPositive, kTargetPovNegative);

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.povDirection.components[(int)kTargetPovPositive] = buttonIsPressed;
            expectedState.povDirection.components[(int)kTargetPovNegative] = !buttonIsPressed;

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies correct behavior when multiple POV mapper contributions occur to the same virtual POV direction and all POV mappers receive the same input state.
    // As long as one POV mapper receives an input of "pressed" then the virtual POV direction should also be pressed.
    TEST_CASE(PovMapper_ContributeFromButtonValue_SamePovSameInput)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Down;
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr PovMapper mappers[] = {
                PovMapper(kTargetPov),
                PovMapper(kTargetPov),
                PovMapper(kTargetPov),
                PovMapper(kTargetPov),
                PovMapper(kTargetPov)
            };

            SState expectedState;
            ZeroMemory(&expectedState, sizeof(expectedState));
            expectedState.povDirection.components[(int)kTargetPov] = buttonIsPressed;

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            for (auto& mapper : mappers)
                mapper.ContributeFromButtonValue(actualState, buttonIsPressed);

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies correct behavior when multiple POV mapper contributions occur to the same virtual POV direction but mappers receive different input state.
    // As long as one POV mapper receives an input of "pressed" then the virtual POV direction should also be pressed.
    TEST_CASE(PovMapper_ContributeFromButtonValue_SamePovDifferentInput)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Left;

        constexpr PovMapper mappersPressed[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        constexpr PovMapper mappersNotPressed[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = true;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappersPressed)
            mapper.ContributeFromButtonValue(actualState, true);
        for (auto& mapper : mappersNotPressed)
            mapper.ContributeFromButtonValue(actualState, false);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies the nominal behavior in which a POV mapper is asked to contribute a trigger value to a POV hat direction.
    // Expected behavior is the POV direction is not pressed at the start and becomes pressed once the trigger value hits a threshold, but the exact transition point is not defined.
    // Sweeps the entire range of possible trigger values.
    TEST_CASE(PovMapper_ContributeFromTriggerValue_Nominal)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Right;

        // Expected sequence, based on a trigger value sweep, is not pressed followed by pressed.
        // The final two values are the same as a way of simplifying the implementation thus disabling a final transition and triggering a test failure.
        constexpr bool kExpectedPovSequence[] = {false, true, true};
        int currentSequenceIndex = 0;

        for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
        {
            const PovMapper mapper(kTargetPov);

            SState possibleExpectedStates[2];
            ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
            possibleExpectedStates[0].povDirection.components[(int)kTargetPov] = kExpectedPovSequence[currentSequenceIndex];
            possibleExpectedStates[1].povDirection.components[(int)kTargetPov] = kExpectedPovSequence[currentSequenceIndex + 1];

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

            if (actualState == possibleExpectedStates[0])
            {
                continue;
            }
            else if (actualState == possibleExpectedStates[1])
            {
                currentSequenceIndex += 1;
                continue;
            }
            else
            {
                TEST_FAILED_BECAUSE(L"Out-of-sequence value %d produced by a POV mapper with trigger input %d.", (int)actualState.povDirection.components[(int)kTargetPov], triggerValue);
            }
        }

        // The last value in the allowed values array is a sentinel just for ease of implementation.
        // We do, however, expect that all other values will have been reached.
        TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedPovSequence) - 1) - 1);
    }

    // Same as above, but with a negative POV direction specified as well.
    TEST_CASE(PovMapper_ContributeFromTriggerValue_NominalBidirectional)
    {
        constexpr EPovDirection kTargetPovPositive = EPovDirection::Right;
        constexpr EPovDirection kTargetPovNegative = EPovDirection::Left;

        // Expected sequence, based on a trigger value sweep, is negative pressed followed by positive pressed.
        // The final two values are the same as a way of simplifying the implementation thus disabling a final transition and triggering a test failure.
        constexpr bool kExpectedPovSequencePositive[] = {false, true, true};
        constexpr bool kExpectedPovSequenceNegative[] = {true, false, false};
        int currentSequenceIndex = 0;

        for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
        {
            const PovMapper mapper(kTargetPovPositive, kTargetPovNegative);

            SState possibleExpectedStates[2];
            ZeroMemory(possibleExpectedStates, sizeof(possibleExpectedStates));
            possibleExpectedStates[0].povDirection.components[(int)kTargetPovPositive] = kExpectedPovSequencePositive[currentSequenceIndex];
            possibleExpectedStates[0].povDirection.components[(int)kTargetPovNegative] = kExpectedPovSequenceNegative[currentSequenceIndex];
            possibleExpectedStates[1].povDirection.components[(int)kTargetPovPositive] = kExpectedPovSequencePositive[currentSequenceIndex + 1];
            possibleExpectedStates[1].povDirection.components[(int)kTargetPovNegative] = kExpectedPovSequenceNegative[currentSequenceIndex + 1];

            SState actualState;
            ZeroMemory(&actualState, sizeof(actualState));
            mapper.ContributeFromTriggerValue(actualState, (uint8_t)triggerValue);

            if (actualState == possibleExpectedStates[0])
            {
                continue;
            }
            else if (actualState == possibleExpectedStates[1])
            {
                currentSequenceIndex += 1;
                continue;
            }
            else
            {
                TEST_FAILED_BECAUSE(L"Out-of-sequence values produced by a POV mapper with trigger input %d.", triggerValue);
            }
        }

        // The last value in the allowed values array is a sentinel just for ease of implementation.
        // We do, however, expect that all other values will have been reached.
        TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedPovSequencePositive) - 1) - 1);
    }

    // Verifies correct behavior when multiple POV mappers all contribute to the same virtual POV direction with minimum trigger values as input.
    // The aggregated contribution should always be that the POV direction is not pressed, since no mapper sees any trigger value that could possibly have exceeded the threshold.
    TEST_CASE(PovMapper_ContributeFromTriggerValue_NonePressed)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Up;

        constexpr PovMapper mappers[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = false;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappers)
            mapper.ContributeFromTriggerValue(actualState, kTriggerValueMin);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies correct behavior when multiple POV mappers all contribute to the same virtual POV direction with maximum trigger values.
    // The aggregated contribution should always be that the POV direction is pressed.
    TEST_CASE(PovMapper_ContributeFromTriggerValue_AllPressed)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Down;

        constexpr PovMapper mappers[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = true;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappers)
            mapper.ContributeFromTriggerValue(actualState, kTriggerValueMax);

        TEST_ASSERT(actualState == expectedState);
    }

    // Verifies correct behavior when multiple POV mappers all contribute to the same virtual POV direction and only some are considered pressed based on the input trigger value.
    // For POV mappers this does not matter and the expected output is still that the POV direction is pressed.
    TEST_CASE(PovMapper_ContributeFromTriggerValue_SomePressed)
    {
        constexpr EPovDirection kTargetPov = EPovDirection::Left;

        constexpr PovMapper mappersPressed[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        constexpr PovMapper mappersNotPressed[] = {
            PovMapper(kTargetPov),
            PovMapper(kTargetPov),
            PovMapper(kTargetPov)
        };

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.povDirection.components[(int)kTargetPov] = true;

        SState actualState;
        ZeroMemory(&actualState, sizeof(actualState));
        for (auto& mapper : mappersPressed)
            mapper.ContributeFromTriggerValue(actualState, kTriggerValueMax);
        for (auto& mapper : mappersNotPressed)
            mapper.ContributeFromTriggerValue(actualState, kTriggerValueMin);

        TEST_ASSERT(actualState == expectedState);
    }
}
