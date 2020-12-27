/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ControllerMapperTest.cpp
 *   Unit tests for entire controller layout mapper objects.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerElementMapper.h"
#include "ControllerMapper.h"
#include "ControllerTypes.h"
#include "TestCase.h"

#include <cstdint>
#include <xinput.h>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Mock version of an element mapper, used for testing purposes to ensure that values read from a controller are correctly routed.
    class MockElementMapper : public IElementMapper
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Enumerates possible expected sources of input values from an XInput controller.
        /// Specifies which of the `ContributeFrom` methods is expected to be invoked.
        enum class EExpectedSource
        {
            Analog,
            Button,
            Trigger
        };

        /// Holds an expected input value, one for each allowed type.
        union UExpectedValue
        {
            int16_t analog;
            bool button;
            uint8_t trigger;

            constexpr inline UExpectedValue(int16_t analog) : analog(analog) {}
            constexpr inline UExpectedValue(bool button) : button(button) {}
            constexpr inline UExpectedValue(uint8_t trigger) : trigger(trigger) {}
        };


    private:
        // -------- INSTANCE VARIABLES --------------------------------- //
        
        /// Specifies the expected source of an input value.
        /// Causes a test to fail if the wrong `ContributeFrom` method is invoked on this object.
        const EExpectedSource expectedSource;

        /// Specifies the expected input value, one for each allowed type.
        /// Which member is valid is determined by the value of #expectedSource.
        const UExpectedValue expectedValue;

        /// Holds the address of a counter that is incremented by 1 whenever this element mapper is asked for a contribution.
        int* const contributionCounter;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

        /// Initialization constructor.
        /// Requires that values for all instance variables be provided.
        /// For tests that do not exercise controller capabilities, type and index can be omitted.
        inline MockElementMapper(EExpectedSource expectedSource, UExpectedValue expectedValue, int* contributionCounter = nullptr) : expectedSource(expectedSource), expectedValue(expectedValue), contributionCounter(contributionCounter)
        {
            // Nothing to do here.
        }


        // -------- CONCRETE INSTANCE METHODS -------------------------- //

        void ContributeFromAnalogValue(SState* controllerState, int16_t analogValue) const override
        {
            if (EExpectedSource::Analog != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected analog).");

            if (expectedValue.analog != analogValue)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong analog value (expected %d, got %d).", (int)expectedValue.analog, (int)analogValue);

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        void ContributeFromButtonValue(SState* controllerState, bool buttonPressed) const override
        {
            if (EExpectedSource::Button != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected button).");

            if (expectedValue.button != buttonPressed)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong button value (expected %s, got %s).", (true == expectedValue.button ? L"'true (pressed)'" : L"'false (not pressed)'"), (true == buttonPressed ? L"'true (pressed)'" : L"'false (not pressed)'"));

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        void ContributeFromTriggerValue(SState* controllerState, uint8_t triggerValue) const override
        {
            if (EExpectedSource::Trigger != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected trigger).");

            if (expectedValue.trigger != triggerValue)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong trigger value (expected %d, got %d).", (int)expectedValue.trigger, (int)triggerValue);

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        int GetTargetElementIndex(void) const override
        {
            return 0;
        }

        EElementType GetTargetElementType(void) const override
        {
            return EElementType::Axis;
        }
    };


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Dummy controller state used for tests that need such an instance but do not care about its contents.
    static SState dummyControllerState;

    
    // -------- TEST CASES ------------------------------------------------- //

    // The following sequence of tests, all named `Route`, verify that a mapper will correctly route a value from various parts of an XInput controller.
    // In this context, "route" means that the correct element mapper is invoked with the correct value source (analog for left and right stick axes, trigger for LT and RT, and buttons for all controller buttons including the d-pad).

    // Left stick, horizontal
    TEST_CASE(ControllerMapper_Route_StickLeftX)
    {
        constexpr int16_t kTestValue = 1111;
        int numContributions = 0;

        const Mapper controllerMapper({.stickLeftX = new MockElementMapper(MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.sThumbLX = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Left stick, vertical
    TEST_CASE(ControllerMapper_Route_StickLeftY)
    {
        constexpr int16_t kTestValue = 2233;
        int numContributions = 0;

        const Mapper controllerMapper({.stickLeftY = new MockElementMapper(MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.sThumbLY = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Right stick, horizontal
    TEST_CASE(ControllerMapper_Route_StickRightX)
    {
        constexpr int16_t kTestValue = 4556;
        int numContributions = 0;

        const Mapper controllerMapper({.stickRightX = new MockElementMapper(MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.sThumbRX = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Right stick, vertical
    TEST_CASE(ControllerMapper_Route_StickRightY)
    {
        constexpr int16_t kTestValue = 6789;
        int numContributions = 0;

        const Mapper controllerMapper({.stickRightY = new MockElementMapper(MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.sThumbRY = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad up
    TEST_CASE(ControllerMapper_Route_DpadUp)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadUp = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_UP : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad down
    TEST_CASE(ControllerMapper_Route_DpadDown)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadDown = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_DOWN : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad left
    TEST_CASE(ControllerMapper_Route_DpadLeft)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadLeft = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_LEFT : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad right
    TEST_CASE(ControllerMapper_Route_DpadRight)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadRight = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_RIGHT : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Left trigger (LT)
    TEST_CASE(ControllerMapper_Route_TriggerLT)
    {
        constexpr uint8_t kTestValue = 45;
        int numContributions = 0;

        const Mapper controllerMapper({.triggerLT = new MockElementMapper(MockElementMapper::EExpectedSource::Trigger, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.bLeftTrigger = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Right trigger (RT)
    TEST_CASE(ControllerMapper_Route_TriggerRT)
    {
        constexpr uint8_t kTestValue = 167;
        int numContributions = 0;

        const Mapper controllerMapper({.triggerRT = new MockElementMapper(MockElementMapper::EExpectedSource::Trigger, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.bRightTrigger = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // A button
    TEST_CASE(ControllerMapper_Route_ButtonA)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonA = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_A : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // B button
    TEST_CASE(ControllerMapper_Route_ButtonB)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonB = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_B : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // X button
    TEST_CASE(ControllerMapper_Route_ButtonX)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonX = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_X : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Y button
    TEST_CASE(ControllerMapper_Route_ButtonY)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonY = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_Y : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // LB button
    TEST_CASE(ControllerMapper_Route_ButtonLB)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonLB = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // RB button
    TEST_CASE(ControllerMapper_Route_ButtonRB)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonRB = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Back button
    TEST_CASE(ControllerMapper_Route_ButtonBack)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonBack = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_BACK : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Start button
    TEST_CASE(ControllerMapper_Route_ButtonStart)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonStart = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_START : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // LS button
    TEST_CASE(ControllerMapper_Route_ButtonLS)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonLS = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_LEFT_THUMB : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // RS button
    TEST_CASE(ControllerMapper_Route_ButtonRS)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonRS = new MockElementMapper(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(&dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_RIGHT_THUMB : 0)});

        TEST_ASSERT(1 == numContributions);
    }
}
