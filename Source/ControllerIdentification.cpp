/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * XInputControllerIdentification.cpp
 *      Implementation of helpers for identifying and enumerating
 *      XInput-based game controllers.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "Globals.h"
#include "XInputController.h"

#include <guiddef.h>
#include <Xinput.h>


using namespace Xidi;


// -------- CONSTANTS ------------------------------------------------------ //
// See "XInputControllerIdentification.h" for documentation.

const GUID ControllerIdentification::kXInputProductGUID = { 0xffffffff, 0x0000, 0x0000, { 0x00, 0x00, 'X', 'I', 'N', 'P', 'U', 'T' } };

const GUID ControllerIdentification::kXInputBaseInstGUID = { 0xffffffff, 0x0000, 0x0000, { 'X', 'I', 'N', 'P', 'U', 'T', 0x00, 0x00 } };


// -------- HELPERS -------------------------------------------------------- //
// See "XInputControllerIdentification.h" for documentation.

WORD ControllerIdentification::ExtractInstanceFromXInputInstanceGUID(REFGUID xguid)
{
    return (*((WORD*)(&xguid.Data4[6])));
}

// ---------

void ControllerIdentification::SetInstanceInXInputInstanceGUID(GUID& xguid, const WORD xindex)
{
    *((WORD*)(&xguid.Data4[6])) = xindex;
}


// -------- CLASS METHODS -------------------------------------------------- //
// See "XInputControllerIdentification.h" for documentation.

BOOL ControllerIdentification::DoesDirectInputControllerSupportXInput(EarliestIDirectInput* dicontext, REFGUID instanceGUID)
{
    BOOL deviceSupportsXInput = FALSE;
    
    EarliestIDirectInputDevice* didevice = NULL;
    HRESULT result = dicontext->CreateDevice(instanceGUID, &didevice, NULL);

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
            if (NULL != wcsstr(devinfo.wszPath, L"&IG_") || NULL != wcsstr(devinfo.wszPath, L"&ig_"))
                deviceSupportsXInput = TRUE;
        }

        didevice->Release();
    }

    return deviceSupportsXInput;
}

// ---------

HRESULT ControllerIdentification::EnumerateXInputControllersA(LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef)
{
    for (WORD idx = 0; idx < XInputController::kMaxNumXInputControllers; ++idx)
    {
        // Create a DirectInput device structure
        DIDEVICEINSTANCEA* instanceInfo = new DIDEVICEINSTANCEA;
        ZeroMemory(instanceInfo, sizeof(*instanceInfo));
        instanceInfo->dwSize = sizeof(*instanceInfo);
        MakeInstanceGUID(instanceInfo->guidInstance, idx);
        instanceInfo->guidProduct = kXInputProductGUID;
        instanceInfo->dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD;
        FillXInputControllerNameA(instanceInfo->tszInstanceName, _countof(instanceInfo->tszInstanceName), idx);
        FillXInputControllerNameA(instanceInfo->tszProductName, _countof(instanceInfo->tszProductName), idx);

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

// ---------

HRESULT ControllerIdentification::EnumerateXInputControllersW(LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef)
{
    for (WORD idx = 0; idx < XInputController::kMaxNumXInputControllers; ++idx)
    {
        // Create a DirectInput device structure
        DIDEVICEINSTANCEW* instanceInfo = new DIDEVICEINSTANCEW;
        ZeroMemory(instanceInfo, sizeof(*instanceInfo));
        instanceInfo->dwSize = sizeof(*instanceInfo);
        MakeInstanceGUID(instanceInfo->guidInstance, idx);
        instanceInfo->guidProduct = kXInputProductGUID;
        instanceInfo->dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD;
        FillXInputControllerNameW(instanceInfo->tszInstanceName, _countof(instanceInfo->tszInstanceName), idx);
        FillXInputControllerNameW(instanceInfo->tszProductName, _countof(instanceInfo->tszProductName), idx);

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

// ---------

int ControllerIdentification::FillXInputControllerNameA(LPSTR buf, const size_t bufcount, const DWORD controllerIndex)
{
    CHAR xidiControllerNameFormatString[128];
    LoadStringA(Globals::GetInstanceHandle(), IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT, xidiControllerNameFormatString, _countof(xidiControllerNameFormatString));
    
    return sprintf_s(buf, bufcount, xidiControllerNameFormatString, (controllerIndex + 1));
}

// ---------

int ControllerIdentification::FillXInputControllerNameW(LPWSTR buf, const size_t bufcount, const DWORD controllerIndex)
{
    WCHAR xidiControllerNameFormatString[128];
    LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT, xidiControllerNameFormatString, _countof(xidiControllerNameFormatString));
    
    return swprintf_s(buf, bufcount, xidiControllerNameFormatString, (controllerIndex + 1));
}

// ---------

void ControllerIdentification::GetProductGUID(GUID& xguid)
{
    xguid = kXInputProductGUID;
}

// ---------

void ControllerIdentification::MakeInstanceGUID(GUID& xguid, const WORD xindex)
{
    xguid = kXInputBaseInstGUID;
    SetInstanceInXInputInstanceGUID(xguid, xindex);
}

// ---------

LONG ControllerIdentification::XInputControllerIndexForInstanceGUID(REFGUID instanceGUID)
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
