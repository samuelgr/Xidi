/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file KeyboardMapperTest.cpp
 *   Unit tests for controller element mappers that contribute to a virtual
 *   keyboard key.
 *****************************************************************************/

#include "ElementMapper.h"
#include "Keyboard.h"
#include "MockKeyboard.h"
#include "TestCase.h"

#include <cstdint>
#include <memory>
#include <optional>


namespace XidiTest
{
    using namespace ::Xidi::Controller;
    using ::Xidi::Keyboard::TKeyIdentifier;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Keyboard key identifier used for all test cases in this file.
    static constexpr TKeyIdentifier kTestKeyIdentifier = 55;

    /// Empty virtual controller state used as a comparison target throughout this file.
    static constexpr SState kEmptyVirtualControllerState = {};


    // -------- TEST CASES ------------------------------------------------- //

    // Creates one keyboard mapper for each possible keyboard key and verifies two things.
    // First, verifies that it does not map to any virtual controller element.
    // Second, verifies that it correctly identifies its target keyboard key.
    TEST_CASE(KeyboardMapper_GetTargetElement_Nominal)
    {
        for (TKeyIdentifier i = 0; i < kVirtualKeyboardKeyCount; ++i)
        {
            const KeyboardMapper mapper(i);
            TEST_ASSERT(0 == mapper.GetTargetElementCount());

            const std::optional<SElementIdentifier> maybeTargetElement = mapper.GetTargetElementAt(0);
            TEST_ASSERT(false == maybeTargetElement.has_value());

            TEST_ASSERT(mapper.GetTargetKey() == i);
        }
    }

    // Creates and then clones one keyboard mapper for each possible keyboard key and verifies two things.
    // First, verifies that it does not map to any virtual controller element.
    // Second, verifies that it correctly identifies its target keyboard key.
    TEST_CASE(KeyboardMapper_GetTargetElement_Clone)
    {
        for (TKeyIdentifier i = 0; i < kVirtualKeyboardKeyCount; ++i)
        {
            const KeyboardMapper mapperOriginal(i);
            const std::unique_ptr<IElementMapper> mapperClone = mapperOriginal.Clone();
            TEST_ASSERT(nullptr != dynamic_cast<KeyboardMapper*>(mapperClone.get()));
            TEST_ASSERT(0 == mapperClone->GetTargetElementCount());

            const std::optional<SElementIdentifier> maybeTargetElement = mapperClone->GetTargetElementAt(0);
            TEST_ASSERT(false == maybeTargetElement.has_value());

            TEST_ASSERT(dynamic_cast<KeyboardMapper*>(mapperClone.get())->GetTargetKey() == i);
        }
    }

