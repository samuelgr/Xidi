/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XInputControllerIdentification.cpp
 *      Implementation of helpers for identifying and enumerating
 *      XInput-based game controllers.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"

#include <guiddef.h>
#include <Xinput.h>


using namespace Xidi;


// -------- CONSTANTS ------------------------------------------------------ //
// See "XInputControllerIdentification.h" for documentation.

const GUID ControllerIdentification::kXInputProductGUID = { 0xffffffff, 0x0000, 0x0000,{ 0x00, 0x00, 'X', 'I', 'N', 'P', 'U', 'T' } };

const GUID ControllerIdentification::kXInputInstGUID[4] = {
    { 0xffffffff, 0x0000, 0x0000,{ 0x00, 'X', 'I', 'N', 'P', 'U', 'T', '1' } },
    { 0xffffffff, 0x0000, 0x0000,{ 0x00, 'X', 'I', 'N', 'P', 'U', 'T', '2' } },
    { 0xffffffff, 0x0000, 0x0000,{ 0x00, 'X', 'I', 'N', 'P', 'U', 'T', '3' } },
    { 0xffffffff, 0x0000, 0x0000,{ 0x00, 'X', 'I', 'N', 'P', 'U', 'T', '4' } },
};


// -------- CLASS METHODS -------------------------------------------------- //
// See "XInputControllerIdentification.h" for documentation.

BOOL ControllerIdentification::DoesDirectInputControllerSupportXInput(VersionedIDirectInput* dicontext, REFGUID instanceGUID)
{
    BOOL deviceSupportsXInput = FALSE;
    
    VersionedIDirectInputDevice* didevice = NULL;
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

BOOL ControllerIdentification::EnumerateXInputControllersA(LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef)
{
    for (DWORD idx = 0; idx < 4; ++idx)
    {
        XINPUT_STATE dummyState;
        DWORD result = XInputGetState(idx, &dummyState);

        // If the controller is connected, the result is ERROR_SUCCESS, otherwise ERROR_DEVICE_NOT_CONNECTED.
        // The state is not needed, just the return code to determine if there is a controller at the specified index or not.
        if (ERROR_SUCCESS == result)
        {
            // Create a DirectInput device structure
            DIDEVICEINSTANCEA* instanceInfo = new DIDEVICEINSTANCEA;
            ZeroMemory(instanceInfo, sizeof(*instanceInfo));
            instanceInfo->dwSize = sizeof(*instanceInfo);
            instanceInfo->guidInstance = kXInputInstGUID[idx];
            instanceInfo->guidProduct = kXInputProductGUID;
            instanceInfo->dwDevType = DI8DEVTYPE_GAMEPAD;
            sprintf_s(instanceInfo->tszInstanceName, _countof(instanceInfo->tszInstanceName), "XInput Controller %u", (unsigned)(idx + 1));
            sprintf_s(instanceInfo->tszProductName, _countof(instanceInfo->tszProductName), "XInput Controller %u", (unsigned)(idx + 1));

            // Submit the device to the application.
            HRESULT appResult = lpCallback(instanceInfo, pvRef);

            // Clean up.
            delete instanceInfo;

            // See if the application wants to enumerate more devices.
            if (DIENUM_CONTINUE != appResult)
                return DIENUM_STOP;
        }
    }

    return DIENUM_CONTINUE;
}

// ---------

BOOL ControllerIdentification::EnumerateXInputControllersW(LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef)
{
    for (DWORD idx = 0; idx < 4; ++idx)
    {
        XINPUT_STATE dummyState;
        DWORD result = XInputGetState(idx, &dummyState);
        
        // If the controller is connected, the result is ERROR_SUCCESS, otherwise ERROR_DEVICE_NOT_CONNECTED.
        // The state is not needed, just the return code to determine if there is a controller at the specified index or not.
        if (ERROR_SUCCESS == result)
        {
            // Create a DirectInput device structure
            DIDEVICEINSTANCEW* instanceInfo = new DIDEVICEINSTANCEW;
            ZeroMemory(instanceInfo, sizeof(*instanceInfo));
            instanceInfo->dwSize = sizeof(*instanceInfo);
            instanceInfo->guidInstance = kXInputInstGUID[idx];
            instanceInfo->guidProduct = kXInputProductGUID;
            instanceInfo->dwDevType = DI8DEVTYPE_GAMEPAD;
            swprintf_s(instanceInfo->tszInstanceName, _countof(instanceInfo->tszInstanceName), L"XInput Controller %u", (unsigned)(idx + 1));
            swprintf_s(instanceInfo->tszProductName, _countof(instanceInfo->tszProductName), L"XInput Controller %u", (unsigned)(idx + 1));

            // Submit the device to the application.
            HRESULT appResult = lpCallback(instanceInfo, pvRef);
            
            // Clean up.
            delete instanceInfo;

            // See if the application wants to enumerate more devices.
            if (DIENUM_CONTINUE != appResult)
                return DIENUM_STOP;
        }
    }

    return DIENUM_CONTINUE;
}

// ---------

LONG ControllerIdentification::XInputControllerIndexForInstanceGUID(REFGUID instanceGUID)
{
    for (LONG i = 0; i < _countof(kXInputInstGUID); ++i)
    {
        if (kXInputInstGUID[i] == instanceGUID)
            return i;
    }
    
    return -1;
}
