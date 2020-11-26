/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ControllerIdentification.cpp
 *   Implementation of functions for identifying and enumerating XInput-based
 *   game controllers.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "Globals.h"
#include "XInputController.h"

#include <guiddef.h>
#include <Xinput.h>


namespace Xidi
{
    namespace ControllerIdentification
    {
        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Extracts and returns the instance index from an XInput controller's GUID.
        /// Does not verify that the supplied GUID actually represents an XInput instance GUID.
        /// @param [in] xguid XInput controller's instance GUID.
        /// @return Instance index (a.k.a. XInput player number).
        static WORD ExtractInstanceFromXInputInstanceGUID(REFGUID xguid)
        {
            return (*((WORD*)(&xguid.Data4[6])));
        }

        /// Turns the provided base instance GUID for an XInput controller into an instance GUID for a controller of the specified index.
        /// Does not verify that the supplied GUID actually represents an XInput instance GUID.
        /// @param [in,out] xguid GUID whose XInput instance field should be set.
        /// @param [in] xindex Instance index (a.k.a. XInput player number).
        void SetInstanceInXInputInstanceGUID(GUID& xguid, const WORD xindex)
        {
            *((WORD*)(&xguid.Data4[6])) = xindex;
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "ControllerIdentification.h" for documentation.

        template <typename EarliestIDirectInputType, typename EarliestIDirectInputDeviceType> BOOL DoesDirectInputControllerSupportXInput(EarliestIDirectInputType* dicontext, REFGUID instanceGUID, std::wstring* devicePath)
        {
            BOOL deviceSupportsXInput = FALSE;

            EarliestIDirectInputDeviceType* didevice = nullptr;
            HRESULT result = dicontext->CreateDevice(instanceGUID, &didevice, nullptr);

            if (DI_OK == result)
            {
                // Get the GUID and device path of the DirectInput device.
                DIPROPGUIDANDPATH devinfo;
                ZeroMemory(&devinfo, sizeof(devinfo));

                devinfo.diph.dwHeaderSize = sizeof(devinfo.diph);
                devinfo.diph.dwSize = sizeof(devinfo);
                devinfo.diph.dwHow = DIPH_DEVICE;

                result = didevice->GetProperty(DIPROP_GUIDANDPATH, &devinfo.diph);

                if (DI_OK == result)
                {
                    // The documented "best" way of determining if a device supports XInput is to look for "&IG_" in the device path string.
                    if (nullptr != wcsstr(devinfo.wszPath, L"&IG_") || nullptr != wcsstr(devinfo.wszPath, L"&ig_"))
                    {
                        deviceSupportsXInput = TRUE;

                        if (nullptr != devicePath)
                            *devicePath = devinfo.wszPath;
                    }
                }

                didevice->Release();
            }

            return deviceSupportsXInput;
        }

        template BOOL DoesDirectInputControllerSupportXInput<typename EarliestIDirectInputA, typename EarliestIDirectInputDeviceA>(EarliestIDirectInputA*, REFGUID, std::wstring*);
        template BOOL DoesDirectInputControllerSupportXInput<typename EarliestIDirectInputW, typename EarliestIDirectInputDeviceW>(EarliestIDirectInputW*, REFGUID, std::wstring*);

        // ---------

        template <typename DeviceInstanceType> BOOL EnumerateXInputControllers(BOOL(FAR PASCAL* lpCallback)(const DeviceInstanceType*, LPVOID), LPVOID pvRef)
        {
            for (WORD idx = 0; idx < XInputController::kMaxNumXInputControllers; ++idx)
            {
                // Create a DirectInput device structure
                DeviceInstanceType* instanceInfo = new DeviceInstanceType;
                ZeroMemory(instanceInfo, sizeof(*instanceInfo));
                instanceInfo->dwSize = sizeof(*instanceInfo);
                MakeInstanceGUID(instanceInfo->guidInstance, idx);
                instanceInfo->guidProduct = kXInputProductGUID;
                instanceInfo->dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD;
                FillXInputControllerName(instanceInfo->tszInstanceName, _countof(instanceInfo->tszInstanceName), idx);
                FillXInputControllerName(instanceInfo->tszProductName, _countof(instanceInfo->tszProductName), idx);

                // Submit the device to the application.
                HRESULT appResult = lpCallback(instanceInfo, pvRef);

                // Clean up.
                delete instanceInfo;

                // See if the application wants to enumerate more devices.
                if (DIENUM_CONTINUE != appResult)
                    return DIENUM_STOP;

            }

            return DIENUM_CONTINUE;
        }

        template BOOL EnumerateXInputControllers(LPDIENUMDEVICESCALLBACKA, LPVOID);
        template BOOL EnumerateXInputControllers(LPDIENUMDEVICESCALLBACKW, LPVOID);

        // ---------

        template <> int FillXInputControllerName<LPSTR>(LPSTR buf, const size_t bufcount, const DWORD controllerIndex)
        {
            CHAR xidiControllerNameFormatString[128];
            LoadStringA(Globals::GetInstanceHandle(), IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT, xidiControllerNameFormatString, _countof(xidiControllerNameFormatString));

            return sprintf_s(buf, bufcount, xidiControllerNameFormatString, (controllerIndex + 1));
        }

        template <> int FillXInputControllerName<LPWSTR>(LPWSTR buf, const size_t bufcount, const DWORD controllerIndex)
        {
            WCHAR xidiControllerNameFormatString[128];
            LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT, xidiControllerNameFormatString, _countof(xidiControllerNameFormatString));

            return swprintf_s(buf, bufcount, xidiControllerNameFormatString, (controllerIndex + 1));
        }

        // ---------

        void GetProductGUID(GUID& xguid)
        {
            xguid = kXInputProductGUID;
        }

        // ---------

        void MakeInstanceGUID(GUID& xguid, const WORD xindex)
        {
            xguid = kXInputBaseInstGUID;
            SetInstanceInXInputInstanceGUID(xguid, xindex);
        }

        // ---------

        LONG XInputControllerIndexForInstanceGUID(REFGUID instanceGUID)
        {
            LONG resultIndex = -1;
            WORD xindex = ExtractInstanceFromXInputInstanceGUID(instanceGUID);

            if (xindex < XInputController::kMaxNumXInputControllers)
            {
                GUID realXInputGUID;
                MakeInstanceGUID(realXInputGUID, xindex);

                if (realXInputGUID == instanceGUID)
                    resultIndex = (LONG)xindex;
            }

            return resultIndex;
        }
    }
}
