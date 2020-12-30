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

    // Test mapper that contains a POV.
    // Contains 4 axes (RotX and RotY are skipped), 12 buttons, and a POV.
    static const Mapper kTestMapperWithPov = Mapper({
        .stickLeftX = new AxisMapper(EAxis::X),
        .stickLeftY = new AxisMapper(EAxis::Y),
        .stickRightX = new AxisMapper(EAxis::Z),
        .stickRightY = new AxisMapper(EAxis::RotZ),
        .dpadUp = new PovMapper(EPovDirection::Up),
        .dpadDown = new PovMapper(EPovDirection::Down),
        .dpadLeft = new PovMapper(EPovDirection::Left),
        .dpadRight = new PovMapper(EPovDirection::Right),
        .triggerLT = new ButtonMapper(EButton::B7),
        .triggerRT = new ButtonMapper(EButton::B8),
        .buttonA = new ButtonMapper(EButton::B1),
        .buttonB = new ButtonMapper(EButton::B2),
        .buttonX = new ButtonMapper(EButton::B3),
        .buttonY = new ButtonMapper(EButton::B4),
        .buttonLB = new ButtonMapper(EButton::B5),
        .buttonRB = new ButtonMapper(EButton::B6),
        .buttonBack = new ButtonMapper(EButton::B9),
        .buttonStart = new ButtonMapper(EButton::B10),
        .buttonLS = new ButtonMapper(EButton::B11),
        .buttonRS = new ButtonMapper(EButton::B12)
    });

    // Test mapper that does not contain a POV.
    // Contains 4 axes (RotX and RotY are deliberately skipped), and 16 buttons.
    static const Mapper kTestMapperWithoutPov = Mapper({
        .stickLeftX = new AxisMapper(EAxis::X),
        .stickLeftY = new AxisMapper(EAxis::Y),
        .stickRightX = new AxisMapper(EAxis::Z),
        .stickRightY = new AxisMapper(EAxis::RotZ),
        .dpadUp = new ButtonMapper(EButton::B13),
        .dpadDown = new ButtonMapper(EButton::B14),
        .dpadLeft = new ButtonMapper(EButton::B15),
        .dpadRight = new ButtonMapper(EButton::B16),
        .triggerLT = new ButtonMapper(EButton::B7),
        .triggerRT = new ButtonMapper(EButton::B8),
        .buttonA = new ButtonMapper(EButton::B1),
        .buttonB = new ButtonMapper(EButton::B2),
        .buttonX = new ButtonMapper(EButton::B3),
        .buttonY = new ButtonMapper(EButton::B4),
        .buttonLB = new ButtonMapper(EButton::B5),
        .buttonRB = new ButtonMapper(EButton::B6),
        .buttonBack = new ButtonMapper(EButton::B9),
        .buttonStart = new ButtonMapper(EButton::B10),
        .buttonLS = new ButtonMapper(EButton::B11),
        .buttonRS = new ButtonMapper(EButton::B12)
    });


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Main checks that are part of the CreateSuccess suite of test cases.
    /// Given the information needed to construct a data format object and the data format specification that is expected to result, constructs the data format object, ensures success, and ensures expectation matches actual result.
    /// Causes the test case to fail if any of the checks or operations are unsuccessful.
    /// @param [in] appFormatSpec Test data in the form of an application data format specification.
    /// @param [in] controllerCapabilities Test data in the form of a description of a controller's capabilities. Likely obtained from one of the mappers defined above.
    /// @param [in] expectedDataFormatSpec Test data in the form of a data format spec that is expected to be the result of creating a data format object using the provided controller capabilities.
    static void TestDataFormatCreateSuccess(const DIDATAFORMAT& appFormatSpec, const Controller::SCapabilities& controllerCapabilities, const DataFormat::SDataFormatSpec& expectedDataFormatSpec)
    {
        std::unique_ptr<DataFormat> dataFormat = DataFormat::CreateFromApplicationFormatSpec(appFormatSpec, controllerCapabilities);
        TEST_ASSERT(nullptr != dataFormat);

        const DataFormat::SDataFormatSpec& actualDataFormatSpec = dataFormat->GetSpec();
        TEST_ASSERT(actualDataFormatSpec == expectedDataFormatSpec);
    }


    // -------- TEST CASES ------------------------------------------------- //
    
    // The following sequence of tests, which together comprise the CreateSuccess suite, verify that a data format can successfully be created given a valid specification.
    // Each test case follows the basic steps of declaring test data, manually creating the expected data format specification, generating the actual data format specification, and comparing the two, repeating the last few steps for both of the mapper types above.
    // Since each data format spec is manually created based on the known capabilities of the mappers defined above, any changes to the mapper definitions will need to be reflected in updates to the test cases.
    
    // DirectInput's built-in DIJOYSTATE data format, specified by constant c_dfDIJoystick
    TEST_CASE(DataFormat_CreateSuccess_DIJOYSTATE)
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

    // DirectInput's built-in DIJOYSTATE2 data format, specified by constant c_dfDIJoystick2
    TEST_CASE(DataFormat_CreateSuccess_DIJOYSTATE2)
    {
        DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
            {.pguid = &GUID_XAxis,  .dwOfs = DIJOFS_X,                                          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = DIJOFS_Y,                                          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = DIJOFS_Z,                                          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = DIJOFS_RX,                                         .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = DIJOFS_RY,                                         .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = DIJOFS_RZ,                                         .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = DIJOFS_SLIDER(0),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = DIJOFS_SLIDER(1),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(0),                                     .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(1),                                     .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(2),                                     .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = &GUID_POV,    .dwOfs = DIJOFS_POV(3),                                     .dwType = (DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE),     .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(0),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(1),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(2),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(3),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(4),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(5),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(6),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(7),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(8),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(9),                                  .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(10),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(11),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(12),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(13),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(14),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(15),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(16),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(17),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(18),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(19),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(20),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(21),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(22),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(23),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(24),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(25),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(26),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(27),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(28),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(29),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(30),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(31),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(32),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(33),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(34),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(35),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(36),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(37),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(38),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(39),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(40),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(41),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(42),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(43),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(44),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(45),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(46),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(47),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(48),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(49),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(50),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(51),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(52),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(53),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(54),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(55),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(56),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(57),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(58),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(59),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(60),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(61),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(62),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(63),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(64),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(65),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(66),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(67),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(68),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(69),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(70),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(71),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(72),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(73),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(74),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(75),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(76),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(77),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(78),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(79),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(80),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(81),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(82),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(83),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(84),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(85),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(86),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(87),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(88),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(89),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(90),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(91),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(92),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(93),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(94),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(95),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(96),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(97),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(98),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(99),                                 .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(100),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(101),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(102),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(103),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(104),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(105),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(106),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(107),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(108),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(109),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(110),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(111),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(112),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(113),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(114),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(115),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(116),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(117),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(118),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(119),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(120),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(121),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(122),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(123),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(124),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(125),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(126),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = nullptr,      .dwOfs = DIJOFS_BUTTON(127),                                .dwType = (DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE),  .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lVX)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lVY)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lVZ)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lVRx)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lVRy)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lVRz)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, rglVSlider[0])), .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, rglVSlider[1])), .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lAX)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lAY)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lAZ)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lARx)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lARy)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lARz)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, rglASlider[0])), .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, rglASlider[1])), .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_XAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lFX)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_YAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lFY)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_ZAxis,  .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lFZ)),           .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RxAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lFRx)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RyAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lFRy)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_RzAxis, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, lFRz)),          .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, rglFSlider[0])), .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0},
            {.pguid = &GUID_Slider, .dwOfs = (DWORD)(FIELD_OFFSET(DIJOYSTATE2, rglFSlider[1])), .dwType = (DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE),    .dwFlags = 0}
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
