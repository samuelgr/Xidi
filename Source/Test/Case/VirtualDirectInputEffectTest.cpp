/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
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
    using ::Xidi::Controller::EPhysicalDeviceStatus;
    using ::Xidi::Controller::EPovDirection;
    using ::Xidi::Controller::Mapper;
    using ::Xidi::Controller::PovMapper;
    using ::Xidi::Controller::SPhysicalState;
    using ::Xidi::Controller::TControllerIdentifier;
    using ::Xidi::Controller::VirtualController;
    using ::Xidi::Controller::ForceFeedback::SAssociatedAxes;
    using ::Xidi::Controller::ForceFeedback::SEnvelope;
    using ::Xidi::Controller::ForceFeedback::TEffectIdentifier;
    using ::Xidi::Controller::ForceFeedback::TEffectTimeMs;
    using ::Xidi::Controller::ForceFeedback::TEffectValue;


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
    struct STestDataPacket
    {
        TAxisValue axisX;
        TAxisValue axisY;
        TAxisValue axisZ;
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
    /// Describes a layout with 3 axes, a POV, and 4 buttons, with force feedback actuators on the X and Y axes.
    static const Mapper kTestMapper(
        {
            .stickLeftX = std::make_unique<AxisMapper>(EAxis::X),
            .stickLeftY = std::make_unique<AxisMapper>(EAxis::Y),
            .stickRightX = std::make_unique<AxisMapper>(EAxis::Z),
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
            .leftMotor = {.isPresent = true, .mode = EActuatorMode::SingleAxis, .singleAxis = {.axis = EAxis::X, .direction = EAxisDirection::Both}},
            .rightMotor = {.isPresent = true, .mode = EActuatorMode::SingleAxis, .singleAxis = {.axis = EAxis::Y, .direction = EAxisDirection::Both}}
        }
    );

    /// Object format specification for #STestDataPacket.
    static DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
        {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisX),      .dwType = DIDFT_AXIS    | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_YAxis,  .dwOfs = offsetof(STestDataPacket, axisY),      .dwType = DIDFT_AXIS    | DIDFT_ANYINSTANCE,    .dwFlags = 0},
        {.pguid = &GUID_ZAxis,  .dwOfs = offsetof(STestDataPacket, axisZ),      .dwType = DIDFT_AXIS    | DIDFT_ANYINSTANCE,    .dwFlags = 0},
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
        .deviceStatus = EPhysicalDeviceStatus::Ok,
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Creates and returns a smart pointer to a mock physical controller object set up correctly for force feedback but with neutral state.
    /// @param [in] mapper Read-only reference to the mapper object to use, defaults to the test mapper at the top of this file.
    /// @return Initialized mock physical controller object.
    static inline std::unique_ptr<MockPhysicalController> CreateMockPhysicalController(const Mapper& mapper = kTestMapper)
    {
        return std::make_unique<MockPhysicalController>(kTestControllerIdentifier, mapper, &kNeutralPhysicalState, 1);
    }

    /// Creates and returns a DirectInput device object that by default uses the mapper at the top of this file.
    /// @param [in] controllerIdentifier Identifier of the controller to use when creating the underlying virtual controller object, defaults to the identifier at the top of this file.
    /// @return Smart pointer to a new virtual DirectInput device object.
    static inline std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> CreateTestDirectInputDevice(TControllerIdentifier controllerIdentifier = kTestControllerIdentifier)
    {
        return std::make_unique<VirtualDirectInputDevice<ECharMode::W>>(std::make_unique<VirtualController>(controllerIdentifier));
    }

    /// Creates and returns a DirectInput device object that by default uses the mapper and data packet format at the top of this file.
    /// The new object has its data format set and is acquired in exclusive mode before being returned, so it is immediately ready for force feedback effect operations.
    /// @param [in] mockPhysicalController Read-only reference to a mock physical controller object.
    /// @param [in] dataFormatSpec Pointer to a read-only DirectInput data packet format specification, defaults to using the data packet structure at the top of this file.
    /// @return Smart pointer to a new virtual DirectInput device object.
    static inline std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> CreateAndAcquireTestDirectInputDevice(const MockPhysicalController& mockPhysicalController, LPCDIDATAFORMAT dataFormatSpec = &kTestFormatSpec)
    {
        std::unique_ptr<VirtualDirectInputDevice<ECharMode::W>> newDirectInputDevice = CreateTestDirectInputDevice(mockPhysicalController.GetControllerIdentifier());
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

    /// Computes the DirectInput object identifier for an axis.
    /// @param [in] axis Axis for which an object ID is desired.
    /// @param [in] mapper Read-only reference to the mapper object to use, defaults to the test mapper at the top of this file.
    /// @return Object identifier for the desired axis.
    static inline DWORD ObjectIdForAxis(EAxis axis, const Mapper& mapper = kTestMapper)
    {
        if (false == mapper.GetCapabilities().HasAxis(axis))
            TEST_FAILED_BECAUSE("Mapper does not contain the axis for which an object ID was requested.");

        return (DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE((int)mapper.GetCapabilities().FindAxis(axis)));
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
        TEST_ASSERT(DI_OK == diEffect->StartInternal(1, 0, 0));
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

    // Plays an effect and verifies its status is reported correctly.
    TEST_CASE(VirtualDirectInputEffect_GetEffectStatus)
    {
        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.InitializeDefaultAssociatedAxes();
        ffEffect.InitializeDefaultDirection();
        ffEffect.SetDuration(100);
        ffEffect.SetTypeSpecificParameters({.valid = true});

        DWORD effectStatus = 0;

        TEST_ASSERT(DI_OK == diEffect->GetEffectStatus(&effectStatus));
        TEST_ASSERT(0 == (effectStatus & DIEGES_PLAYING));

        TEST_ASSERT(DI_OK == diEffect->Download());
        TEST_ASSERT(DI_OK == diEffect->GetEffectStatus(&effectStatus));
        TEST_ASSERT(0 == (effectStatus & DIEGES_PLAYING));

        TEST_ASSERT(DI_OK == diEffect->StartInternal(1, 0, 0));
        TEST_ASSERT(DI_OK == diEffect->GetEffectStatus(&effectStatus));
        TEST_ASSERT(DIEGES_PLAYING == (effectStatus & DIEGES_PLAYING));

        TEST_ASSERT(DI_OK == diEffect->Stop());
        TEST_ASSERT(DI_OK == diEffect->GetEffectStatus(&effectStatus));
        TEST_ASSERT(0 == (effectStatus & DIEGES_PLAYING));
    }

    // Plays an effect and unloads it while it is playing.
    TEST_CASE(VirtualDirectInputEffect_UnloadIsStop)
    {
        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);
        
        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.InitializeDefaultAssociatedAxes();
        ffEffect.InitializeDefaultDirection();
        ffEffect.SetDuration(100);
        ffEffect.SetTypeSpecificParameters({.valid = true});

        TEST_ASSERT(DI_OK == diEffect->Download());
        TEST_ASSERT(DI_OK == diEffect->StartInternal(1, 0, 0));

        TEST_ASSERT(DI_OK == diEffect->Unload());
        TEST_ASSERT(false == ffDevice.IsEffectPlaying(ffEffect.Identifier()));
        TEST_ASSERT(false == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
    }

    // Plays an effect and releases it while it is playing.
    TEST_CASE(VirtualDirectInputEffect_ReleaseIsUnload)
    {
        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        
        // For this test it is necessary to manipulate raw pointers just like a real DirectInput application would.
        auto diEffect = CreateTestDirectInputEffect(*diDevice).release();
        const TEffectIdentifier kForceFeedbackEffectIdentifier = diEffect->UnderlyingEffect().Identifier();

        // Any reference to the effect will become invalid when the effect is deleted using COM methods.
        do
        {
            MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
            ffEffect.InitializeDefaultAssociatedAxes();
            ffEffect.InitializeDefaultDirection();
            ffEffect.SetDuration(100);
            ffEffect.SetTypeSpecificParameters({.valid = true});
        } while (false);

        TEST_ASSERT(DI_OK == diEffect->Download());
        TEST_ASSERT(DI_OK == diEffect->StartInternal(1, 0, 0));
        TEST_ASSERT(true == ffDevice.IsEffectPlaying(kForceFeedbackEffectIdentifier));
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(kForceFeedbackEffectIdentifier));

        TEST_ASSERT(0 == diEffect->Release());
        TEST_ASSERT(false == ffDevice.IsEffectPlaying(kForceFeedbackEffectIdentifier));
        TEST_ASSERT(false == ffDevice.IsEffectOnDevice(kForceFeedbackEffectIdentifier));
    }


    // The following sequence of tests, which together comprise the SetParameters suite, exercises the SetParameters method for changing the parameters of an effect.
    // Scopes are highly varied, so more details are provided with each test case.

    // Associated axes, identified by offset into the application's data packet.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_AssociatedAxesByOffset)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        DWORD axes[] = {offsetof(STestDataPacket, axisX), offsetof(STestDataPacket, axisY)};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_OBJECTOFFSETS, .cAxes = _countof(axes), .rgdwAxes = axes};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_AXES | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasAssociatedAxes());
        constexpr SAssociatedAxes kExpectedAssociatedAxes = {.count = _countof(axes), .type = {EAxis::X, EAxis::Y}};
        const SAssociatedAxes kActualAssociatedAxes = ffEffect.GetAssociatedAxes().value();
        TEST_ASSERT(kActualAssociatedAxes == kExpectedAssociatedAxes);
    }

    // Associated axes, identified by object ID.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_AssociatedAxesByObjectId)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        DWORD axes[] = {ObjectIdForAxis(EAxis::X), ObjectIdForAxis(EAxis::Y)};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_OBJECTIDS, .cAxes = _countof(axes), .rgdwAxes = axes};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_AXES | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasAssociatedAxes());
        constexpr SAssociatedAxes kExpectedAssociatedAxes = {.count = _countof(axes), .type = {EAxis::X, EAxis::Y}};
        const SAssociatedAxes kActualAssociatedAxes = ffEffect.GetAssociatedAxes().value();
        TEST_ASSERT(kActualAssociatedAxes == kExpectedAssociatedAxes);
    }

    // Associated axes, without any identification method specified. This should fail.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_AssociatedAxesByNothing)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        DWORD axes[] = {offsetof(STestDataPacket, axisX), offsetof(STestDataPacket, axisY)};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = 0, .cAxes = _countof(axes), .rgdwAxes = axes};
        TEST_ASSERT(DIERR_INVALIDPARAM == diEffect->SetParametersInternal(&kParameters, (DIEP_AXES | DIEP_NODOWNLOAD)));
        TEST_ASSERT(false == ffEffect.HasAssociatedAxes());
    }

    // Associated axes, but one of the axes specified does not support force feedback. This should fail.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_AssociatedAxesWithUnsupportedAxis)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        DWORD axes[] = {offsetof(STestDataPacket, axisZ)};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_OBJECTOFFSETS, .cAxes = _countof(axes), .rgdwAxes = axes};
        TEST_ASSERT(DIERR_INVALIDPARAM == diEffect->SetParametersInternal(&kParameters, (DIEP_AXES | DIEP_NODOWNLOAD)));
        TEST_ASSERT(false == ffEffect.HasAssociatedAxes());
    }

    // Direction, using Cartesian coordinates with a 2-axis effect.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_DirectionCartesian)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        LONG directionCartesian[] = {1, 1};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_CARTESIAN, .cAxes = 2, .rglDirection = directionCartesian };
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_DIRECTION | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasDirection());
        TEST_ASSERT(2 == ffEffect.Direction().GetNumAxes());
        
        const std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> kExpectedCoordinates = {(TEffectValue)directionCartesian[0], (TEffectValue)directionCartesian[1]};
        std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> actualCoordinates = {};
        TEST_ASSERT(_countof(directionCartesian) == ffEffect.Direction().GetCartesianCoordinates(&actualCoordinates[0], (int)actualCoordinates.size()));
        TEST_ASSERT(actualCoordinates == kExpectedCoordinates);
    }

    // Direction, using polar coordinates with a 2-axis effect.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_DirectionPolar)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        LONG directionPolar[] = {4500};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_POLAR, .cAxes = 2, .rglDirection = directionPolar};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_DIRECTION | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasDirection());
        TEST_ASSERT(2 == ffEffect.Direction().GetNumAxes());

        const std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> kExpectedCoordinates = {(TEffectValue)directionPolar[0]};
        std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> actualCoordinates = {};
        TEST_ASSERT(_countof(directionPolar) == ffEffect.Direction().GetPolarCoordinates(&actualCoordinates[0], (int)actualCoordinates.size()));
        TEST_ASSERT(actualCoordinates == kExpectedCoordinates);
    }

    // Direction, using spherical coordinates with a 2-axis effect.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_DirectionSpherical)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        LONG directionSpherical[] = {13500};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_SPHERICAL, .cAxes = 2, .rglDirection = directionSpherical};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_DIRECTION | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasDirection());
        TEST_ASSERT(2 == ffEffect.Direction().GetNumAxes());

        const std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> kExpectedCoordinates = {(TEffectValue)directionSpherical[0]};
        std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> actualCoordinates = {};
        TEST_ASSERT(_countof(directionSpherical) == ffEffect.Direction().GetSphericalCoordinates(&actualCoordinates[0], (int)actualCoordinates.size()));
        TEST_ASSERT(actualCoordinates == kExpectedCoordinates);
    }

    // Direction, using Cartesian coordinates that are all zero to produce an omnidirectional effect.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_DirectionOmnidirectional)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        LONG directionCartesian[] = {0, 0};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_CARTESIAN, .cAxes = 2, .rglDirection = directionCartesian};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_DIRECTION | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasDirection());
        TEST_ASSERT(true == ffEffect.Direction().IsOmnidirectional());
        TEST_ASSERT(2 == ffEffect.Direction().GetNumAxes());
        
        const std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> kExpectedCoordinates = {(TEffectValue)directionCartesian[0], (TEffectValue)directionCartesian[1]};
        std::array<TEffectValue, Controller::ForceFeedback::kEffectAxesMaximumNumber> actualCoordinates = {};
        TEST_ASSERT(_countof(directionCartesian) == ffEffect.Direction().GetCartesianCoordinates(&actualCoordinates[0], (int)actualCoordinates.size()));
        TEST_ASSERT(actualCoordinates == kExpectedCoordinates);
    }

    // Duration
    TEST_CASE(VirtualDirectInputEffect_SetParameters_Duration)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kDuration = 1000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwDuration = (DWORD)kDuration * 1000};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_DURATION | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasDuration());
        TEST_ASSERT(kDuration == ffEffect.GetDuration());
    }

    // Envelope, both setting and clearing
    TEST_CASE(VirtualDirectInputEffect_SetParameters_Envelope)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr SEnvelope kEnvelope = {.attackTime = 111, .attackLevel = 222, .fadeTime = 333, .fadeLevel = 444};
        DIENVELOPE diEnvelope = {.dwSize = sizeof(DIENVELOPE), .dwAttackLevel = (DWORD)kEnvelope.attackLevel, .dwAttackTime = (DWORD)kEnvelope.attackTime * 1000, .dwFadeLevel = (DWORD)kEnvelope.fadeLevel, .dwFadeTime = (DWORD)kEnvelope.fadeTime * 1000};
        const DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .lpEnvelope = &diEnvelope};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_ENVELOPE | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasEnvelope());
        TEST_ASSERT(kEnvelope == ffEffect.GetEnvelope());

        const DIEFFECT kParametersClearEnvelope = {.dwSize = sizeof(DIEFFECT), .lpEnvelope = nullptr};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParametersClearEnvelope, (DIEP_ENVELOPE | DIEP_NODOWNLOAD)));

        TEST_ASSERT(false == ffEffect.HasEnvelope());
    }

    // Gain
    TEST_CASE(VirtualDirectInputEffect_SetParameters_Gain)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kGain = 1000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwGain = (DWORD)kGain};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_GAIN | DIEP_NODOWNLOAD)));
        TEST_ASSERT(kGain == ffEffect.GetGain());
    }

    // Sample period
    TEST_CASE(VirtualDirectInputEffect_SetParameters_SamplePeriod)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kSamplePeriod = 10000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwSamplePeriod = (DWORD)kSamplePeriod * 1000};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_SAMPLEPERIOD | DIEP_NODOWNLOAD)));
        TEST_ASSERT(kSamplePeriod == ffEffect.GetSamplePeriod());
    }

    // Start delay
    TEST_CASE(VirtualDirectInputEffect_SetParameters_StartDelay)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kStartDelay = 50000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwStartDelay = (DWORD)kStartDelay * 1000};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_STARTDELAY | DIEP_NODOWNLOAD)));
        TEST_ASSERT(kStartDelay == ffEffect.GetStartDelay());
    }

    // Start delay, but ignored because the size of the input structure is too small
    TEST_CASE(VirtualDirectInputEffect_SetParameters_StartDelayIgnored)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kStartDelay = 50000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT_DX5), .dwStartDelay = (DWORD)kStartDelay * 1000};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_STARTDELAY | DIEP_NODOWNLOAD)));
        TEST_ASSERT(0 == ffEffect.GetStartDelay());
    }

    // Type-specific parameters
    TEST_CASE(VirtualDirectInputEffect_SetParameters_TypeSpecific)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        SMockTypeSpecificParameters expectedTypeSpecificParameters = {.valid = true, .param1 = 55, .param2 = 6789};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .cbTypeSpecificParams = sizeof(expectedTypeSpecificParameters), .lpvTypeSpecificParams = &expectedTypeSpecificParameters};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&parameters, (DIEP_TYPESPECIFICPARAMS | DIEP_NODOWNLOAD)));
        
        TEST_ASSERT(true == ffEffect.HasTypeSpecificParameters());

        SMockTypeSpecificParameters actualTypeSpecificParameters = ffEffect.GetTypeSpecificParameters().value();
        TEST_ASSERT(actualTypeSpecificParameters == expectedTypeSpecificParameters);
    }

    // Type-specific parameters that are invalid and cannot be automatically fixed
    TEST_CASE(VirtualDirectInputEffect_SetParameters_InvalidTypeSpecificParameters)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.SetCanFixInvalidTypeSpecificParameters(false);

        SMockTypeSpecificParameters invalidTypeSpecificParameters = {.valid = false, .param1 = 1000, .param2 = 2000};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .cbTypeSpecificParams = sizeof(invalidTypeSpecificParameters), .lpvTypeSpecificParams = &invalidTypeSpecificParameters};
        TEST_ASSERT(DIERR_INVALIDPARAM == diEffect->SetParametersInternal(&parameters, (DIEP_TYPESPECIFICPARAMS | DIEP_NODOWNLOAD)));
        TEST_ASSERT(false == ffEffect.HasTypeSpecificParameters());
    }

    // Type-specific parameters that are invalid but can be automatically fixed
    TEST_CASE(VirtualDirectInputEffect_SetParameters_InvalidFixableTypeSpecificParameters)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.SetCanFixInvalidTypeSpecificParameters(true);

        SMockTypeSpecificParameters invalidTypeSpecificParameters = {.valid = false, .param1 = 1000, .param2 = 2000};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .cbTypeSpecificParams = sizeof(invalidTypeSpecificParameters), .lpvTypeSpecificParams = &invalidTypeSpecificParameters};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&parameters, (DIEP_TYPESPECIFICPARAMS | DIEP_NODOWNLOAD)));

        TEST_ASSERT(true == ffEffect.HasTypeSpecificParameters());

        const SMockTypeSpecificParameters kExpectedTypeSpecificParameters = {.valid = true, .param1 = invalidTypeSpecificParameters.param1, .param2 = invalidTypeSpecificParameters.param2};
        const SMockTypeSpecificParameters kActualTypeSpecificParameters = ffEffect.GetTypeSpecificParameters().value();
        TEST_ASSERT(kActualTypeSpecificParameters == kExpectedTypeSpecificParameters);
    }

    // Specifies a complete set of parameters and automatically downloads, but does not start, the effect.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_CompleteAndDownload)
    {
        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        DWORD axes[] = {ObjectIdForAxis(EAxis::X), ObjectIdForAxis(EAxis::Y)};
        LONG directionCartesian[] = {1, 1};
        SMockTypeSpecificParameters typeSpecificParams = {.valid = true};

        const DIEFFECT kParameters = {
            .dwSize = sizeof(DIEFFECT),
            .dwFlags = (DIEFF_CARTESIAN | DIEFF_OBJECTIDS),
            .dwDuration = 1000000,
            .cAxes = 2,
            .rgdwAxes = axes,
            .rglDirection = directionCartesian,
            .cbTypeSpecificParams = sizeof(SMockTypeSpecificParameters),
            .lpvTypeSpecificParams = (LPVOID)&typeSpecificParams
        };

        TEST_ASSERT(DI_OK == diEffect->SetParametersInternal(&kParameters, (DIEP_DURATION | DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS), 0));
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
        TEST_ASSERT(false == ffDevice.IsEffectPlaying(ffEffect.Identifier()));
    }

    // Specifies a complete set of parameters and automatically starts the effect.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_CompleteAndStart)
    {
        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        DWORD axes[] = {ObjectIdForAxis(EAxis::X), ObjectIdForAxis(EAxis::Y)};
        LONG directionCartesian[] = {1, 1};
        SMockTypeSpecificParameters typeSpecificParams = {.valid = true};

        const DIEFFECT kParameters = {
            .dwSize = sizeof(DIEFFECT),
            .dwFlags = (DIEFF_CARTESIAN | DIEFF_OBJECTIDS),
            .dwDuration = 1000000,
            .cAxes = 2,
            .rgdwAxes = axes,
            .rglDirection = directionCartesian,
            .cbTypeSpecificParams = sizeof(SMockTypeSpecificParameters),
            .lpvTypeSpecificParams = (LPVOID)&typeSpecificParams
        };

        TEST_ASSERT(DI_OK == diEffect->SetParametersInternal(&kParameters, (DIEP_DURATION | DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START), 0));
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
        TEST_ASSERT(true == ffDevice.IsEffectPlaying(ffEffect.Identifier()));
    }

    // Specifies an empty set of new parameters and with the download operation skipped, so nothing should happen.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_Empty)
    {
        constexpr DIEFFECT kEffectParameters = {.dwSize = sizeof(DIEFFECT)};

        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.InitializeDefaultAssociatedAxes();
        ffEffect.InitializeDefaultDirection();
        ffEffect.SetDuration(100);
        ffEffect.SetTypeSpecificParameters({.valid = true});

        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kEffectParameters, DIEP_NODOWNLOAD));
        TEST_ASSERT(false == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
    }

    // Specifies an empty set of new parameters but with no flags, so the effect should be downloaded.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_DownloadOnly)
    {
        constexpr DIEFFECT kEffectParameters = {.dwSize = sizeof(DIEFFECT)};

        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.InitializeDefaultAssociatedAxes();
        ffEffect.InitializeDefaultDirection();
        ffEffect.SetDuration(100);
        ffEffect.SetTypeSpecificParameters({.valid = true});

        TEST_ASSERT(DI_OK == diEffect->SetParametersInternal(&kEffectParameters, 0));
        TEST_ASSERT(true == ffDevice.IsEffectOnDevice(ffEffect.Identifier()));
    }

    // Specifies too many behavior flags, so the operation should fail.
    TEST_CASE(VirtualDirectInputEffect_SetParameters_TooManyBehaviorFlags)
    {
         constexpr DIEFFECT kEffectParameters = {.dwSize = sizeof(DIEFFECT)};

        auto physicalController = CreateMockPhysicalController();
        auto& ffDevice = physicalController->GetForceFeedbackDevice();

        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.InitializeDefaultAssociatedAxes();
        ffEffect.InitializeDefaultDirection();
        ffEffect.SetDuration(100);
        ffEffect.SetTypeSpecificParameters({.valid = true});

        TEST_ASSERT(DIERR_INVALIDPARAM == diEffect->SetParametersInternal(&kEffectParameters, DIEP_NODOWNLOAD | DIEP_NORESTART | DIEP_START));
    }


    // The following sequence of tests, which together comprise the GetParameters suite, exercises the GetParameters method for retrieving the parameters of an effect.
    // Each test case focuses on a single parameter.

    // Associated axes, identified by offset into the application's data packet.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_AssociatedAxesByOffset)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.SetAssociatedAxes({.count = 2, .type = {EAxis::X, EAxis::Y}});

        const DWORD kExpectedAxes[] = {offsetof(STestDataPacket, axisX), offsetof(STestDataPacket, axisY)};
        DWORD actualAxes[_countof(kExpectedAxes)] = {};

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_OBJECTOFFSETS, .cAxes = _countof(actualAxes), .rgdwAxes = actualAxes};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_AXES));
        
        for (int i = 0; i < _countof(actualAxes); ++i)
            TEST_ASSERT(actualAxes[i] == kExpectedAxes[i]);
    }

    // Associated axes, identified by object ID.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_AssociatedAxesByObjectId)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.SetAssociatedAxes({.count = 2, .type = {EAxis::X, EAxis::Y}});

        const DWORD kExpectedAxes[] = {ObjectIdForAxis(EAxis::X), ObjectIdForAxis(EAxis::Y)};
        DWORD actualAxes[_countof(kExpectedAxes)] = {};

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_OBJECTIDS, .cAxes = _countof(actualAxes), .rgdwAxes = actualAxes};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_AXES));
        
        for (int i = 0; i < _countof(actualAxes); ++i)
            TEST_ASSERT(actualAxes[i] == kExpectedAxes[i]);
    }

    // Associated axes, without any identification method specified. This should fail.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_AssociatedAxesByNothing)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        DWORD axes[2] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = 0, .cAxes = _countof(axes), .rgdwAxes = axes};
        TEST_ASSERT(DIERR_INVALIDPARAM == diEffect->GetParameters(&parameters, DIEP_AXES));
    }

    // Associated axes but with a buffer size that is too small. This should retrieve the correct buffer size.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_AssociatedAxesInsufficientBuffer)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        ffEffect.SetAssociatedAxes({.count = 2, .type = {EAxis::X, EAxis::Y}});

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_OBJECTOFFSETS, .cAxes = 0, .rgdwAxes = nullptr};
        TEST_ASSERT(DIERR_MOREDATA == diEffect->GetParameters(&parameters, DIEP_AXES));
        TEST_ASSERT(ffEffect.GetAssociatedAxes().value().count == parameters.cAxes);
    }

    // Direction in Cartesian coordinates.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionCartesian)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {1, 3};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingCartesian(kExpectedCoordinates, _countof(kExpectedCoordinates)));

        LONG actualCoordinates[_countof(kExpectedCoordinates)] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_CARTESIAN, .cAxes = _countof(actualCoordinates), .rglDirection = actualCoordinates};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DIRECTION));

        for (int i = 0; i < _countof(actualCoordinates); ++i)
            TEST_ASSERT((TEffectValue)actualCoordinates[i] == kExpectedCoordinates[i]);
    }

    // Direction in Cartesian coordinates but with the expectation that DirectInput will pick this coordinate system because it was the original one used.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionOriginalCartesian)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {1, 3};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingCartesian(kExpectedCoordinates, _countof(kExpectedCoordinates)));

        constexpr DWORD kInputFlags = (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL | DIEFF_OBJECTIDS);
        constexpr DWORD kExpectedOutputFlags = (DIEFF_CARTESIAN | DIEFF_OBJECTIDS);

        LONG actualCoordinates[_countof(kExpectedCoordinates)] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = kInputFlags, .cAxes = _countof(actualCoordinates), .rglDirection = actualCoordinates};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DIRECTION));
        TEST_ASSERT(kExpectedOutputFlags == parameters.dwFlags);

        for (int i = 0; i < _countof(actualCoordinates); ++i)
            TEST_ASSERT((TEffectValue)actualCoordinates[i] == kExpectedCoordinates[i]);
    }

    // Direction in polar coordinates.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionPolar)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {20000, 0};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingPolar(kExpectedCoordinates, _countof(kExpectedCoordinates) - 1));

        LONG actualCoordinates[_countof(kExpectedCoordinates)] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_POLAR, .cAxes = _countof(actualCoordinates), .rglDirection = actualCoordinates};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DIRECTION));

        for (int i = 0; i < _countof(actualCoordinates); ++i)
            TEST_ASSERT((TEffectValue)actualCoordinates[i] == kExpectedCoordinates[i]);
    }

    // Direction in polar coordinates but with the expectation that DirectInput will pick this coordinate system because it was the original one used.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionOriginalPolar)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {20000, 0};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingPolar(kExpectedCoordinates, _countof(kExpectedCoordinates) - 1));

        constexpr DWORD kInputFlags = (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL | DIEFF_OBJECTIDS);
        constexpr DWORD kExpectedOutputFlags = (DIEFF_POLAR | DIEFF_OBJECTIDS);

        LONG actualCoordinates[_countof(kExpectedCoordinates)] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = kInputFlags, .cAxes = _countof(actualCoordinates), .rglDirection = actualCoordinates};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DIRECTION));
        TEST_ASSERT(kExpectedOutputFlags == parameters.dwFlags);

        for (int i = 0; i < _countof(actualCoordinates); ++i)
            TEST_ASSERT((TEffectValue)actualCoordinates[i] == kExpectedCoordinates[i]);
    }

    // Direction in spherical coordinates.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionSpherical)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {20000, 0};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingSpherical(kExpectedCoordinates, _countof(kExpectedCoordinates) - 1));

        LONG actualCoordinates[_countof(kExpectedCoordinates)] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = DIEFF_SPHERICAL, .cAxes = _countof(actualCoordinates), .rglDirection = actualCoordinates};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DIRECTION));

        for (int i = 0; i < _countof(actualCoordinates); ++i)
            TEST_ASSERT((TEffectValue)actualCoordinates[i] == kExpectedCoordinates[i]);
    }

    // Direction in spherical coordinates but with the expectation that DirectInput will pick this coordinate system because it was the original one used.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionOriginalSpherical)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {20000, 0};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingSpherical(kExpectedCoordinates, _countof(kExpectedCoordinates) - 1));

        constexpr DWORD kInputFlags = (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL | DIEFF_OBJECTOFFSETS);
        constexpr DWORD kExpectedOutputFlags = (DIEFF_SPHERICAL | DIEFF_OBJECTOFFSETS);

        LONG actualCoordinates[_countof(kExpectedCoordinates)] = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = kInputFlags, .cAxes = _countof(actualCoordinates), .rglDirection = actualCoordinates};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DIRECTION));
        TEST_ASSERT(kExpectedOutputFlags == parameters.dwFlags);

        for (int i = 0; i < _countof(actualCoordinates); ++i)
            TEST_ASSERT((TEffectValue)actualCoordinates[i] == kExpectedCoordinates[i]);
    }

    // Direction but with a buffer size that is too small. This should retrieve the correct buffer size.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_DirectionInsufficientBuffer)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedCoordinates[] = {20000, 0};
        TEST_ASSERT(true == ffEffect.Direction().SetDirectionUsingSpherical(kExpectedCoordinates, _countof(kExpectedCoordinates) - 1));

        constexpr DWORD kInputFlags = (DIEFF_CARTESIAN | DIEFF_POLAR | DIEFF_SPHERICAL | DIEFF_OBJECTOFFSETS);
        constexpr DWORD kExpectedOutputFlags = (DIEFF_SPHERICAL | DIEFF_OBJECTOFFSETS);

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .dwFlags = kInputFlags, .cAxes = 0, .rglDirection = nullptr};
        TEST_ASSERT(DIERR_MOREDATA == diEffect->GetParameters(&parameters, DIEP_DIRECTION));
        TEST_ASSERT(ffEffect.Direction().GetNumAxes() == parameters.cAxes);
    }

    // Duration
    TEST_CASE(VirtualDirectInputEffect_GetParameters_Duration)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kInputDuration = 5;
        constexpr TEffectTimeMs kExpectedDuration = kInputDuration * 1000;
        TEST_ASSERT(true == ffEffect.SetDuration(kInputDuration));

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT)};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_DURATION));
        TEST_ASSERT((TEffectTimeMs)parameters.dwDuration == kExpectedDuration);
    }

    // Envelope
    TEST_CASE(VirtualDirectInputEffect_GetParameters_Envelope)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr SEnvelope kInputEnvelope = {.attackTime = 111, .attackLevel = 222, .fadeTime = 333, .fadeLevel = 444};
        constexpr DIENVELOPE kExpectedEnvelope = {.dwSize = sizeof(DIENVELOPE), .dwAttackLevel = (DWORD)kInputEnvelope.attackLevel, .dwAttackTime = (DWORD)kInputEnvelope.attackTime * 1000, .dwFadeLevel = (DWORD)kInputEnvelope.fadeLevel, .dwFadeTime = (DWORD)kInputEnvelope.fadeTime * 1000};
        TEST_ASSERT(true == ffEffect.SetEnvelope(kInputEnvelope));

        DIENVELOPE actualEnvelope = {.dwSize = sizeof(DIENVELOPE)};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .lpEnvelope = &actualEnvelope};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_ENVELOPE));
        TEST_ASSERT(0 == memcmp(&actualEnvelope, &kExpectedEnvelope, sizeof(DIENVELOPE)));
    }

    // Envelope but with none present, so output structure should hold a null pointer.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_EnvelopeAbsent)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();
        TEST_ASSERT(false == ffEffect.HasEnvelope());

        DIENVELOPE envelope = {.dwSize = sizeof(DIENVELOPE)};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .lpEnvelope = &envelope};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_ENVELOPE));
        TEST_ASSERT(nullptr == parameters.lpEnvelope);
    }

    // Gain
    TEST_CASE(VirtualDirectInputEffect_GetParameters_Gain)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kExpectedGain = 1234;
        TEST_ASSERT(true == ffEffect.SetGain(kExpectedGain));

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT)};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_GAIN));
        TEST_ASSERT((TEffectValue)parameters.dwGain == kExpectedGain);
    }

    // Sample period
    TEST_CASE(VirtualDirectInputEffect_GetParameters_SamplePeriod)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kInputSamplePeriod = 5;
        constexpr TEffectTimeMs kExpectedSamplePeriod = kInputSamplePeriod * 1000;
        TEST_ASSERT(true == ffEffect.SetSamplePeriod(kInputSamplePeriod));

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT)};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_SAMPLEPERIOD));
        TEST_ASSERT((TEffectTimeMs)parameters.dwSamplePeriod == kExpectedSamplePeriod);
    }

    // Start delay
    TEST_CASE(VirtualDirectInputEffect_GetParameters_StartDelay)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kInputStartDelay = 100;
        constexpr TEffectTimeMs kExpectedStartDelay = kInputStartDelay * 1000;
        TEST_ASSERT(true == ffEffect.SetStartDelay(kInputStartDelay));

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT)};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_STARTDELAY));
        TEST_ASSERT((TEffectTimeMs)parameters.dwStartDelay == kExpectedStartDelay);
    }

    // Start delay, but ignored because the size of the input structure is too small
    TEST_CASE(VirtualDirectInputEffect_GetParameters_StartDelayIgnored)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectTimeMs kInputStartDelay = 100;
        constexpr TEffectTimeMs kExpectedStartDelay = kInputStartDelay * 1000;
        TEST_ASSERT(true == ffEffect.SetStartDelay(kInputStartDelay));

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT_DX5)};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_STARTDELAY));
        TEST_ASSERT(0 == (TEffectTimeMs)parameters.dwStartDelay);
    }

    // Type-specific parameters
    TEST_CASE(VirtualDirectInputEffect_GetParameters_TypeSpecific)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr SMockTypeSpecificParameters kExpectedTypeSpecificParameters = {.valid = true, .param1 = 1234, .param2 = 5678};
        TEST_ASSERT(true == ffEffect.SetTypeSpecificParameters(kExpectedTypeSpecificParameters));

        SMockTypeSpecificParameters actualTypeSpecificParameters = {};
        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .cbTypeSpecificParams = sizeof(actualTypeSpecificParameters), .lpvTypeSpecificParams = &actualTypeSpecificParameters};
        TEST_ASSERT(DI_OK == diEffect->GetParameters(&parameters, DIEP_TYPESPECIFICPARAMS));
        TEST_ASSERT(actualTypeSpecificParameters == kExpectedTypeSpecificParameters);
    }
    
    // Type-specific parameters but with a buffer size that is too small. This should retrieve the correct buffer size.
    TEST_CASE(VirtualDirectInputEffect_GetParameters_TypeSpecificInsufficientBuffer)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr SMockTypeSpecificParameters kTypeSpecificParameters = {.valid = true, .param1 = 1234, .param2 = 5678};
        TEST_ASSERT(true == ffEffect.SetTypeSpecificParameters(kTypeSpecificParameters));

        DIEFFECT parameters = {.dwSize = sizeof(DIEFFECT), .cbTypeSpecificParams = 0, .lpvTypeSpecificParams = nullptr};
        TEST_ASSERT(DIERR_MOREDATA == diEffect->GetParameters(&parameters, DIEP_TYPESPECIFICPARAMS));
        TEST_ASSERT(sizeof(SMockTypeSpecificParameters) == parameters.cbTypeSpecificParams);
    }
}
