/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file VirtualControllerTest.cpp
 *   Unit tests for virtual controller objects.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerElementMapper.h"
#include "ControllerTypes.h"
#include "TestCase.h"
#include "VirtualController.h"
#include "XInputInterface.h"

#include <cmath>
#include <cstdint>
#include <deque>
#include <initializer_list>
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
    using ::Xidi::Controller::EPovDirection;
    using ::Xidi::Controller::Mapper;
    using ::Xidi::Controller::PovMapper;


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


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Mock version the XInput interface, used for test purposes to provide fake XInput data to a virtual controller.
    class MockXInput : public IXInput
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Defines the behavior of a mock method call.
        /// @tparam OutputObjectType Type of method output parameter.
        template <typename OutputObjectType> struct SMethodCallSpec
        {
            DWORD returnCode;                                               ///< Desired return code.
            std::optional<OutputObjectType> maybeOutputObject;              ///< Desired output object. If absent, no object is copied to the output parameter.
            int repeatTimes;                                                ///< Number of times the call should be repeated before it is removed. Zero means the call should happen exactly once.
        };

    
    private:
        // -------- INSTANCE VARIALBES ------------------------------------- //

        /// Expected user index. All calls will fail if they do not match.
        const DWORD kUserIndex;

        /// Expected behavior for calls to #GetState.
        std::deque<SMethodCallSpec<XINPUT_STATE>> callsGetState;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        /// Requires an XInput user index.
        MockXInput(DWORD kUserIndex) : kUserIndex(kUserIndex), callsGetState()
        {
            // Nothing to do here.
        }


    private:
        // -------- CLASS METHODS ------------------------------------------ //

        /// Performs a mock method call.
        /// @tparam OutputObjectType Type of method output parameter.
        /// @param [in] methodName Method name to use when generating error messages.
        /// @param [in,out] callSpecs Ordered list of method call specifications, which is modified during this call as appropriate.
        /// @param [out] outputBuf If the call specification includes an output object, this buffer is filled with the contents of that object.
        /// @return Return code specified in the call specification.
        template <typename OutputObjectType> static DWORD DoMockMethodCall(const wchar_t* methodName, std::deque<SMethodCallSpec<OutputObjectType>>& callSpecs, OutputObjectType* outputBuf)
        {
            if (callSpecs.empty())
                TEST_FAILED_BECAUSE(L"%s: unexpected method call.", methodName);
            
            SMethodCallSpec<OutputObjectType>& callSpec = callSpecs.front();
            const DWORD returnCode = callSpec.returnCode;

            if (callSpec.maybeOutputObject.has_value())
            {
                const OutputObjectType outputObject = callSpec.maybeOutputObject.value();
                memcpy(outputBuf, &outputObject, sizeof(outputObject));
            }

            if (0 == callSpec.repeatTimes)
                callSpecs.pop_front();
            else
                callSpec.repeatTimes -= 1;

            return returnCode;
        }


    public:
        // -------- INSTANCE METHODS --------------------------------------- //

        /// Submits an expected call for the #GetState method.
        /// @param [in] callSpec Specifications that describe the desired behavior of the call.
        inline void ExpectCallGetState(const SMethodCallSpec<XINPUT_STATE>& callSpec)
        {
            callsGetState.push_back(callSpec);
        }

        /// Submits multiple expected calls for the #GetState method.
        /// @param [in] callSpec Specifications that describe the desired behavior of the call.
        inline void ExpectCallGetState(std::initializer_list<const SMethodCallSpec<XINPUT_STATE>> callSpecs)
        {
            for (auto& callSpec : callSpecs)
                ExpectCallGetState(callSpec);
        }


        // -------- CONCRETE INSTANCE METHODS ------------------------------ //

        DWORD GetState(DWORD dwUserIndex, XINPUT_STATE* pState) override
        {
            if (kUserIndex != dwUserIndex)
                TEST_FAILED_BECAUSE(L"XInputGetState: user index mismatch (expected %u, got %u).", kUserIndex, dwUserIndex);
            else if (dwUserIndex >= XUSER_MAX_COUNT)
                TEST_FAILED_BECAUSE(L"XInputGetState: user index too large (%u versus maximum %u).", dwUserIndex, XUSER_MAX_COUNT);

            return DoMockMethodCall(L"XInputGetState", callsGetState, pState);
        }
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

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

        controller.ApplyProperties(&controllerState);
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

        VirtualController controller(0, kTestSingleAxisMapper, std::make_unique<MockXInput>(0));
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
            VirtualController controller(0, *mapper, std::make_unique<MockXInput>(0));
            TEST_ASSERT(mapper->GetCapabilities() == controller.GetCapabilities());
        }
    }
    
    // Verifies that virtual controllers correctly fill in controller state structures based on data received from XInput controllers.
    // Each time the virtual controller queries XInput it gets a new data packet.
    TEST_CASE(VirtualController_GetState_Nominal)
    {
        constexpr int kTotalXInputCalls = 4;
        constexpr VirtualController::TControllerIdentifier kControllerIndex = 2;

        std::unique_ptr<MockXInput> mockXInput = std::make_unique<MockXInput>(kControllerIndex);
        mockXInput->ExpectCallGetState({
            {.returnCode = ERROR_SUCCESS, .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A}})},
            {.returnCode = ERROR_SUCCESS, .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_B}})},
            {.returnCode = ERROR_SUCCESS, .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_X}})},
            {.returnCode = ERROR_SUCCESS, .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_Y}})}
        });

        // Button assignments are based on the mapper defined at the top of this file.
        constexpr Controller::SState kExpectedStates[] = {
            {.button = {true, false, false, false}},    // A
            {.button = {false, true, false, false}},    // B
            {.button = {false, false, true, false}},    // X
            {.button = {false, false, false, true}},    // Y
        };

        static_assert(_countof(kExpectedStates) == kTotalXInputCalls, L"Wrong number of expected controller states versus calls to XInput.");

        VirtualController controller(kControllerIndex, kTestMapper, std::move(mockXInput));
        for (const auto& expectedState : kExpectedStates)
        {
            Controller::SState actualState;
            controller.GetState(&actualState);
            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies that virtual controllers correctly fill in controller state structures based on data received from XInput controllers.
    // Each time the virtual controller queries XInput it gets the same data packet.
    TEST_CASE(VirtualController_GetState_SameState)
    {
        constexpr int kTotalXInputCalls = 4;
        constexpr VirtualController::TControllerIdentifier kControllerIndex = 3;

        std::unique_ptr<MockXInput> mockXInput = std::make_unique<MockXInput>(kControllerIndex);
        mockXInput->ExpectCallGetState(
            {.returnCode = ERROR_SUCCESS, .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X}}), .repeatTimes = (kTotalXInputCalls - 1)}
        );

        // Button assignments are based on the mapper defined at the top of this file.
        constexpr Controller::SState kExpectedStates[] = {
            {.button = {true, false, true, false}},     // A, X
            {.button = {true, false, true, false}},     // A, X
            {.button = {true, false, true, false}},     // A, X
            {.button = {true, false, true, false}}      // A, X
        };

        static_assert(_countof(kExpectedStates) == kTotalXInputCalls, L"Wrong number of expected controller states versus calls to XInput.");

        VirtualController controller(kControllerIndex, kTestMapper, std::move(mockXInput));
        for (const auto& expectedState : kExpectedStates)
        {
            Controller::SState actualState;
            controller.GetState(&actualState);
            TEST_ASSERT(actualState == expectedState);
        }
    }

    // Verifies that virtual controllers are correctly reported as being completely neutral when an XInput error occurs.
    TEST_CASE(VirtualController_GetState_XInputErrorMeansNeutral)
    {
        constexpr int kTotalXInputCalls = 9;
        constexpr VirtualController::TControllerIdentifier kControllerIndex = 1;

        // It is not obvious from documentation how packet numbers are supposed to behave across error conditions.
        // Nominal case is packet number increases, and the other two possibilities are packet number stays the same or decreases. All three are tested below in that order.
        std::unique_ptr<MockXInput> mockXInput = std::make_unique<MockXInput>(kControllerIndex);
        mockXInput->ExpectCallGetState({
            {.returnCode = ERROR_SUCCESS,               .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y}})},
            {.returnCode = ERROR_DEVICE_NOT_CONNECTED},
            {.returnCode = ERROR_SUCCESS,               .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 2, .Gamepad = {.wButtons = XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y}})},
            {.returnCode = ERROR_SUCCESS,               .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_Y}})},
            {.returnCode = ERROR_INVALID_ACCESS},
            {.returnCode = ERROR_SUCCESS,               .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 3, .Gamepad = {.wButtons = XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_Y}})},
            {.returnCode = ERROR_SUCCESS,               .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 4, .Gamepad = {.wButtons = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y}})},
            {.returnCode = ERROR_NOT_SUPPORTED},
            {.returnCode = ERROR_SUCCESS,               .maybeOutputObject = XINPUT_STATE({.dwPacketNumber = 1, .Gamepad = {.wButtons = XINPUT_GAMEPAD_X | XINPUT_GAMEPAD_Y}})}
        });

        // When XInput calls fail, the controller state should be completely neutral.
        // Button assignments are based on the mapper defined at the top of this file.
        constexpr Controller::SState kExpectedStates[] = {
            {.button = {true, false, false, true}},     // A, Y
            {},
            {.button = {true, false, false, true}},     // A, Y
            {.button = {false, true, false, true}},     // B, Y
            {},
            {.button = {false, true, false, true}},     // B, Y
            {.button = {false, false, true, true}},     // X, Y
            {},
            {.button = {false, false, true, true}}      // X, Y
        };

        static_assert(_countof(kExpectedStates) == kTotalXInputCalls, L"Wrong number of expected controller states versus calls to XInput.");

        VirtualController controller(kControllerIndex, kTestMapper, std::move(mockXInput));
        for (const auto& expectedState : kExpectedStates)
        {
            Controller::SState actualState;
            controller.GetState(&actualState);
            TEST_ASSERT(actualState == expectedState);
        }
    }


    // The following sequence of tests, which together comprise the ApplyAxisProperties suite, verify that properties can be correctly applied to an axis value.
    // Each test case follows the basic steps of declaring test data, sweeping through raw axis values, and verifying that the output curve matches expectation.

    // Nominal case. Default property values.
    TEST_CASE(VirtualController_ApplyAxisProperties_Nominal)
    {
        TestVirtualControllerApplyAxisProperties(Controller::kAnalogValueMin, Controller::kAnalogValueMax, VirtualController::kAxisDeadzoneDefault, VirtualController::kAxisSaturationDefault);
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
}
