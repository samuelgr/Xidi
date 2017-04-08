/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *   Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file WrapperIDirectInput.cpp
 *   Implementation of the wrapper class for IDirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "Log.h"
#include "MapperFactory.h"
#include "WrapperIDirectInput.h"
#include "WrapperIDirectInputDevice.h"
#include "XInputController.h"

using namespace Xidi;


// -------- LOCAL TYPES ---------------------------------------------------- //

namespace Xidi
{
    /// Contains all information required to intercept callbacks to EnumDevices.
    struct SEnumDevicesCallbackInfo
    {
        WrapperIDirectInput* instance;                                      ///< #WrapperIDirectInput instance that invoked the enumeration.
        LPDIENUMDEVICESCALLBACK lpCallback;                                 ///< Application-specified callback that should be invoked.
        LPVOID pvRef;                                                       ///< Application-specified argument to be provided to the application-specified callback.
        BOOL callbackReturnCode;                                            ///< Indicates if the application requested that enumeration continue or stop.
        BOOL systemHasXInputDevices;                                        ///< Indicates if the system has any XInput-compatible devices attached during device enumeration.
    };
}


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInput.h" for documentation.

WrapperIDirectInput::WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode) : underlyingDIObject(underlyingDIObject), underlyingDIObjectUsesUnicode(underlyingDIObjectUsesUnicode) {}


// -------- HELPERS -------------------------------------------------------- //
// See "WrapperIDirectInput.h" for documentation.

void WrapperIDirectInput::LogCreateDeviceNonXInput(void)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_WRAPPERIDIRECTINPUT_CREATE_NONXINPUT);
}

// --------

void WrapperIDirectInput::LogCreateDeviceXInput(unsigned int index)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_WRAPPERIDIRECTINPUT_CREATE_XINPUT_FORMAT, index);
}

// --------

void WrapperIDirectInput::LogEnumDevice(LPCTSTR deviceName)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUT_ENUM_DEVICES_ENUM_FORMAT, deviceName);
}

// --------

void WrapperIDirectInput::LogEnumFinishEarly(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUT_ENUM_DEVICES_FINISH_EARLY);
}

// --------

void WrapperIDirectInput::LogEnumSkipDevice(LPCTSTR deviceName)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUT_ENUM_DEVICES_SKIP_FORMAT, deviceName);
}

// --------

void WrapperIDirectInput::LogEnumXidiDevices(void)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUT_ENUM_DEVICES_XIDI);
}

// --------

void WrapperIDirectInput::LogStartEnumDevices(void)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUT_ENUM_DEVICES_START);
}

// --------

void WrapperIDirectInput::LogFinishEnumDevices(void)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUT_ENUM_DEVICES_FINISH);
}


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
        LogCreateDeviceNonXInput();
        return underlyingDIObject->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
    }
    else
    {
        // Is an XInput GUID, so create a fake device that will communicate with the XInput controller of the specified index.
        LogCreateDeviceXInput(xinputIndex + 1);
        *lplpDirectInputDevice = new WrapperIDirectInputDevice(underlyingDIObjectUsesUnicode, new XInputController(xinputIndex), MapperFactory::CreateMapper());
        return DI_OK;
    }
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
#if DIRECTINPUT_VERSION >= 0x0800
    const BOOL gameControllersRequested = (DI8DEVCLASS_ALL == dwDevType || DI8DEVCLASS_GAMECTRL == dwDevType || DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(dwDevType));
    const DWORD gameControllerDevClass = DI8DEVCLASS_GAMECTRL;
#else
    const BOOL gameControllersRequested = (0 == dwDevType || DIDEVTYPE_JOYSTICK == dwDevType);
    const DWORD gameControllerDevClass = DIDEVTYPE_JOYSTICK;
