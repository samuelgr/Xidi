/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file WrapperIDirectInputTest.cpp
 *   Unit tests for the top-level DirectInput interface object, with
 *   particular emphasis on how it interacts with system-supplied DirectInput
 *   interface objects.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "MockDirectInput.h"
#include "MockDirectInputDevice.h"
#include "WrapperIDirectInput.h"
#include "TestCase.h"


namespace XidiTest
{
    using namespace ::Xidi;
    

    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Device information taken from a real Xbox One controller attached via an Xbox Wireless Adapter.
    /// This type of controller supports XInput.
    static const SDirectInputDeviceInfo kXboxOneWirelessXInputController = {
        .instance = {
            .dwSize = sizeof(DIDEVICEINSTANCE),
            .guidInstance = {0xfce41180, 0x2924, 0x11ed, {0x80, 0x01, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},
            .guidProduct = {0x0b12045e, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}},
            .dwDevType = 0x00010215,
            .tszInstanceName = L"Controller (Xbox One For Windows)",
            .tszProductName	= L"Controller (Xbox One For Windows)",
            .guidFFDriver = {},
            .wUsagePage = 1,
            .wUsage = 5
        },
        .capabilities = {
            .dwSize = sizeof(DIDEVCAPS),
            .dwFlags = 0x00000005,
            .dwDevType = 0x00010215,
            .dwAxes = 5,
            .dwButtons = 16,
            .dwPOVs = 1,
            .dwFFSamplePeriod = 0,
            .dwFFMinTimeResolution = 0,
            .dwFirmwareRevision = 0,
            .dwHardwareRevision = 0,
            .dwFFDriverVersion = 0
        },
        .properties = {
            {
                &DIPROP_GUIDANDPATH, {
                    .guidandpath = {
                        .diph = {.dwSize = sizeof(DIPROPGUIDANDPATH), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE},
                        .guidClass = {0x745a17a0, 0x74d3, 0x11d0, {0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda}},
                        .wszPath = L"\\\\?\\hid#vid_045e&pid_0b12&ig_00#9&2e649ca1&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
                    }
                }
            }
        }
    };

    /// Device information taken from a real Xbox One controller attached via Bluetooth.
    /// This type of controller supports XInput.
    static const SDirectInputDeviceInfo kXboxOneBluetoothXInputController = {
        .instance = {
            .dwSize = sizeof(DIDEVICEINSTANCE),
            .guidInstance = {0x8bf6c1d0, 0x4700, 0x11ed, {0x80, 0x02, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},
            .guidProduct = {0x0b13045e, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}},
            .dwDevType = 0x00010215,
            .tszInstanceName = L"Bluetooth LE XINPUT compatible input device",
            .tszProductName = L"Bluetooth LE XINPUT compatible input device",
            .guidFFDriver = {},
            .wUsagePage = 1,
            .wUsage = 5
        },
        .capabilities = {
            .dwSize = sizeof(DIDEVCAPS),
            .dwFlags = 0x00000005,
            .dwDevType = 0x00010215,
            .dwAxes = 5,
            .dwButtons = 16,
            .dwPOVs = 1,
            .dwFFSamplePeriod = 0,
            .dwFFMinTimeResolution = 0,
            .dwFirmwareRevision = 0,
            .dwHardwareRevision = 0,
            .dwFFDriverVersion = 0
        },
        .properties = {
            {
                &DIPROP_GUIDANDPATH, {
                    .guidandpath = {
                        .diph = {.dwSize = sizeof(DIPROPGUIDANDPATH), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE},
                        .guidClass = {0x745a17a0, 0x74d3, 0x11d0, {0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda}},
                        .wszPath = L"\\\\?\\hid#{00001812-0000-1000-8000-00805f9b34fb}&dev&vid_045e&pid_0b13&rev_0513&5cba3788986a&ig_00#c&2eaed628&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
                    }
                }
            }
        }
    };

