/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperIDirectInput8.cpp
 *      Implementation of the wrapper class for IDirectInput8.
 *****************************************************************************/

#include "Hashers.h"
#include "WrapperIDirectInput8.h"
#include "WrapperIDirectInputDevice8.h"

using namespace XinputControllerDirectInput;


// -------- LOCAL TYPES ---------------------------------------------------- //

// Contains all information required to intercept callbacks to EnumDevices.
namespace XinputControllerDirectInput
{
    struct SEnumDevicesCallbackInfo
    {
        WrapperIDirectInput8* instance;
        LPDIENUMDEVICESCALLBACK lpCallback;
        LPVOID pvRef;
    };
}

// Contains all information required to intercept callbacks to EnumDevicesBySemantics.
namespace XinputControllerDirectInput
{
    struct SEnumDevicesBySemanticsCallbackInfo
    {
        WrapperIDirectInput8* instance;
        LPDIENUMDEVICESBYSEMANTICSCB lpCallback;
        LPVOID pvRef;
    };
}


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInput8.h" for documentation.

WrapperIDirectInput8::WrapperIDirectInput8(IDirectInput8* underlyingDIObject) : underlyingDIObject(underlyingDIObject) {}


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
    // Create the device, as requested by the application.
    IDirectInputDevice8* createdDevice = NULL;
    HRESULT result = underlyingDIObject->CreateDevice(rguid, &createdDevice, pUnkOuter);
    if (DI_OK != result) return result;
    
    // Supply the application with the correct interface, potentially intercepted.
    *lplpDirectInputDevice = objectFactory.CreateDirectInputDeviceForController(createdDevice, rguid);
    return DI_OK;
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
    
    objectFactory.ResetEnumeratedControllers();
    return underlyingDIObject->EnumDevices(dwDevType, &WrapperIDirectInput8::CallbackEnumDevices, (LPVOID)&callbackInfo, dwFlags);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput8::EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    SEnumDevicesBySemanticsCallbackInfo callbackInfo;
    callbackInfo.instance = this;
    callbackInfo.lpCallback = lpCallback;
    callbackInfo.pvRef = pvRef;
    
    objectFactory.ResetEnumeratedControllers();
    return underlyingDIObject->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, &WrapperIDirectInput8::CallbackEnumDevicesBySemantics, (LPVOID)&callbackInfo, dwFlags);
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
    callbackInfo->instance->objectFactory.SubmitEnumeratedController(lpddi->guidProduct, lpddi->guidInstance);

    return callbackInfo->lpCallback(lpddi, callbackInfo->pvRef);
}

// ---------

BOOL STDMETHODCALLTYPE WrapperIDirectInput8::CallbackEnumDevicesBySemantics(LPCDIDEVICEINSTANCE lpddi, LPDIRECTINPUTDEVICE8 lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef)
{
    SEnumDevicesBySemanticsCallbackInfo* callbackInfo = (SEnumDevicesBySemanticsCallbackInfo*)pvRef;
    callbackInfo->instance->objectFactory.SubmitEnumeratedController(lpddi->guidProduct, lpddi->guidInstance);

    return callbackInfo->lpCallback(lpddi, lpdid, dwFlags, dwRemaining, callbackInfo->pvRef);
}