#endif
    
    SEnumDevicesCallbackInfo callbackInfo;
    callbackInfo.instance = this;
    callbackInfo.lpCallback = lpCallback;
    callbackInfo.pvRef = pvRef;
    callbackInfo.callbackReturnCode = DIENUM_CONTINUE;
    callbackInfo.systemHasXInputDevices = FALSE;

    HRESULT enumResult = DI_OK;

    LogStartEnumDevices();

    // Enumerating game controllers requires some manipulation.
    if (gameControllersRequested)
    {
        // First scan the system for any XInput-compatible game controllers that match the enumeration request.
        enumResult = underlyingDIObject->EnumDevices(gameControllerDevClass, &WrapperIDirectInput::CallbackEnumGameControllersXInputScan, (LPVOID)&callbackInfo, dwFlags);
        if (DI_OK != enumResult) return enumResult;

        // Second, if the system has XInput controllers, enumerate them.
        // These will be the first controllers seen by the application.
        if (TRUE == callbackInfo.systemHasXInputDevices)
        {
            LogEnumXidiDevices();
            
            if (underlyingDIObjectUsesUnicode)
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersW((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
            else
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersA((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);

            if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
            {
                LogEnumFinishEarly();
                return enumResult;
            }
        }

        // Third, enumerate all other game controllers, filtering out those that support XInput.
        enumResult = underlyingDIObject->EnumDevices(gameControllerDevClass, &WrapperIDirectInput::CallbackEnumGameControllersFilterXInput, (LPVOID)&callbackInfo, dwFlags);
        if (DI_OK != enumResult) return enumResult;

        if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
        {
            LogEnumFinishEarly();
            return enumResult;
        }

        // Finally, if the system did not have any XInput controllers, enumerate them anyway.
        // These will be the last controllers seen by the application.
        if (FALSE == callbackInfo.systemHasXInputDevices)
        {
            LogEnumXidiDevices();

            if (underlyingDIObjectUsesUnicode)
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersW((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
            else
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersA((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);

            if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
            {
                LogEnumFinishEarly();
                return enumResult;
            }
        }
    }

    // Enumerate anything else the application requested, filtering out game controllers.
    enumResult = underlyingDIObject->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumOtherDevices, (LPVOID)&callbackInfo, dwFlags);
    if (DI_OK != enumResult) return enumResult;

    if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
    {
        LogEnumFinishEarly();
        return enumResult;
    }
    
    LogFinishEnumDevices();
    return enumResult;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance)
{
    return underlyingDIObject->FindDevice(rguidClass, ptszName, pguidInstance);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::GetDeviceStatus(REFGUID rguidInstance)
{
    // Check if the specified instance GUID is an XInput GUID.
    LONG xinputIndex = ControllerIdentification::XInputControllerIndexForInstanceGUID(rguidInstance);

    if (-1 == xinputIndex)
    {
        // Not an XInput GUID, so ask the underlying implementation for status.
        return underlyingDIObject->GetDeviceStatus(rguidInstance);
    }
    else
    {
        // Is an XInput GUID, so specify that it is connected.
        return DI_OK;
    }
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

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumGameControllersXInputScan(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

    // If the present controller supports XInput, indicate as much and stop the enumeration.
    if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject, lpddi->guidInstance))
    {
        callbackInfo->systemHasXInputDevices = TRUE;
        return DIENUM_STOP;
    }

    return DIENUM_CONTINUE;
}

// --------

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumGameControllersFilterXInput(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

    // Do not enumerate controllers that support XInput; these are enumerated separately.
    if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject, lpddi->guidInstance))
    {
        LogEnumSkipDevice(lpddi->tszProductName);
        return DIENUM_CONTINUE;
    }

    LogEnumDevice(lpddi->tszProductName);
    callbackInfo->callbackReturnCode = callbackInfo->lpCallback(lpddi, callbackInfo->pvRef);
    return callbackInfo->callbackReturnCode;
}

// --------

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumOtherDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

    // Identify devices that are game controllers.
#if DIRECTINPUT_VERSION >= 0x0800
    const BOOL isDeviceGameController = ((DI8DEVTYPE_JOYSTICK == GET_DIDEVICE_TYPE(lpddi->dwDevType)) || (DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(lpddi->dwDevType)) || (DI8DEVTYPE_DRIVING == GET_DIDEVICE_TYPE(lpddi->dwDevType)) || (DI8DEVTYPE_FLIGHT == GET_DIDEVICE_TYPE(lpddi->dwDevType)) || (DI8DEVTYPE_1STPERSON == GET_DIDEVICE_TYPE(lpddi->dwDevType)));
#else
    const BOOL isDeviceGameController = (DIDEVTYPE_JOYSTICK == GET_DIDEVICE_TYPE(lpddi->dwDevType));
#endif

    // Game controllers would already have been enumerated, so skip over any that are encountered.
    if (FALSE == isDeviceGameController)
    {
        LogEnumDevice(lpddi->tszProductName);
        callbackInfo->callbackReturnCode = callbackInfo->lpCallback(lpddi, callbackInfo->pvRef);
        return callbackInfo->callbackReturnCode;
    }

    return DIENUM_CONTINUE;
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
