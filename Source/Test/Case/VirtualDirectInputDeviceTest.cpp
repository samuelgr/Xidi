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
#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Mapper.h"
#include "MockXInput.h"
#include "TestCase.h"
#include "VirtualDirectInputDevice.h"

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
    static_assert(0 == (sizeof(STestDataPacket) % 4), L"Test data packet size must be divisible by 4.");


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Test value of controller identifier used throughout these test cases.
    static constexpr VirtualController::TControllerIdentifier kTestControllerIdentifier = 1;

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
    static inline std::unique_ptr<VirtualController> CreateTestVirtualController(std::unique_ptr<MockXInput> xinput = std::make_unique<MockXInput>(kTestControllerIdentifier))
    {
        return std::make_unique<VirtualController>(kTestControllerIdentifier, kTestMapper, std::move(xinput));
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
                TEST_ASSERT((DIDOI_ASPECTPOSITION | DIDOI_POLLED) == lpddoi->dwFlags);

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
                TEST_ASSERT(DIDOI_POLLED == lpddoi->dwFlags);

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
                TEST_ASSERT(DIDOI_POLLED == lpddoi->dwFlags);

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


    // The following sequence of tests, which together comprise the GetCapabilities suite, exercise the DirectInputDevice interface method of the same name..
    // Scopes vary, so more details are provided with each test case.

    // Nominal behavior in which a structure is passed, properly initialized with the size member set.
    // Expected outcome is the structure is filled with corrrect controller capabilities.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_Nominal)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        const DIDEVCAPS kExpectedCapabilities =
        {
            .dwSize = sizeof(DIDEVCAPS),
            .dwFlags = (DIDC_ATTACHED | DIDC_EMULATED | DIDC_POLLEDDEVICE | DIDC_POLLEDDATAFORMAT),
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
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_Legacy)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());

        const DIDEVCAPS_DX3 kExpectedCapabilities =
        {
            .dwSize = sizeof(DIDEVCAPS_DX3),
            .dwFlags = (DIDC_ATTACHED | DIDC_EMULATED | DIDC_POLLEDDEVICE | DIDC_POLLEDDATAFORMAT),
            .dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD,
            .dwAxes = kTestMapper.GetCapabilities().numAxes,
            .dwButtons = kTestMapper.GetCapabilities().numButtons,
            .dwPOVs = (DWORD)((true == kTestMapper.GetCapabilities().hasPov) ? 1 : 0)
        };

        DIDEVCAPS_DX3 actualCapabilities;
        FillMemory(&actualCapabilities, sizeof(actualCapabilities), 0xcd);
        actualCapabilities.dwSize = sizeof(DIDEVCAPS_DX3);

        TEST_ASSERT(DI_OK == diController.GetCapabilities((DIDEVCAPS*)&actualCapabilities));
        TEST_ASSERT(0 == memcmp(&actualCapabilities, &kExpectedCapabilities, sizeof(kExpectedCapabilities)));
    }
    
    // A null pointer is passed. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_BadPointer)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        TEST_ASSERT(DI_OK != diController.GetCapabilities(nullptr));
    }

    // A valid pointer is passed but with the size member not initialized. This is expected to cause the method to fail.
    TEST_CASE(VirtualDirectInputDevice_GetCapabilities_InvalidSize)
    {
        VirtualDirectInputDevice<ECharMode::W> diController(CreateTestVirtualController());
        DIDEVCAPS capabilities;
        ZeroMemory(&capabilities, sizeof(capabilities));
        TEST_ASSERT(DI_OK != diController.GetCapabilities(&capabilities));
    }
}
