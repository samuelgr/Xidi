/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualControllerTest.cpp
 *   Unit tests for virtual controller objects.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "MockPhysicalController.h"
#include "StateChangeEventBuffer.h"
#include "TestCase.h"
#include "VirtualController.h"

#include <cmath>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <xinput.h>


namespace XidiTest
{
    using namespace ::Xidi;
    using ::Xidi::Controller::AxisMapper;
    using ::Xidi::Controller::ButtonMapper;
    using ::Xidi::Controller::EAxis;
    using ::Xidi::Controller::EButton;
    using ::Xidi::Controller::EElementType;
    using ::Xidi::Controller::EPovDirection;
    using ::Xidi::Controller::Mapper;
    using ::Xidi::Controller::PovMapper;
    using ::Xidi::Controller::SPhysicalState;
    using ::Xidi::Controller::StateChangeEventBuffer;
    using ::Xidi::Controller::TControllerIdentifier;
    using ::Xidi::Controller::VirtualController;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Axis to use when testing with a single axis.
    static constexpr EAxis kTestSingleAxis = EAxis::X;

    /// Test mapper for axis property tests. Contains a single axis.
    static const Mapper kTestSingleAxisMapper({
        .stickLeftX = std::make_unique<AxisMapper>(kTestSingleAxis)
    });

    /// Test mapper used for larger controller state tests.
    /// Describes a virtual controller with 4 axes, 4 buttons, and a POV.
    /// Contains only a subset of the XInput controller elements.
    static const Mapper kTestMapper({
        .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
        .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
        .stickRightX = std::make_unique<AxisMapper>(EAxis::RotX),
        .stickRightY = std::make_unique<AxisMapper>(EAxis::RotY),
        .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
        .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
        .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
        .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
        .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
        .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
        .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
        .buttonY = std::make_unique<ButtonMapper>(EButton::B4)
    });

    /// Number of milliseconds to wait before declaring a timeout while waiting for a state change event to be signalled.
    /// Should be some small multiple of the approximate default system time slice length.
    static constexpr DWORD kTestStateChangeEventTimeoutMilliseconds = 100;


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Modifies a controller state object by applying to it an updated value contained within a state change event.
    /// @param [in] eventData State change event data.
    /// @param [in,out] controllerState Controller state object to be modified.
    static void ApplyUpdateToControllerState(const StateChangeEventBuffer::SEventData& eventData, Controller::SState& controllerState)
    {
        switch (eventData.element.type)
        {
        case EElementType::Axis:
            controllerState.axis[(int)eventData.element.axis] = eventData.value.axis;
            break;

        case EElementType::Button:
            controllerState.button[(int)eventData.element.button] = eventData.value.button;
            break;

        case EElementType::Pov:
            controllerState.povDirection = eventData.value.povDirection;
            break;
        }
    }
    
    /// Computes and returns the deadzone value that corresponds to the specified percentage of an axis' physical range of motion.
    /// @param [in] pct Desired percentage.
    /// @return Corresponding deadzone value.
    static constexpr uint32_t DeadzoneValueByPercentage(uint32_t pct)
    {
        return ((VirtualController::kAxisDeadzoneMax - VirtualController::kAxisDeadzoneMin) * pct) / 100;
    }

    /// Computes and returns the saturation value that corresponds to the specified percentage of an axis' physical range of motion.
    /// @param [in] pct Desired percentage.
    /// @return Corresponding saturation value.
    static constexpr uint32_t SaturationValueByPercentage(uint32_t pct)
    {
        return ((VirtualController::kAxisSaturationMax - VirtualController::kAxisSaturationMin) * pct) / 100;
    }
    
    /// Helper function for performing the boilerplate operations needed to ask a virtual controller object to apply axis properties to an input axis value and retrieve the result.
    /// @param [in] controller Virtual controller object pre-configured with a mapper and the right axis properties for the test.
    /// @param [in] inputAxisValue Axis value to provide as input.
    /// @return Result obtained from the virtual controller transforming the input axis values using the properties with which it was previously configured.
    static int32_t GetAxisPropertiesApplyResult(const VirtualController& controller, int32_t inputAxisValue)
    {
        Controller::SState controllerState;
        ZeroMemory(&controllerState, sizeof(controllerState));
        controllerState.axis[(int)EAxis::X] = inputAxisValue;

        controller.ApplyProperties(controllerState);
        return controllerState.axis[(int)kTestSingleAxis];
    }

