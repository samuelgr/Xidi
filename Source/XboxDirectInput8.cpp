/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XboxDirectInput8.cpp
 *      Implementation of the wrapper class for IDirectInput8.
 *****************************************************************************/

#include "XboxDirectInput8.h"
#include "ControllerIdentification.h"

using namespace XboxControllerDirectInput;


// -------- TYPE DEFINITIONS ----------------------------------------------- //

// Packed information for callbacks generated as a result of a call to EnumDevices.
struct EnumDevicesCallbackInfo
{
    XboxDirectInput8* instance;
    LPDIENUMDEVICESCALLBACK lpCallback;
    LPVOID pvRef;
};

// Packed information for callbacks generated as a result of a call to EnumDevicesBySemantics.
struct EnumDevicesBySemanticsCallbackInfo
{
    XboxDirectInput8* instance;
    LPDIENUMDEVICESBYSEMANTICSCB lpCallback;
    LPVOID pvRef;
};


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "XboxDirectInput8.h" for documentation.

XboxDirectInput8::XboxDirectInput8(IDirectInput8* underlyingDIObject) : underlyingDIObject(underlyingDIObject) {}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT __stdcall XboxDirectInput8::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    return underlyingDIObject->QueryInterface(riid, ppvObj);
}

// ---------

ULONG __stdcall XboxDirectInput8::AddRef(void)
{
    return underlyingDIObject->AddRef();
}

// ---------

ULONG __stdcall XboxDirectInput8::Release(void)
{
    ULONG numRemainingRefs = underlyingDIObject->Release();
    
    if (0 == numRemainingRefs)
    {
        delete this;
    }
    
    return numRemainingRefs;
}


// -------- METHODS: IDirectInput8 ----------------------------------------- //
// See DirectInput documentation for more information.

HRESULT __stdcall XboxDirectInput8::CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
    return underlyingDIObject->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
}

// ---------

HRESULT __stdcall XboxDirectInput8::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
    return underlyingDIObject->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}

// ---------

HRESULT __stdcall XboxDirectInput8::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    EnumDevicesCallbackInfo cbInfo;
    cbInfo.instance = this;
    cbInfo.lpCallback = lpCallback;
    cbInfo.pvRef = pvRef;
    
    return underlyingDIObject->EnumDevices(dwDevType, &XboxDirectInput8::CallbackEnumDevices, (LPVOID)&cbInfo, dwFlags);
}

// ---------

HRESULT __stdcall XboxDirectInput8::EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    EnumDevicesBySemanticsCallbackInfo cbInfo;
    cbInfo.instance = this;
    cbInfo.lpCallback = lpCallback;
    cbInfo.pvRef = pvRef;
    
    return underlyingDIObject->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, &XboxDirectInput8::CallbackEnumDevicesBySemantics, (LPVOID)&cbInfo, dwFlags);
}

// ---------

HRESULT __stdcall XboxDirectInput8::FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance)
{
    return underlyingDIObject->FindDevice(rguidClass, ptszName, pguidInstance);
}

// ---------

HRESULT __stdcall XboxDirectInput8::GetDeviceStatus(REFGUID rguidInstance)
{
    return underlyingDIObject->GetDeviceStatus(rguidInstance);
}

// ---------

HRESULT __stdcall XboxDirectInput8::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
    return underlyingDIObject->Initialize(hinst, dwVersion);
}

// ---------

HRESULT __stdcall XboxDirectInput8::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    return underlyingDIObject->RunControlPanel(hwndOwner, dwFlags);
}


// -------- CALLBACKS: IDirectInput8 --------------------------------------- //
// See "XboxDirectInput8.h" for documentation.

BOOL __stdcall XboxDirectInput8::CallbackEnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    EnumDevicesCallbackInfo* cbInfo = (EnumDevicesCallbackInfo*)pvRef;

    return cbInfo->lpCallback(lpddi, cbInfo->pvRef);
}

// ---------

BOOL __stdcall XboxDirectInput8::CallbackEnumDevicesBySemantics(LPCDIDEVICEINSTANCE lpddi, LPDIRECTINPUTDEVICE8 lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef)
{
    EnumDevicesBySemanticsCallbackInfo* cbInfo = (EnumDevicesBySemanticsCallbackInfo*)pvRef;

    return cbInfo->lpCallback(lpddi, lpdid, dwFlags, dwRemaining, cbInfo->pvRef);
}
