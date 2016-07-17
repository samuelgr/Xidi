/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperIDirectInput8.cpp
 *      Implementation of the wrapper class for IDirectInput8.
 *****************************************************************************/

#include "ControllerIdentification.h"
#include "Mapper/OldGamepad.h"
#include "WrapperIDirectInput8.h"
#include "WrapperIDirectInputDevice8.h"
#include "XInputController.h"

using namespace Xidi;


// -------- LOCAL TYPES ---------------------------------------------------- //

// Contains all information required to intercept callbacks to EnumDevices.
namespace Xidi
{
    struct SEnumDevicesCallbackInfo
    {
        WrapperIDirectInput8* instance;
        LPDIENUMDEVICESCALLBACK lpCallback;
        LPVOID pvRef;
    };
}


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInput8.h" for documentation.

WrapperIDirectInput8::WrapperIDirectInput8(IDirectInput8* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode) : underlyingDIObject(underlyingDIObject), underlyingDIObjectUsesUnicode(underlyingDIObjectUsesUnicode) {}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT result = S_OK;
    
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput8))
    {
        AddRef();
        *ppvObj = this;
    }
    else
    {
        result = underlyingDIObject->QueryInterface(riid, ppvObj);
    }
    
    return result;
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInput8::AddRef(void)
{
    return underlyingDIObject->AddRef();
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInput8::Release(void)
{
    ULONG numRemainingRefs = underlyingDIObject->Release();
    
    if (0 == numRemainingRefs)
        delete this;
    
    return numRemainingRefs;
}


// -------- METHODS: IDirectInput8 ----------------------------------------- //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
    // Check if the specified instance GUID is an XInput GUID.
    LONG xinputIndex = ControllerIdentification::XInputControllerIndexForInstanceGUID(rguid);

    if (-1 == xinputIndex)
    {
        // Not an XInput GUID, so just create the device as requested by the application.
        return underlyingDIObject->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
    }
    else
    {
        // Is an XInput GUID, so create a fake device that will communicate with the XInput controller of the specified index.
        *lplpDirectInputDevice = new WrapperIDirectInputDevice8(underlyingDIObjectUsesUnicode, new XInputController(xinputIndex), new Mapper::OldGamepad());
        return DI_OK;
    }
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
    return underlyingDIObject->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    SEnumDevicesCallbackInfo callbackInfo;
    callbackInfo.instance = this;
    callbackInfo.lpCallback = lpCallback;
    callbackInfo.pvRef = pvRef;
    
    BOOL xinputEnumResult = DIENUM_CONTINUE;

    // Only enumerate XInput controllers if the application requests a type that includes game controllers.
    if (DI8DEVCLASS_ALL == dwDevType || DI8DEVCLASS_GAMECTRL == dwDevType)
    {
        // Currently force feedback is not suported.
        if (!(dwFlags & DIEDFL_FORCEFEEDBACK))
        {
            if (underlyingDIObjectUsesUnicode)
                xinputEnumResult = ControllerIdentification::EnumerateXInputControllersW((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
            else
                xinputEnumResult = ControllerIdentification::EnumerateXInputControllersA((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);
        }
    }

    // If either no XInput devices were enumerated or the application wants to continue enumeration, hand the process off to the native DirectInput library.
    // The callback below will filter out any DirectInput devices that are also XInput-based devices.
    if (DIENUM_CONTINUE == xinputEnumResult)
        return underlyingDIObject->EnumDevices(dwDevType, &WrapperIDirectInput8::CallbackEnumDevices, (LPVOID)&callbackInfo, dwFlags);
    else
        return DI_OK;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance)
{
    return underlyingDIObject->FindDevice(rguidClass, ptszName, pguidInstance);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::GetDeviceStatus(REFGUID rguidInstance)
{
    return underlyingDIObject->GetDeviceStatus(rguidInstance);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
    return underlyingDIObject->Initialize(hinst, dwVersion);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    return underlyingDIObject->RunControlPanel(hwndOwner, dwFlags);
}


// -------- CALLBACKS: IDirectInput8 --------------------------------------- //
// See "WrapperIDirectInput8.h" for documentation.

BOOL STDMETHODCALLTYPE WrapperIDirectInput8::CallbackEnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;
    
    // Do not enumerate controllers that support XInput; these are enumerated separately.
    if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject, lpddi->guidInstance))
        return DIENUM_CONTINUE;
    
    return callbackInfo->lpCallback(lpddi, callbackInfo->pvRef);
}