    /// Device information taken from a real Logitech Rumblepad controller attached via USB with the force feedback driver installed.
    /// This type of controller does not support XInput.
    static const SDirectInputDeviceInfo kLogitechRumblepadNonXInputController = {
        .instance = {
            .dwSize = sizeof(DIDEVICEINSTANCE),
            .guidInstance = {0xa45ccd20, 0x7f71, 0x11ec, {0x80, 0x01, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},
            .guidProduct = {0xc218046d, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}},
            .dwDevType = 0x00010214,
            .tszInstanceName = L"Logitech RumblePad 2 USB",
            .tszProductName = L"Logitech RumblePad 2 USB",
            .guidFFDriver = {0x8d533a48, 0x7a5f, 0x11d3, {0x82, 0x97, 0x00, 0x50, 0xda, 0x1a, 0x72, 0xd3}},
            .wUsagePage = 1,
            .wUsage = 4
        },
        .capabilities = {
            .dwSize = sizeof(DIDEVCAPS),
            .dwFlags = 0x0000df05,
            .dwDevType = 0x00010214,
            .dwAxes = 4,
            .dwButtons = 12,
            .dwPOVs = 1,
            .dwFFSamplePeriod = 1000,
            .dwFFMinTimeResolution = 1000,
            .dwFirmwareRevision = 1,
            .dwHardwareRevision = 1,
            .dwFFDriverVersion = 1289
        },
        .properties = {
            {
                &DIPROP_GUIDANDPATH, {
                    .guidandpath = {
                        .diph = {.dwSize = sizeof(DIPROPGUIDANDPATH), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE},
                        .guidClass = {0x745a17a0, 0x74d3, 0x11d0, {0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda}},
                        .wszPath = L"\\\\?\\hid#vid_046d&pid_c218#9&f82fd59&2&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
                    }
                }
            }
        }
    };

    /// Device information taken from a real Logitech Rumblepad controller attached via USB but without any drivers installed for force feedback.
    /// This type of controller does not support XInput.
    static const SDirectInputDeviceInfo kGenericNoForceFeedbackNonXInputController = {
        .instance = {
            .dwSize = sizeof(DIDEVICEINSTANCE),
            .guidInstance = {0x4e4af2c0, 0x68d9, 0x11ed, {0x80, 0x01, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},
            .guidProduct = {0xc218046d, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}},
            .dwDevType = 0x00010114,
            .tszInstanceName = L"Logitech RumblePad 2 USB",
            .tszProductName = L"Logitech RumblePad 2 USB",
            .guidFFDriver = {0x8d533a48, 0x7a5f, 0x11d3, {0x82, 0x97, 0x00, 0x50, 0xda, 0x1a, 0x72, 0xd3}},
            .wUsagePage = 1,
            .wUsage = 4
        },
        .capabilities = {
            .dwSize = sizeof(DIDEVCAPS),
            .dwFlags = 0x00000005,
            .dwDevType = 0x00010114,
            .dwAxes = 4,
            .dwButtons = 12,
            .dwPOVs = 1,
            .dwFFSamplePeriod = 0,
            .dwFFMinTimeResolution = 0,
            .dwFirmwareRevision = 0,
            .dwHardwareRevision = 0,
            .dwFFDriverVersion = 0
        },
        .properties = {
            {
                &DIPROP_GUIDANDPATH, {
                    .guidandpath = {
                        .diph = {.dwSize = sizeof(DIPROPGUIDANDPATH), .dwHeaderSize = sizeof(DIPROPHEADER), .dwObj = 0, .dwHow = DIPH_DEVICE},
                        .guidClass = {0x745a17a0, 0x74d3, 0x11d0, {0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda}},
                        .wszPath = L"\\\\?\\hid#vid_046d&pid_c218#6&1f24a0f&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
                    }
                }
            }
        }
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Checks if a DirectInput device instance structure represents a Xidi virtual controller by comparing product and instance GUIDs.
    /// @param [in] deviceInstance Device instance to check.
    /// @return `true` if the instance is a Xidi virtual controller, `false` otherwise.
    static bool IsXidiVirtualControllerInstance(const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType& deviceInstance)
    {
        if (kVirtualControllerProductGuid != deviceInstance.guidProduct)
            return false;

        static constexpr GUID kReferenceInstanceGuid = kVirtualControllerInstanceBaseGuid;
        if (0 != memcmp(&deviceInstance.guidInstance, &kReferenceInstanceGuid, offsetof(GUID, Data4[4])))
            return false;

        return true;
    }

    /// Creates a test DirectInput interface object that wraps a mock DirectInput interface object.
    /// @param [in] mockDirectInput Mock DirectInput interface object to use as the underlying DirectInput interface object.
    /// @return Newly-constructed wrapper object.
    static inline WrapperIDirectInput<kDirectInputTestCharMode> MakeTestWrapperIDirectInput(MockDirectInput& mockDirectInput)
    {
        return WrapperIDirectInput<kDirectInputTestCharMode>((DirectInputType<kDirectInputTestCharMode>::LatestIDirectInputType*)&mockDirectInput);
    }



    // -------- TEST CASES ------------------------------------------------- //

    // The following sequence of tests, which together comprise the EnumDevices suite, verify correct device enumeration behavior and order.
    // Scopes are highly varied, so more details are provided with each test case.

    // No devices attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumDevices_NoSystemDevices)
    {
        MockDirectInput mockDirectInput;
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        int numVirtualDevicesEnumerated = 0;

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, [](const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType* lpddi, LPVOID pvRef) -> BOOL
            {
                int& numVirtualDevicesEnumerated = *((int*)pvRef);

                TEST_ASSERT(IsXidiVirtualControllerInstance(*lpddi));
                numVirtualDevicesEnumerated += 1;

                return DIENUM_CONTINUE;
            },
            &numVirtualDevicesEnumerated, DIEDFL_ATTACHEDONLY)
        );