    /// Main test body for all axis property tests.
    /// Axis properties are deadzone, range, and saturation. The net result is to divide the expected output values into 5 regions.
    /// Region 1 is the negative saturation region, from extreme negative to the negative saturation cutoff point, in which output values are always the configured range minimum.
    /// Region 2 is the negative axis region, from negative saturation cutoff to negative deadzone cutoff, in which output values steadily progress from configured range minimum to neutral.
    /// Region 3 is the deadzone region, from negative deadzone cutoff to positive deadzone cutoff, in which output values are always the configured range neutral.
    /// Region 4 is the positive axis region, from positive deadzone cutoff to positive saturation cutoff, in which output values steadily progress from configured range neutral to maximum.
    /// Region 5 is the positive saturation region, from positive saturation cutoff to extreme positive, in which output values are always the configured range maximum.
    /// Throughout the test monotonicity of the axis output is also verified.
    /// See DirectInput documentation for more information on how properties work, which in turn covers allowed values for the parameters.
    /// @param [in] rangeMin Minimum range value with which to configure the axis.
    /// @param [in] rangeMax Maximum range value with which to configure the axis.
    /// @param [in] deadzone Deadzone value with which to configure the axis, expressed as a proportion of the axis' physical range of motion.
    /// @param [in] saturation Saturation value with which to configure the axis, expressed as a proportion of the axis' physical range of motion.
    static void TestVirtualControllerApplyAxisProperties(int32_t rangeMin, int32_t rangeMax, uint32_t deadzone = VirtualController::kAxisDeadzoneMin, uint32_t saturation = VirtualController::kAxisSaturationMax)
    {
        const int32_t rangeNeutral = ((rangeMin + rangeMax) / 2);
        
        // Cutoff points between regions.
        const int32_t kRawSaturationCutoffNegative = Controller::kAnalogValueNeutral + ((int32_t)((double)(Controller::kAnalogValueMin - Controller::kAnalogValueNeutral) * ((double)saturation / (double)VirtualController::kAxisSaturationMax)));
        const int32_t kRawDeadzoneCutoffNegative = Controller::kAnalogValueNeutral + ((int32_t)((double)(Controller::kAnalogValueMin - Controller::kAnalogValueNeutral) * ((double)deadzone / (double)VirtualController::kAxisDeadzoneMax)));
        const int32_t kRawDeadzoneCutoffPositive = Controller::kAnalogValueNeutral + ((int32_t)((double)(Controller::kAnalogValueMax - Controller::kAnalogValueNeutral) * ((double)deadzone / (double)VirtualController::kAxisDeadzoneMax)));
        const int32_t kRawSaturationCutoffPositive = Controller::kAnalogValueNeutral + ((int32_t)((double)(Controller::kAnalogValueMax - Controller::kAnalogValueNeutral) * ((double)saturation / (double)VirtualController::kAxisSaturationMax)));

        // Output monotonicity check variable.
        int32_t lastOutputAxisValue = rangeMin;

        VirtualController controller(0, kTestSingleAxisMapper);
        TEST_ASSERT(true == controller.SetAxisDeadzone(kTestSingleAxis, deadzone));
        TEST_ASSERT(true == controller.SetAxisRange(kTestSingleAxis, rangeMin, rangeMax));
        TEST_ASSERT(true == controller.SetAxisSaturation(kTestSingleAxis, saturation));
        TEST_ASSERT(controller.GetAxisDeadzone(kTestSingleAxis) == deadzone);
        TEST_ASSERT(controller.GetAxisRange(kTestSingleAxis) == std::make_pair(rangeMin, rangeMax));
        TEST_ASSERT(controller.GetAxisSaturation(kTestSingleAxis) == saturation);
        
        // Region 1
        for (int32_t inputAxisValue = Controller::kAnalogValueMin; inputAxisValue < kRawSaturationCutoffNegative; ++inputAxisValue)
        {
            const int32_t expectedOutputAxisValue = rangeMin;
            const int32_t actualOutputAxisValue = GetAxisPropertiesApplyResult(controller, inputAxisValue);
            TEST_ASSERT(actualOutputAxisValue == expectedOutputAxisValue);
            TEST_ASSERT(actualOutputAxisValue >= lastOutputAxisValue);
            lastOutputAxisValue = actualOutputAxisValue;
        }

        // Region 2
        // Allow for a small amount of mathematical imprecision by checking for an absolute value difference instead of equality.
        for (int32_t inputAxisValue = kRawSaturationCutoffNegative; inputAxisValue < kRawDeadzoneCutoffNegative; ++inputAxisValue)
        {
            const double kRegionStepSize = (double)(rangeNeutral - rangeMin) / (double)(kRawDeadzoneCutoffNegative - kRawSaturationCutoffNegative);
            const double expectedOutputAxisValue = (double)rangeMin + ((double)(inputAxisValue - kRawSaturationCutoffNegative) * kRegionStepSize);
            const int32_t actualOutputAxisValue = GetAxisPropertiesApplyResult(controller, inputAxisValue);
            TEST_ASSERT(abs(actualOutputAxisValue - expectedOutputAxisValue) <= 1.0);
            TEST_ASSERT(actualOutputAxisValue >= lastOutputAxisValue);
            lastOutputAxisValue = actualOutputAxisValue;
        }

        // Region 3
        for (int32_t inputAxisValue = kRawDeadzoneCutoffNegative; inputAxisValue <= kRawDeadzoneCutoffPositive; ++inputAxisValue)
        {
            const int32_t expectedOutputAxisValue = rangeNeutral;
            const int32_t actualOutputAxisValue = GetAxisPropertiesApplyResult(controller, inputAxisValue);
            TEST_ASSERT(actualOutputAxisValue == expectedOutputAxisValue);
            TEST_ASSERT(actualOutputAxisValue >= lastOutputAxisValue);
            lastOutputAxisValue = actualOutputAxisValue;
        }

        // Region 4
        // Allow for a small amount of mathematical imprecision by checking for an absolute value difference instead of equality.
        for (int32_t inputAxisValue = (1 + kRawDeadzoneCutoffPositive); inputAxisValue <= kRawSaturationCutoffPositive; ++inputAxisValue)
        {
            const double kRegionStepSize = (double)(rangeMax - rangeNeutral) / (double)(kRawSaturationCutoffPositive - kRawDeadzoneCutoffPositive);
            const double expectedOutputAxisValue = (double)rangeNeutral + ((double)(inputAxisValue - kRawDeadzoneCutoffPositive) * kRegionStepSize);
            const int32_t actualOutputAxisValue = GetAxisPropertiesApplyResult(controller, inputAxisValue);
            TEST_ASSERT(abs(actualOutputAxisValue - expectedOutputAxisValue) <= 1.0);
            TEST_ASSERT(actualOutputAxisValue >= lastOutputAxisValue);
            lastOutputAxisValue = actualOutputAxisValue;
        }

        // Region 5
        for (int32_t inputAxisValue = (1 + kRawSaturationCutoffPositive); inputAxisValue <= Controller::kAnalogValueMax; ++inputAxisValue)
        {
            const int32_t expectedOutputAxisValue = rangeMax;
            const int32_t actualOutputAxisValue = GetAxisPropertiesApplyResult(controller, inputAxisValue);
            TEST_ASSERT(actualOutputAxisValue == expectedOutputAxisValue);
            TEST_ASSERT(actualOutputAxisValue >= lastOutputAxisValue);
            lastOutputAxisValue = actualOutputAxisValue;
        }
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies that virtual controllers correctly retrieve and return their associated capabilities.
    TEST_CASE(VirtualController_GetCapabilities)
    {
        const Mapper* mappers[] = {&kTestSingleAxisMapper, &kTestMapper};

        for (auto mapper : mappers)
        {
            VirtualController controller(0, *mapper);
            TEST_ASSERT(mapper->GetCapabilities() == controller.GetCapabilities());
        }
    }
    
    // Verifies that virtual controllers correctly fill in controller state structures based on data received from XInput controllers.
    // Each time the virtual controller queries XInput it gets a new data packet.
    TEST_CASE(VirtualController_GetState_Nominal)
    {
        constexpr TControllerIdentifier kControllerIndex = 2;
        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_B}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_X}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_Y}}}
        };

        // Button assignments are based on the mapper defined at the top of this file.
        constexpr Controller::SState kExpectedStates[] = {
            {.button = 0b0001},    // A
            {.button = 0b0010},    // B
            {.button = 0b0100},    // X
            {.button = 0b1000},    // Y
        };

        VirtualController controller(kControllerIndex, kTestMapper);
        for (int i = 0; i < _countof(kExpectedStates); ++i)
        {
            controller.RefreshState(kPhysicalStates[i]);

            const Controller::SState kActualState = controller.GetState();
            TEST_ASSERT(kActualState == kExpectedStates[i]);
        }
    }

    // Verifies that virtual controllers correctly fill in controller state structures based on data received from XInput controllers.
    // Each time the virtual controller queries XInput it gets the same data packet.
    TEST_CASE(VirtualController_GetState_SameState)
    {
        constexpr TControllerIdentifier kControllerIndex = 3;
        constexpr SPhysicalState kPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X}}};

        // Button assignments are based on the mapper defined at the top of this file.
        constexpr Controller::SState kExpectedStates[] = {
            {.button = 0b0101},     // A, X
            {.button = 0b0101},     // A, X
            {.button = 0b0101},     // A, X
            {.button = 0b0101}      // A, X
        };

        VirtualController controller(kControllerIndex, kTestMapper);
        for (const auto& expectedState : kExpectedStates)
        {
            controller.RefreshState(kPhysicalState);

            const Controller::SState actualState = controller.GetState();
            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies that virtual controllers are correctly reported as being completely neutral when an XInput error occurs.
    TEST_CASE(VirtualController_GetState_XInputErrorMeansNeutral)
    {
        constexpr TControllerIdentifier kControllerIndex = 1;

        // It is not obvious from documentation how packet numbers are supposed to behave across error conditions.
        // Nominal case is packet number increases, and the other two possibilities are packet number stays the same or decreases. All three are tested below in that order.
        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS,                .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y}}},
            {.errorCode = ERROR_DEVICE_NOT_CONNECTED},
            {.errorCode = ERROR_SUCCESS,                .state = {.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y}}},
            {.errorCode = ERROR_SUCCESS,                .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_Y}}},
            {.errorCode = ERROR_INVALID_ACCESS},
            {.errorCode = ERROR_SUCCESS,                .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_Y}}},
            {.errorCode = ERROR_SUCCESS,                .state = {.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y}}},
            {.errorCode = ERROR_NOT_SUPPORTED},
            {.errorCode = ERROR_SUCCESS,                .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y}}}
        };

        // When XInput calls fail, the controller state should be completely neutral.
        // Button assignments are based on the mapper defined at the top of this file.
        constexpr Controller::SState kExpectedStates[] = {
            {.button = 0b1001},     // A, Y
            {},
            {.button = 0b1001},     // A, Y
            {.button = 0b1010},     // B, Y
            {},
            {.button = 0b1010},     // B, Y
            {.button = 0b1100},     // X, Y
            {},
            {.button = 0b1100}      // X, Y
        };

        VirtualController controller(kControllerIndex, kTestMapper);
        for (int i = 0; i < _countof(kExpectedStates); ++i)
        {
            controller.RefreshState(kPhysicalStates[i]);

            const Controller::SState kActualState = controller.GetState();
            TEST_ASSERT(kActualState == kExpectedStates[i]);
        }
    }

    // Verifies that attempting to obtain a controller lock results in an object that does, in fact, own the mutex with which it is associated.
    TEST_CASE(VirtualController_Lock)
    {
        VirtualController controller(0, kTestSingleAxisMapper);
        auto lock = controller.Lock();
        TEST_ASSERT(true == lock.owns_lock());
    }


    // The following sequence of tests, which together comprise the ApplyAxisProperties suite, verify that properties can be correctly applied to an axis value.
    // Each test case follows the basic steps of declaring test data, sweeping through raw axis values, and verifying that the output curve matches expectation.

    // Nominal case. Default property values.
    TEST_CASE(VirtualController_ApplyAxisProperties_Nominal)
    {
        TestVirtualControllerApplyAxisProperties(Controller::kAnalogValueMin, Controller::kAnalogValueMax, VirtualController::kAxisDeadzoneMin, VirtualController::kAxisSaturationMax);
    }

    // Deadzone sweep in increments of 5%, no saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_Deadzone)
    {
        constexpr int32_t kDeadzoneIncrement = DeadzoneValueByPercentage(5);

        for (uint32_t deadzone = VirtualController::kAxisDeadzoneMin; deadzone <= VirtualController::kAxisDeadzoneMax; deadzone += kDeadzoneIncrement)
            TestVirtualControllerApplyAxisProperties(Controller::kAnalogValueMin, Controller::kAnalogValueMax, deadzone, VirtualController::kAxisSaturationMax);
    }

    // Saturation sweep in increments of 5%, no deadzone.
    TEST_CASE(VirtualController_ApplyAxisProperties_Saturation)
    {
        constexpr int32_t kSaturationIncrement = SaturationValueByPercentage(5);

        for (uint32_t saturation = VirtualController::kAxisSaturationMin; saturation <= VirtualController::kAxisSaturationMax; saturation += kSaturationIncrement)
            TestVirtualControllerApplyAxisProperties(Controller::kAnalogValueMin, Controller::kAnalogValueMax, VirtualController::kAxisDeadzoneMin, saturation);
    }

    // Range is a large pair of values centered at zero. Tested first without deadzone or saturation and then with two different fairly common configurations of deadzone and saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_RangeLarge)
    {
        TestVirtualControllerApplyAxisProperties(-10000000, 10000000);
        TestVirtualControllerApplyAxisProperties(-10000000, 10000000, DeadzoneValueByPercentage(10), SaturationValueByPercentage(90));
        TestVirtualControllerApplyAxisProperties(-10000000, 10000000, DeadzoneValueByPercentage(25), SaturationValueByPercentage(75));
    }

    // Range is a large pair of values all of which are positive. Tested first without deadzone or saturation and then with two different fairly common configurations of deadzone and saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_RangeLargePositive)
    {
        TestVirtualControllerApplyAxisProperties(0, 10000000);
        TestVirtualControllerApplyAxisProperties(0, 10000000, DeadzoneValueByPercentage(10), SaturationValueByPercentage(90));
        TestVirtualControllerApplyAxisProperties(0, 10000000, DeadzoneValueByPercentage(25), SaturationValueByPercentage(75));
    }

    // Range is a large pair of values all of which are negative. Tested first without deadzone or saturation and then with two different fairly common configurations of deadzone and saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_RangeLargeNegative)
    {
        TestVirtualControllerApplyAxisProperties(-10000000, 0);
        TestVirtualControllerApplyAxisProperties(-10000000, 0, DeadzoneValueByPercentage(10), SaturationValueByPercentage(90));
        TestVirtualControllerApplyAxisProperties(-10000000, 0, DeadzoneValueByPercentage(25), SaturationValueByPercentage(75));
    }

    // Range is a small pair of values centered at zero. Tested first without deadzone or saturation and then with two different fairly common configurations of deadzone and saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_RangeSmall)
    {
        TestVirtualControllerApplyAxisProperties(-100, 100);
        TestVirtualControllerApplyAxisProperties(-100, 100, DeadzoneValueByPercentage(10), SaturationValueByPercentage(90));
        TestVirtualControllerApplyAxisProperties(-10000000, 0, DeadzoneValueByPercentage(25), SaturationValueByPercentage(75));
    }

    // Range is a small pair of values all of which are positive. Tested first without deadzone or saturation and then with two different fairly common configurations of deadzone and saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_RangeSmallPositive)
    {
        TestVirtualControllerApplyAxisProperties(0, 100);
        TestVirtualControllerApplyAxisProperties(0, 100, DeadzoneValueByPercentage(10), SaturationValueByPercentage(90));
        TestVirtualControllerApplyAxisProperties(-10000000, 0, DeadzoneValueByPercentage(25), SaturationValueByPercentage(75));
    }

    // Range is a small pair of values all of which are negative. Tested first without deadzone or saturation and then with two different fairly common configurations of deadzone and saturation.
    TEST_CASE(VirtualController_ApplyAxisProperties_RangeSmallNegative)
    {
        TestVirtualControllerApplyAxisProperties(-100, 0);
        TestVirtualControllerApplyAxisProperties(-100, 0, DeadzoneValueByPercentage(10), SaturationValueByPercentage(90));
        TestVirtualControllerApplyAxisProperties(-10000000, 0, DeadzoneValueByPercentage(25), SaturationValueByPercentage(75));
    }


    // The following sequence of tests, which together comprise the SetProperty suite, verify that properties are correctly set if valid and rejected if invalid.
    // Each test case follows the basic steps of declaring test data, attempting to set properties, and verifying that the outcome matches expectation.

    // Valid deadzone value set on a single axis and then on all axes.
    TEST_CASE(VirtualController_SetProperty_DeadzoneValid)
    {
        constexpr uint32_t kTestDeadzoneValue = VirtualController::kAxisDeadzoneDefault / 2;
        constexpr EAxis kTestDeadzoneAxis = EAxis::RotX;

        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(true == controller.SetAxisDeadzone(kTestDeadzoneAxis, kTestDeadzoneValue));
        
        for (int i = 0; i < (int)EAxis::Count; ++i)
        {
            if ((int)kTestDeadzoneAxis == i)
                TEST_ASSERT(kTestDeadzoneValue == controller.GetAxisDeadzone((EAxis)i));
            else
                TEST_ASSERT(VirtualController::kAxisDeadzoneDefault == controller.GetAxisDeadzone((EAxis)i));
        }

        TEST_ASSERT(true == controller.SetAllAxisDeadzone(kTestDeadzoneValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(kTestDeadzoneValue == controller.GetAxisDeadzone((EAxis)i));
    }

    // Invalid deadzone value set on a single axis and then on all axes.
    TEST_CASE(VirtualController_SetProperty_DeadzoneInvalid)
    {
        constexpr uint32_t kTestDeadzoneValue = VirtualController::kAxisDeadzoneMax + 1;
        constexpr EAxis kTestDeadzoneAxis = EAxis::RotX;

        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(false == controller.SetAxisDeadzone(kTestDeadzoneAxis, kTestDeadzoneValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(VirtualController::kAxisDeadzoneDefault == controller.GetAxisDeadzone((EAxis)i));

        TEST_ASSERT(false == controller.SetAllAxisDeadzone(kTestDeadzoneValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(VirtualController::kAxisDeadzoneDefault == controller.GetAxisDeadzone((EAxis)i));
    }

    // Valid force feedback gain value.
    TEST_CASE(VirtualController_SetProperty_ForceFeedbackGainValid)
    {
        constexpr uint32_t kTestFfGainValue = VirtualController::kFfGainDefault / 2;
        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(true == controller.SetForceFeedbackGain(kTestFfGainValue));
    }

    // Invalid force feedback gain value.
    TEST_CASE(VirtualController_SetProperty_ForceFeedbackGainInvalid)
    {
        constexpr uint32_t kTestFfGainValue = VirtualController::kFfGainMax + 1;
        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(false == controller.SetForceFeedbackGain(kTestFfGainValue));
    }

    // Valid range values set on a single axis and then on all axes.
    TEST_CASE(VirtualController_SetProperty_RangeValid)
    {
        constexpr std::pair<int32_t, int32_t> kTestRangeValue = std::make_pair(-100, 50000);
        constexpr EAxis kTestRangeAxis = EAxis::Y;

        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(true == controller.SetAxisRange(kTestRangeAxis, kTestRangeValue.first, kTestRangeValue.second));

        for (int i = 0; i < (int)EAxis::Count; ++i)
        {
            if ((int)kTestRangeAxis == i)
                TEST_ASSERT(kTestRangeValue == controller.GetAxisRange((EAxis)i));
            else
                TEST_ASSERT(std::make_pair(Controller::kAnalogValueMin, Controller::kAnalogValueMax) == controller.GetAxisRange((EAxis)i));
        }

        TEST_ASSERT(true == controller.SetAllAxisRange(kTestRangeValue.first, kTestRangeValue.second));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(kTestRangeValue == controller.GetAxisRange((EAxis)i));
    }

    // Invalid range values set on a single axis and then on all axes.
    TEST_CASE(VirtualController_SetProperty_RangeInvalid)
    {
        constexpr std::pair<int32_t, int32_t> kTestRangeValue = std::make_pair(50000, 50000);
        constexpr EAxis kTestRangeAxis = EAxis::Y;

        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(false == controller.SetAxisRange(kTestRangeAxis, kTestRangeValue.first, kTestRangeValue.second));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(std::make_pair(Controller::kAnalogValueMin, Controller::kAnalogValueMax) == controller.GetAxisRange((EAxis)i));

        TEST_ASSERT(false == controller.SetAllAxisRange(kTestRangeValue.first, kTestRangeValue.second));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(std::make_pair(Controller::kAnalogValueMin, Controller::kAnalogValueMax) == controller.GetAxisRange((EAxis)i));
    }

    // Valid saturation value set on a single axis and then on all axes.
    TEST_CASE(VirtualController_SetProperty_SaturationValid)
    {
        constexpr uint32_t kTestSaturationValue = VirtualController::kAxisSaturationDefault / 2;
        constexpr EAxis kTestSaturationAxis = EAxis::RotY;

        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(true == controller.SetAxisSaturation(kTestSaturationAxis, kTestSaturationValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
        {
            if ((int)kTestSaturationAxis == i)
                TEST_ASSERT(kTestSaturationValue == controller.GetAxisSaturation((EAxis)i));
            else
                TEST_ASSERT(VirtualController::kAxisSaturationDefault == controller.GetAxisSaturation((EAxis)i));
        }

        TEST_ASSERT(true == controller.SetAllAxisSaturation(kTestSaturationValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(kTestSaturationValue == controller.GetAxisSaturation((EAxis)i));
    }

    // Invalid saturation value set on a single axis and then on all axes.
    TEST_CASE(VirtualController_SetProperty_SaturationInvalid)
    {
        constexpr uint32_t kTestSaturationValue = VirtualController::kAxisSaturationMax + 1;
        constexpr EAxis kTestSaturationAxis = EAxis::RotY;

        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(false == controller.SetAxisSaturation(kTestSaturationAxis, kTestSaturationValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(VirtualController::kAxisSaturationDefault == controller.GetAxisSaturation((EAxis)i));

        TEST_ASSERT(false == controller.SetAllAxisSaturation(kTestSaturationValue));

        for (int i = 0; i < (int)EAxis::Count; ++i)
            TEST_ASSERT(VirtualController::kAxisSaturationDefault == controller.GetAxisSaturation((EAxis)i));
    }

    // Valid property changes that should result in a transformation being applied to the current controller state view even without a state change.
    TEST_CASE(VirtualController_SetProperty_AutoApplyToExistingState)
    {
        constexpr int32_t kTestAxisRangeMin = 500;
        constexpr int32_t kTestAxisRangeMax = 1000;
        constexpr int32_t kTestAxisRangeExpectedNeutralValue = (kTestAxisRangeMin + kTestAxisRangeMax) / 2;

        constexpr SPhysicalState kPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1}};
        constexpr Controller::SState kExpectedStateBefore = {.axis = {0, 0, 0, 0, 0, 0}};
        constexpr Controller::SState kExpectedStateAfter = {.axis = {kTestAxisRangeExpectedNeutralValue, kTestAxisRangeExpectedNeutralValue, 0, kTestAxisRangeExpectedNeutralValue, kTestAxisRangeExpectedNeutralValue, 0}};

        VirtualController controller(0, kTestMapper);
        controller.RefreshState(kPhysicalState);

        const Controller::SState kActualStateBefore = controller.GetState();
        TEST_ASSERT(kActualStateBefore == kExpectedStateBefore);

        controller.SetAllAxisRange(kTestAxisRangeMin, kTestAxisRangeMax);
        const Controller::SState kActualStateAfter = controller.GetState();
        TEST_ASSERT(kActualStateAfter == kExpectedStateAfter);
    }


    // The following sequence of tests, which together comprise the EventBuffer suite, verify that buffered events function correctly.
    // Each test case follows the basic steps of declaring test data, providing a controller with one or more updated controller state snapshots, and verifying that the resulting controller state is consistent with the input updates.

    // Verifies that by default buffered events are disabled.
    TEST_CASE(VirtualController_EventBuffer_DefaultDisabled)
    {
        VirtualController controller(0, kTestMapper);
        TEST_ASSERT(0 == controller.GetEventBufferCapacity());
    }

    // Verifies that buffered events can be enabled.
    TEST_CASE(VirtualController_EventBuffer_CanEnable)
    {
        constexpr uint32_t kEventBufferCapacity = 64;

        VirtualController controller(0, kTestMapper);
        controller.SetEventBufferCapacity(kEventBufferCapacity);
        TEST_ASSERT(kEventBufferCapacity == controller.GetEventBufferCapacity());
    }

    // Applies some neutral state updates to the virtual controller and verifies that no events are generated.
    TEST_CASE(VirtualController_EventBuffer_Neutral)
    {
        constexpr TControllerIdentifier kControllerIndex = 0;
        constexpr uint32_t kEventBufferCapacity = 64;

        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 3}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 5}}
        };

        VirtualController controller(kControllerIndex, kTestMapper);
        controller.SetEventBufferCapacity(kEventBufferCapacity);

        for (int i = 0; i < _countof(kPhysicalStates); ++i)
            controller.RefreshState(kPhysicalStates[i]);

        TEST_ASSERT(0 == controller.GetEventBufferCount());
    }

    // Applies some actual state updates to the virtual controller and verifies that events are correctly generated.
    // The final view of controller state should be the same regardless of whether it is obtained via snapshot or via buffered events.
    TEST_CASE(VirtualController_EventBuffer_MultipleUpdates)
    {
        constexpr TControllerIdentifier kControllerIndex = 0;
        constexpr uint32_t kEventBufferCapacity = 64;

        // Avoid using vertical components of the analog sticks to avoid having to worry about axis inversion.
        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A, .sThumbLX = 1111, .sThumbRX = 2222}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A, .sThumbLX = 3333, .sThumbRX = 4444}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y | XINPUT_GAMEPAD_DPAD_UP, .sThumbLX = -5555, .sThumbRX = -6666}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_DPAD_LEFT}}}
        };

        // Values come from the mapper at the top of this file.
        constexpr Controller::SState kExpectedControllerStates[] = {
            {.axis = {1111, 0, 0, 2222, 0, 0},   .button = 0b0001},
            {.axis = {3333, 0, 0, 4444, 0, 0},   .button = 0b0001},
            {.axis = {-5555, 0, 0, -6666, 0, 0}, .button = 0b1001, .povDirection = {.components = {true, false, false, false}}},
            {.axis = {0, 0, 0, 0, 0, 0},         .button = 0b0000, .povDirection = {.components = {false, false, true, false}}}
        };

        static_assert(_countof(kPhysicalStates) == _countof(kExpectedControllerStates), "Mismatch between number of physical and virtual controller states.");

        // Each iteration of the loop adds one more event to the test.
        // First iteration tests only a single state change, second iteration tests two state changes, and so on.
        for (unsigned int i = 1; i <= _countof(kPhysicalStates); ++i)
        {
            VirtualController controller(kControllerIndex, kTestMapper);
            controller.SetEventBufferCapacity(kEventBufferCapacity);

            uint32_t lastEventCount = controller.GetEventBufferCount();
            TEST_ASSERT(0 == lastEventCount);
            for (unsigned int j = 0; j < i; ++j)
            {
                controller.RefreshState(kPhysicalStates[j]);
                
                TEST_ASSERT(controller.GetEventBufferCount() > lastEventCount);
                lastEventCount = controller.GetEventBufferCount();
            }

            Controller::SState actualStateFromSnapshot = controller.GetState();
            Controller::SState actualStateFromBufferedEvents;
            ZeroMemory(&actualStateFromBufferedEvents, sizeof(actualStateFromBufferedEvents));

            for (unsigned int j = 0; j < controller.GetEventBufferCount(); ++j)
                ApplyUpdateToControllerState(controller.GetEventBufferEvent(j).data, actualStateFromBufferedEvents);

            TEST_ASSERT(actualStateFromSnapshot == kExpectedControllerStates[i - 1]);
            TEST_ASSERT(actualStateFromBufferedEvents == kExpectedControllerStates[i - 1]);
        }
    }

    // Applies some actual state updates to the virtual controller and verifies that events are correctly generated, with certain controller elements filtered out.
    // Similar to above, but the expected states are different because of the event filters.
    TEST_CASE(VirtualController_EventBuffer_UpdatesWithFilter)
    {
        constexpr TControllerIdentifier kControllerIndex = 0;
        constexpr uint32_t kEventBufferCapacity = 64;

        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A, .sThumbLX = 1111, .sThumbLY = 2222}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A, .sThumbLX = 3333, .sThumbLY = 4444}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y | XINPUT_GAMEPAD_DPAD_UP, .sThumbLX = -5555, .sThumbLY = -6666}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_DPAD_LEFT}}}
        };

        // Values come from the mapper at the top of this file.
        // Because the axes are filtered out using an event filter, their values are expected to be 0 irrespective of the values retrieved from XInput.
        constexpr Controller::SState kExpectedControllerStates[] = {
            {.button = 0b0001},
            {.button = 0b0001},
            {.button = 0b1001, .povDirection = {.components = {true, false, false, false}}},
            {.button = 0b0000, .povDirection = {.components = {false, false, true, false}}}
        };

        static_assert(_countof(kPhysicalStates) == _countof(kExpectedControllerStates), "Mismatch between number of physical and virtual controller states.");

        // Each iteration of the loop adds one more event to the test.
        // First iteration tests only a single state change, second iteration tests two state changes, and so on.
        for (unsigned int i = 1; i <= _countof(kPhysicalStates); ++i)
        {
            VirtualController controller(kControllerIndex, kTestMapper);
            controller.SetEventBufferCapacity(kEventBufferCapacity);
            controller.EventFilterRemoveElement({.type = EElementType::Axis, .axis = EAxis::X});
            controller.EventFilterRemoveElement({.type = EElementType::Axis, .axis = EAxis::Y});

            uint32_t lastEventCount = controller.GetEventBufferCount();
            TEST_ASSERT(0 == lastEventCount);
            for (unsigned int j = 0; j < i; ++j)
            {
                controller.RefreshState(kPhysicalStates[j]);

                TEST_ASSERT(controller.GetEventBufferCount() >= lastEventCount);
                lastEventCount = controller.GetEventBufferCount();
            }

            Controller::SState actualStateFromBufferedEvents;
            ZeroMemory(&actualStateFromBufferedEvents, sizeof(actualStateFromBufferedEvents));

            for (unsigned int j = 0; j < controller.GetEventBufferCount(); ++j)
                ApplyUpdateToControllerState(controller.GetEventBufferEvent(j).data, actualStateFromBufferedEvents);

            TEST_ASSERT(actualStateFromBufferedEvents == kExpectedControllerStates[i - 1]);
        }
    }

    // Submits multiple physical state changes to the physical controller associated with a virtual controller such that every single physical state change causes a virtual controller state change.
    // Enables state change notifications and verifies that each physical controller state change causes a notification to be fired.
    TEST_CASE(VirtualController_StateChangeNotification_Nominal)
    {
        constexpr TControllerIdentifier kControllerIndex = 2;

        // All of the buttons used in these physical states are part of the test mapper defined at the top of this file.
        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A                                              }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B                           }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_DPAD_LEFT}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 5, .Gamepad = {.wButtons =                    XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_DPAD_LEFT}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 6, .Gamepad = {.wButtons =                    XINPUT_GAMEPAD_B                           }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 7}}
        };

        const HANDLE kStateChangeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        TEST_ASSERT((nullptr != kStateChangeEvent) && (INVALID_HANDLE_VALUE != kStateChangeEvent));

        MockPhysicalController physicalController(kControllerIndex, kPhysicalStates, _countof(kPhysicalStates));

        VirtualController controller(kControllerIndex, kTestMapper);
        controller.SetStateChangeEvent(kStateChangeEvent);

        for (int i = 1; i < _countof(kPhysicalStates); ++i)
        {
            physicalController.RequestAdvancePhysicalState();
            TEST_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(kStateChangeEvent, kTestStateChangeEventTimeoutMilliseconds));
        }
    }

    // Submits multiple physical state changes to the physical controller associated with a virtual controller such that every other physical state change causes a virtual controller state change.
    // Enables state change notifications and verifies that each physical controller state change causes a notification to be fired if there is a corresponding virtual controller state change.
    TEST_CASE(VirtualController_StateChangeNotification_SomePhysicalStatesIneffective)
    {
        constexpr TControllerIdentifier kControllerIndex = 3;

        // Only some of the buttons used in these physical states are part of the test mapper defined at the top of this file.
        // Left and right shoulder buttons are not mapped to any virtual controller element, so pressing and releasing them counts as a physical controller state change but not as a virtual controller state change.
        constexpr SPhysicalState kPhysicalStates[] = {
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A                                                                                        }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_LEFT_SHOULDER                                                         }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_DPAD_UP                                }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 5, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_RIGHT_SHOULDER}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 6, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_LEFT_SHOULDER                          | XINPUT_GAMEPAD_RIGHT_SHOULDER}}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 7, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_LEFT_SHOULDER                                                         }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 8, .Gamepad = {.wButtons =                    XINPUT_GAMEPAD_LEFT_SHOULDER                                                         }}},
            {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 9}}
        };
        static_assert(0 != (_countof(kPhysicalStates) % 2), "An even number of states is required beyond the initial physical state.");

        const HANDLE kStateChangeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        TEST_ASSERT((nullptr != kStateChangeEvent) && (INVALID_HANDLE_VALUE != kStateChangeEvent));

        MockPhysicalController physicalController(kControllerIndex, kPhysicalStates, _countof(kPhysicalStates));

        VirtualController controller(kControllerIndex, kTestMapper);
        controller.SetStateChangeEvent(kStateChangeEvent);

        for (int i = 1; i < _countof(kPhysicalStates); i += 2)
        {
            physicalController.RequestAdvancePhysicalState();
            TEST_ASSERT(WAIT_OBJECT_0 == WaitForSingleObject(kStateChangeEvent, kTestStateChangeEventTimeoutMilliseconds));

            physicalController.RequestAdvancePhysicalState();
            TEST_ASSERT(WAIT_TIMEOUT == WaitForSingleObject(kStateChangeEvent, kTestStateChangeEventTimeoutMilliseconds));
        }
    }
}
