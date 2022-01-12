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

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ApiWindows.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Mapper.h"
#include "TestCase.h"
#include "VirtualDirectInputDevice.h"
#include "VirtualDirectInputEffect.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <set>


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
    using ::Xidi::Controller::SElementIdentifier;
    using ::Xidi::Controller::SPhysicalState;
    using ::Xidi::Controller::TControllerIdentifier;
    using ::Xidi::Controller::VirtualController;


    // -------- INTERNAL TYPES --------------------------------------------- //

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
    static constexpr TControllerIdentifier kTestControllerIdentifier = 1;

    /// Test mapper used throughout these test cases.
    /// Describes a layout with 4 axes, a POV, and 8 buttons.
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
        .buttonY = std::make_unique<ButtonMapper>(EButton::B4),
        .buttonLB = std::make_unique<ButtonMapper>(EButton::B5),
        .buttonRB = std::make_unique<ButtonMapper>(EButton::B6),
        .buttonBack = std::make_unique<ButtonMapper>(EButton::B7),
        .buttonStart = std::make_unique<ButtonMapper>(EButton::B8)
    });

    /// Object format specification for #STestDataPacket.
    static DIOBJECTDATAFORMAT testObjectFormatSpec[] = {
        {.pguid = &GUID_XAxis,  .dwOfs = offsetof(STestDataPacket, axisX),      .dwType = DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
        {.pguid = &GUID_YAxis,  .dwOfs = offsetof(STestDataPacket, axisY),      .dwType = DIDFT_AXIS   | DIDFT_ANYINSTANCE, .dwFlags = 0},
        {.pguid = &GUID_POV,    .dwOfs = offsetof(STestDataPacket, pov),        .dwType = DIDFT_POV    | DIDFT_ANYINSTANCE, .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[0]),  .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[1]),  .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[2]),  .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0},
        {.pguid = &GUID_Button, .dwOfs = offsetof(STestDataPacket, button[3]),  .dwType = DIDFT_BUTTON | DIDFT_ANYINSTANCE, .dwFlags = 0}
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


    /// Creates and returns a virtual controller object that uses the test mapper at the top of this file and a mock XInput interface object, which can be optionally specified with expected calls.
    /// @return Smart pointer to the new virtual controller object.
    static inline std::unique_ptr<VirtualController> CreateTestVirtualController(void)
    {
        return std::make_unique<VirtualController>(kTestControllerIdentifier, kTestMapper);
    }


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Applies all of the buffered events in the specified array of events to the specified test data packet structure.
    /// In doing so, ensures sequence numbers are monotonic.
    /// @param [out] testDataPacket Data packet to which to apply the events.
    /// @param [in] objectData Array of buffered event object data elements to apply to the data packet.
    /// @param [in] numEvents Number of events in the array.
    /// @param [in] lastSequence Starting sequence number for monotonicity checking, defaults to `INT_MIN`.
    /// @return Last (highest) sequence number seen in the array of buffered event data.
    static int ApplyEventsToTestDataPacket(STestDataPacket& testDataPacket, LPCDIDEVICEOBJECTDATA objectData, int numEvents, int lastSequence = INT_MIN)
    {
        for (int i = 0; i < numEvents; ++i)
        {
            TEST_ASSERT((int)objectData[i].dwSequence > lastSequence);
            lastSequence = objectData[i].dwSequence;

            const size_t kDataAddress = (size_t)&testDataPacket + (size_t)objectData[i].dwOfs;

            if (objectData[i].dwOfs >= offsetof(STestDataPacket, button))
                *((TButtonValue*)kDataAddress) = (TButtonValue)objectData[i].dwData;
            else
                *((TAxisValue*)kDataAddress) = (TAxisValue)objectData[i].dwData;
        }

        return lastSequence;
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies that virtual controllers can be acquired as long as the data format is already set.
    // Otherwise acquisition is completely a no-op.
    TEST_CASE(VirtualDirectInputDevice_Acquire)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DIERR_INVALIDPARAM == diController.Acquire());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.Acquire());
    }

    // Verifies that virtual controllers can be unacquired without restriction.
    // Acquisition and unacquisition is completely a no-op.
    TEST_CASE(VirtualDirectInputDevice_Unacquire)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.Unacquire());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.Acquire());
        TEST_ASSERT(DI_OK == diController.Unacquire());
        TEST_ASSERT(DI_OK == diController.Unacquire());
    }

    
    // The following sequence of tests, which together comprise the EnumObjects suite, verify that objects present on virtual controllers are correctly enumerated.
    // Scopes are highly varied, so more details are provided with each test case.

    // Verifies that axes are enumerated correctly.
    // Checks over several (but not all) elements of the instance information returned, and ensures complete coverage of all axes reported as available by the virtual controller.
    // Since the data format is not set, offsets are expected to be native data packet offsets, and therefore valid, even if they do not mean anything to the application.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_OnlyAxes_NoDataFormat)
    {
        struct SSeen
        {
            std::set<EAxis> axes;
            std::set<DWORD> instances;
        } seen;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                SSeen& seen = *((SSeen*)pvRef);
                
                TEST_ASSERT((nullptr != lpddoi) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*lpddoi) == lpddoi->dwSize);
                TEST_ASSERT(DataFormat::kInvalidOffsetValue != lpddoi->dwOfs);
                TEST_ASSERT(DIDFT_GETTYPE(lpddoi->dwType) == DIDFT_ABSAXIS);
                TEST_ASSERT((DIDOI_ASPECTPOSITION) == lpddoi->dwFlags);

                EAxis seenAxis;
                if (GUID_XAxis == lpddoi->guidType)
                    seenAxis = EAxis::X;
                else if (GUID_YAxis == lpddoi->guidType)
                    seenAxis = EAxis::Y;
                else if (GUID_ZAxis == lpddoi->guidType)
                    seenAxis = EAxis::Z;
                else if (GUID_RxAxis == lpddoi->guidType)
                    seenAxis = EAxis::RotX;
                else if (GUID_RyAxis == lpddoi->guidType)
                    seenAxis = EAxis::RotY;
                else if (GUID_RzAxis == lpddoi->guidType)
                    seenAxis = EAxis::RotZ;
                else
                    TEST_FAILED_BECAUSE(L"Unrecognized axis GUID.");

                TEST_ASSERT(true == kTestMapper.GetCapabilities().HasAxis(seenAxis));
                TEST_ASSERT(false == seen.axes.contains(seenAxis));
                seen.axes.insert(seenAxis);

                DWORD seenInstance = DIDFT_GETINSTANCE(lpddoi->dwType);
                TEST_ASSERT(seenInstance <= kTestMapper.GetCapabilities().numAxes);
                TEST_ASSERT(false == seen.instances.contains(seenInstance));
                seen.instances.insert(seenInstance);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&seen, DIDFT_ABSAXIS)
        );

        TEST_ASSERT(seen.axes.size() == kTestMapper.GetCapabilities().numAxes);
        TEST_ASSERT(seen.instances.size() == kTestMapper.GetCapabilities().numAxes);
    }

    // Same basic idea as above, but with the data format set, so only offsets are checked.
    // Controller elements that have a data format offset should report that offset, all others should report an invalid offset.
    // Even though this is not documented DirectInput behavior, it is observable by testing DirectInput itself, and some games depend on it.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_OnlyAxes_WithDataFormat)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                TEST_ASSERT(nullptr == pvRef);

                if (GUID_XAxis == lpddoi->guidType)
                    TEST_ASSERT(offsetof(STestDataPacket, axisX) == lpddoi->dwOfs);
                else if (GUID_YAxis == lpddoi->guidType)
                    TEST_ASSERT(offsetof(STestDataPacket, axisY) == lpddoi->dwOfs);
                else if (GUID_RxAxis == lpddoi->guidType)
                    TEST_ASSERT(DataFormat::kInvalidOffsetValue == lpddoi->dwOfs);
                else if (GUID_RyAxis == lpddoi->guidType)
                    TEST_ASSERT(DataFormat::kInvalidOffsetValue == lpddoi->dwOfs);

                return DIENUM_CONTINUE;
            },
            nullptr, DIDFT_ABSAXIS)
        );
    }

    // Verifies that buttons are enumerated correctly.
    // Checks over several (but not all) elements of the instance information returned, and ensures complete coverage of all buttons reported as available by the virtual controller.
    // Since the data format is not set, offsets are expected to be native data packet offsets, and therefore valid, even if they do not mean anything to the application.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_OnlyButtons_NoDataFormat)
    {
        std::set<EButton> seenButtons;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                std::set<EButton>& seenButtons = *((std::set<EButton>*)pvRef);

                TEST_ASSERT((nullptr != lpddoi) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*lpddoi) == lpddoi->dwSize);
                TEST_ASSERT(GUID_Button == lpddoi->guidType);
                TEST_ASSERT(DataFormat::kInvalidOffsetValue != lpddoi->dwOfs);
                TEST_ASSERT(DIDFT_GETTYPE(lpddoi->dwType) == DIDFT_PSHBUTTON);
                TEST_ASSERT(0 == lpddoi->dwFlags);

                EButton seenButton = (EButton)DIDFT_GETINSTANCE(lpddoi->dwType);
                TEST_ASSERT(true == kTestMapper.GetCapabilities().HasButton(seenButton));
                TEST_ASSERT(false == seenButtons.contains(seenButton));
                seenButtons.insert(seenButton);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&seenButtons, DIDFT_PSHBUTTON)
        );

        TEST_ASSERT(seenButtons.size() == kTestMapper.GetCapabilities().numButtons);
    }

    // Same basic idea as above, but with the data format set, so only offsets are checked.
    // Controller elements that have a data format offset should report that offset, all others should report an invalid offset.
    // Even though this is not documented DirectInput behavior, it is observable by testing DirectInput itself, and some games depend on it.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_OnlyButtons_WithDataFormat)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                TEST_ASSERT(nullptr == pvRef);

                const DWORD kButtonNumber = (DWORD)DIDFT_GETINSTANCE(lpddoi->dwType);
                if (kButtonNumber < _countof(STestDataPacket::button))
                    TEST_ASSERT(offsetof(STestDataPacket, button[kButtonNumber]) == lpddoi->dwOfs);
                else
                    TEST_ASSERT(DataFormat::kInvalidOffsetValue == lpddoi->dwOfs);

                return DIENUM_CONTINUE;
            },
            nullptr, DIDFT_PSHBUTTON)
        );
    }

    // Verifies that the POV is enumerated correctly via EnumObjects.
    // Checks over several (but not all) elements of the instance information returned, and verifies that up to at most 1 POV is reported, depending on the virtual controller capabilities.
    // Since the data format is not set, offsets are expected to be native data packet offsets, and therefore valid, even if they do not mean anything to the application.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_OnlyPov_NoDataFormat)
    {
        bool seenPov = false;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                bool& seenPov = *((bool*)pvRef);

                TEST_ASSERT((nullptr != lpddoi) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*lpddoi) == lpddoi->dwSize);
                TEST_ASSERT(GUID_POV == lpddoi->guidType);
                TEST_ASSERT(DataFormat::kInvalidOffsetValue != lpddoi->dwOfs);
                TEST_ASSERT(DIDFT_GETTYPE(lpddoi->dwType) == DIDFT_POV);
                TEST_ASSERT(DIDFT_GETINSTANCE(lpddoi->dwType) == 0);
                TEST_ASSERT(0 == lpddoi->dwFlags);

                TEST_ASSERT(false == seenPov);
                seenPov = true;

                return DIENUM_CONTINUE;
            },
            (LPVOID)&seenPov, DIDFT_POV)
        );

        TEST_ASSERT(kTestMapper.GetCapabilities().hasPov == seenPov);
    }

    // Same basic idea as above, but with the data format set, so only offsets are checked.
    // Controller elements that have a data format offset should report that offset, all others should report an invalid offset.
    // Even though this is not documented DirectInput behavior, it is observable by testing DirectInput itself, and some games depend on it.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_OnlyPov_WithDataFormat)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                TEST_ASSERT(nullptr == pvRef);
                TEST_ASSERT(offsetof(STestDataPacket, pov) == lpddoi->dwOfs);
                return DIENUM_CONTINUE;
            },
            nullptr, DIDFT_POV)
        );
    }

    // No objects match the enumeration request, so the callback should never be invoked.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_NoMatchingObjects)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                TEST_FAILED_BECAUSE(L"Unexpected invocation of the EnumObjects callback.");
            },
            nullptr, (DIDFT_RELAXIS | DIDFT_TGLBUTTON | DIDFT_VENDORDEFINED))
        );
    }

    // The special value `DIDFT_ALL` is passed as an enumeration specification, so all controller elements should be enumerated.
    // Verified by simple numeric consistency check of the number of times the callback is invoked.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_AllObjects)
    {
        const int kExpectedNumCallbacks = kTestMapper.GetCapabilities().numAxes + kTestMapper.GetCapabilities().numButtons + ((true == kTestMapper.GetCapabilities().hasPov) ? 1 : 0);
        int actualNumCallbacks = 0;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                *((int*)pvRef) += 1;
                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualNumCallbacks, DIDFT_ALL)
        );

        TEST_ASSERT(actualNumCallbacks == kExpectedNumCallbacks);
    }

    // Application tells DirectInput to stop enumerating objects early, so DirectInput is expected to obey.
    // Verified by checking that the callback is only invoked once.
    TEST_CASE(VirtualDirectInputDevice_EnumObjects_StopEarly)
    {
        const int kExpectedNumCallbacks = 1;
        int actualNumCallbacks = 0;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                *((int*)pvRef) += 1;
                return DIENUM_STOP;
            },
            (LPVOID)&actualNumCallbacks, DIDFT_ALL)
        );

        TEST_ASSERT(actualNumCallbacks == kExpectedNumCallbacks);
    }


    // The following sequence of tests, which together comprise the GetCapabilities suite, exercise the DirectInputDevice interface method of the same name.
    // Scopes vary, so more details are provided with each test case.

    // Nominal behavior in which a structure is passed, properly initialized with the size member set.
    // Expected outcome is the structure is filled with corrrect controller capabilities.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_Nominal)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        const DIDEVCAPS kExpectedCapabilities = {
            .dwSize = sizeof(DIDEVCAPS),
            .dwFlags = (DIDC_ATTACHED | DIDC_EMULATED),
            .dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD,
            .dwAxes = kTestMapper.GetCapabilities().numAxes,
            .dwButtons = kTestMapper.GetCapabilities().numButtons,
            .dwPOVs = (DWORD)((true == kTestMapper.GetCapabilities().hasPov) ? 1 : 0)
        };

        DIDEVCAPS actualCapabilities;
        FillMemory(&actualCapabilities, sizeof(actualCapabilities), 0xcd);
        actualCapabilities.dwSize = sizeof(DIDEVCAPS);

        TEST_ASSERT(DI_OK == diController.GetCapabilities(&actualCapabilities));
        TEST_ASSERT(0 == memcmp(&actualCapabilities, &kExpectedCapabilities, sizeof(kExpectedCapabilities)));
    }

    // Same as above, except the structure is an older version which is supported for compatibility.
    // The older structure, with suffix _DX3, is a strict subset of the more modern version.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_Legacy)
    {
        constexpr uint8_t kPoisonByte = 0xcd;
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        DIDEVCAPS expectedCapabilities;
        FillMemory(&expectedCapabilities, sizeof(expectedCapabilities), kPoisonByte);
        *((DIDEVCAPS_DX3*)&expectedCapabilities) = {
            .dwSize = sizeof(DIDEVCAPS_DX3),
            .dwFlags = (DIDC_ATTACHED | DIDC_EMULATED),
            .dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD,
            .dwAxes = kTestMapper.GetCapabilities().numAxes,
            .dwButtons = kTestMapper.GetCapabilities().numButtons,
            .dwPOVs = (DWORD)((true == kTestMapper.GetCapabilities().hasPov) ? 1 : 0)
        };

        DIDEVCAPS actualCapabilities;
        FillMemory(&actualCapabilities, sizeof(actualCapabilities), kPoisonByte);
        actualCapabilities.dwSize = sizeof(DIDEVCAPS_DX3);

        TEST_ASSERT(DI_OK == diController.GetCapabilities((DIDEVCAPS*)&actualCapabilities));
        TEST_ASSERT(0 == memcmp(&actualCapabilities, &expectedCapabilities, sizeof(expectedCapabilities)));
    }
    
    // A null pointer is passed. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_BadPointer)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(FAILED(diController.GetCapabilities(nullptr)));
    }

    // A valid pointer is passed but with the size member not initialized. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_InvalidSize)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DIDEVCAPS capabilities;
        ZeroMemory(&capabilities, sizeof(capabilities));
        TEST_ASSERT(FAILED(diController.GetCapabilities(&capabilities)));
    }


    // The following sequence of tests, which together comprise the GetDeviceInfo suite, exercise the DirectInputDevice interface method of the same name.
    // Scopes vary, so more details are provided with each test case.

    // Nominal behavior in which a structure is passed, properly initialized with the size member set.
    // Expected outcome is the structure is filled with corrrect controller capabilities.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceInfo_Nominal)
    {
        constexpr uint8_t kPoisonByte = 0xcd;
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        DIDEVICEINSTANCE expectedDeviceInfo;
        FillMemory(&expectedDeviceInfo, sizeof(expectedDeviceInfo), kPoisonByte);
        expectedDeviceInfo.dwSize = sizeof(expectedDeviceInfo);
        FillVirtualControllerInfo(expectedDeviceInfo, kTestControllerIdentifier);

        DIDEVICEINSTANCE actualDeviceInfo;
        FillMemory(&actualDeviceInfo, sizeof(actualDeviceInfo), kPoisonByte);
        actualDeviceInfo.dwSize = sizeof(actualDeviceInfo);

        TEST_ASSERT(DI_OK == diController.GetDeviceInfo(&actualDeviceInfo));
        TEST_ASSERT(0 == memcmp(&actualDeviceInfo, &expectedDeviceInfo, sizeof(expectedDeviceInfo)));
    }

    // Same as above, except the structure is an older version which is supported for compatibility.
    // The older structure, with suffix _DX3, is a strict subset of the more modern version.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceInfo_Legacy)
    {
        constexpr uint8_t kPoisonByte = 0xcd;
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        DIDEVICEINSTANCE expectedDeviceInfo;
        FillMemory(&expectedDeviceInfo, sizeof(expectedDeviceInfo), kPoisonByte);
        expectedDeviceInfo.dwSize = sizeof(DIDEVICEINSTANCE_DX3);
        FillVirtualControllerInfo(expectedDeviceInfo, kTestControllerIdentifier);

        DIDEVICEINSTANCE actualDeviceInfo;
        FillMemory(&actualDeviceInfo, sizeof(actualDeviceInfo), kPoisonByte);
        actualDeviceInfo.dwSize = sizeof(DIDEVICEINSTANCE_DX3);

        TEST_ASSERT(DI_OK == diController.GetDeviceInfo(&actualDeviceInfo));
        TEST_ASSERT(0 == memcmp(&actualDeviceInfo, &expectedDeviceInfo, sizeof(expectedDeviceInfo)));
    }

    // A null pointer is passed. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceInfo_BadPointer)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(FAILED(diController.GetDeviceInfo(nullptr)));
    }

    // A valid pointer is passed but with the size member not initialized. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceInfo_InvalidSize)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DIDEVICEINSTANCE deviceInfo;
        ZeroMemory(&deviceInfo, sizeof(deviceInfo));
        TEST_ASSERT(FAILED(diController.GetDeviceInfo(&deviceInfo)));
    }


    // The following sequence of tests, which together comprise the GetDeviceData suite, exercise the DirectInputDevice interface method of the same name.
    // Scopes vary, so more details are provided with each test case.

    // Exercises the nominal case in which events are buffered and retrieved using various queries.
    // Three types of accesses are exercised: peek, query event count, and buffer flush.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceData_NominalPeek)
    {
        constexpr DWORD kBufferSize = 16;
        constexpr DIPROPDWORD kBufferSizeProperty = {.diph = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE}, .dwData = kBufferSize};
        constexpr SPhysicalState kPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X), .sThumbLX = -1234, .sThumbRX = 5678}}};

        // Set based on the number of controller elements present in the above XINPUT_STATE structure that are also contained in STestDataPacket.
        // In this case, the right thumbstick has no matching offset, but all the other three controller components are represented.
        constexpr DWORD kExpectedNumEvents = 3;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_BUFFERSIZE, (LPCDIPROPHEADER)&kBufferSizeProperty));

        // This must occur after the buffer size property is set because the latter enables event buffering.
        diController.GetVirtualController().RefreshState(kPhysicalState);

        // Based on the mapper defined at the top of this file. POV does not need to be filled in because its state is not changing and so it will not generate an event.
        constexpr STestDataPacket kExpectedDataPacketResult = {.axisX = -1234, .button = {DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed, DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed}};

        // To get the actual data packet, retrieve buffered events from the controller and modify the data packet one event at a time.
        // First access is with DIGGD_PEEK so that no buffered events are removed.
        STestDataPacket actualDataPacketResult;
        ZeroMemory(&actualDataPacketResult, sizeof(actualDataPacketResult));

        DIDEVICEOBJECTDATA objectData[kBufferSize];
        DWORD numObjectDataElements = _countof(objectData);

        TEST_ASSERT(DI_OK == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), objectData, &numObjectDataElements, DIGDD_PEEK));
        TEST_ASSERT(kExpectedNumEvents == numObjectDataElements);
        ApplyEventsToTestDataPacket(actualDataPacketResult, objectData, numObjectDataElements);
        TEST_ASSERT(0 == memcmp(&actualDataPacketResult, &kExpectedDataPacketResult, sizeof(kExpectedDataPacketResult)));

        // Second access is a query for the number of events without any retrieval or removal. Should be the same as before.
        // This query follows the IDirectInputDevice8::GetDeviceData documentation.
        numObjectDataElements = INFINITE;
        TEST_ASSERT(DI_OK == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), nullptr, &numObjectDataElements, DIGDD_PEEK));
        TEST_ASSERT(kExpectedNumEvents == numObjectDataElements);

        // Third access removes all the events without retrieving them.
        // This is also documented in the IDirectInputDevice8::GetDeviceData documentation.
        numObjectDataElements = INFINITE;
        TEST_ASSERT(DI_OK == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), nullptr, &numObjectDataElements, 0));
        TEST_ASSERT(kExpectedNumEvents == numObjectDataElements);

        // Finally, query again for the number of events left in the buffer. Result should be 0.
        numObjectDataElements = INFINITE;
        TEST_ASSERT(DI_OK == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), nullptr, &numObjectDataElements, DIGDD_PEEK));
        TEST_ASSERT(0 == numObjectDataElements);
    }

    // Same as above, but without peek. Exercises the one remaining type of access, namely retrieving and popping events at the same time.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceData_NominalPop)
    {
        constexpr DWORD kBufferSize = 16;
        constexpr DIPROPDWORD kBufferSizeProperty = {.diph = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE}, .dwData = kBufferSize};
        constexpr SPhysicalState kPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X), .sThumbLX = -1234, .sThumbRX = 5678}}};

        // Set based on the number of controller elements present in the above XINPUT_STATE structure that are also contained in STestDataPacket.
        // In this case, the right thumbstick has no matching offset, but all the other three controller components are represented.
        constexpr DWORD kExpectedNumEvents = 3;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_BUFFERSIZE, (LPCDIPROPHEADER)&kBufferSizeProperty));

        // This must occur after the buffer size property is set because the latter enables event buffering.
        diController.GetVirtualController().RefreshState(kPhysicalState);

        // Based on the mapper defined at the top of this file. POV does not need to be filled in because its state is not changing and so it will not generate an event.
        constexpr STestDataPacket kExpectedDataPacketResult = {.axisX = -1234, .button = {DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed, DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed}};

        // To get the actual data packet, retrieve buffered events from the controller and modify the data packet one event at a time.
        STestDataPacket actualDataPacketResult;
        ZeroMemory(&actualDataPacketResult, sizeof(actualDataPacketResult));

        DIDEVICEOBJECTDATA objectData[kBufferSize];
        DWORD numObjectDataElements = _countof(objectData);

        TEST_ASSERT(DI_OK == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), objectData, &numObjectDataElements, 0));
        TEST_ASSERT(kExpectedNumEvents == numObjectDataElements);
        ApplyEventsToTestDataPacket(actualDataPacketResult, objectData, numObjectDataElements);
        TEST_ASSERT(0 == memcmp(&actualDataPacketResult, &kExpectedDataPacketResult, sizeof(kExpectedDataPacketResult)));

        // Since events were retrieved and popped simultaneously, querying for the number of events left in the buffer should yield a result of 0.
        // This access is technically a flush operation, but it should work anyway.
        numObjectDataElements = INFINITE;
        TEST_ASSERT(DI_OK == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), nullptr, &numObjectDataElements, 0));
        TEST_ASSERT(0 == numObjectDataElements);
    }

    // Data format is not set. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceData_DataFormatNotSet)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DWORD numObjectDataElements = INFINITE;
        TEST_ASSERT(FAILED(diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), nullptr, &numObjectDataElements, 0)));
    }

    // Buffering is not enabled. This is expected to cause the method to fail with a specific error code.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceData_BufferingNotEnabled)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DWORD numObjectDataElements = INFINITE;
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DIERR_NOTBUFFERED == diController.GetDeviceData(sizeof(DIDEVICEOBJECTDATA), nullptr, &numObjectDataElements, 0));
    }


    // The following sequence of tests, which together comprise the GetDeviceState suite, exercise the DirectInputDevice interface method of the same name.
    // Scopes vary, so more details are provided with each test case.

    // Nominal situation in which all inputs are valid and a controller reports its state.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceState_Nominal)
    {
        constexpr SPhysicalState kPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X), .sThumbLX = -1234, .sThumbRX = 5678}}};

        // Based on the mapper defined at the top of this file. POV is filled in to reflect its centered state.
        constexpr STestDataPacket kExpectedDataPacketResult = {.axisX = -1234, .pov = EPovValue::Center, .button = {DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed, DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed}};

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        diController.GetVirtualController().RefreshState(kPhysicalState);
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));

        STestDataPacket actualDataPacketResult;
        FillMemory(&actualDataPacketResult, sizeof(actualDataPacketResult), 0xcd);
        TEST_ASSERT(DI_OK == diController.GetDeviceState(sizeof(actualDataPacketResult), &actualDataPacketResult));
        TEST_ASSERT(0 == memcmp(&actualDataPacketResult, &kExpectedDataPacketResult, sizeof(kExpectedDataPacketResult)));
    }

    // Data format is not set before requesting device state.
    // Method is expected to fail.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceState_DataFormatNotSet)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        STestDataPacket dataPacket;
        TEST_ASSERT(FAILED(diController.GetDeviceState(sizeof(dataPacket), &dataPacket)));
    }
    
    // Null pointer is passed, though the data packet size is correct.
    // Method is expected to fail.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceState_BadPointer)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(FAILED(diController.GetDeviceState(sizeof(STestDataPacket), nullptr)));
    }

    // Same as the nominal situation, except the supplied buffer is much larger than a data packet's actual size.
    // Method is expected to succeed.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceState_SizeTooBig)
    {
        constexpr SPhysicalState kPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {.dwPacketNumber = 1, .Gamepad = {.wButtons = (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_X), .sThumbLX = -1234, .sThumbRX = 5678}}};

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        diController.GetVirtualController().RefreshState(kPhysicalState);
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));

        // First element is based on the mapper defined at the top of this file. POV is filled in to reflect its centered state.
        // Second element is zeroed out as a comparison target with the actual data packet.
        constexpr STestDataPacket kExpectedDataPacketResult[2] = {
            {.axisX = -1234, .pov = EPovValue::Center, .button = {DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed, DataFormat::kButtonValuePressed, DataFormat::kButtonValueNotPressed}},
            {}
        };

        // This entire array is passed as the data packet buffer. It should be entirely zeroed out, except for those elements that are indicated in the expected result above.
        STestDataPacket actualDataPacketResult[2];
        FillMemory(actualDataPacketResult, sizeof(actualDataPacketResult), 0xcd);
        TEST_ASSERT(DI_OK == diController.GetDeviceState(sizeof(actualDataPacketResult), actualDataPacketResult));
        TEST_ASSERT(0 == memcmp(actualDataPacketResult, kExpectedDataPacketResult, sizeof(kExpectedDataPacketResult)));
    }

    // All inputs are valid except the size of the data packet passed during the method call is smaller than the size that was originally specified.
    // Method is expected to fail.
    TEST_CASE(VirtualDirectInputDevice_GetDeviceState_SizeTooSmall)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        STestDataPacket dataPacket;
        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(FAILED(diController.GetDeviceState(sizeof(dataPacket) - 1, &dataPacket)));
    }


    // The following sequence of tests, which together comprise the GetObjectInfo suite, exercise the DirectInputDevice interface method of the same name.
    // Scopes vary, so more details are provided with each test case.

    // Nominal behavior in which a structure is passed, properly initialized with the size member set.
    // Objects are enumerated with EnumObjects, and the output from GetObjectInfo is compared with the enumerated object for consistency.
    // Input to GetObjectInfo exercises both identification by offset and identification by instance and type.
    TEST_CASE(VirtualDirectInputDevice_GetObjectInfo_Nominal)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                VirtualDirectInputDevice<ECharMode::W>& diController = *((VirtualDirectInputDevice<ECharMode::W>*)pvRef);

                const DIDEVICEOBJECTINSTANCE& kExpectedObjectInstance = *lpddoi;
                DIDEVICEOBJECTINSTANCE actualObjectInstance;

                // First identify the enumerated object by offset.
                // Based on the test data packet at the top of this file, not all elements have offsets, so this part of the test case is not always valid.
                if (DataFormat::kInvalidOffsetValue != kExpectedObjectInstance.dwOfs)
                {
                    ZeroMemory(&actualObjectInstance, sizeof(actualObjectInstance));
                    actualObjectInstance.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
                    TEST_ASSERT(DI_OK == diController.GetObjectInfo(&actualObjectInstance, kExpectedObjectInstance.dwOfs, DIPH_BYOFFSET));
                    TEST_ASSERT(0 == memcmp(&actualObjectInstance, &kExpectedObjectInstance, sizeof(kExpectedObjectInstance)));
                }

                // Next try by instance type and ID.
                ZeroMemory(&actualObjectInstance, sizeof(actualObjectInstance));
                actualObjectInstance.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
                TEST_ASSERT(DI_OK == diController.GetObjectInfo(&actualObjectInstance, kExpectedObjectInstance.dwType, DIPH_BYID));
                TEST_ASSERT(0 == memcmp(&actualObjectInstance, &kExpectedObjectInstance, sizeof(kExpectedObjectInstance)));

                return DIENUM_CONTINUE;
            },
            &diController, DIDFT_ALL)
        );
    }

    // Same as above, except the structure is an older version which is supported for compatibility.
    // The older structure, with suffix _DX3, is a strict subset of the more modern version.
    TEST_CASE(VirtualDirectInputDevice_GetObjectInfo_Legacy)
    {
        constexpr uint8_t kPoisonByte = 0xcd;
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        TEST_ASSERT(DI_OK == diController.SetDataFormat(&kTestFormatSpec));
        TEST_ASSERT(DI_OK == diController.EnumObjects([](LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) -> BOOL
            {
                VirtualDirectInputDevice<ECharMode::W>& diController = *((VirtualDirectInputDevice<ECharMode::W>*)pvRef);

                DIDEVICEOBJECTINSTANCE expectedObjectInstance;
                FillMemory(&expectedObjectInstance, sizeof(expectedObjectInstance), kPoisonByte);
                CopyMemory(&expectedObjectInstance, lpddoi, sizeof(DIDEVICEOBJECTINSTANCE_DX3));
                expectedObjectInstance.dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);

                DIDEVICEOBJECTINSTANCE actualObjectInstance;

                // First identify the enumerated object by offset.
                // Based on the test data packet at the top of this file, not all elements have offsets, so this part of the test case is not always valid.
                if (DataFormat::kInvalidOffsetValue != expectedObjectInstance.dwOfs)
                {
                    FillMemory(&actualObjectInstance, sizeof(actualObjectInstance), kPoisonByte);
                    ZeroMemory(&actualObjectInstance, sizeof(DIDEVICEOBJECTINSTANCE_DX3));
                    actualObjectInstance.dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);
                    TEST_ASSERT(DI_OK == diController.GetObjectInfo(&actualObjectInstance, expectedObjectInstance.dwOfs, DIPH_BYOFFSET));
                    TEST_ASSERT(0 == memcmp(&actualObjectInstance, &expectedObjectInstance, sizeof(expectedObjectInstance)));
                }

                // Next try by instance type and ID.
                FillMemory(&actualObjectInstance, sizeof(actualObjectInstance), kPoisonByte);
                ZeroMemory(&actualObjectInstance, sizeof(DIDEVICEOBJECTINSTANCE_DX3));
                actualObjectInstance.dwSize = sizeof(DIDEVICEOBJECTINSTANCE_DX3);
                TEST_ASSERT(DI_OK == diController.GetObjectInfo(&actualObjectInstance, expectedObjectInstance.dwType, DIPH_BYID));
                TEST_ASSERT(0 == memcmp(&actualObjectInstance, &expectedObjectInstance, sizeof(expectedObjectInstance)));

                return DIENUM_CONTINUE;
            },
            &diController, DIDFT_ALL)
        );
    }

    // A null pointer is passed. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetObjectInfo_BadPointer)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(FAILED(diController.GetObjectInfo(nullptr, DIDFT_MAKEINSTANCE(0) | DIDFT_PSHBUTTON, DIPH_BYID)));
    }

    // A valid pointer is passed but with the size member not initialized. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetObjectInfo_InvalidSize)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DIDEVICEOBJECTINSTANCE objectInstance;
        ZeroMemory(&objectInstance, sizeof(objectInstance));
        TEST_ASSERT(FAILED(diController.GetObjectInfo(&objectInstance, DIDFT_MAKEINSTANCE(0) | DIDFT_PSHBUTTON, DIPH_BYID)));
    }

    // All inputs are valid, but no matching object exists based on the object specification.
    TEST_CASE(VirtualDirectInputDevice_GetObjectInfo_ObjectNotFound)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DIDEVICEOBJECTINSTANCE objectInstance = {.dwSize = sizeof(DIDEVICEOBJECTINSTANCE)};

        // One axis beyond the maximum.
        TEST_ASSERT(DIERR_OBJECTNOTFOUND == diController.GetObjectInfo(&objectInstance, DIDFT_MAKEINSTANCE(kTestMapper.GetCapabilities().numAxes) | DIDFT_ABSAXIS, DIPH_BYID));

        // One button beyond the maximum.
        TEST_ASSERT(DIERR_OBJECTNOTFOUND == diController.GetObjectInfo(&objectInstance, DIDFT_MAKEINSTANCE(kTestMapper.GetCapabilities().numButtons) | DIDFT_PSHBUTTON, DIPH_BYID));

        // Using an offset that definitely does exist in the data packet, but the data format has not been set.
        TEST_ASSERT(DIERR_OBJECTNOTFOUND == diController.GetObjectInfo(&objectInstance, 0, DIPH_BYOFFSET));

        // Specifying the whole device, which is not an allowed mechanism for identifying an object for this method, meaning the parameters are invalid.
        TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetObjectInfo(&objectInstance, 0, DIPH_DEVICE));
    }


    // The following sequence of tests, which together comprise the Properties suite, exercise the DirectInputDevice interface methods GetProperty and SetProperty.
    // Scopes vary, so more details are provided with each test case.

    // Nominal situation of setting some supported properties to valid values and reading them back.
    // For read-only properties the write is expected to fail.
    TEST_CASE(VirtualDirectInputDevice_Properties_Nominal)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        // Buffer size
        do {
            constexpr DIPROPHEADER kBufferSizeHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            constexpr DIPROPDWORD kExpectedBufferSize = {.diph = kBufferSizeHeader, .dwData = 543};
            DIPROPDWORD actualBufferSize = {.diph = kBufferSizeHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_BUFFERSIZE, (LPCDIPROPHEADER)&kExpectedBufferSize));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_BUFFERSIZE, (LPDIPROPHEADER)&actualBufferSize));
            TEST_ASSERT(0 == memcmp(&actualBufferSize, &kExpectedBufferSize, sizeof(kExpectedBufferSize)));
        } while (false);

        // Deadzone
        do {
            constexpr DIPROPHEADER kDeadzoneHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPDWORD kExpectedDeadzone = {.diph = kDeadzoneHeader, .dwData = 1234};
            DIPROPDWORD actualDeadzone = {.diph = kDeadzoneHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_DEADZONE, (LPCDIPROPHEADER)&kExpectedDeadzone));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_DEADZONE, (LPDIPROPHEADER)&actualDeadzone));
            TEST_ASSERT(0 == memcmp(&actualDeadzone, &kExpectedDeadzone, sizeof(kExpectedDeadzone)));
        } while (false);

        // Force feedback gain
        do {
            constexpr DIPROPHEADER kFfGainHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            constexpr DIPROPDWORD kExpectedFfGain = {.diph = kFfGainHeader, .dwData = 6677};
            DIPROPDWORD actualFfGain = {.diph = kFfGainHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_FFGAIN, (LPCDIPROPHEADER)&kExpectedFfGain));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_FFGAIN, (LPDIPROPHEADER)&actualFfGain));
            TEST_ASSERT(0 == memcmp(&actualFfGain, &kExpectedFfGain, sizeof(kExpectedFfGain)));
        } while (false);

        // Range
        do {
            constexpr DIPROPHEADER kRangeHeader = {.dwSize = sizeof(DIPROPRANGE), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPRANGE kExpectedRange = {.diph = kRangeHeader, .lMin = -45665, .lMax = 100222};
            DIPROPRANGE actualRange = {.diph = kRangeHeader, .lMin = -1, .lMax = -1};
            TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_RANGE, (LPCDIPROPHEADER)&kExpectedRange));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_RANGE, (LPDIPROPHEADER)&actualRange));
            TEST_ASSERT(0 == memcmp(&actualRange, &kExpectedRange, sizeof(kExpectedRange)));
        } while (false);

        // Saturation
        do {
            constexpr DIPROPHEADER kSaturationHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPDWORD kExpectedSaturation = {.diph = kSaturationHeader, .dwData = 9876};
            DIPROPDWORD actualSaturation = {.diph = kSaturationHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.SetProperty(DIPROP_SATURATION, (LPCDIPROPHEADER)&kExpectedSaturation));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_SATURATION, (LPDIPROPHEADER)&actualSaturation));
            TEST_ASSERT(0 == memcmp(&actualSaturation, &kExpectedSaturation, sizeof(kExpectedSaturation)));
        } while (false);

        // Joystick ID (read-only)
        do {
            constexpr DIPROPHEADER kJoystickIdHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            constexpr DIPROPDWORD kExpectedJoystickId = {.diph = kJoystickIdHeader, .dwData = kTestControllerIdentifier};
            DIPROPDWORD actualJoystickId = {.diph = kJoystickIdHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_JOYSTICKID, (LPCDIPROPHEADER)&kExpectedJoystickId)));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_JOYSTICKID, (LPDIPROPHEADER)&actualJoystickId));
            TEST_ASSERT(0 == memcmp(&actualJoystickId, &kExpectedJoystickId, sizeof(kExpectedJoystickId)));
        } while (false);


        // Logical Range (read-only)
        do {
            constexpr DIPROPHEADER kLogicalRangeHeader = {.dwSize = sizeof(DIPROPRANGE), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPRANGE kExpectedLogicalRange = {.diph = kLogicalRangeHeader, .lMin = Controller::kAnalogValueMin, .lMax = Controller::kAnalogValueMax};
            DIPROPRANGE actualLogicalRange = {.diph = kLogicalRangeHeader, .lMin = -1, .lMax = -1};
            TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_LOGICALRANGE, (LPCDIPROPHEADER)&kExpectedLogicalRange)));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_LOGICALRANGE, (LPDIPROPHEADER)&actualLogicalRange));
            TEST_ASSERT(0 == memcmp(&actualLogicalRange, &kExpectedLogicalRange, sizeof(kExpectedLogicalRange)));
        } while (false);

        // Physical Range (read-only)
        do {
            constexpr DIPROPHEADER kPhysicalRangeHeader = {.dwSize = sizeof(DIPROPRANGE), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPRANGE kExpectedPhysicalRange = {.diph = kPhysicalRangeHeader, .lMin = Controller::kAnalogValueMin, .lMax = Controller::kAnalogValueMax};
            DIPROPRANGE actualPhysicalRange = {.diph = kPhysicalRangeHeader, .lMin = -1, .lMax = -1};
            TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_PHYSICALRANGE, (LPCDIPROPHEADER)&kExpectedPhysicalRange)));
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_PHYSICALRANGE, (LPDIPROPHEADER)&actualPhysicalRange));
            TEST_ASSERT(0 == memcmp(&actualPhysicalRange, &kExpectedPhysicalRange, sizeof(kExpectedPhysicalRange)));
        } while (false);
    }

    // Verifies that axis mode is reported as absolute and is presented as read/write but in practice is read-only.
    TEST_CASE(VirtualDirectInputDevice_Properties_AxisMode)
    {
        constexpr DIPROPHEADER kAxisModeHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DIPROPDWORD propertyValue;

        // Set axis mode to absolute. This has no effect and should succeed, but the return code could be something other than DI_OK.
        propertyValue = {.diph = kAxisModeHeader, .dwData = DIPROPAXISMODE_ABS};
        TEST_ASSERT(SUCCEEDED(diController.SetProperty(DIPROP_AXISMODE, (LPCDIPROPHEADER)&propertyValue)));

        // Set axis mode to relative. This is not supported and should fail.
        propertyValue = {.diph = kAxisModeHeader, .dwData = DIPROPAXISMODE_REL};
        TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_AXISMODE, (LPCDIPROPHEADER)&propertyValue)));

        // Retrieve axis mode. Result should be absolute axis mode.
        propertyValue = {.diph = kAxisModeHeader, .dwData = (DWORD)-1};
        TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_AXISMODE, (LPDIPROPHEADER)&propertyValue));
        TEST_ASSERT(DIPROPAXISMODE_ABS == propertyValue.dwData);
    }

    // Verifies that default property values are correct. Does not perform any writes.
    TEST_CASE(VirtualDirectInputDevice_Properties_DefaultValues)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        // Buffer size
        do {
            constexpr DIPROPHEADER kBufferSizeHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            constexpr DIPROPDWORD kExpectedBufferSize = {.diph = kBufferSizeHeader, .dwData = 0};
            DIPROPDWORD actualBufferSize = {.diph = kBufferSizeHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_BUFFERSIZE, (LPDIPROPHEADER)&actualBufferSize));
            TEST_ASSERT(0 == memcmp(&actualBufferSize, &kExpectedBufferSize, sizeof(kExpectedBufferSize)));
        } while (false);

        // Deadzone
        do {
            constexpr DIPROPHEADER kDeadzoneHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPDWORD kExpectedDeadzone = {.diph = kDeadzoneHeader, .dwData = Controller::VirtualController::kAxisDeadzoneDefault};
            DIPROPDWORD actualDeadzone = {.diph = kDeadzoneHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_DEADZONE, (LPDIPROPHEADER)&actualDeadzone));
            TEST_ASSERT(0 == memcmp(&actualDeadzone, &kExpectedDeadzone, sizeof(kExpectedDeadzone)));
        } while (false);

        // Force feedback gain
        do {
            constexpr DIPROPHEADER kFfGainHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            constexpr DIPROPDWORD kExpectedFfGain = {.diph = kFfGainHeader, .dwData = Controller::VirtualController::kFfGainDefault};
            DIPROPDWORD actualFfGain = {.diph = kFfGainHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_FFGAIN, (LPDIPROPHEADER)&actualFfGain));
            TEST_ASSERT(0 == memcmp(&actualFfGain, &kExpectedFfGain, sizeof(kExpectedFfGain)));
        } while (false);

        // Range
        do {
            constexpr DIPROPHEADER kRangeHeader = {.dwSize = sizeof(DIPROPRANGE), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPRANGE kExpectedRange = {.diph = kRangeHeader, .lMin = Controller::kAnalogValueMin, .lMax = Controller::kAnalogValueMax};
            DIPROPRANGE actualRange = {.diph = kRangeHeader, .lMin = -1, .lMax = -1};
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_RANGE, (LPDIPROPHEADER)&actualRange));
            TEST_ASSERT(0 == memcmp(&actualRange, &kExpectedRange, sizeof(kExpectedRange)));
        } while (false);

        // Saturation
        do {
            constexpr DIPROPHEADER kSaturationHeader = {.dwSize = sizeof(DIPROPDWORD), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            constexpr DIPROPDWORD kExpectedSaturation = {.diph = kSaturationHeader, .dwData = Controller::VirtualController::kAxisSaturationDefault};
            DIPROPDWORD actualSaturation = {.diph = kSaturationHeader, .dwData = (DWORD)-1};
            TEST_ASSERT(DI_OK == diController.GetProperty(DIPROP_SATURATION, (LPDIPROPHEADER)&actualSaturation));
            TEST_ASSERT(0 == memcmp(&actualSaturation, &kExpectedSaturation, sizeof(kExpectedSaturation)));
        } while (false);
    }

    // Passes the wrong type of property structure to GetProperty and SetProperty for the given property.
    // Both methods are expected to fail in this situation.
    TEST_CASE(VirtualDirectInputDevice_Properties_WrongStruct)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        // Buffer size
        do {
            constexpr DIPROPHEADER kBufferSizeHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            DIPROPSTRING propBufferSize = {.diph = kBufferSizeHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.SetProperty(DIPROP_BUFFERSIZE, (LPCDIPROPHEADER)&propBufferSize));
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_BUFFERSIZE, (LPDIPROPHEADER)&propBufferSize));
        } while (false);

        // Deadzone
        do {
            constexpr DIPROPHEADER kDeadzoneHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            DIPROPSTRING propDeadzone = {.diph = kDeadzoneHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.SetProperty(DIPROP_DEADZONE, (LPCDIPROPHEADER)&propDeadzone));
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_DEADZONE, (LPDIPROPHEADER)&propDeadzone));
        } while (false);

        // Force feedback gain
        do {
            constexpr DIPROPHEADER kFfGainHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            DIPROPSTRING propFfGain = {.diph = kFfGainHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.SetProperty(DIPROP_FFGAIN, (LPCDIPROPHEADER)&propFfGain));
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_FFGAIN, (LPDIPROPHEADER)&propFfGain));
        } while (false);

        // Range
        do {
            constexpr DIPROPHEADER kRangeHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            DIPROPSTRING propRange = {.diph = kRangeHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.SetProperty(DIPROP_RANGE, (LPCDIPROPHEADER)&propRange));
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_RANGE, (LPDIPROPHEADER)&propRange));
        } while (false);

        // Saturation
        do {
            constexpr DIPROPHEADER kSaturationHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            DIPROPSTRING propSaturation = {.diph = kSaturationHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.SetProperty(DIPROP_SATURATION, (LPCDIPROPHEADER)&propSaturation));
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_SATURATION, (LPDIPROPHEADER)&propSaturation));
        } while (false);

        // Joystick ID (read-only)
        do {
            constexpr DIPROPHEADER kJoystickIdHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE};
            DIPROPSTRING propJoystickId = {.diph = kJoystickIdHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_JOYSTICKID, (LPDIPROPHEADER)&propJoystickId));
        } while (false);


        // Logical Range (read-only)
        do {
            constexpr DIPROPHEADER kLogicalRangeHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            DIPROPSTRING propLogicalRange = {.diph = kLogicalRangeHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_LOGICALRANGE, (LPDIPROPHEADER)&propLogicalRange));
        } while (false);

        // Physical Range (read-only)
        do {
            constexpr DIPROPHEADER kPhysicalRangeHeader = {.dwSize = sizeof(DIPROPSTRING), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = DIDFT_MAKEINSTANCE(0) | DIDFT_ABSAXIS, .dwHow = DIPH_BYID};
            DIPROPSTRING propPhysicalRange = {.diph = kPhysicalRangeHeader};
            TEST_ASSERT(DIERR_INVALIDPARAM == diController.GetProperty(DIPROP_PHYSICALRANGE, (LPDIPROPHEADER)&propPhysicalRange));
        } while (false);
    }

    // Passes nullptr to GetProperty and SetProperty.
    // Both methods are expected to fail in this situation.
    TEST_CASE(VirtualDirectInputDevice_Properties_BadPointer)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        // Buffer size
        TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_BUFFERSIZE, nullptr)));
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_BUFFERSIZE, nullptr)));

        // Deadzone
        TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_DEADZONE, nullptr)));
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_DEADZONE, nullptr)));

        // Force feedback gain
        TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_FFGAIN, nullptr)));
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_FFGAIN, nullptr)));

        // Range
        TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_RANGE, nullptr)));
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_RANGE, nullptr)));

        // Saturation
        TEST_ASSERT(FAILED(diController.SetProperty(DIPROP_SATURATION, nullptr)));
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_SATURATION, nullptr)));

        // Joystick ID (read-only)
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_JOYSTICKID, nullptr)));

        // Logical Range (read-only)
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_LOGICALRANGE, nullptr)));

        // Physical Range (read-only)
        TEST_ASSERT(FAILED(diController.GetProperty(DIPROP_PHYSICALRANGE, nullptr)));
    }


    // The following sequence of tests, which together comprise the EffectInfo suite, exercise the DirectInputDevice interface methods that involve force feedback effects.
    // Scopes are highly varied, so more details are provided with each test case.

    // Verifies that the GUIDs known to be supported are actually supported and objects with those GUIDs can be created.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_CreateWithSupportedGuids)
    {
        const GUID kExpectedSupportedGuids[] = {GUID_ConstantForce};

        for (const auto& kExpectedSupportedGuid : kExpectedSupportedGuids)
        {
            TEST_ASSERT(true == VirtualDirectInputDevice<ECharMode::A>::ForceFeedbackEffectCanCreateObject(kExpectedSupportedGuid));
            TEST_ASSERT(true == VirtualDirectInputDevice<ECharMode::W>::ForceFeedbackEffectCanCreateObject(kExpectedSupportedGuid));
        }

        for (const auto& kExpectedSupportedGuid : kExpectedSupportedGuids)
        {
            VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
            IDirectInputEffect* createdEffect = nullptr;
            
            TEST_ASSERT(DI_OK == diController.CreateEffect(kExpectedSupportedGuid, nullptr, &createdEffect, nullptr));
            TEST_ASSERT(nullptr != createdEffect);

            GUID createdEffectGuid = {};
            TEST_ASSERT(DI_OK == createdEffect->GetEffectGuid(&createdEffectGuid));
            TEST_ASSERT(kExpectedSupportedGuid == createdEffectGuid);
        }
    }

    // Enumerates all effects and verifies correct common information is provided, like type flags and parameter support.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumAll)
    {
        const std::set<GUID> kExpectedSeenGuids = {GUID_ConstantForce, GUID_RampForce, GUID_Square, GUID_Sine, GUID_Triangle, GUID_SawtoothUp, GUID_SawtoothDown, GUID_CustomForce};
        std::set<GUID> actualSeenGuids;

        for (const auto& kExpectedSeenGuid : kExpectedSeenGuids)
        {
            if (false == VirtualDirectInputDevice<ECharMode::W>::ForceFeedbackEffectCanCreateObject(kExpectedSeenGuid))
                actualSeenGuids.insert(kExpectedSeenGuid);
        }

        constexpr DWORD kExpectedEffectTypeFlags = (DIEFT_FFATTACK | DIEFT_FFFADE);
        constexpr DWORD kExpectedEffectParams = (DIEP_AXES | DIEP_DIRECTION | DIEP_DURATION | DIEP_ENVELOPE | DIEP_GAIN | DIEP_SAMPLEPERIOD | DIEP_STARTDELAY | DIEP_TYPESPECIFICPARAMS);

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                std::set<GUID>& seenGuids = *((std::set<GUID>*)pvRef);

                TEST_ASSERT((nullptr != pdei) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*pdei) == pdei->dwSize);

                TEST_ASSERT(kExpectedEffectTypeFlags == (pdei->dwEffType & kExpectedEffectTypeFlags));
                TEST_ASSERT(kExpectedEffectParams == (pdei->dwStaticParams & kExpectedEffectParams));
                TEST_ASSERT(kExpectedEffectParams == (pdei->dwDynamicParams & kExpectedEffectParams));

                TEST_ASSERT(false == seenGuids.contains(pdei->guid));
                seenGuids.insert(pdei->guid);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenGuids, DIEFT_ALL)
        );

        TEST_ASSERT(actualSeenGuids == kExpectedSeenGuids);
    }

    // Enumerates all effects and verifies information is identical to that provided by the GetEffectInfo method.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_GetInfoAll)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                VirtualDirectInputDevice<ECharMode::W>& diController = *((VirtualDirectInputDevice<ECharMode::W>*)pvRef);
                TEST_ASSERT((nullptr != pdei) && (nullptr != pvRef));

                DIEFFECTINFO effectInfo = {.dwSize = sizeof(DIEFFECTINFO)};
                TEST_ASSERT(DI_OK == diController.GetEffectInfo(&effectInfo, pdei->guid));
                TEST_ASSERT(0 == memcmp(&effectInfo, pdei, sizeof(effectInfo)));

                return DIENUM_CONTINUE;
            },
            (LPVOID)&diController, DIEFT_ALL)
        );
    }

    // Enumerates constant force effects only and verifies correct information is provided.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumConstantForce)
    {
        const std::set<GUID> kExpectedSeenGuids = {GUID_ConstantForce};
        std::set<GUID> actualSeenGuids;

        for (const auto& kExpectedSeenGuid : kExpectedSeenGuids)
        {
            if (false == VirtualDirectInputDevice<ECharMode::W>::ForceFeedbackEffectCanCreateObject(kExpectedSeenGuid))
                actualSeenGuids.insert(kExpectedSeenGuid);
        }

        constexpr DWORD kExpectedEffectType = DIEFT_CONSTANTFORCE;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                std::set<GUID>& seenGuids = *((std::set<GUID>*)pvRef);

                TEST_ASSERT((nullptr != pdei) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*pdei) == pdei->dwSize);

                TEST_ASSERT(kExpectedEffectType == DIEFT_GETTYPE(pdei->dwEffType));

                TEST_ASSERT(false == seenGuids.contains(pdei->guid));
                seenGuids.insert(pdei->guid);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenGuids, kExpectedEffectType)
        );

        TEST_ASSERT(actualSeenGuids == kExpectedSeenGuids);
    }

    // Enumerates ramp force effects only and verifies correct information is provided.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumRampForce)
    {
        const std::set<GUID> kExpectedSeenGuids = {GUID_RampForce};
        std::set<GUID> actualSeenGuids;

        for (const auto& kExpectedSeenGuid : kExpectedSeenGuids)
        {
            if (false == VirtualDirectInputDevice<ECharMode::W>::ForceFeedbackEffectCanCreateObject(kExpectedSeenGuid))
                actualSeenGuids.insert(kExpectedSeenGuid);
        }

        constexpr DWORD kExpectedEffectType = DIEFT_RAMPFORCE;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                std::set<GUID>& seenGuids = *((std::set<GUID>*)pvRef);

                TEST_ASSERT((nullptr != pdei) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*pdei) == pdei->dwSize);

                TEST_ASSERT(kExpectedEffectType == DIEFT_GETTYPE(pdei->dwEffType));

                TEST_ASSERT(false == seenGuids.contains(pdei->guid));
                seenGuids.insert(pdei->guid);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenGuids, kExpectedEffectType)
        );

        TEST_ASSERT(actualSeenGuids == kExpectedSeenGuids);
    }

    // Enumerates periodic force effects only and verifies correct information is provided.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumPeriodic)
    {
        const std::set<GUID> kExpectedSeenGuids = {GUID_Square, GUID_Sine, GUID_Triangle, GUID_SawtoothUp, GUID_SawtoothDown};
        std::set<GUID> actualSeenGuids;

        for (const auto& kExpectedSeenGuid : kExpectedSeenGuids)
        {
            if (false == VirtualDirectInputDevice<ECharMode::W>::ForceFeedbackEffectCanCreateObject(kExpectedSeenGuid))
                actualSeenGuids.insert(kExpectedSeenGuid);
        }

        constexpr DWORD kExpectedEffectType = DIEFT_PERIODIC;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                std::set<GUID>& seenGuids = *((std::set<GUID>*)pvRef);

                TEST_ASSERT((nullptr != pdei) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*pdei) == pdei->dwSize);

                TEST_ASSERT(kExpectedEffectType == DIEFT_GETTYPE(pdei->dwEffType));

                TEST_ASSERT(false == seenGuids.contains(pdei->guid));
                seenGuids.insert(pdei->guid);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenGuids, kExpectedEffectType)
        );

        TEST_ASSERT(actualSeenGuids == kExpectedSeenGuids);
    }

    // Enumerates custom force effects only and verifies correct information is provided.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumCustomForce)
    {
        const std::set<GUID> kExpectedSeenGuids = {GUID_CustomForce};
        std::set<GUID> actualSeenGuids;

        for (const auto& kExpectedSeenGuid : kExpectedSeenGuids)
        {
            if (false == VirtualDirectInputDevice<ECharMode::W>::ForceFeedbackEffectCanCreateObject(kExpectedSeenGuid))
                actualSeenGuids.insert(kExpectedSeenGuid);
        }

        constexpr DWORD kExpectedEffectType = DIEFT_CUSTOMFORCE;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                std::set<GUID>& seenGuids = *((std::set<GUID>*)pvRef);

                TEST_ASSERT((nullptr != pdei) && (nullptr != pvRef));
                TEST_ASSERT(sizeof(*pdei) == pdei->dwSize);

                TEST_ASSERT(kExpectedEffectType == DIEFT_GETTYPE(pdei->dwEffType));

                TEST_ASSERT(false == seenGuids.contains(pdei->guid));
                seenGuids.insert(pdei->guid);

                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenGuids, kExpectedEffectType)
        );

        TEST_ASSERT(actualSeenGuids == kExpectedSeenGuids);
    }

    // Attempts to enumerate unsupported types of effects, which should result in no calls to the enumeration callback.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumNone)
    {
        constexpr DWORD kExpectedEffectType = DIEFT_STARTDELAY;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK == diController.EnumEffects([](LPCDIEFFECTINFO pdei, LPVOID pvRef) -> BOOL
            {
                TEST_FAILED_BECAUSE(L"Unexpected invocation of enumeration function.");
                return DIENUM_CONTINUE;
            },
            nullptr, kExpectedEffectType)
        );
    }

    // Creates several effects and attempts to enumerate them all.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumCreated)
    {
        constexpr int kNumTestEffects = 10;
        std::set<IDirectInputEffect*> expectedSeenEffects;
        std::set<IDirectInputEffect*> actualSeenEffects;

        const GUID kEffectGuid = GUID_ConstantForce;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        for (int i = 0; i < kNumTestEffects; ++i)
        {
            IDirectInputEffect* testEffect = nullptr;
            TEST_ASSERT(DI_OK == diController.CreateEffect(kEffectGuid, nullptr, &testEffect, nullptr));
            TEST_ASSERT(nullptr != testEffect);

            expectedSeenEffects.insert(testEffect);
        }

        TEST_ASSERT(DI_OK == diController.EnumCreatedEffectObjects([](LPDIRECTINPUTEFFECT peff, LPVOID pvRef) -> BOOL
            {
                std::set<IDirectInputEffect*>& seenEffects = *((std::set<IDirectInputEffect*>*)pvRef);
                seenEffects.insert(peff);
                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenEffects, 0));

        TEST_ASSERT(actualSeenEffects == expectedSeenEffects);
    }

    // Creates several effects and attempts to enumerate them all, but stops enumeration after the first effect.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumCreatedAndStop)
    {
        constexpr int kNumTestEffects = 10;
        std::set<IDirectInputEffect*> expectedSeenEffects;
        std::set<IDirectInputEffect*> actualSeenEffects;

        const GUID kEffectGuid = GUID_ConstantForce;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        for (int i = 0; i < kNumTestEffects; ++i)
        {
            IDirectInputEffect* testEffect = nullptr;
            TEST_ASSERT(DI_OK == diController.CreateEffect(kEffectGuid, nullptr, &testEffect, nullptr));
            TEST_ASSERT(nullptr != testEffect);

            expectedSeenEffects.insert(testEffect);
        }

        TEST_ASSERT(DI_OK == diController.EnumCreatedEffectObjects([](LPDIRECTINPUTEFFECT peff, LPVOID pvRef) -> BOOL
            {
                std::set<IDirectInputEffect*>& seenEffects = *((std::set<IDirectInputEffect*>*)pvRef);

                if (false == seenEffects.empty())
                    TEST_FAILED_BECAUSE(L"Unexpected invocation of enumeration function.");

                seenEffects.insert(peff);
                return DIENUM_STOP;
            },
            (LPVOID)&actualSeenEffects, 0));

        for (const auto& actualSeenEffect : actualSeenEffects)
            TEST_ASSERT(true == expectedSeenEffects.contains(actualSeenEffect));
    }

    // Creates several effects, destroys some of them, and attempts to enumerate the remainder.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_DestroyThenEnumCreated)
    {
        constexpr int kNumTestEffects = 10;
        std::set<IDirectInputEffect*> expectedSeenEffects;
        std::set<IDirectInputEffect*> actualSeenEffects;

        const GUID kEffectGuid = GUID_ConstantForce;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        for (int i = 0; i < kNumTestEffects; ++i)
        {
            IDirectInputEffect* testEffect = nullptr;
            TEST_ASSERT(DI_OK == diController.CreateEffect(kEffectGuid, nullptr, &testEffect, nullptr));
            TEST_ASSERT(nullptr != testEffect);

            if (0 == (i % 2))
                expectedSeenEffects.insert(testEffect);
            else
                testEffect->Release();
        }

        TEST_ASSERT(DI_OK == diController.EnumCreatedEffectObjects([](LPDIRECTINPUTEFFECT peff, LPVOID pvRef) -> BOOL
            {
                std::set<IDirectInputEffect*>& seenEffects = *((std::set<IDirectInputEffect*>*)pvRef);
                seenEffects.insert(peff);
                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenEffects, 0));

        TEST_ASSERT(actualSeenEffects == expectedSeenEffects);
    }

    // Creates several effects, attempts to enumerate them all, and destroys each during the enumeration callback.
    // DirectInput documentation explicitly states that this behavior is permitted.
    TEST_CASE(VirtualDirectInputDevice_ForceFeedbackEffect_EnumCreatedAndDestroy)
    {
        constexpr int kNumTestEffects = 10;
        std::set<IDirectInputEffect*> expectedSeenEffects;
        std::set<IDirectInputEffect*> actualSeenEffects;

        const GUID kEffectGuid = GUID_ConstantForce;

        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        for (int i = 0; i < kNumTestEffects; ++i)
        {
            IDirectInputEffect* testEffect = nullptr;
            TEST_ASSERT(DI_OK == diController.CreateEffect(kEffectGuid, nullptr, &testEffect, nullptr));
            TEST_ASSERT(nullptr != testEffect);

            expectedSeenEffects.insert(testEffect);
        }

        TEST_ASSERT(DI_OK == diController.EnumCreatedEffectObjects([](LPDIRECTINPUTEFFECT peff, LPVOID pvRef) -> BOOL
            {
                std::set<IDirectInputEffect*>& seenEffects = *((std::set<IDirectInputEffect*>*)pvRef);
                seenEffects.insert(peff);

                peff->Release();

                return DIENUM_CONTINUE;
            },
            (LPVOID)&actualSeenEffects, 0));

        TEST_ASSERT(actualSeenEffects == expectedSeenEffects);

        TEST_ASSERT(DI_OK == diController.EnumCreatedEffectObjects([](LPDIRECTINPUTEFFECT peff, LPVOID pvRef) -> BOOL
            {
                TEST_FAILED_BECAUSE(L"Unexpected invocation of enumeration function.");
                return DIENUM_CONTINUE;
            },
            nullptr, 0));
    }
}
