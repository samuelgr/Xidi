/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperTest.cpp
 *   Unit tests for entire controller layout mapper objects.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"
#include "TestCase.h"

#include <cstdint>
#include <memory>
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

        void ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const override
        {
            if (EExpectedSource::Analog != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Analog).", (int)expectedSource);

            if (expectedValue.analog != analogValue)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong analog value (expected %d, got %d).", (int)expectedValue.analog, (int)analogValue);

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        void ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const override
        {
            if (EExpectedSource::Button != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Button).", (int)expectedSource);

            if (expectedValue.button != buttonPressed)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong button value (expected %s, got %s).", (true == expectedValue.button ? L"'true (pressed)'" : L"'false (not pressed)'"), (true == buttonPressed ? L"'true (pressed)'" : L"'false (not pressed)'"));

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        void ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const override
        {
            if (EExpectedSource::Trigger != expectedSource)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong value source (expected enumerator %d, got Trigger).", (int)expectedSource);

            if (expectedValue.trigger != triggerValue)
                TEST_FAILED_BECAUSE(L"MockElementMapper: wrong trigger value (expected %d, got %d).", (int)expectedValue.trigger, (int)triggerValue);

            if (nullptr != contributionCounter)
                *contributionCounter += 1;
        }

        SElementIdentifier GetTargetElement(void) const override
        {
            return {};
        }
    };


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Dummy controller state used for tests that need such an instance but do not care about its contents.
    static SState dummyControllerState;

    
    // -------- TEST CASES ------------------------------------------------- //

    // The following sequence of tests, which together comprise the Route suite, verify that a mapper will correctly route a value from various parts of an XInput controller.
    // In this context, "route" means that the correct element mapper is invoked with the correct value source (analog for left and right stick axes, trigger for LT and RT, and buttons for all controller buttons including the d-pad).

    // Left stick, horizontal
    TEST_CASE(ControllerMapper_Route_StickLeftX)
    {
        constexpr int16_t kTestValue = 1111;
        int numContributions = 0;

        const Mapper controllerMapper({.stickLeftX = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.sThumbLX = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Left stick, vertical
    TEST_CASE(ControllerMapper_Route_StickLeftY)
    {
        constexpr int16_t kTestValue = 2233;
        constexpr int16_t kInvertedTestValue = -kTestValue;
        int numContributions = 0;

        const Mapper controllerMapper({.stickLeftY = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kInvertedTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.sThumbLY = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Right stick, horizontal
    TEST_CASE(ControllerMapper_Route_StickRightX)
    {
        constexpr int16_t kTestValue = 4556;
        int numContributions = 0;

        const Mapper controllerMapper({.stickRightX = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.sThumbRX = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Right stick, vertical
    TEST_CASE(ControllerMapper_Route_StickRightY)
    {
        constexpr int16_t kTestValue = 6789;
        constexpr int16_t kInvertedTestValue = -kTestValue;
        int numContributions = 0;

        const Mapper controllerMapper({.stickRightY = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Analog, kInvertedTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.sThumbRY = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad up
    TEST_CASE(ControllerMapper_Route_DpadUp)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadUp = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_UP : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad down
    TEST_CASE(ControllerMapper_Route_DpadDown)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadDown = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_DOWN : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad left
    TEST_CASE(ControllerMapper_Route_DpadLeft)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadLeft = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_LEFT : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // D-pad right
    TEST_CASE(ControllerMapper_Route_DpadRight)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.dpadRight = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_DPAD_RIGHT : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Left trigger (LT)
    TEST_CASE(ControllerMapper_Route_TriggerLT)
    {
        constexpr uint8_t kTestValue = 45;
        int numContributions = 0;

        const Mapper controllerMapper({.triggerLT = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Trigger, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.bLeftTrigger = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // Right trigger (RT)
    TEST_CASE(ControllerMapper_Route_TriggerRT)
    {
        constexpr uint8_t kTestValue = 167;
        int numContributions = 0;

        const Mapper controllerMapper({.triggerRT = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Trigger, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.bRightTrigger = kTestValue});

        TEST_ASSERT(1 == numContributions);
    }

    // A button
    TEST_CASE(ControllerMapper_Route_ButtonA)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonA = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_A : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // B button
    TEST_CASE(ControllerMapper_Route_ButtonB)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonB = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_B : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // X button
    TEST_CASE(ControllerMapper_Route_ButtonX)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonX = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_X : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Y button
    TEST_CASE(ControllerMapper_Route_ButtonY)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonY = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_Y : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // LB button
    TEST_CASE(ControllerMapper_Route_ButtonLB)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonLB = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // RB button
    TEST_CASE(ControllerMapper_Route_ButtonRB)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonRB = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Back button
    TEST_CASE(ControllerMapper_Route_ButtonBack)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonBack = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_BACK : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // Start button
    TEST_CASE(ControllerMapper_Route_ButtonStart)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonStart = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_START : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // LS button
    TEST_CASE(ControllerMapper_Route_ButtonLS)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonLS = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_LEFT_THUMB : 0)});

        TEST_ASSERT(1 == numContributions);
    }

    // RS button
    TEST_CASE(ControllerMapper_Route_ButtonRS)
    {
        constexpr bool kTestValue = true;
        int numContributions = 0;

        const Mapper controllerMapper({.buttonRS = std::make_unique<MockElementMapper>(MockElementMapper::EExpectedSource::Button, kTestValue, &numContributions)});
        controllerMapper.MapXInputState(dummyControllerState, {.wButtons = (kTestValue ? XINPUT_GAMEPAD_RIGHT_THUMB : 0)});

        TEST_ASSERT(1 == numContributions);
    }


    // The following sequence of tests, which together comprise the Capabilities suite, verify that a mapper correctly produces a virtual controller's capabilities given a set of element mappers.
    // Each test case presents a different controller configuration. The formula for each test case body is create its expected capabilities, obtain a mapper, obtain the mapper's capabilities, and compare the two capabilities objects.
    // First are some synthetic capabilities and towards the end are the known and documented mappers.

    // Empty mapper.
    // Nothing should be present on the virtual controller.
    TEST_CASE(ControllerMapper_Capabilities_EmptyMapper)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .numAxes = 0,
            .numButtons = 0,
            .hasPov = false
        });

        const Mapper mapper({
            // Empty.
        });

        const SCapabilities kActualCapabilities = mapper.GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // Mapper with only buttons, and they are disjoint.
    // Virtual controller should have only buttons, and the number present is based on the highest button to which an element mapper writes.
    TEST_CASE(ControllerMapper_Capabilities_DisjointButtons)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .numAxes = 0,
            .numButtons = 10,
            .hasPov = false
        });

        const Mapper mapper({
            .stickLeftX = std::make_unique<ButtonMapper>(EButton::B2),
            .dpadUp = std::make_unique<ButtonMapper>(EButton::B6),
            .dpadLeft = std::make_unique<ButtonMapper>(EButton::B10),
            .buttonLB = std::make_unique<ButtonMapper>(EButton::B4)
        });

        const SCapabilities kActualCapabilities = mapper.GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // Mapper with only buttons, and all mappers write to the same button.
    // Virtual controller should have only buttons, and the number present is based on the button to which all element mappers write.
    TEST_CASE(ControllerMapper_Capabilities_SingleButton)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .numAxes = 0,
            .numButtons = 6,
            .hasPov = false
        });

        const Mapper mapper({
            .stickLeftY = std::make_unique<ButtonMapper>(EButton::B6),
            .dpadDown = std::make_unique<ButtonMapper>(EButton::B6),
            .buttonStart = std::make_unique<ButtonMapper>(EButton::B6)
        });

        const SCapabilities kActualCapabilities = mapper.GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // Mapper with only axes.
    // Virtual controller should have only axes based on the axes to which the element mappers write.
    TEST_CASE(ControllerMapper_Capabilities_MultipleAxes)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .axisType = {EAxis::Y, EAxis::RotX},
            .numAxes = 2,
            .numButtons = 0,
            .hasPov = false
        });

        const Mapper mapper({
            .stickRightX = std::make_unique<AxisMapper>(EAxis::Y),
            .dpadDown = std::make_unique<AxisMapper>(EAxis::RotX),
            .buttonStart = std::make_unique<AxisMapper>(EAxis::RotX),
            .buttonRS = std::make_unique<AxisMapper>(EAxis::Y)
        });

        const SCapabilities kActualCapabilities = mapper.GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // Mapper with only a POV, and only part of it receives values from mappers.
    // Virtual controller should have only a POV and nothing else.
    TEST_CASE(ControllerMapper_Capabilities_IncompletePov)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .numAxes = 0,
            .numButtons = 0,
            .hasPov = true
        });

        const Mapper mapper({
            .stickRightX = std::make_unique<PovMapper>(EPovDirection::Left)
        });

        const SCapabilities kActualCapabilities = mapper.GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // Mapper with only a complete POV.
    // Virtual controller should have only a POV and nothing else.
    TEST_CASE(ControllerMapper_Capabilities_CompletePov)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .numAxes = 0,
            .numButtons = 0,
            .hasPov = true
        });

        const Mapper mapper({
            .stickLeftY = std::make_unique<PovMapper>(EPovDirection::Left),
            .stickRightX = std::make_unique<PovMapper>(EPovDirection::Right),
            .triggerLT = std::make_unique<PovMapper>(EPovDirection::Up),
            .triggerRT = std::make_unique<PovMapper>(EPovDirection::Down),
            .buttonA = std::make_unique<PovMapper>(EPovDirection::Left),
            .buttonY = std::make_unique<PovMapper>(EPovDirection::Left),
            .buttonLS = std::make_unique<PovMapper>(EPovDirection::Up),
            .buttonRS = std::make_unique<PovMapper>(EPovDirection::Down)
        });

        const SCapabilities kActualCapabilities = mapper.GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // StandardGamepad, a known and documented mapper.
    TEST_CASE(ControllerMapper_Capabilities_StandardGamepad)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .axisType = {EAxis::X, EAxis::Y, EAxis::Z, EAxis::RotZ},
            .numAxes = 4,
            .numButtons = 12,
            .hasPov = true
        });

        const Mapper* const mapper = Mapper::GetByName(L"StandardGamepad");
        TEST_ASSERT(nullptr != mapper);

        const SCapabilities kActualCapabilities = mapper->GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // DigitalGamepad, a known and documented mapper.
    TEST_CASE(ControllerMapper_Capabilities_DigitalGamepad)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .axisType = {EAxis::X, EAxis::Y, EAxis::Z, EAxis::RotZ},
            .numAxes = 4,
            .numButtons = 12,
            .hasPov = false
        });

        const Mapper* const mapper = Mapper::GetByName(L"DigitalGamepad");
        TEST_ASSERT(nullptr != mapper);

        const SCapabilities kActualCapabilities = mapper->GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // ExtendedGamepad, a known and documented mapper.
    TEST_CASE(ControllerMapper_Capabilities_ExtendedGamepad)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .axisType = {EAxis::X, EAxis::Y, EAxis::Z, EAxis::RotX, EAxis::RotY, EAxis::RotZ},
            .numAxes = 6,
            .numButtons = 10,
            .hasPov = true
        });

        const Mapper* const mapper = Mapper::GetByName(L"ExtendedGamepad");
        TEST_ASSERT(nullptr != mapper);

        const SCapabilities kActualCapabilities = mapper->GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // XInputNative, a known and documented mapper.
    TEST_CASE(ControllerMapper_Capabilities_XInputNative)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .axisType = {EAxis::X, EAxis::Y, EAxis::Z, EAxis::RotX, EAxis::RotY, EAxis::RotZ},
            .numAxes = 6,
            .numButtons = 10,
            .hasPov = true
        });

        const Mapper* const mapper = Mapper::GetByName(L"XInputNative");
        TEST_ASSERT(nullptr != mapper);

        const SCapabilities kActualCapabilities = mapper->GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }

    // XInputSharedTriggers, a known and documented mapper.
    TEST_CASE(ControllerMapper_Capabilities_XInputSharedTriggers)
    {
        constexpr SCapabilities kExpectedCapabilities({
            .axisType = {EAxis::X, EAxis::Y, EAxis::Z, EAxis::RotX, EAxis::RotY},
            .numAxes = 5,
            .numButtons = 10,
            .hasPov = true
        });

        const Mapper* const mapper = Mapper::GetByName(L"XInputSharedTriggers");
        TEST_ASSERT(nullptr != mapper);

        const SCapabilities kActualCapabilities = mapper->GetCapabilities();
        TEST_ASSERT(kActualCapabilities == kExpectedCapabilities);
    }


    // The following sequence of tests, which together comprise the State suite, verify that a mapper correctly handles certain corner cases when writing to controller state.
    // The formula for each test case body is create an expected controller state, obtain a mapper, ask it to write to a controller state, and finally compare expected and actual states.
    
    // An empty mapper is expected to produce all zeroes in its output controller state, irrespective of the XInput controller's state.
    TEST_CASE(ControllerMapper_State_ZeroOnEmpty)
    {
        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));

        const Mapper mapper({
            // Empty
        });

        SState actualState;
        FillMemory(&actualState, sizeof(actualState), 0xcd);
        mapper.MapXInputState(actualState, {});
        TEST_ASSERT(actualState == expectedState);

        FillMemory(&actualState, sizeof(actualState), 0xcd);
        mapper.MapXInputState(actualState, {
            .wButtons = 32767,
            .bLeftTrigger = 128,
            .bRightTrigger = 128,
            .sThumbLX = 16383,
            .sThumbLY = -16383,
            .sThumbRX = -16383,
            .sThumbRY = 16383
        });
        TEST_ASSERT(actualState == expectedState);
    }

    // Even though intermediate contributions may result in analog axis values that exceed the allowed range, mappers are expected to saturate at the allowed range.
    // This test verifies correct saturation in the positive direction.
    TEST_CASE(ControllerMapper_State_AnalogSaturationPositive)
    {
        constexpr int32_t kInvertedInputValue = kAnalogValueMin;
        constexpr int32_t kNonInvertedInputValue = kAnalogValueMax;
        constexpr int32_t kExpectedOutputValue = kAnalogValueMax;
        
        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::X] = kExpectedOutputValue;

        const Mapper mapper({
            .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
            .stickLeftY = std::make_unique<AxisMapper>(EAxis::X),
            .stickRightX = std::make_unique<AxisMapper>(EAxis::X),
            .stickRightY = std::make_unique<AxisMapper>(EAxis::X)
        });

        SState actualState;
        mapper.MapXInputState(actualState, {
            .sThumbLX = kNonInvertedInputValue,
            .sThumbLY = kInvertedInputValue,
            .sThumbRX = kNonInvertedInputValue,
            .sThumbRY = kInvertedInputValue
        });
        TEST_ASSERT(actualState == expectedState);
    }

    // Even though intermediate contributions may result in analog axis values that exceed the allowed range, mappers are expected to saturate at the allowed range.
    // This test verifies correct saturation in the negative direction.
    TEST_CASE(ControllerMapper_State_AnalogSaturationNegative)
    {
        constexpr int32_t kInvertedInputValue = kAnalogValueMax;
        constexpr int32_t kNonInvertedInputValue = kAnalogValueMin;
        constexpr int32_t kExpectedOutputValue = kAnalogValueMin;
        
        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::RotX] = kExpectedOutputValue;

        const Mapper mapper({
            .stickLeftX = std::make_unique<AxisMapper>(EAxis::RotX),
            .stickLeftY = std::make_unique<AxisMapper>(EAxis::RotX),
            .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
            .stickRightY = std::make_unique<AxisMapper>(EAxis::RotX)
        });

        SState actualState;
        mapper.MapXInputState(actualState, {
            .sThumbLX = kNonInvertedInputValue,
            .sThumbLY = kInvertedInputValue,
            .sThumbRX = kNonInvertedInputValue,
            .sThumbRY = kInvertedInputValue
        });
        TEST_ASSERT(actualState == expectedState);
    }

    // Incoming controller data uses a range slightly different from virtual controller range.
    // Furthermore, the vertical axes on analog sticks use opposite polarity from what virtual controllers expect and present.
    // Mappers are expected to ensure values are correctly filtered and inverted to compensate.
    TEST_CASE(ControllerMapper_State_AnalogFilterAndInvert)
    {
        constexpr int32_t kExtremeNegativeInputValue = (int32_t)INT16_MIN;
        constexpr int32_t kNonInvertedExpectedOutputValue = kAnalogValueMin;
        constexpr int32_t kInvertedExpectedOutputValue = kAnalogValueMax;

        SState expectedState;
        ZeroMemory(&expectedState, sizeof(expectedState));
        expectedState.axis[(int)EAxis::X] = kNonInvertedExpectedOutputValue;
        expectedState.axis[(int)EAxis::Y] = kInvertedExpectedOutputValue;
        expectedState.axis[(int)EAxis::RotX] = kNonInvertedExpectedOutputValue;
        expectedState.axis[(int)EAxis::RotY] = kInvertedExpectedOutputValue;

        const Mapper mapper({
            .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
            .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
            .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
            .stickRightY = std::make_unique<AxisMapper>(EAxis::RotY)
        });

        SState actualState;
        mapper.MapXInputState(actualState, {
            .sThumbLX = kExtremeNegativeInputValue,
            .sThumbLY = kExtremeNegativeInputValue,
            .sThumbRX = kExtremeNegativeInputValue,
            .sThumbRY = kExtremeNegativeInputValue
        });
        TEST_ASSERT(actualState == expectedState);
    }
}
