/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualDirectInputEffectTest.cpp
 *   Unit tests for DirectInput interface objects that wrap force feedback
 *   effect objects.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerTypes.h"
#include "Mapper.h"
#include "MockForceFeedbackEffect.h"
#include "MockPhysicalController.h"
#include "TestCase.h"
#include "VirtualController.h"
#include "VirtualDirectInputEffect.h"

#include <memory>


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
    using ::Xidi::Controller::TControllerIdentifier;
    using ::Xidi::Controller::VirtualController;


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Testing class for DirectInput effect objects with type-specific parameters.
    /// Internally uses the mock force feedback effect type and does not require any type conversion between internal and DirectInput type-specific parameter structures.
    class TestVirtualDirectInputEffect : public VirtualDirectInputEffectWithTypeSpecificParameters<ECharMode::W, SMockTypeSpecificParameters, SMockTypeSpecificParameters>
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        inline TestVirtualDirectInputEffect(VirtualDirectInputDevice<ECharMode::W>& associatedDevice, const MockEffectWithTypeSpecificParameters& effect, const GUID& effectGuid) : VirtualDirectInputEffectWithTypeSpecificParameters<ECharMode::W, SMockTypeSpecificParameters, SMockTypeSpecificParameters>(associatedDevice, effect, effectGuid)
        {
            // Nothing to do here.
        }


    protected:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //

        SMockTypeSpecificParameters ConvertFromDirectInput(const SMockTypeSpecificParameters& diTypeSpecificParams) const override
        {
            return diTypeSpecificParams;
        }

        SMockTypeSpecificParameters ConvertToDirectInput(const SMockTypeSpecificParameters& typeSpecificParams) const override
        {
            return typeSpecificParams;
        }
    };

    /// Data packet structure definition used throughout these test cases.
    /// Deliberately contains spots for fewer elements than the test mapper defines so that some controller elements are left without offsets.
    struct STestDataPacket
    {
        TAxisValue axisX;
        TAxisValue axisY;
        EPovValue pov;
        TButtonValue button[4];
    };
    static_assert(0 == (sizeof(STestDataPacket) % 4), "Test data packet size must be divisible by 4.");


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Test value of controller identifier used throughout these test cases.
    static constexpr TControllerIdentifier kTestControllerIdentifier = 0;

    /// Test mapper used throughout these test cases.
    /// Describes a layout with 2 axes, a POV, and 4 buttons, with force feedback actuators on the X and Y axes.
    static const Mapper kTestMapper(
        {
            .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
            .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
            .dpadUp = std::make_unique<PovMapper>(EPovDirection::Up),
            .dpadDown = std::make_unique<PovMapper>(EPovDirection::Down),
            .dpadLeft = std::make_unique<PovMapper>(EPovDirection::Left),
            .dpadRight = std::make_unique<PovMapper>(EPovDirection::Right),
            .buttonA = std::make_unique<ButtonMapper>(EButton::B1),
            .buttonB = std::make_unique<ButtonMapper>(EButton::B2),
            .buttonX = std::make_unique<ButtonMapper>(EButton::B3),
            .buttonY = std::make_unique<ButtonMapper>(EButton::B4)
        },
        {
            .leftMotor = {.isPresent = true, .axis = EAxis::X, .direction = EAxisDirection::Both},
            .rightMotor = {.isPresent = true, .axis = EAxis::Y, .direction = EAxisDirection::Both}
        }
    );

    /// Object format specification for #STestDataPacket.
    static DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
        {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisX),      .dwType = DIDFT_AXIS    | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_YAxis,  .dwOfs = offsetof(STestDataPacket, axisY),      .dwType = DIDFT_AXIS    | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_POV,    .dwOfs = offsetof(STestDataPacket, pov),        .dwType = DIDFT_POV     | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[0]),  .dwType = DIDFT_BUTTON  | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[1]),  .dwType = DIDFT_BUTTON  | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[2]),  .dwType = DIDFT_BUTTON  | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[3]),  .dwType = DIDFT_BUTTON  | DIDFT_ANYINSTANCE,    .dwFlags = 0}
    };

    /// Complete application data format specification for #STestDataPacket.
    static constexpr DIDATAFORMAT kTestFormatSpec = {
        .dwSize = sizeof(DIDATAFORMAT),
        .dwObjSize = sizeof(DIOBJECTDATAFORMAT),
        .dwFlags = DIDF_ABSAXIS,
        .dwDataSize = sizeof(STestDataPacket),
        .dwNumObjs = _countof(testObjectFormatSpec),
        .rgodf = testObjectFormatSpec
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Creates and returns a DirectInput device object that uses the mapper at the top of this file, sufficient for force feedback effect tests.
    /// The new object is pre-set to use the test data packet format defined at the top of this file.
    /// @return Smart pointer to a new virtual DirectInput device object.
    static inline std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> CreateTestDirectInputDevice(void)
    {
        std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> newDirectInputDevice = std::make_unique<VirtualDirectInputDevice<ECharMode::W>>(std::make_unique<VirtualController>(kTestControllerIdentifier, kTestMapper));
        TEST_ASSERT(DI_OK == newDirectInputDevice->SetDataFormat(&kTestFormatSpec));
        return newDirectInputDevice;
    }


    // -------- TEST CASES ------------------------------------------------- //

    // TODO
}
