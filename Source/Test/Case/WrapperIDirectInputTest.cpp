/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
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
        .supportsXInput = true,
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
        .supportsXInput = true,
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
        .supportsXInput = false,
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
        .supportsXInput = false,
        .instance = {
            .dwSize = sizeof(DIDEVICEINSTANCE),
            .guidInstance = {0x4e4af2c0, 0x68d9, 0x11ed, {0x80, 0x01, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}},
            .guidProduct = {0xc218046d, 0x0000, 0x0000, {0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44}},
            .dwDevType = 0x00010114,
            .tszInstanceName = L"Logitech RumblePad 2 USB (Generic Driver)",
            .tszProductName = L"Logitech RumblePad 2 USB (Generic Driver)",
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


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Enumerates the possible orderings of DirectInput device enumeration.
    /// Specifies what enumeration order is expected during test cases.
    enum class EExpectedEnumerationOrder
    {
        SystemDevicesFirst,                                                 ///< System devices should be enumerated before Xidi virtual devices.
        XidiVirtualControllersFirst,                                        ///< Xidi virtual controllers should be enumerated before system devices.
    };

    /// Describes the state of a DirectInput device enumeration.
    class EnumerationState
    {
    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Expected enumeration order.
        const EExpectedEnumerationOrder kExpectedOrder;

        /// Number of system devices expected to be enumerated.
        const size_t kExpectedNumSystemDevices;
        
        /// Number of Xidi virtual controllers expected to be enumerated.
        const size_t kExpectedNumXidiVirtualControllers;

        /// Actual number of system devices enumerated.
        size_t numSystemDevicesEnumerated;

        /// Actual number of Xidi virtual controllers enumerated.
        size_t numXidiVirtualControllersEnumerated;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        /// Requires an expected order and expected numbers of devices, with the system device count defaulting to 0 and the Xidi virtual controller count defaulting to what the system supports.
        inline EnumerationState(EExpectedEnumerationOrder expectedOrder, size_t expectedNumSystemDevices = 0, size_t expectedNumXidiVirtualControllers = Controller::kPhysicalControllerCount) : kExpectedOrder(expectedOrder), kExpectedNumSystemDevices(expectedNumSystemDevices), kExpectedNumXidiVirtualControllers(expectedNumXidiVirtualControllers), numSystemDevicesEnumerated(0), numXidiVirtualControllersEnumerated(0)
        {
            // Nothing to do here.
        }


        // -------- CLASS METHODS ------------------------------------------ //

        /// DirectInput device enumeration callback, which uses the reference parameter to track enumeration state.
        /// @param [in] deviceInstancePtr Pointer to the device instance structure.
        /// @param [in] enumerationStatePtr Typeless pointer to the enumeration state structure.
        /// @return Always `DIENUM_CONTINUE` so that more devices are enumerated.
        static BOOL __stdcall CheckEnumeratedDeviceCallback(const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType* deviceInstancePtr, LPVOID enumerationStatePtr)
        {
            ((EnumerationState*)enumerationStatePtr)->CheckEnumeratedDevice(*deviceInstancePtr);
            return DIENUM_CONTINUE;
        }

        /// Checks if a DirectInput device instance structure represents a Xidi virtual controller by comparing product and instance GUIDs as well as HID usage data.
        /// @param [in] deviceInstance Device instance to check.
        /// @return `true` if the instance is a Xidi virtual controller, `false` otherwise.
        static bool IsValidXidiVirtualControllerInstance(const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType& deviceInstance)
        {
            const auto maybeVirtualControllerId = VirtualControllerIdFromInstanceGuid(deviceInstance.guidInstance);
            if (false == maybeVirtualControllerId.has_value())
                return false;

            const GUID expectedVirtualControllerGuid = VirtualControllerGuid(maybeVirtualControllerId.value());
            if ((deviceInstance.guidProduct != expectedVirtualControllerGuid) || (deviceInstance.guidInstance != expectedVirtualControllerGuid))
                return false;

            static const SHidUsageData expectedHidUsageData = HidUsageDataForControllerElement({.type = Controller::EElementType::WholeController});
            const SHidUsageData actualHidUsageData = {.usagePage = deviceInstance.wUsagePage, .usage = deviceInstance.wUsage};
            if (actualHidUsageData != expectedHidUsageData)
                return false;

            return true;
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Checks if the state represented by this object reflects the fact that all expected system devices have already been enumerated.
        /// @return `true` if all system devices have been enumerated, `false` otherwise.
        inline bool DoneEnumeratingSystemDevices(void) const
        {
            return (kExpectedNumSystemDevices == numSystemDevicesEnumerated);
        }

        /// Checks if the state represented by this object reflects the fact that all expected Xidi virtual controllers have already been enumerated.
        /// @return `true` if all Xidi virtual controllers have been enumerated, `false` otherwise.
        inline bool DoneEnumeratingXidiVirtualControllers(void) const
        {
            return (kExpectedNumXidiVirtualControllers == numXidiVirtualControllersEnumerated);
        }

        /// Checks if the state represented by this object reflects the fact that all expected devices have already been enumerated.
        /// @return `true` if all expected devices have been enumerated, `false` otherwise.
        inline bool EnumerationComplete(void) const
        {
            return (DoneEnumeratingSystemDevices() && DoneEnumeratingXidiVirtualControllers());
        }

    private:
        /// Checks the specified DirectInput device instance for proper enumeration ordering and updates internal state accordingly.
        /// If the order is incorrect a test failure is flagged.
        /// @param [in] deviceInstance Pointer to the device instance structure.
        void CheckEnumeratedDevice(const DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType& deviceInstance)
        {
            switch (kExpectedOrder)
            {
            case EExpectedEnumerationOrder::SystemDevicesFirst:
                if (DoneEnumeratingSystemDevices())
                {
                    TEST_ASSERT(true == IsValidXidiVirtualControllerInstance(deviceInstance));
                    TEST_ASSERT(false == DoneEnumeratingXidiVirtualControllers());
                    numXidiVirtualControllersEnumerated += 1;
                }
                else
                {
                    TEST_ASSERT(false == IsValidXidiVirtualControllerInstance(deviceInstance));
                    numSystemDevicesEnumerated += 1;
                }
                break;

            case EExpectedEnumerationOrder::XidiVirtualControllersFirst:
                if (DoneEnumeratingXidiVirtualControllers())
                {
                    TEST_ASSERT(false == IsValidXidiVirtualControllerInstance(deviceInstance));
                    TEST_ASSERT(false == DoneEnumeratingSystemDevices());
                    numSystemDevicesEnumerated += 1;
                }
                else
                {
                    TEST_ASSERT(true == IsValidXidiVirtualControllerInstance(deviceInstance));
                    numXidiVirtualControllersEnumerated += 1;
                }
                break;

            default:
                TEST_FAILED_BECAUSE(L"Test implementation error due to invalid enumerator %d for expected device enumeration order.", (int)kExpectedOrder);
            }
        }
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Retrieves and returns the number of system devices that support XInput held by a mock DirectInput object.
    /// @param [in] mockDirectInput Mock DirectInput object to query.
    /// @return Number of XInput system devices present.
    inline size_t GetXInputSystemDeviceCount(const MockDirectInput& mockDirectInput)
    {
        return mockDirectInput.GetSystemDeviceCountFiltered([](const SDirectInputDeviceInfo& deviceInfo) -> bool
            {
                return deviceInfo.SupportsXInput();
            }
        );
    }

    /// Retrieves and returns the number of system devices that do not support XInput held by a mock DirectInput object.
    /// @param [in] mockDirectInput Mock DirectInput object to query.
    /// @return Number of non-XInput system devices present.
    inline size_t GetNonXInputSystemDeviceCount(const MockDirectInput& mockDirectInput)
    {
        return mockDirectInput.GetSystemDeviceCountFiltered([](const SDirectInputDeviceInfo& deviceInfo) -> bool
            {
                return !deviceInfo.SupportsXInput();
            }
        );
    }

    /// Retrieves and returns the number of system devices that support force feedback but do not support XInput held by a mock DirectInput object.
    /// @param [in] mockDirectInput Mock DirectInput object to query.
    /// @return Number of non-XInput system devices present.
    inline size_t GetForceFeedbackNonXInputSystemDeviceCount(const MockDirectInput& mockDirectInput)
    {
        return mockDirectInput.GetSystemDeviceCountFiltered([](const SDirectInputDeviceInfo& deviceInfo) -> bool
            {
                return (deviceInfo.SupportsForceFeedback() && !deviceInfo.SupportsXInput());
            }
        );
    }


    /// Creates a test DirectInput interface object that wraps a mock DirectInput interface object.
    /// @param [in] mockDirectInput Mock DirectInput interface object to use as the underlying DirectInput interface object.
    /// @return Newly-constructed wrapper object.
    static inline WrapperIDirectInput<kDirectInputTestCharMode> MakeTestWrapperIDirectInput(MockDirectInput& mockDirectInput)
    {
        return WrapperIDirectInput<kDirectInputTestCharMode>((DirectInputType<kDirectInputTestCharMode>::LatestIDirectInputType*)&mockDirectInput);
    }


    // -------- TEST CASES ------------------------------------------------- //

    // The following sequence of tests, which together comprise the EnumDevices suite, verify correct device enumeration behavior and order in the most common case of looking for all attached devices.
    // Scopes vary, so more details are provided with each test case.

    // No devices attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumDevices_NoSystemDevices)
    {
        MockDirectInput mockDirectInput;
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst);

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some XInput devices are attached to the system, and no non-XInput devices are attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumDevices_XInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kXboxOneWirelessXInputController, kXboxOneBluetoothXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst);

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some non-XInput devices are attached to the system, and no XInput devices are attached to the system.
    // The non-XInput devices should be presented first followed by all Xidi virtual controllers.
    TEST_CASE(WrapperIDirectInput_EnumDevices_NonXInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kLogitechRumblepadNonXInputController, kGenericNoForceFeedbackNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::SystemDevicesFirst, mockDirectInput.GetSystemDeviceCount());

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some XInput and non-XInput devices are attached to the system.
    // The Xidi virtual controllers should be presented first followed by all non-XInput system devices.
    TEST_CASE(WrapperIDirectInput_EnumDevices_MixedSystemDevices)
    {
        MockDirectInput mockDirectInput({kGenericNoForceFeedbackNonXInputController, kXboxOneBluetoothXInputController, kLogitechRumblepadNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst, GetNonXInputSystemDeviceCount(mockDirectInput));

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }


    // The following sequence of tests, which together comprise the EnumForceFeedbackDevices suite, verify correct device enumeration behavior and order when the enumeration is restricted to force feedback devices.
    // Scopes vary, so more details are provided with each test case.

    // No devices attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumForceFeedbackDevices_NoSystemDevices)
    {
        MockDirectInput mockDirectInput;
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst);

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some XInput devices are attached to the system, and no non-XInput devices are attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumForceFeedbackDevices_XInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kXboxOneWirelessXInputController, kXboxOneBluetoothXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst);

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some non-XInput devices that support force feedback are attached to the system, and no XInput devices are attached to the system.
    // The non-XInput devices should be presented first followed by all Xidi virtual controllers.
    TEST_CASE(WrapperIDirectInput_EnumForceFeedbackDevices_ForceFeedbackNonXInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kLogitechRumblepadNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::SystemDevicesFirst, mockDirectInput.GetSystemDeviceCount());

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some non-XInput devices that do not support force feedback are attached to the system, and no XInput devices are attached to the system.
    // Only Xidi virtual controllers should be enumerated, and all of them should be enumerated.
    TEST_CASE(WrapperIDirectInput_EnumForceFeedbackDevices_NonForceFeedbackNonXInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kGenericNoForceFeedbackNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst);

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // Some non-XInput devices are attached to the system with varying support for force feedback, and no XInput devices are attached to the system.
    // The non-XInput devices should be presented first followed by all Xidi virtual controllers.
    TEST_CASE(WrapperIDirectInput_EnumForceFeedbackDevices_MixedForceFeedbackNonXInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kLogitechRumblepadNonXInputController, kGenericNoForceFeedbackNonXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::SystemDevicesFirst, GetForceFeedbackNonXInputSystemDeviceCount(mockDirectInput));

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }

    // A mix of XInput and non-XInput devices are attached to the system, in the latter case with varying support for force feedback.
    // The Xidi virtual controllers should be presented first followed by all non-XInput system devices that support force feedback.
    TEST_CASE(WrapperIDirectInput_EnumForceFeedbackDevices_MixedForceFeedbackMixedXInputSystemDevices)
    {
        MockDirectInput mockDirectInput({kLogitechRumblepadNonXInputController, kXboxOneBluetoothXInputController, kGenericNoForceFeedbackNonXInputController, kXboxOneWirelessXInputController});
        auto testDirectInput = MakeTestWrapperIDirectInput(mockDirectInput);

        EnumerationState enumerationState(EExpectedEnumerationOrder::XidiVirtualControllersFirst, GetForceFeedbackNonXInputSystemDeviceCount(mockDirectInput));

        TEST_ASSERT(DI_OK == testDirectInput.EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumerationState::CheckEnumeratedDeviceCallback, &enumerationState, DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK));
        TEST_ASSERT(enumerationState.EnumerationComplete());
    }
}
