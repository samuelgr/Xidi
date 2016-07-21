/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperIDirectInput.cpp
 *      Implementation of the wrapper class for IDirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "MapperFactory.h"
#include "WrapperIDirectInput.h"
#include "WrapperIDirectInputDevice.h"
#include "XInputController.h"

using namespace Xidi;


// -------- LOCAL TYPES ---------------------------------------------------- //

// Contains all information required to intercept callbacks to EnumDevices.
namespace Xidi
{
    struct SEnumDevicesCallbackInfo
    {
        WrapperIDirectInput* instance;
        LPDIENUMDEVICESCALLBACK lpCallback;
        LPVOID pvRef;
    };
}


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInput.h" for documentation.

WrapperIDirectInput::WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode) : underlyingDIObject(underlyingDIObject), underlyingDIObjectUsesUnicode(underlyingDIObjectUsesUnicode) {}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT result = S_OK;
    
#if DIRECTINPUT_VERSION >= 0x0800
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput8A) || IsEqualIID(riid, IID_IDirectInput8W))
#else
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput7A) || IsEqualIID(riid, IID_IDirectInput7W) || IsEqualIID(riid, IID_IDirectInput2A) || IsEqualIID(riid, IID_IDirectInput2W) || IsEqualIID(riid, IID_IDirectInputA) || IsEqualIID(riid, IID_IDirectInputW))
#endif
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

ULONG STDMETHODCALLTYPE WrapperIDirectInput::AddRef(void)
{
    return underlyingDIObject->AddRef();
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInput::Release(void)
{
    ULONG numRemainingRefs = underlyingDIObject->Release();
    
    if (0 == numRemainingRefs)
        delete this;
    
    return numRemainingRefs;
}


// -------- METHODS: IDirectInput COMMON ----------------------------------- //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::CreateDevice(REFGUID rguid, EarliestIDirectInputDevice** lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
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
        *lplpDirectInputDevice = new WrapperIDirectInputDevice(underlyingDIObjectUsesUnicode, new XInputController(xinputIndex), MapperFactory::CreateMapper());
        return DI_OK;
    }
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    SEnumDevicesCallbackInfo callbackInfo;
    callbackInfo.instance = this;
    callbackInfo.lpCallback = lpCallback;
    callbackInfo.pvRef = pvRef;
    
    BOOL xinputEnumResult = DIENUM_CONTINUE;

    // Only enumerate XInput controllers if the application requests a type that includes game controllers.
#if DIRECTINPUT_VERSION >= 0x0800
    if (DI8DEVCLASS_ALL == dwDevType || DI8DEVCLASS_GAMECTRL == dwDevType || DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(dwDevType))
#else
    if (0 == dwDevType || (DIDEVTYPE_JOYSTICK == GET_DIDEVICE_TYPE(dwDevType) && (0 == GET_DIDEVICE_SUBTYPE(dwDevType) || DIDEVTYPEJOYSTICK_GAMEPAD == GET_DIDEVICE_TYPE(dwDevType))))
#endif
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
        return underlyingDIObject->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumDevices, (LPVOID)&callbackInfo, dwFlags);
    else
        return DI_OK;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance)
{
    return underlyingDIObject->FindDevice(rguidClass, ptszName, pguidInstance);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::GetDeviceStatus(REFGUID rguidInstance)
{
    return underlyingDIObject->GetDeviceStatus(rguidInstance);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::Initialize(HINSTANCE hinst, DWORD dwVersion)
{
    return underlyingDIObject->Initialize(hinst, dwVersion);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    return underlyingDIObject->RunControlPanel(hwndOwner, dwFlags);
}


// -------- CALLBACKS: IDirectInput COMMON --------------------------------- //
// See "WrapperIDirectInput.h" for documentation.

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;
    
    // Do not enumerate controllers that support XInput; these are enumerated separately.
    if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject, lpddi->guidInstance))
        return DIENUM_CONTINUE;
    
    return callbackInfo->lpCallback(lpddi, callbackInfo->pvRef);
}


#if DIRECTINPUT_VERSION >= 0x0800
// -------- METHODS: IDirectInput8 ONLY ------------------------------------ //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
    return underlyingDIObject->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}
#else
// -------- METHODS: IDirectInput LEGACY ----------------------------------- //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
{
    // Make sure the supplied IID is valid.
    if (underlyingDIObjectUsesUnicode)
    {
        if (!(IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDevice7W)))
            return E_INVALIDARG;
    }
    else
    {
        if (!(IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDevice7A)))
            return E_INVALIDARG;
    }

    // Create a device the normal way.
    return CreateDevice(rguid, (EarliestIDirectInputDevice**)lplpDirectInputDevice, pUnkOuter);
    
}
#endif
