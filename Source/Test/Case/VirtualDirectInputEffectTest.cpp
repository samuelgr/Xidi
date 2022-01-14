/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file VirtualDirectInputEffectTest.cpp
 *   Unit tests for DirectInput interface objects that wrap force feedback
 *   effect objects.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
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
    using ::Xidi::Controller::SPhysicalState;
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

    /// GUID used to identify all test force feedback effect objects.
    static constexpr GUID kTestEffectGuid = {0x12345678, 0x9abc, 0xdef0, {'X', 'I', 'D', 'I', 'T', 'E', 'S', 'T'}};

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

    /// Neutral physical state for use with mock physical controller objects.
    static constexpr SPhysicalState kNeutralPhysicalState = {
        .errorCode = ERROR_SUCCESS,
        .state = {.dwPacketNumber = 1}
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Creates and returns a smart pointer to a mock physical controller object set up correctly for force feedback but with neutral state.
    /// @return Initialized mock physical controller object.
    static inline std::unique_ptr<MockPhysicalController> CreateMockPhysicalController(void)
    {
        return std::make_unique<MockPhysicalController>(kTestControllerIdentifier, &kNeutralPhysicalState, 1);
    }

    /// Creates and returns a DirectInput device object that by default uses the mapper at the top of this file.
    /// @param [in] controllerIdentifier Identifier of the controller to use when creating the underlying virtual controller object, defaults to the identifier at the top of this file.
    /// @param [in] mapper Read-only reference to the mapper object to use, defaults to the test mapper at the top of this file.
    /// @return Smart pointer to a new virtual DirectInput device object.
    static inline std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> CreateTestDirectInputDevice(TControllerIdentifier controllerIdentifier = kTestControllerIdentifier, const Mapper& mapper = kTestMapper)
    {
        return std::make_unique<VirtualDirectInputDevice<ECharMode::W>>(std::make_unique<VirtualController>(controllerIdentifier, mapper));
    }

    /// Creates and returns a DirectInput device object that by default uses the mapper and data packet format at the top of this file.
    /// The new object has its data format set and is acquired in exclusive mode before being returned, so it is immediately ready for force feedback effect operations.
    /// @param [in] mockPhysicalController Read-only reference to a mock physical controller object.
    /// @param [in] mapper Read-only reference to the mapper object to use, defaults to the test mapper at the top of this file.
    /// @param [in] dataFormatSpec Pointer to a read-only DirectInput data packet format specification, defaults to using the data packet structure at the top of this file.
    /// @return Smart pointer to a new virtual DirectInput device object.
    static inline std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> CreateAndAcquireTestDirectInputDevice(const MockPhysicalController& mockPhysicalController, const Mapper& mapper = kTestMapper, LPCDIDATAFORMAT dataFormatSpec = &kTestFormatSpec)
    {
        std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> newDirectInputDevice = CreateTestDirectInputDevice(mockPhysicalController.GetControllerIdentifier(), mapper);
        TEST_ASSERT(DI_OK == newDirectInputDevice->SetDataFormat(dataFormatSpec));
        TEST_ASSERT(DI_OK == newDirectInputDevice->SetCooperativeLevel(nullptr, DISCL_EXCLUSIVE | DISCL_FOREGROUND));
        TEST_ASSERT(DI_OK == newDirectInputDevice->Acquire());
        return newDirectInputDevice;
    }

    /// Creates and returns a DirectInput force feedback effect object that can be used for tests.
    /// @param [in] associatedDevice Device with which the effect object should be associated.
    /// @param [in] effectGuid GUID to use to identify the effect, defaults to the test GUID at the top of this file.
    /// @return Smart pointer to a new force feedback effect object.
    static inline std::unique_ptr<TestVirtualDirectInputEffect> CreateTestDirectInputEffect(VirtualDirectInputDevice<ECharMode::W>& associatedDevice, const GUID& effectGuid = kTestEffectGuid)
    {
        return std::make_unique<TestVirtualDirectInputEffect>(associatedDevice, MockEffectWithTypeSpecificParameters(), effectGuid);
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies that effect GUID retrieval operates correctly.
    TEST_CASE(VirtualDirectInputEffect_GetEffectGuid)
    {
        const GUID* kTestGuids[] = {&kTestEffectGuid, &GUID_ConstantForce, &GUID_Sine};

        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);

        for (const auto kTestGuid : kTestGuids)
        {
            auto diEffect = CreateTestDirectInputEffect(*diDevice, *kTestGuid);
            
            const GUID& kExpectedGuid = *kTestGuid;
            GUID actualGuid = {};

            TEST_ASSERT(DI_OK == diEffect->GetEffectGuid(&actualGuid));
            TEST_ASSERT(actualGuid == kExpectedGuid);
        }
    }

    // Exercises the nominal situation of creating an effect, setting some parameters, downloading it to the device, starting it, stopping it, and unloading it.
    TEST_CASE(VirtualDirectInputEffect_Nominal)
    {
        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);
        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        // By default the effect is incomplete.
        TEST_ASSERT(false == ffEffect.IsCompletelyDefined());
        TEST_ASSERT(DIERR_INCOMPLETEEFFECT == diEffect->Download());

        // Initialize the effect to some defaults.
        ffEffect.InitializeDefaultAssociatedAxes();
        ffEffect.InitializeDefaultDirection();
        ffEffect.SetDuration(100);
        ffEffect.SetTypeSpecificParameters({.valid = true});

        // Effect should be complete now.
        TEST_ASSERT(true == ffEffect.IsCompletelyDefined());
        TEST_ASSERT(DI_OK == diEffect->Download());
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
        TEST_ASSERT(false == ffDevice.IsEffectPlaying(ffEffect.Identifier()));

        // Starting the effect should mark it as playing.
        TEST_ASSERT(DI_OK == diEffect->StartPlayback(1, 0, 0));
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
        TEST_ASSERT(true == ffDevice.IsEffectPlaying(ffEffect.Identifier()));

        // Stopping the effect should mark it as not playing.
        TEST_ASSERT(DI_OK == diEffect->Stop());
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
        TEST_ASSERT(false == ffDevice.IsEffectPlaying(ffEffect.Identifier()));

        // Unloading the effect should remove it from the device.
        TEST_ASSERT(DI_OK == diEffect->Unload());
        TEST_ASSERT(false == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
    }
}