        TEST_ASSERT(Controller::kPhysicalControllerCount == numVirtualDevicesEnumerated);
    }

    // Some XInput devices are attached to the system, and no non-XInput devices are attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumDevices_XInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kXboxOneWirelessXInputController, kXboxOneBluetoothXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        int numVirtualDevicesEnumerated = 0;

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, [](const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType* lpddi, LPVOID pvRef) -> BOOL
            {
                int& numVirtualDevicesEnumerated = *((int*)pvRef);

                TEST_ASSERT(IsXidiVirtualControllerInstance(*lpddi));
                numVirtualDevicesEnumerated += 1;

                return DIENUM_CONTINUE;
            },
            &numVirtualDevicesEnumerated, DIEDFL_ATTACHEDONLY)
        );

        TEST_ASSERT(Controller::kPhysicalControllerCount == numVirtualDevicesEnumerated);
    }

    // Some non-XInput devices are attached to the system, and no XInput devices are attached to the system.
    // The non-XInput devices should be presented first followed by all Xidi virtual controllers.
    TEST_CASE(WrapperIDirectInput_EnumDevices_NonXInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kLogitechRumblepadNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        int deviceCounter = mockDirectInput.GetSystemDeviceCount();
        const int kExpectedFinalDeviceCounter = -1 * (int)Controller::kPhysicalControllerCount;

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, [](const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType* lpddi, LPVOID pvRef) -> BOOL
            {
                int& deviceCounter = *((int*)pvRef);

                if (0 < deviceCounter)
                    TEST_ASSERT(false == IsXidiVirtualControllerInstance(*lpddi));
                else
                    TEST_ASSERT(true == IsXidiVirtualControllerInstance(*lpddi));

                deviceCounter -= 1;

                return DIENUM_CONTINUE;
            },
            &deviceCounter, DIEDFL_ATTACHEDONLY)
        );

        TEST_ASSERT(kExpectedFinalDeviceCounter == deviceCounter);
    }

    // Some XInput and non-XInput devices are attached to the system.
    // The Xidi virtual controllers should be presented first followed by all non-XInput system devices.
    TEST_CASE(WrapperIDirectInput_EnumDevices_MixedSystemDevices)
    {
        MockDirectInput mockDirectInput({kXboxOneBluetoothXInputController, kLogitechRumblepadNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        int deviceCounter = Controller::kPhysicalControllerCount;
        const int kExpectedFinalDeviceCounter = -1;

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, [](const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType* lpddi, LPVOID pvRef) -> BOOL
            {
                int& deviceCounter = *((int*)pvRef);

                if (0 < deviceCounter)
                    TEST_ASSERT(true == IsXidiVirtualControllerInstance(*lpddi));
                else
                    TEST_ASSERT(false == IsXidiVirtualControllerInstance(*lpddi));

                deviceCounter -= 1;

                return DIENUM_CONTINUE;
            },
            &deviceCounter, DIEDFL_ATTACHEDONLY)
        );

        TEST_ASSERT(kExpectedFinalDeviceCounter == deviceCounter);
    }
}
