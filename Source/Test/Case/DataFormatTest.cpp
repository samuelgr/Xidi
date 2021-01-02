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

#include "ApiDirectInput.h"
#include "ControllerElementMapper.h"
#include "ControllerMapper.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "TestCase.h"

#include <memory>
#include <optional>


namespace XidiTest
{
    using namespace ::Xidi;
    using ::Xidi::Controller::AxisMapper;
    using ::Xidi::Controller::ButtonMapper;
    using ::Xidi::Controller::EAxis;
    using ::Xidi::Controller::EElementType;
    using ::Xidi::Controller::EButton;
    using ::Xidi::Controller::EPovDirection;
    using ::Xidi::Controller::Mapper;
    using ::Xidi::Controller::PovMapper;
    using ::Xidi::Controller::SElementIdentifier;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Test mapper that contains a POV.
    /// Contains 4 axes (RotX and RotY are skipped), 12 buttons, and a POV.
    static const Mapper kTestMapperWithPov({
        .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
        .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
        .stickRightX = std::make_unique<AxisMapper>(EAxis::Z),
        .stickRightY = std::make_unique<AxisMapper>(EAxis::RotZ),
        .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
        .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
        .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
        .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
        .triggerLT = std::make_unique<ButtonMapper>(EButton::B7),
        .triggerRT = std::make_unique<ButtonMapper>(EButton::B8),
        .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
        .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
        .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
        .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
        .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
        .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
        .buttonBack = std::make_unique<ButtonMapper>(EButton::B9),
        .buttonStart = std::make_unique<ButtonMapper>(EButton::B10),
        .buttonLS = std::make_unique<ButtonMapper>(EButton::B11),
        .buttonRS = std::make_unique<ButtonMapper>(EButton::B12)
    });

    /// Test mapper that does not contain a POV.
    /// Contains 4 axes (RotX and RotY are deliberately skipped), and 16 buttons.
    static const Mapper kTestMapperWithoutPov({
        .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
        .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
        .stickRightX = std::make_unique<AxisMapper>(EAxis::Z),
        .stickRightY = std::make_unique<AxisMapper>(EAxis::RotZ),
        .dpadUp = std::make_unique<ButtonMapper>(EButton::B13),
        .dpadDown = std::make_unique<ButtonMapper>(EButton::B14),
        .dpadLeft = std::make_unique<ButtonMapper>(EButton::B15),
        .dpadRight = std::make_unique<ButtonMapper>(EButton::B16),
        .triggerLT = std::make_unique<ButtonMapper>(EButton::B7),
        .triggerRT = std::make_unique<ButtonMapper>(EButton::B8),
        .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
        .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
        .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
        .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
        .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
        .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
        .buttonBack = std::make_unique<ButtonMapper>(EButton::B9),
        .buttonStart = std::make_unique<ButtonMapper>(EButton::B10),
        .buttonLS = std::make_unique<ButtonMapper>(EButton::B11),
        .buttonRS = std::make_unique<ButtonMapper>(EButton::B12)
    });


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Main checks that are part of the CreateSuccess suite of test cases.
    /// Given the information needed to construct a data format object and the data format specification that is expected to result, constructs the data format object, ensures success, and ensures expectation matches actual result.
    /// Once the object has been created, iterates through the specification and verifies that the data format object correctly translates between elements and offsets.
    /// Causes the test case to fail if any of the checks or operations are unsuccessful.
    /// @param [in] appFormatSpec Test data in the form of an application data format specification.
    /// @param [in] controllerCapabilities Test data in the form of a description of a controller's capabilities. Likely obtained from one of the mappers defined above.
    /// @param [in] expectedDataFormatSpec Test data in the form of a data format spec that is expected to be the result of creating a data format object using the provided controller capabilities.
    static void TestDataFormatCreateSuccess(const DIDATAFORMAT& appFormatSpec, const Controller::SCapabilities& controllerCapabilities, const DataFormat::SDataFormatSpec& expectedDataFormatSpec)
    {
        // Create the data format object and assert success, then compare the resulting specification object with the expected specification object.
        std::unique_ptr<DataFormat> dataFormat = DataFormat::CreateFromApplicationFormatSpec(appFormatSpec, controllerCapabilities);
        TEST_ASSERT(nullptr != dataFormat);

        const DataFormat::SDataFormatSpec& actualDataFormatSpec = dataFormat->GetSpec();
        TEST_ASSERT(actualDataFormatSpec == expectedDataFormatSpec);

        // Iterate through the entire expected specification object and verify that all elements are correctly mapped to and from offsets.
        // This exercises all of the element-mapping methods of the data format object in both directions: element to offset and offset to element.
        // First axes, then buttons, and finally the POV.
        for (int i = 0; i < _countof(expectedDataFormatSpec.axisOffset); ++i)
        {
            const SElementIdentifier expectedAxisIdentifier = {.type = EElementType::Axis, .axis = (EAxis)i};
            const TOffset expectedAxisOffset = expectedDataFormatSpec.axisOffset[i];

            if (DataFormat::kInvalidOffsetValue == expectedAxisOffset)
            {
                TEST_ASSERT(false == dataFormat->HasElement(expectedAxisIdentifier));
                TEST_ASSERT(false == dataFormat->GetOffsetForElement(expectedAxisIdentifier).has_value());
            }
            else
            {
                const std::optional<SElementIdentifier> maybeActualAxisIdentifier = dataFormat->GetElementForOffset(expectedAxisOffset);
                const std::optional<TOffset> maybeActualAxisOffset = dataFormat->GetOffsetForElement(expectedAxisIdentifier);
                TEST_ASSERT(true == maybeActualAxisIdentifier.has_value());
                TEST_ASSERT(true == maybeActualAxisOffset.has_value());

                const SElementIdentifier actualAxisIdentifier = maybeActualAxisIdentifier.value();
                const TOffset actualAxisOffset = maybeActualAxisOffset.value();
                TEST_ASSERT(true == dataFormat->HasElement(expectedAxisIdentifier));
                TEST_ASSERT(actualAxisIdentifier == expectedAxisIdentifier);
                TEST_ASSERT(actualAxisOffset == expectedAxisOffset);
                
            }
        }

        for (int i = 0; i < _countof(expectedDataFormatSpec.buttonOffset); ++i)
        {
            const SElementIdentifier expectedButtonIdentifier = {.type = EElementType::Button, .button = (EButton)i};
            const TOffset expectedButtonOffset = expectedDataFormatSpec.buttonOffset[i];

            if (DataFormat::kInvalidOffsetValue == expectedButtonOffset)
            {
                TEST_ASSERT(false == dataFormat->HasElement(expectedButtonIdentifier));
                TEST_ASSERT(false == dataFormat->GetOffsetForElement(expectedButtonIdentifier).has_value());
            }
            else
            {
                const std::optional<SElementIdentifier> maybeActualButtonIdentifier = dataFormat->GetElementForOffset(expectedButtonOffset);
                const std::optional<TOffset> maybeActualButtonOffset = dataFormat->GetOffsetForElement(expectedButtonIdentifier);
                TEST_ASSERT(true == maybeActualButtonIdentifier.has_value());
                TEST_ASSERT(true == maybeActualButtonOffset.has_value());

                const SElementIdentifier actualButtonIdentifier = maybeActualButtonIdentifier.value();
                const TOffset actualButtonOffset = maybeActualButtonOffset.value();
                TEST_ASSERT(true == dataFormat->HasElement(expectedButtonIdentifier));
                TEST_ASSERT(actualButtonIdentifier == expectedButtonIdentifier);
                TEST_ASSERT(actualButtonOffset == expectedButtonOffset);
            }
        }

        do
        {
            const SElementIdentifier expectedPovIdentifier = {.type = EElementType::Pov};
            const TOffset expectedPovOffset = expectedDataFormatSpec.povOffset;

            if (DataFormat::kInvalidOffsetValue == expectedPovOffset)
            {
                TEST_ASSERT(false == dataFormat->HasElement(expectedPovIdentifier));
                TEST_ASSERT(false == dataFormat->GetOffsetForElement(expectedPovIdentifier).has_value());
            }
            else
            {
                const std::optional<SElementIdentifier> maybeActualPovIdentifier = dataFormat->GetElementForOffset(expectedPovOffset);
                const std::optional<TOffset> maybeActualPovOffset = dataFormat->GetOffsetForElement(expectedPovIdentifier);
                TEST_ASSERT(true == maybeActualPovIdentifier.has_value());
                TEST_ASSERT(true == maybeActualPovOffset.has_value());

                const SElementIdentifier actualPovIdentifier = maybeActualPovIdentifier.value();
                const TOffset actualPovOffset = maybeActualPovOffset.value();
                TEST_ASSERT(true == dataFormat->HasElement(expectedPovIdentifier));
                TEST_ASSERT(actualPovIdentifier == expectedPovIdentifier);
                TEST_ASSERT(actualPovOffset == expectedPovOffset);
            }
        } while (false);
    }

