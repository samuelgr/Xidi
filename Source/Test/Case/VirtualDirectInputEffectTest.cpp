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
    using ::Xidi::Controller::ForceFeedback::SAssociatedAxes;
    using ::Xidi::Controller::ForceFeedback::SEnvelope;
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
            .leftMotor = {.isPresent = true, .axis = EAxis::X, .direction = EAxisDirection::Both},
            .rightMotor = {.isPresent = true, .axis = EAxis::Y, .direction = EAxisDirection::Both}
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

    // Duration
    TEST_CASE(VirtualDirectInputEffect_SetParameters_Duration)
    {
        auto physicalController = CreateMockPhysicalController();
        auto diDevice = CreateAndAcquireTestDirectInputDevice(*physicalController);
        auto diEffect = CreateTestDirectInputEffect(*diDevice);

        MockEffectWithTypeSpecificParameters& ffEffect = (MockEffectWithTypeSpecificParameters&)diEffect->UnderlyingEffect();

        constexpr TEffectValue kDuration = 1000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwDuration = (DWORD)kDuration};
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
        DIENVELOPE diEnvelope = {.dwSize = sizeof(DIENVELOPE), .dwAttackLevel = (DWORD)kEnvelope.attackLevel, .dwAttackTime = (DWORD)kEnvelope.attackTime, .dwFadeLevel = (DWORD)kEnvelope.fadeLevel, .dwFadeTime = (DWORD)kEnvelope.fadeTime};
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
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwSamplePeriod = (DWORD)kSamplePeriod};
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

        constexpr TEffectTimeMs kStartDelay = 5000000;
        constexpr DIEFFECT kParameters = {.dwSize = sizeof(DIEFFECT), .dwStartDelay = (DWORD)kStartDelay};
        TEST_ASSERT(DI_DOWNLOADSKIPPED == diEffect->SetParametersInternal(&kParameters, (DIEP_STARTDELAY | DIEP_NODOWNLOAD)));
        TEST_ASSERT(kStartDelay == ffEffect.GetStartDelay());
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
}