    // Verifies the nominal behavior in which a keyboard mapper is asked to contribute some arbitrary analog value to a keyboard key.
    // Expected behavior is the keyboard key is pressed at the extreme analog values and not pressed towards neutral, but the exact transition thresholds are not defined.
    // Sweeps the entire range of possible analog values.
    TEST_CASE(KeyboardMapper_ContributeFromAnalogValue_Nominal)
    {
        MockKeyboard expectedKeyboardStateUnpressed;

        MockKeyboard expectedKeyboardStatePressed;
        expectedKeyboardStatePressed.SubmitKeyPressedState(kTestKeyIdentifier);

        // Expected sequence, based on an analog value sweep, is pressed, not pressed, and finally pressed.
        // The final two values are the same as a way of simplifying the implementation thus disabling a final transition and triggering a test failure.
        const MockKeyboard* const kExpectedKeyboardSequence[] = { &expectedKeyboardStatePressed, &expectedKeyboardStateUnpressed, &expectedKeyboardStatePressed, &expectedKeyboardStatePressed };
        int currentSequenceIndex = 0;

        for (int32_t analogValue = kAnalogValueMin; analogValue <= kAnalogValueMax; ++analogValue)
        {
            const KeyboardMapper mapper(kTestKeyIdentifier);
            const MockKeyboard* possibleExpectedStates[2] = {kExpectedKeyboardSequence[currentSequenceIndex], kExpectedKeyboardSequence[currentSequenceIndex + 1]};

            MockKeyboard actualState;
            SState actualVirtualControllerState = kEmptyVirtualControllerState;

            actualState.BeginCapture();
            mapper.ContributeFromAnalogValue(actualVirtualControllerState, (int16_t)analogValue);
            actualState.EndCapture();

            TEST_ASSERT(actualVirtualControllerState == kEmptyVirtualControllerState);

            if (actualState == *possibleExpectedStates[0])
            {
                continue;
            }
            else if (actualState == *possibleExpectedStates[1])
            {
                currentSequenceIndex += 1;
                continue;
            }
            else
            {
                TEST_FAILED_BECAUSE(L"Out-of-sequence key state produced by a keyboard key mapper with analog input %d.", analogValue);
            }
        }

        // The last value in the allowed values array is a sentinel just for ease of implementation.
        // We do, however, expect that all other values will have been reached.
        TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedKeyboardSequence) - 1) - 1);
    }

    // Verifies a sequence of contributions of extreme and neutral analog values lead to a keyboard mapper pressing, then unpressing, a keyboard key.
    TEST_CASE(KeyboardMapper_ContributeFromAnalogValue_PressUnpressSequence)
    {
        constexpr int16_t kAnalogValuePress = kAnalogValueMax;
        constexpr int16_t kAnalogValueRelease = kAnalogValueNeutral;
        
        MockKeyboard expectedKeyboardStateUnpressed;

        MockKeyboard expectedKeyboardStatePressed;
        expectedKeyboardStatePressed.SubmitKeyPressedState(kTestKeyIdentifier);

        MockKeyboard actualKeyboardState;
        SState unusedVirtualControllerState;

        const KeyboardMapper mapper(kTestKeyIdentifier);

        actualKeyboardState.BeginCapture();
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStateUnpressed);
        mapper.ContributeFromAnalogValue(unusedVirtualControllerState, kAnalogValuePress);
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStatePressed);
        mapper.ContributeFromAnalogValue(unusedVirtualControllerState, kAnalogValueRelease);
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStateUnpressed);
        actualKeyboardState.EndCapture();
    }

    // Verifies the nominal behavior in which a keyboard mapper is asked to contribute some arbitrary button press state to a keyboard key.
    TEST_CASE(KeyboardMapper_ContributeFromButtonValue_Nominal)
    {
        constexpr bool kButtonStates[] = {false, true};

        for (bool buttonIsPressed : kButtonStates)
        {
            constexpr KeyboardMapper mapper(kTestKeyIdentifier);

            SState unusedVirtualControllerState;

            MockKeyboard expectedState;
            if (true == buttonIsPressed)
                expectedState.SubmitKeyPressedState(kTestKeyIdentifier);

            MockKeyboard actualState;
            actualState.BeginCapture();
            mapper.ContributeFromButtonValue(unusedVirtualControllerState, buttonIsPressed);
            actualState.EndCapture();

            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies a sequence of contributions of pressed and unpressed button values lead to a keyboard mapper pressing, then unpressing, a keyboard key.
    TEST_CASE(KeyboardMapper_ContributeFromButtonValue_PressUnpressSequence)
    {
        constexpr bool kButtonValuePress = true;
        constexpr bool kButtonValueRelease = false;

        MockKeyboard expectedKeyboardStateUnpressed;

        MockKeyboard expectedKeyboardStatePressed;
        expectedKeyboardStatePressed.SubmitKeyPressedState(kTestKeyIdentifier);

        MockKeyboard actualKeyboardState;
        SState unusedVirtualControllerState;

        const KeyboardMapper mapper(kTestKeyIdentifier);

        actualKeyboardState.BeginCapture();
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStateUnpressed);
        mapper.ContributeFromButtonValue(unusedVirtualControllerState, kButtonValuePress);
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStatePressed);
        mapper.ContributeFromButtonValue(unusedVirtualControllerState, kButtonValueRelease);
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStateUnpressed);
        actualKeyboardState.EndCapture();
    }

    // Verifies the nominal behavior in which a keyboard mapper is asked to contribute a trigger value to a keyboard key.
    // Expected behavior is the keyboard key is not pressed at the start and becomes pressed once the trigger value hits a threshold, but the exact transition point is not defined.
    // Sweeps the entire range of possible trigger values.
    TEST_CASE(KeyboardMapper_ContributeFromTriggerValue_Nominal)
    {
        MockKeyboard expectedKeyboardStateUnpressed;

        MockKeyboard expectedKeyboardStatePressed;
        expectedKeyboardStatePressed.SubmitKeyPressedState(kTestKeyIdentifier);

        // Expected sequence, based on an analog value sweep, is pressed, not pressed, and finally pressed.
        // The final two values are the same as a way of simplifying the implementation thus disabling a final transition and triggering a test failure.
        const MockKeyboard* const kExpectedKeyboardSequence[] = { &expectedKeyboardStatePressed, &expectedKeyboardStateUnpressed, &expectedKeyboardStatePressed, &expectedKeyboardStatePressed };
        int currentSequenceIndex = 0;

        for (int32_t triggerValue = kTriggerValueMin; triggerValue <= kTriggerValueMax; ++triggerValue)
        {
            const KeyboardMapper mapper(kTestKeyIdentifier);
            const MockKeyboard* possibleExpectedStates[2] = { kExpectedKeyboardSequence[currentSequenceIndex], kExpectedKeyboardSequence[currentSequenceIndex + 1] };

            MockKeyboard actualState;
            SState actualVirtualControllerState = kEmptyVirtualControllerState;

            actualState.BeginCapture();
            mapper.ContributeFromTriggerValue(actualVirtualControllerState, (uint8_t)triggerValue);
            actualState.EndCapture();

            TEST_ASSERT(actualVirtualControllerState == kEmptyVirtualControllerState);

            if (actualState == *possibleExpectedStates[0])
            {
                continue;
            }
            else if (actualState == *possibleExpectedStates[1])
            {
                currentSequenceIndex += 1;
                continue;
            }
            else
            {
                TEST_FAILED_BECAUSE(L"Out-of-sequence key state produced by a keyboard key mapper with trigger input %d.", triggerValue);
            }
        }

        // The last value in the allowed values array is a sentinel just for ease of implementation.
        // We do, however, expect that all other values will have been reached.
        TEST_ASSERT(currentSequenceIndex == (_countof(kExpectedKeyboardSequence) - 1) - 1);
    }

    // Verifies a sequence of contributions of extreme and neutral trigger values lead to a keyboard mapper pressing, then unpressing, a keyboard key.
    TEST_CASE(KeyboardMapper_ContributeFromTriggerValue_PressUnpressSequence)
    {
        constexpr uint8_t kTriggerValuePress = kTriggerValueMax;
        constexpr uint8_t kTriggerValueRelease = kTriggerValueMin;

        MockKeyboard expectedKeyboardStateUnpressed;

        MockKeyboard expectedKeyboardStatePressed;
        expectedKeyboardStatePressed.SubmitKeyPressedState(kTestKeyIdentifier);

        MockKeyboard actualKeyboardState;
        SState unusedVirtualControllerState;

        const KeyboardMapper mapper(kTestKeyIdentifier);

        actualKeyboardState.BeginCapture();
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStateUnpressed);
        mapper.ContributeFromTriggerValue(unusedVirtualControllerState, kTriggerValuePress);
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStatePressed);
        mapper.ContributeFromTriggerValue(unusedVirtualControllerState, kTriggerValueRelease);
        TEST_ASSERT(actualKeyboardState == expectedKeyboardStateUnpressed);
        actualKeyboardState.EndCapture();
    }

    // Verifies that a keyboard mapper causes a key to be released when it is asked for a neutral contribution.
    TEST_CASE(KeyboardMapper_ContributeNeutral)
    {
        constexpr KeyboardMapper mapper(kTestKeyIdentifier);

        SState unusedVirtualControllerState;

        MockKeyboard expectedState;
        
        MockKeyboard actualState;
        actualState.SubmitKeyPressedState(kTestKeyIdentifier);

        actualState.BeginCapture();
        mapper.ContributeNeutral(unusedVirtualControllerState);
        actualState.EndCapture();

        TEST_ASSERT(actualState == expectedState);
    }
}