    /// Main checks that are part of the CreateFailure suite of test cases.
    /// Given the information needed to construct a data format object, attempts to construct the data format object and asserts that the operation fails.
    /// @param [in] appFormatSpec Test data in the form of an application data format specification.
    /// @param [in] controllerCapabilities Test data in the form of a description of a controller's capabilities. Likely obtained from one of the mappers defined above.
    static void TestDataFormatCreateFailure(const DIDATAFORMAT& appFormatSpec, const Controller::SCapabilities& controllerCapabilities)
    {
        std::unique_ptr<DataFormat> dataFormat = DataFormat::CreateFromApplicationFormatSpec(appFormatSpec, controllerCapabilities);
        TEST_ASSERT(nullptr == dataFormat);
    }


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
            EPovValue expectedPovValue;
        } kPovTestData[] = {
            {.povUp = false, .povDown = false, .povLeft = false, .povRight = false, .expectedPovValue = EPovValue::Center},
            {.povUp = false, .povDown = false, .povLeft = false, .povRight = true,  .expectedPovValue = EPovValue::E},
            {.povUp = false, .povDown = false, .povLeft = true,  .povRight = false, .expectedPovValue = EPovValue::W},
            {.povUp = false, .povDown = false, .povLeft = true,  .povRight = true,  .expectedPovValue = EPovValue::Center},
            {.povUp = false, .povDown = true,  .povLeft = false, .povRight = false, .expectedPovValue = EPovValue::S},
            {.povUp = false, .povDown = true,  .povLeft = false, .povRight = true,  .expectedPovValue = EPovValue::SE},
            {.povUp = false, .povDown = true,  .povLeft = true,  .povRight = false, .expectedPovValue = EPovValue::SW},
            {.povUp = false, .povDown = true,  .povLeft = true,  .povRight = true,  .expectedPovValue = EPovValue::S},
            {.povUp = true,  .povDown = false, .povLeft = false, .povRight = false, .expectedPovValue = EPovValue::N},
            {.povUp = true,  .povDown = false, .povLeft = false, .povRight = true,  .expectedPovValue = EPovValue::NE},
            {.povUp = true,  .povDown = false, .povLeft = true,  .povRight = false, .expectedPovValue = EPovValue::NW},
            {.povUp = true,  .povDown = false, .povLeft = true,  .povRight = true,  .expectedPovValue = EPovValue::N},
            {.povUp = true,  .povDown = true,  .povLeft = false, .povRight = false, .expectedPovValue = EPovValue::Center},
            {.povUp = true,  .povDown = true,  .povLeft = false, .povRight = true,  .expectedPovValue = EPovValue::E},
            {.povUp = true,  .povDown = true,  .povLeft = true,  .povRight = false, .expectedPovValue = EPovValue::W},
            {.povUp = true,  .povDown = true,  .povLeft = true,  .povRight = true,  .expectedPovValue = EPovValue::Center},
        };

        int numFailingInputs = 0;
        for (const auto& povTestData : kPovTestData)
        {
            Controller::SState controllerState;
            controllerState.povDirection[(int)Controller::EPovDirection::Up] = povTestData.povUp;
            controllerState.povDirection[(int)Controller::EPovDirection::Down] = povTestData.povDown;
            controllerState.povDirection[(int)Controller::EPovDirection::Left] = povTestData.povLeft;
            controllerState.povDirection[(int)Controller::EPovDirection::Right] = povTestData.povRight;

            const EPovValue kExpectedPovValue = povTestData.expectedPovValue;
            const EPovValue kActualPovValue = DataFormat::PovDirectionFromControllerState(controllerState);

            if (kActualPovValue != kExpectedPovValue)
            {
                PrintFormatted(L"Wrong POV direction for states up=%s down=%s left=%s right=%s (expected %d, got %d).", ((true == povTestData.povUp) ? L"true" : L"false"), ((true == povTestData.povDown) ? L"true" : L"false"), ((true == povTestData.povLeft) ? L"true" : L"false"), ((true == povTestData.povRight) ? L"true" : L"false"), (int)kExpectedPovValue, (int)kActualPovValue);
                numFailingInputs += 1;
            }
        }

        TEST_ASSERT(0 == numFailingInputs);
    }

    // Verifies that application data packets are correctly written given both a specification and a controller state.
    // First is a single-element test, in which most of the data packet is zero except for one element from the controller state.
    // Second is an all-element test, in which the entire set of object format specifications is provided and the data format object is expected to write a complete packet.
    TEST_CASE(DataFormat_WriteDataPacket)
    {
        struct STestDataPacket
        {
            TAxisValue axisX;
            TAxisValue axisY;
            TAxisValue axisZ;
            EPovValue pov;
            TButtonValue button[8];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        // Controller state that will be used throughout this test.
        constexpr Controller::SState kTestControllerState = {
            .axis = {1111, 2222, 3333, 4444, 5555, 6666},
            .button = {true, true, false, false, true, true, false, false, true, true, false, false, true, true, false, false},
            .povDirection = {true, false, false, true}
        };

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_XAxis, .dwOfs = offsetof(STestDataPacket, axisX),     .dwType = DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = &GUID_YAxis, .dwOfs = offsetof(STestDataPacket, axisY),     .dwType = DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = &GUID_ZAxis, .dwOfs = offsetof(STestDataPacket, axisZ),     .dwType = DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, pov),       .dwType = DIDFT_POV    | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[0]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[1]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[2]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[3]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[4]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[5]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[6]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,     .dwOfs = offsetof(STestDataPacket, button[7]), .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}
        };
        
        // Single element tests, one object format specification at a time.
        for (int i = 0; i < _countof(testObjectFormatSpec); ++i)
        {
            // Data format specification consists of exactly one controller element.
            const DIDATAFORMAT kTestFormatSpec = {
                .dwSize = sizeof(DIDATAFORMAT),
                .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
                .dwFlags = DIDF_ABSAXIS,
                .dwDataSize = sizeof(STestDataPacket),
                .dwNumObjs = 1,
                .rgodf = &testObjectFormatSpec[i]
            };

            std::unique_ptr<DataFormat> dataFormat = DataFormat::CreateFromApplicationFormatSpec(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
            TEST_ASSERT(nullptr != dataFormat);

            // Generate the expected data packet.
            // Other tests verify element mapping and POV direction value generation, so we can rely on the data format object to perform these tasks in this test.
            // Once the target element is known, extract the correct value from the test controller state object and place it at the right offset within the expected data packet.
            STestDataPacket expectedDataPacket;
            ZeroMemory(&expectedDataPacket, sizeof(expectedDataPacket));

            TEST_ASSERT(true == dataFormat->GetElementForOffset(testObjectFormatSpec[i].dwOfs).has_value());
            const SElementIdentifier kTestElement = dataFormat->GetElementForOffset(testObjectFormatSpec[i].dwOfs).value();

            switch (kTestElement.type)
            {
            case EElementType::Axis:
                *((TAxisValue*)(((size_t)&expectedDataPacket) + (size_t)testObjectFormatSpec[i].dwOfs)) = kTestControllerState.axis[(int)kTestElement.axis];
                break;

            case EElementType::Button:
                *((TButtonValue*)(((size_t)&expectedDataPacket) + (size_t)testObjectFormatSpec[i].dwOfs)) = ((true == kTestControllerState.button[(int)kTestElement.button]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed);
                break;

            case EElementType::Pov:
                *((EPovValue*)(((size_t)&expectedDataPacket) + (size_t)testObjectFormatSpec[i].dwOfs)) = DataFormat::PovDirectionFromControllerState(kTestControllerState);
                break;

            default:
                TEST_FAILED_BECAUSE(L"Expected axis, button, or POV.");
            }

            // Generate the actual data packet.
            // Poison-initialize the entire data packet, and then pass the test controller state to the mapper.
            STestDataPacket actualDataPacket;
            FillMemory(&actualDataPacket, sizeof(actualDataPacket), 0xcd);
            TEST_ASSERT(true == dataFormat->WriteDataPacket(&actualDataPacket, sizeof(actualDataPacket), kTestControllerState));

            // Finally, check if the data format object correctly wrote the data packet.
            TEST_ASSERT(0 == memcmp(&actualDataPacket, &expectedDataPacket, sizeof(expectedDataPacket)));
        }

        // One final test with the entire data specification all at once.
        // Logic is the same as the single-element case, except here the expected output is much simpler to generate.
        do
        {
            const DIDATAFORMAT kTestFormatSpec = {
                .dwSize = sizeof(DIDATAFORMAT),
                .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
                .dwFlags = DIDF_ABSAXIS,
                .dwDataSize = sizeof(STestDataPacket),
                .dwNumObjs = _countof(testObjectFormatSpec),
                .rgodf = testObjectFormatSpec
            };

            std::unique_ptr<DataFormat> dataFormat = DataFormat::CreateFromApplicationFormatSpec(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
            TEST_ASSERT(nullptr != dataFormat);

            const STestDataPacket expectedDataPacket = {
                .axisX = kTestControllerState.axis[(int)EAxis::X],
                .axisY = kTestControllerState.axis[(int)EAxis::Y],
                .axisZ = kTestControllerState.axis[(int)EAxis::Z],
                .pov = DataFormat::PovDirectionFromControllerState(kTestControllerState),
                .button = {
                    ((true == kTestControllerState.button[0]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[1]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[2]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[3]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[4]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[5]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[6]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed),
                    ((true == kTestControllerState.button[7]) ? DataFormat::kButtonValuePressed : DataFormat::kButtonValueNotPressed)
                }
            };

            STestDataPacket actualDataPacket;
            FillMemory(&actualDataPacket, sizeof(actualDataPacket), 0xcd);
            TEST_ASSERT(true == dataFormat->WriteDataPacket(&actualDataPacket, sizeof(actualDataPacket), kTestControllerState));
            TEST_ASSERT(0 == memcmp(&actualDataPacket, &expectedDataPacket, sizeof(expectedDataPacket)));
        } while (false);
    }


    // The following sequence of tests, which together comprise the CreateSuccess suite, verify that a data format can successfully be created given a valid specification.
    // Each test case follows the basic steps of declaring test data, manually creating the expected data format specification, generating the actual data format specification, and comparing the two, repeating the last few steps for both of the mapper types above.
    // Since each data format spec is manually created based on the known capabilities of the mappers defined above, any changes to the mapper definitions will need to be reflected in updates to the test cases.

    // Tests a simple data packet with two axis values and allows them to be any type of axis.
    // Axis objects are declared in the object specification in increasing offset order, and axes are expected to be selected in the order they appear in the object format specification array.
    TEST_CASE(DataFormat_CreateSuccess_AxisAnyAscending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = DIDOI_ASPECTPOSITION}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: X
        // axisValue2: Y
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Y}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with two axis values and allows them to be any type of axis.
    // Axis objects are declared in the object specification in decreasing offset order, so the assignment of axis to offset is inverted compared to the ascending version of this test.
    TEST_CASE(DataFormat_CreateSuccess_AxisAnyDescending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = DIDOI_ASPECTPOSITION}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: Y
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Y}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with two axis values, where the specific axis types are specified by GUID.
    // Axis objects are declared in the object specification in increasing offset order.
    // However, because specific axis types are specified, there is no impact on the assignment of axes to offsets.
    TEST_CASE(DataFormat_CreateSuccess_AxisSpecificByGuidAscending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = DIDOI_ASPECTPOSITION}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: RotZ
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }
    
    // Tests a simple data packet with two axis values, where the specific axis types are specified by GUID.
    // Axis objects are declared in the object specification in decreasing offset order.
    // However, because specific axis types are specified, there is no impact on the assignment of axes to offsets.
    TEST_CASE(DataFormat_CreateSuccess_AxisBySpecificGuidDescending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = DIDOI_ASPECTPOSITION}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: RotZ
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with two axis values, where the specific axis types are specified by index into the controller capabilities.
    // Axis objects are declared in the object specification in increasing offset order.
    // However, because specific axis types are specified, there is no impact on the assignment of axes to offsets.
    TEST_CASE(DataFormat_CreateSuccess_AxisSpecificByIndexAscending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(3), .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(0), .dwFlags = DIDOI_ASPECTPOSITION}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: RotZ
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with two axis values, where the specific axis types are specified by index into the controller capabilities.
    // Axis objects are declared in the object specification in decreasing offset order.
    // However, because specific axis types are specified, there is no impact on the assignment of axes to offsets.
    TEST_CASE(DataFormat_CreateSuccess_AxisSpecificByIndexDescending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(0), .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(3), .dwFlags = DIDOI_ASPECTPOSITION}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: RotZ
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with two axis values, where the specific axis types are specified by GUID and have an index number, which based on implementation assumptions described in "DataFormat.cpp" must be 0.
    // Axis objects are declared in the object specification in increasing offset order.
    // However, because specific axis types are specified, there is no impact on the assignment of axes to offsets.
    TEST_CASE(DataFormat_CreateSuccess_AxisSpecificByGuidAndIndexAscending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(0), .dwFlags = DIDOI_ASPECTPOSITION},
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(0), .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: RotZ
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with two axis values, where the specific axis types are specified by GUID and have an index number, which based on implementation assumptions described in "DataFormat.cpp" must be 0.
    // Axis objects are declared in the object specification in decreasing offset order.
    // However, because specific axis types are specified, there is no impact on the assignment of axes to offsets.
    TEST_CASE(DataFormat_CreateSuccess_AxisSpecificByGuidAndIndexDescending)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(0), .dwFlags = DIDOI_ASPECTPOSITION},
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(0), .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // axisValue1: RotZ
        // axisValue2: X
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(STestDataPacket, axisValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(STestDataPacket, axisValue2));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with three axis values, where each axis is specified such that no controller element matches and so the resulting data format specification is empty.
    // Axis objects are declared in the object specification in decreasing offset order, but the order is immaterial.
    TEST_CASE(DataFormat_CreateSuccess_AxisNoMatch)
    {
        struct STestDataPacket
        {
            TAxisValue padding1[13];
            TAxisValue axisValue1;
            TAxisValue axisValue2;
            TAxisValue padding2[8];
            TAxisValue axisValue3;
            TAxisValue padding3[2];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_RxAxis, .dwOfs = offsetof(STestDataPacket, axisValue2), .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,     .dwFlags = 0},  // GUID requests an axis type not present in the virtual controller
            {.pguid = &GUID_YAxis,  .dwOfs = offsetof(STestDataPacket, axisValue3), .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_MAKEINSTANCE(1), .dwFlags = 0},  // GUID requests an axis type that is present, but instance index is out of bounds
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, axisValue1), .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_MAKEINSTANCE(8), .dwFlags = 0}   // No GUID type filter, and instance index is out of bounds
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // Expected to be empty because no elements were able to be selected for any object format specifications.
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a data packet with several button locations, each specified in a different way.
    // Button value fields are deliberately numbered out of order so that the object format specification remains readable but the offsets themselves are out of order.
    // See test case body for a description of inputs and expected outputs.
    TEST_CASE(DataFormat_CreateSuccess_Button)
    {
        struct STestDataPacket
        {
            TButtonValue padding1[11];
            TButtonValue buttonValue4;
            TButtonValue padding2[22];
            TButtonValue buttonValue6;
            TButtonValue padding3[33];
            TButtonValue buttonValue5;
            TButtonValue padding4[44];
            TButtonValue buttonValue1;
            TButtonValue padding5[55];
            TButtonValue buttonValue3;
            TButtonValue padding6[66];
            TButtonValue buttonValue7;
            TButtonValue padding7[77];
            TButtonValue buttonValue2;
            TButtonValue padding8[88];
            TButtonValue buttonValue8;
            TButtonValue padding9[100];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, buttonValue1), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_MAKEINSTANCE(1),  .dwFlags = 0},                      // Matches button 2 by specifically identifying it
            {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, buttonValue2), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,      .dwFlags = DIDOI_ASPECTPOSITION},   // Matches button 1 because it is the next available
            {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, buttonValue3), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,      .dwFlags = 0},                      // Matches button 3 because it is the next available, since button 2 was already matched
            {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, buttonValue4), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_MAKEINSTANCE(2),  .dwFlags = DIDOI_ASPECTPOSITION},   // No match because button 3 is specified but it was already matched
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue5), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_MAKEINSTANCE(15), .dwFlags = 0},                      // Matches button 16 only in the without-POV case, otherwise the button index is out of bounds
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue6), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_MAKEINSTANCE(11), .dwFlags = DIDOI_ASPECTFORCE},      // No match because the flags are not supported
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue7), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,      .dwFlags = 0},                      // Matches button 4 because it is the next available
            {.pguid = &GUID_Slider, .dwOfs = offsetof(STestDataPacket, buttonValue8), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,      .dwFlags = DIDOI_ASPECTPOSITION}    // No match because the GUID type does not specify a button
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // Several, but not all, buttons are expected to match controller elements, based on the specification above.
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Button, .button = EButton::B2}, offsetof(STestDataPacket, buttonValue1));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Button, .button = EButton::B1}, offsetof(STestDataPacket, buttonValue2));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Button, .button = EButton::B3}, offsetof(STestDataPacket, buttonValue3));
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Button, .button = EButton::B4}, offsetof(STestDataPacket, buttonValue7));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);


        // Based on the specification above, there are additional situations that match when using the without-POV test data that could not match when using the with-POV test data.
        expectedDataFormatSpec.SetOffsetForElement({.type = EElementType::Button, .button = EButton::B16}, offsetof(STestDataPacket, buttonValue5));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // Tests a simple data packet with one POV value whose specification selects it by a type filter value only (i.e. no GUID, and no instance index).
    // A match is expected with the virtual controller that does have a POV, whereas no match is expected with the virtual controller that does not have a POV.
    TEST_CASE(DataFormat_CreateSuccess_PovAny)
    {
        struct STestDataPacket
        {
            EPovValue povValue;            
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, povValue), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, .dwFlags = 0},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        // POV should be selected for a virtual controller that has a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        // A single unused POV is expected for a virtual controller that does not have a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }

    // Tests a simple data packet with one POV value whose specification selects it by a type filter value and GUID but no instance index.
    // A match is expected with the virtual controller that does have a POV, whereas no match is expected with the virtual controller that does not have a POV.
    TEST_CASE(DataFormat_CreateSuccess_PovSpecificByGuid)
    {
        struct STestDataPacket
        {
            EPovValue povValue;            
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_POV, .dwOfs = offsetof(STestDataPacket, povValue), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, .dwFlags = 0},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        // POV should be selected for a virtual controller that has a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        // A single unused POV is expected for a virtual controller that does not have a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }

    // Tests a simple data packet with one POV value whose specification selects it by a type filter value and instance index but no GUID.
    // A match is expected with the virtual controller that does have a POV, whereas no match is expected with the virtual controller that does not have a POV.
    TEST_CASE(DataFormat_CreateSuccess_PovSpecificByIndex)
    {
        struct STestDataPacket
        {
            EPovValue povValue;            
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, povValue), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_MAKEINSTANCE(0), .dwFlags = 0},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        // POV should be selected for a virtual controller that has a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        // A single unused POV is expected for a virtual controller that does not have a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }

    // Tests a simple data packet with one POV value whose specification selects it by a type filter value, GUID, and instance index.
    // A match is expected with the virtual controller that does have a POV, whereas no match is expected with the virtual controller that does not have a POV.
    TEST_CASE(DataFormat_CreateSuccess_PovSpecificByGuidAndIndex)
    {
        struct STestDataPacket
        {
            EPovValue povValue;            
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_POV, .dwOfs = offsetof(STestDataPacket, povValue), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_MAKEINSTANCE(0), .dwFlags = 0},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        // POV should be selected for a virtual controller that has a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        // A single unused POV is expected for a virtual controller that does not have a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }

    // Tests a simple data packet with one POV value whose specification selects it by a type filter value, GUID, and instance index. An aspect flag is also added.
    // A match is expected with the virtual controller that does have a POV, whereas no match is expected with the virtual controller that does not have a POV.
    TEST_CASE(DataFormat_CreateSuccess_PovSpecificByGuidAndIndexPlusAspect)
    {
        struct STestDataPacket
        {
            EPovValue povValue;            
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_POV, .dwOfs = offsetof(STestDataPacket, povValue), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_MAKEINSTANCE(0), .dwFlags = DIDOI_ASPECTPOSITION},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        // POV should be selected for a virtual controller that has a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        // A single unused POV is expected for a virtual controller that does not have a POV.
        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(STestDataPacket));
        expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(STestDataPacket, povValue));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }

    // Tests a data packet with several POV locations, each specified in a different way.
    // None of them are able to match with the POV for various reasons.
    // See test case body for a description of inputs and reasons why they do not match the POV.
    TEST_CASE(DataFormat_CreateSuccess_PovNoMatch)
    {
        struct STestDataPacket
        {
            EPovValue povValue2;
            EPovValue povValue1;
            EPovValue povValue3;
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_POV,    .dwOfs = offsetof(STestDataPacket, povValue1), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_MAKEINSTANCE(1),  .dwFlags = 0},                    // No match because the index is out of bounds
            {.pguid = &GUID_Slider, .dwOfs = offsetof(STestDataPacket, povValue2), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE,      .dwFlags = DIDOI_ASPECTPOSITION}, // No match because the GUID type does not specify a POV
            {.pguid = &GUID_POV,    .dwOfs = offsetof(STestDataPacket, povValue3), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE,      .dwFlags = DIDOI_ASPECTACCEL}     // No match because the flags are not supported
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        // Expected to be empty because no elements were able to be selected for any object format specifications.
        // However, because POVs are present in the format specification, they are unused and thus tracked by the data format object.
        // It is assumed that only object specifications recognized as POV specifications (i.e. consistent GUID and type, supported flag value, and so on) need to be tracked as unused POVs.
        DataFormat::SDataFormatSpec expectedDataFormatSpec(sizeof(STestDataPacket));
        expectedDataFormatSpec.SubmitUnusedPovOffset(offsetof(STestDataPacket, povValue1));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpec);
        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpec);
    }

    // DirectInput's built-in DIJOYSTATE structure.
    // Applications that use this data format pass the contstant c_dfDIJoystick as a data format specification.
    TEST_CASE(DataFormat_CreateSuccess_DIJoystick)
    {
        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
             {.pguid = &GUID_XAxis,  .dwOfs = DIJOFS_X,              .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_YAxis,  .dwOfs = DIJOFS_Y,              .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_ZAxis,  .dwOfs = DIJOFS_Z,              .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_RxAxis, .dwOfs = DIJOFS_RX,             .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_RyAxis, .dwOfs = DIJOFS_RY,             .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_RzAxis, .dwOfs = DIJOFS_RZ,             .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_Slider, .dwOfs = DIJOFS_SLIDER(0),      .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_Slider, .dwOfs = DIJOFS_SLIDER(1),      .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
             {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(0),         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
             {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(1),         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
             {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(2),         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
             {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(3),         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(0),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(1),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(2),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(3),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(4),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(5),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(6),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(7),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(8),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(9),      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(10),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(11),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(12),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(13),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(14),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(15),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(16),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(17),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(18),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(19),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(20),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(21),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(22),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(23),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(24),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(25),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(26),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(27),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(28),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(29),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(30),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
             {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(31),     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0}
        };
        
        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(DIJOYSTATE),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(DIJOYSTATE));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(DIJOYSTATE, lX));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Y}, offsetof(DIJOYSTATE, lY));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Z}, offsetof(DIJOYSTATE, lZ));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(DIJOYSTATE, lRz));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(DIJOYSTATE, rgdwPOV[0]));
        for (int i = 0; i < kTestMapperWithPov.GetCapabilities().numButtons; ++i)
            expectedDataFormatSpecWithPov.SetOffsetForElement({ .type = EElementType::Button, .button = (EButton)i }, offsetof(DIJOYSTATE, rgbButtons[i]));
        for (int i = 1; i < _countof(DIJOYSTATE::rgdwPOV); ++i)
            expectedDataFormatSpecWithPov.SubmitUnusedPovOffset(offsetof(DIJOYSTATE, rgdwPOV[i]));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(DIJOYSTATE));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(DIJOYSTATE, lX));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Y}, offsetof(DIJOYSTATE, lY));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Z}, offsetof(DIJOYSTATE, lZ));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(DIJOYSTATE, lRz));
        for (int i = 0; i < kTestMapperWithoutPov.GetCapabilities().numButtons; ++i)
            expectedDataFormatSpecWithoutPov.SetOffsetForElement({ .type = EElementType::Button, .button = (EButton)i }, offsetof(DIJOYSTATE, rgbButtons[i]));
        for (int i = 0; i < _countof(DIJOYSTATE::rgdwPOV); ++i)
            expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(DIJOYSTATE, rgdwPOV[i]));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }

    // DirectInput's built-in DIJOYSTATE2 structure.
    // Applications that use this data format pass the contstant c_dfDIJoystick2 as a data format specification.
    TEST_CASE(DataFormat_CreateSuccess_DIJoystick2)
    {
        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_XAxis,  .dwOfs = DIJOFS_X,                              .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = DIJOFS_Y,                              .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = DIJOFS_Z,                              .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = DIJOFS_RX,                             .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = DIJOFS_RY,                             .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = DIJOFS_RZ,                             .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = DIJOFS_SLIDER(0),                      .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = DIJOFS_SLIDER(1),                      .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(0),                         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(1),                         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(2),                         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(3),                         .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(0),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(1),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(2),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(3),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(4),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(5),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(6),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(7),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(8),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(9),                      .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(10),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(11),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(12),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(13),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(14),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(15),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(16),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(17),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(18),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(19),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(20),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(21),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(22),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(23),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(24),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(25),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(26),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(27),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(28),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(29),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(30),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(31),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(32),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(33),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(34),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(35),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(36),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(37),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(38),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(39),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(40),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(41),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(42),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(43),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(44),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(45),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(46),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(47),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(48),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(49),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(50),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(51),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(52),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(53),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(54),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(55),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(56),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(57),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(58),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(59),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(60),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(61),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(62),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(63),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(64),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(65),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(66),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(67),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(68),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(69),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(70),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(71),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(72),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(73),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(74),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(75),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(76),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(77),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(78),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(79),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(80),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(81),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(82),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(83),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(84),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(85),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(86),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(87),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(88),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(89),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(90),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(91),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(92),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(93),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(94),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(95),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(96),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(97),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(98),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(99),                     .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(100),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(101),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(102),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(103),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(104),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(105),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(106),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(107),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(108),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(109),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(110),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(111),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(112),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(113),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(114),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(115),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(116),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(117),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(118),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(119),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(120),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(121),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(122),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(123),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(124),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(125),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(126),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(127),                    .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(DIJOYSTATE2, lVX),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = offsetof(DIJOYSTATE2, lVY),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = offsetof(DIJOYSTATE2, lVZ),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = offsetof(DIJOYSTATE2, lVRx),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = offsetof(DIJOYSTATE2, lVRy),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(DIJOYSTATE2, lVRz),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(DIJOYSTATE2, rglVSlider[0]),  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(DIJOYSTATE2, rglVSlider[1]),  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(DIJOYSTATE2, lAX),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = offsetof(DIJOYSTATE2, lAY),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = offsetof(DIJOYSTATE2, lAZ),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = offsetof(DIJOYSTATE2, lARx),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = offsetof(DIJOYSTATE2, lARy),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(DIJOYSTATE2, lARz),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(DIJOYSTATE2, rglASlider[0]),  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(DIJOYSTATE2, rglASlider[1]),  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = offsetof(DIJOYSTATE2, lFX),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = offsetof(DIJOYSTATE2, lFY),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = offsetof(DIJOYSTATE2, lFZ),            .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = offsetof(DIJOYSTATE2, lFRx),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = offsetof(DIJOYSTATE2, lFRy),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = offsetof(DIJOYSTATE2, lFRz),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(DIJOYSTATE2, rglFSlider[0]),  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(DIJOYSTATE2, rglFSlider[1]),  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(DIJOYSTATE2),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };


        DataFormat::SDataFormatSpec expectedDataFormatSpecWithPov(sizeof(DIJOYSTATE2));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(DIJOYSTATE2, lX));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Y}, offsetof(DIJOYSTATE2, lY));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Z}, offsetof(DIJOYSTATE2, lZ));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(DIJOYSTATE2, lRz));
        expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Pov}, offsetof(DIJOYSTATE2, rgdwPOV[0]));
        for (int i = 0; i < kTestMapperWithPov.GetCapabilities().numButtons; ++i)
            expectedDataFormatSpecWithPov.SetOffsetForElement({.type = EElementType::Button, .button = (EButton)i}, offsetof(DIJOYSTATE2, rgbButtons[i]));
        for (int i = 1; i < _countof(DIJOYSTATE2::rgdwPOV); ++i)
            expectedDataFormatSpecWithPov.SubmitUnusedPovOffset(offsetof(DIJOYSTATE2, rgdwPOV[i]));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithPov.GetCapabilities(), expectedDataFormatSpecWithPov);


        DataFormat::SDataFormatSpec expectedDataFormatSpecWithoutPov(sizeof(DIJOYSTATE2));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::X}, offsetof(DIJOYSTATE2, lX));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Y}, offsetof(DIJOYSTATE2, lY));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::Z}, offsetof(DIJOYSTATE2, lZ));
        expectedDataFormatSpecWithoutPov.SetOffsetForElement({.type = EElementType::Axis, .axis = EAxis::RotZ}, offsetof(DIJOYSTATE2, lRz));
        for (int i = 0; i < kTestMapperWithoutPov.GetCapabilities().numButtons; ++i)
            expectedDataFormatSpecWithoutPov.SetOffsetForElement({ .type = EElementType::Button, .button = (EButton)i }, offsetof(DIJOYSTATE2, rgbButtons[i]));
        for (int i = 0; i < _countof(DIJOYSTATE2::rgdwPOV); ++i)
            expectedDataFormatSpecWithoutPov.SubmitUnusedPovOffset(offsetof(DIJOYSTATE2, rgdwPOV[i]));

        TestDataFormatCreateSuccess(kTestFormatSpec, kTestMapperWithoutPov.GetCapabilities(), expectedDataFormatSpecWithoutPov);
    }


    // The following sequence of tests, which together comprise the CreateFailure suite, verify that data format creation fails if the specification is invalid.
    // Each test case follows the basic steps of declaring test data, attempting to create a data format object, and verifying that the operation fails.
    
    // Data packet size is improperly aligned.
    TEST_CASE(DataFormat_CreateFailure_DataPacketSizeMisaligned)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue;
        };

        static_assert(0 != (sizeof(STestDataPacket) % 4), L"Test data packet size must not be divisible by 4 for this test.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),          // Reason for rejection: data packet size is not divisible by 4.
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Data packet size is too large.
    TEST_CASE(DataFormat_CreateFailure_DataPacketSizeTooLarge)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4 * DataFormat::kMaxDataPacketSizeBytes];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),          // Reason for rejection: data packet size is too large.
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // No objects are defined.
    TEST_CASE(DataFormat_CreateFailure_NoObjectsDefined)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[2]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpecs[] = {
            {
                .dwSize = sizeof(DIDATAFORMAT),
                .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
                .dwFlags = DIDF_ABSAXIS,
                .dwDataSize = sizeof(STestDataPacket),
                .dwNumObjs = 0,                             // Reason for rejection: number of objects is 0.
                .rgodf = testObjectFormatSpec
            },
            {
                .dwSize = sizeof(DIDATAFORMAT),
                .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
                .dwFlags = DIDF_ABSAXIS,
                .dwDataSize = sizeof(STestDataPacket),
                .dwNumObjs = _countof(testObjectFormatSpec),
                .rgodf = nullptr                            // Reason for rejection: object format specification array pointer is null.
            },
            {
                .dwSize = sizeof(DIDATAFORMAT),
                .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
                .dwFlags = DIDF_ABSAXIS,
                .dwDataSize = sizeof(STestDataPacket),
                .dwNumObjs = 0,                             // Reason for rejection 1: number of objects is 0.
                .rgodf = nullptr                            // Reason for rejection 2: object format specification array pointer is null.
            }
        };

        for (const auto& kTestFormatSpec : kTestFormatSpecs)
            TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Flags are unsupported.
    TEST_CASE(DataFormat_CreateFailure_UnsupportedFlags)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[2]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_RELAXIS,                        // Reason for rejection: flag value is unsupported.
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Non-optional object without a matching controller element due to an unsupported type filter value.
    TEST_CASE(DataFormat_CreateFailure_NonOptionalNoMatchByType)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4];
            TAxisValue axisValue;
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON  | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON  | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[2]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON  | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON  | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue),      .dwType =                  DIDFT_RELAXIS | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: not optional and no match based on the type filter.
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Non-optional object without a matching controller element due to a GUID filter that does not match any virtual controller elements.
    TEST_CASE(DataFormat_CreateFailure_NonOptionalNoMatchByGuid)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4];
            TAxisValue axisValue;
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue[2]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = offsetof(STestDataPacket, buttonValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = offsetof(STestDataPacket, axisValue),      .dwType =                  DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0}      // Reason for rejection: not optional and no match based on GUID.
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }
    
    // Non-optional object without a matching controller element due to an out-of-bounds instance index value.
    TEST_CASE(DataFormat_CreateFailure_NonOptionalNoMatchByIndex)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4];
            TAxisValue axisValue;
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,     .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,     .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[2]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,     .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE,     .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue),      .dwType =                  DIDFT_AXIS   | DIDFT_MAKEINSTANCE(8), .dwFlags = 0}       // Reason for rejection: not optional and no match based on instance index.
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Specified axis offset is beyond the total size of the data packet.
    TEST_CASE(DataFormat_CreateFailure_OffsetOutOfBoundsAxis)
    {
        struct STestDataPacket
        {
            TAxisValue axisValue[4];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = sizeof(STestDataPacket),                 .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0},              // Reason for rejection: offset exceeds the size of the data packet.
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, axisValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Specified button offset is beyond the total size of the data packet.
    TEST_CASE(DataFormat_CreateFailure_OffsetOutOfBoundsButton)
    {
        struct STestDataPacket
        {
            TButtonValue buttonValue[4];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = sizeof(STestDataPacket),                   .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},          // Reason for rejection: offset exceeds the size of the data packet.
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, buttonValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Specified POV offset is beyond the total size of the data packet.
    TEST_CASE(DataFormat_CreateFailure_OffsetOutOfBoundsPov)
    {
        struct STestDataPacket
        {
            EPovValue povValue[4];
        };

        static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, povValue[0]), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, povValue[1]), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, .dwFlags = 0},
            {.pguid = nullptr, .dwOfs = sizeof(STestDataPacket),                .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, .dwFlags = 0},                // Reason for rejection: offset exceeds the size of the data packet.
            {.pguid = nullptr, .dwOfs = offsetof(STestDataPacket, povValue[3]), .dwType = DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, .dwFlags = 0}
        };

        const DIDATAFORMAT kTestFormatSpec = {
            .dwSize = sizeof(DIDATAFORMAT),
            .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
            .dwFlags = DIDF_ABSAXIS,
            .dwDataSize = sizeof(STestDataPacket),
            .dwNumObjs = _countof(testObjectFormatSpec),
            .rgodf = testObjectFormatSpec
        };

        TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
    }

    // Specified axis offset overlaps with a previous object specification. This is enforced by using a union instead of a struct for the test data packet.
    // Each scenario exercises a different combination of controller elements.
    TEST_CASE(DataFormat_CreateFailure_OffsetOverlap)
    {
        union UTestDataPacket
        {
            TAxisValue axisValue;
            TButtonValue buttonValue;
            EPovValue povValue;
        };

        static_assert(0 == (sizeof(UTestDataPacket) % 4), L"Test data packet size must be divisible by 4.");

        DIOBJECTDATAFORMAT testObjectFormatSpecs[][2] = {
            {
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, axisValue),   .dwType = DIDFT_OPTIONAL | DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, buttonValue), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: button offset conflicts with axis offset.
            },
            {
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, axisValue),   .dwType = DIDFT_OPTIONAL | DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, povValue),    .dwType = DIDFT_OPTIONAL | DIDFT_POV    | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: POV offset conflicts with axis offset.
            },
            {
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, buttonValue), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, axisValue),   .dwType = DIDFT_OPTIONAL | DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: axis offset conflicts with button offset.
            },
            {
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, buttonValue), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, povValue),    .dwType = DIDFT_OPTIONAL | DIDFT_POV    | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: POV offset conflicts with button offset.
            },
            {
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, povValue),    .dwType = DIDFT_OPTIONAL | DIDFT_POV    | DIDFT_ANYINSTANCE, .dwFlags = 0},
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, axisValue),   .dwType = DIDFT_OPTIONAL | DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: axis offset conflicts with POV offset.
            },
            {
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, povValue),    .dwType = DIDFT_OPTIONAL | DIDFT_POV    | DIDFT_ANYINSTANCE, .dwFlags = 0},
                {.pguid = nullptr, .dwOfs = offsetof(UTestDataPacket, buttonValue), .dwType = DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}          // Reason for rejection: button offset conflicts with POV offset.
            }
        };

        for (auto& testObjectFormatSpec : testObjectFormatSpecs)
        {
            const DIDATAFORMAT kTestFormatSpec = {
                .dwSize = sizeof(DIDATAFORMAT),
                .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
                .dwFlags = DIDF_ABSAXIS,
                .dwDataSize = sizeof(UTestDataPacket),
                .dwNumObjs = _countof(testObjectFormatSpec),
                .rgodf = testObjectFormatSpec
            };

            TestDataFormatCreateFailure(kTestFormatSpec, kTestMapperWithPov.GetCapabilities());
        }
    }
}
