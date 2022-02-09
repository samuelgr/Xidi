/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file ControllerIdentification.cpp
 *   Implementation of functions for identifying and enumerating Xidi virtual
 *   controllers in the context of DirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Mapper.h"
#include "TemporaryBuffer.h"

#include <memory>
#include <optional>
#include <xinput.h>


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Determines if the specified controller supports force feedback.
    /// @param [in] controllerId Identifier of the controller of interest.
    /// @return `true` if so, `false` otherwise.
    static inline bool DoesControllerSupportForceFeedback(DWORD controllerId)
    {
        const Controller::Mapper* const mapper = Controller::Mapper::GetConfigured((Controller::TControllerIdentifier)controllerId);
        if (nullptr == mapper)
            return false;

        return mapper->GetCapabilities().ForceFeedbackIsSupported();
    }

    /// Extracts and returns the instance index from a Xidi virtual controller's GUID.
    /// Does not verify that the supplied GUID actually represents an XInput instance GUID.
    /// @param [in] xguid Xidi virtual controller's instance GUID.
    /// @return Instance index (a.k.a. XInput player number).
    static inline DWORD ExtractVirtualControllerInstanceFromGuid(REFGUID xguid)
    {
        return (*((DWORD*)(&xguid.Data4[4])));
    }

    /// Turns the provided base instance GUID for a Xidi virtual controller into an instance GUID for a controller of the specified index.
    /// Does not verify that the supplied GUID actually represents a Xidi virtual controller instance GUID.
    /// @param [in,out] xguid GUID whose XInput instance field should be set.
    /// @param [in] xindex Instance index (a.k.a. XInput player number).
    static inline void SetVirtualControllerInstanceInGuid(GUID& xguid, DWORD xindex)
    {
        *((DWORD*)(&xguid.Data4[4])) = xindex;
    }


    // -------- FUNCTIONS -------------------------------------------------- //
    // See "ControllerIdentification.h" for documentation.

    template <typename EarliestIDirectInputType, typename EarliestIDirectInputDeviceType> bool DoesDirectInputControllerSupportXInput(EarliestIDirectInputType* dicontext, REFGUID instanceGUID, std::wstring* devicePath)
    {
        bool deviceSupportsXInput = false;

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
                    deviceSupportsXInput = true;

                    if (nullptr != devicePath)
                        *devicePath = devinfo.wszPath;
                }
            }

            didevice->Release();
        }

        return deviceSupportsXInput;
    }

    template bool DoesDirectInputControllerSupportXInput<typename EarliestIDirectInputA, typename EarliestIDirectInputDeviceA>(EarliestIDirectInputA*, REFGUID, std::wstring*);
    template bool DoesDirectInputControllerSupportXInput<typename EarliestIDirectInputW, typename EarliestIDirectInputDeviceW>(EarliestIDirectInputW*, REFGUID, std::wstring*);

    // ---------

    template <typename DeviceInstanceType> BOOL EnumerateVirtualControllers(BOOL(FAR PASCAL* lpCallback)(const DeviceInstanceType*, LPVOID), LPVOID pvRef, bool forceFeedbackRequired)
    {
        std::unique_ptr<DeviceInstanceType> instanceInfo = std::make_unique<DeviceInstanceType>();

        for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx)
        {
            if ((true == forceFeedbackRequired) && (false == DoesControllerSupportForceFeedback(idx)))
                continue;

            *instanceInfo = {.dwSize = sizeof(*instanceInfo)};
            FillVirtualControllerInfo(*instanceInfo, idx);

            if (DIENUM_CONTINUE != lpCallback(instanceInfo.get(), pvRef))
                return DIENUM_STOP;
        }

        return DIENUM_CONTINUE;
    }

    template BOOL EnumerateVirtualControllers(LPDIENUMDEVICESCALLBACKA, LPVOID, bool);
    template BOOL EnumerateVirtualControllers(LPDIENUMDEVICESCALLBACKW, LPVOID, bool);

    // ---------

    template <typename DeviceInstanceType> void FillVirtualControllerInfo(DeviceInstanceType& instanceInfo, DWORD controllerId)
    {
        MakeVirtualControllerInstanceGuid(instanceInfo.guidInstance, controllerId);
        instanceInfo.guidProduct = kVirtualControllerProductGuid;
        instanceInfo.dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD;
        FillVirtualControllerName(instanceInfo.tszInstanceName, _countof(instanceInfo.tszInstanceName), controllerId);
        FillVirtualControllerName(instanceInfo.tszProductName, _countof(instanceInfo.tszProductName), controllerId);

        // DirectInput versions 5 and higher include extra members in this structure, and this is indicated on input using the size member of the structure.
        if (instanceInfo.dwSize > offsetof(DeviceInstanceType, tszProductName) + sizeof(DeviceInstanceType::tszProductName))
        {
            if (true == DoesControllerSupportForceFeedback(controllerId))
                instanceInfo.guidFFDriver = kVirtualControllerForceFeedbackDriverGuid;
            else
                instanceInfo.guidFFDriver = {};

            // These fields are zeroed out because Xidi does not currently offer any of the functionality they represent.
            instanceInfo.wUsagePage = 0;
            instanceInfo.wUsage = 0;
        }
    }

    template void FillVirtualControllerInfo(DIDEVICEINSTANCEA&, DWORD);
    template void FillVirtualControllerInfo(DIDEVICEINSTANCEW&, DWORD);

    // ---------

    template <> int FillVirtualControllerName<LPSTR>(LPSTR buf, size_t bufcount, DWORD controllerIndex)
    {
        TemporaryBuffer<CHAR> xidiControllerNameFormatString;
        LoadStringA(Globals::GetInstanceHandle(), IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT, xidiControllerNameFormatString.Data(), xidiControllerNameFormatString.Capacity());

        return sprintf_s(buf, bufcount, (LPCSTR)xidiControllerNameFormatString.Data(), (controllerIndex + 1));
    }

    template <> int FillVirtualControllerName<LPWSTR>(LPWSTR buf, size_t bufcount, DWORD controllerIndex)
    {
        TemporaryBuffer<WCHAR> xidiControllerNameFormatString;
        LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT, xidiControllerNameFormatString.Data(), xidiControllerNameFormatString.Capacity());

        return swprintf_s(buf, bufcount, (LPCWSTR)xidiControllerNameFormatString.Data(), (controllerIndex + 1));
    }

    // ---------

    void GetProductGuid(GUID& xguid)
    {
        xguid = kVirtualControllerProductGuid;
    }

    // ---------

    void MakeVirtualControllerInstanceGuid(GUID& xguid, DWORD controllerId)
    {
        xguid = kVirtualControllerInstanceBaseGuid;
        SetVirtualControllerInstanceInGuid(xguid, controllerId);
    }

    // ---------

    std::optional<DWORD> VirtualControllerIdFromInstanceGuid(REFGUID instanceGUID)
    {
        DWORD xindex = ExtractVirtualControllerInstanceFromGuid(instanceGUID);

        if (xindex < XUSER_MAX_COUNT)
        {
            GUID realXInputGUID;
            MakeVirtualControllerInstanceGuid(realXInputGUID, xindex);

            if (realXInputGUID == instanceGUID)
                return xindex;
        }

        return std::nullopt;
    }
}
