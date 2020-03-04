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
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "Log.h"
#include "MapperFactory.h"
#include "VirtualDirectInputDevice.h"
#include "WrapperIDirectInput.h"
#include "XInputController.h"

#include <cstdlib>
#include <unordered_set>

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
        std::unordered_set<GUID> seenInstanceIdentifiers;                   ///< Holds device identifiers seen during device enumeration.
    };
}


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInput.h" for documentation.

WrapperIDirectInput::WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode) : underlyingDIObjectUsesUnicode(underlyingDIObjectUsesUnicode)
{
    if (underlyingDIObjectUsesUnicode)
        this->underlyingDIObject.a = (LatestIDirectInputA*)underlyingDIObject;
    else
        this->underlyingDIObject.w = (LatestIDirectInputW*)underlyingDIObject;
}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if (underlyingDIObjectUsesUnicode)
        return underlyingDIObject.w->QueryInterface(riid, ppvObj);
    else
        return underlyingDIObject.a->QueryInterface(riid, ppvObj);
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInput::AddRef(void)
{
    if (underlyingDIObjectUsesUnicode)
        return underlyingDIObject.w->AddRef();
    else
        return underlyingDIObject.a->AddRef();
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInput::Release(void)
{
    ULONG numRemainingRefs;
    
    if (underlyingDIObjectUsesUnicode)
        numRemainingRefs = underlyingDIObject.w->Release();
    else
        numRemainingRefs = underlyingDIObject.a->Release();

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
        Log::WriteLogMessage(ELogLevel::LogLevelInfo, _T("Binding to a non-XInput device. Xidi will not handle communication with it."));
        
        if (underlyingDIObjectUsesUnicode)
            return underlyingDIObject.w->CreateDevice(rguid, (EarliestIDirectInputDeviceW**)lplpDirectInputDevice, pUnkOuter);
        else
            return underlyingDIObject.a->CreateDevice(rguid, (EarliestIDirectInputDeviceA**)lplpDirectInputDevice, pUnkOuter);
    }
    else
    {
        // Is an XInput GUID, so create a fake device that will communicate with the XInput controller of the specified index.
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, _T("Binding to Xidi virtual XInput device for player %u."), (xinputIndex + 1));
        
        VirtualDirectInputDevice* newWrappedDevice = new VirtualDirectInputDevice(underlyingDIObjectUsesUnicode, new XInputController(xinputIndex), MapperFactory::CreateMapper());
        newWrappedDevice->AddRef();
        *lplpDirectInputDevice = newWrappedDevice;
        
        return DI_OK;
    }
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
#if DIRECTINPUT_VERSION >= 0x0800
    const BOOL gameControllersRequested = (DI8DEVCLASS_ALL == dwDevType || DI8DEVCLASS_GAMECTRL == dwDevType);
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
    callbackInfo.seenInstanceIdentifiers.clear();

    HRESULT enumResult = DI_OK;
    Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Starting to enumerate DirectInput devices."));

    // Enumerating game controllers requires some manipulation.
    if (gameControllersRequested)
    {
        // First scan the system for any XInput-compatible game controllers that match the enumeration request.
        if (underlyingDIObjectUsesUnicode)
            enumResult = underlyingDIObject.w->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumGameControllersXInputScanW, (LPVOID)&callbackInfo, dwFlags);
        else
            enumResult = underlyingDIObject.a->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumGameControllersXInputScanA, (LPVOID)&callbackInfo, dwFlags);
        
        if (DI_OK != enumResult) return enumResult;

        // Second, if the system has XInput controllers, enumerate them.
        // These will be the first controllers seen by the application.
        const BOOL systemHasXInputDevices = (0 != callbackInfo.seenInstanceIdentifiers.size());
        
        if (systemHasXInputDevices)
        {
            Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Enumerate: System has XInput devices, so Xidi virtual XInput devices are being presented to the application before other controllers."));
            
            if (underlyingDIObjectUsesUnicode)
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersW((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
            else
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersA((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);

            if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
            {
                Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Application has terminated enumeration."));
                return enumResult;
            }
        }

        // Third, enumerate all other game controllers, filtering out those that support XInput.
        if (underlyingDIObjectUsesUnicode)
            enumResult = underlyingDIObject.w->EnumDevices(gameControllerDevClass, &WrapperIDirectInput::CallbackEnumDevicesFilteredW, (LPVOID)&callbackInfo, dwFlags);
        else
            enumResult = underlyingDIObject.a->EnumDevices(gameControllerDevClass, &WrapperIDirectInput::CallbackEnumDevicesFilteredA, (LPVOID)&callbackInfo, dwFlags);
        
        if (DI_OK != enumResult) return enumResult;

        if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
        {
            Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Application has terminated enumeration."));
            return enumResult;
        }

        // Finally, if the system did not have any XInput controllers, enumerate them anyway.
        // These will be the last controllers seen by the application.
        if (!systemHasXInputDevices)
        {
            Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Enumerate: System has no XInput devices, so Xidi virtual XInput devices are being presented to the application after other controllers."));

            if (underlyingDIObjectUsesUnicode)
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersW((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
            else
                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllersA((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);

            if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
            {
                Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Application has terminated enumeration."));
                return enumResult;
            }
        }
    }

    // Enumerate anything else the application requested, filtering out game controllers.
    if (underlyingDIObjectUsesUnicode)
        enumResult = underlyingDIObject.w->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumDevicesFilteredW, (LPVOID)&callbackInfo, dwFlags);
    else
        enumResult = underlyingDIObject.a->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumDevicesFilteredA, (LPVOID)&callbackInfo, dwFlags);
    
    if (DI_OK != enumResult) return enumResult;

    if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
    {
        Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Application has terminated enumeration."));
        return enumResult;
    }
    
    Log::WriteLogMessage(ELogLevel::LogLevelDebug, _T("Finished enumerating DirectInput devices."));
    return enumResult;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance)
{
    if (underlyingDIObjectUsesUnicode)
        return underlyingDIObject.w->FindDevice(rguidClass, (LPCWSTR)ptszName, pguidInstance);
    else
        return underlyingDIObject.a->FindDevice(rguidClass, (LPCSTR)ptszName, pguidInstance);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::GetDeviceStatus(REFGUID rguidInstance)
{
    // Check if the specified instance GUID is an XInput GUID.
    LONG xinputIndex = ControllerIdentification::XInputControllerIndexForInstanceGUID(rguidInstance);

    if (-1 == xinputIndex)
    {
        // Not an XInput GUID, so ask the underlying implementation for status.
        if (underlyingDIObjectUsesUnicode)
            return underlyingDIObject.w->GetDeviceStatus(rguidInstance);
        else
            return underlyingDIObject.a->GetDeviceStatus(rguidInstance);
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
    if (underlyingDIObjectUsesUnicode)
        return underlyingDIObject.w->Initialize(hinst, dwVersion);
    else
        return underlyingDIObject.a->Initialize(hinst, dwVersion);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    if (underlyingDIObjectUsesUnicode)
        return underlyingDIObject.w->RunControlPanel(hwndOwner, dwFlags);
    else
        return underlyingDIObject.a->RunControlPanel(hwndOwner, dwFlags);
}


// -------- CALLBACKS: IDirectInput COMMON --------------------------------- //
// See "WrapperIDirectInput.h" for documentation.

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumGameControllersXInputScanA(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

    // If the present controller supports XInput, indicate such by adding it to the set of instance identifiers of interest.
    if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject.t, lpddi->guidInstance))
    {
        WCHAR productName[_countof(lpddi->tszProductName) + 1];
        ZeroMemory(productName, sizeof(productName));
        mbstowcs_s(NULL, productName, _countof(productName) - 1, lpddi->tszProductName, _countof(lpddi->tszProductName));
        
        callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Enumerate: DirectInput device \"%s\" supports XInput and will not be presented to the application."), productName);
    }

    return DIENUM_CONTINUE;
}

// --------

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumGameControllersXInputScanW(LPCDIDEVICEINSTANCEW lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

    // If the present controller supports XInput, indicate such by adding it to the set of instance identifiers of interest.
    if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject.t, lpddi->guidInstance))
    {
        LPCTSTR productName = lpddi->tszProductName;
        
        callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Enumerate: DirectInput device \"%s\" supports XInput and will not be presented to the application."), productName);
    }

    return DIENUM_CONTINUE;
}

// --------

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumDevicesFilteredA(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

    if (0 == callbackInfo->seenInstanceIdentifiers.count(lpddi->guidInstance))
    {
        // If the device has not been seen already, add it to the set and present it to the application.
        WCHAR productName[_countof(lpddi->tszProductName) + 1];
        ZeroMemory(productName, sizeof(productName));
        mbstowcs_s(NULL, productName, _countof(productName) - 1, lpddi->tszProductName, _countof(lpddi->tszProductName));
        
        callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
        callbackInfo->callbackReturnCode = ((LPDIENUMDEVICESCALLBACKA)(callbackInfo->lpCallback))(lpddi, callbackInfo->pvRef);
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Enumerate: DirectInput device \"%s\" is being presented to the application."), productName);
        return callbackInfo->callbackReturnCode;
    }
    else
    {
        // Otherwise, just skip the device and move onto the next one.
        return DIENUM_CONTINUE;
    }
}

// --------

BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumDevicesFilteredW(LPCDIDEVICEINSTANCEW lpddi, LPVOID pvRef)
{
    SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;
    
    if (0 == callbackInfo->seenInstanceIdentifiers.count(lpddi->guidInstance))
    {
        // If the device has not been seen already, add it to the set and present it to the application.
        LPCTSTR productName = lpddi->tszProductName;
        
        callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
        callbackInfo->callbackReturnCode = ((LPDIENUMDEVICESCALLBACKW)(callbackInfo->lpCallback))(lpddi, callbackInfo->pvRef);
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Enumerate: DirectInput device \"%s\" is being presented to the application."), productName);
        return callbackInfo->callbackReturnCode;
    }
    else
    {
        // Otherwise, just skip the device and move onto the next one.
        return DIENUM_CONTINUE;
    }
}


#if DIRECTINPUT_VERSION >= 0x0800
// -------- METHODS: IDirectInput8 ONLY ------------------------------------ //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInput::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
{
    if (underlyingDIObjectUsesUnicode)
        return underlyingDIObject.w->ConfigureDevices(lpdiCallback, (LPDICONFIGUREDEVICESPARAMSW)lpdiCDParams, dwFlags, pvRefData);
    else
        return underlyingDIObject.a->ConfigureDevices(lpdiCallback, (LPDICONFIGUREDEVICESPARAMSA)lpdiCDParams, dwFlags, pvRefData);
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
        {
            Log::WriteLogMessage(ELogLevel::LogLevelWarning, _T("CreateDeviceEx failed due to an invalid IID."));
            return E_INVALIDARG;
        }
    }
    else
    {
        if (!(IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDevice7A)))
        {
            Log::WriteLogMessage(ELogLevel::LogLevelWarning, _T("CreateDeviceEx failed due to an invalid IID."));
            return E_INVALIDARG;
        }
    }

    // Create a device the normal way.
    return CreateDevice(rguid, (EarliestIDirectInputDevice**)lplpDirectInputDevice, pUnkOuter);
    
}
#endif
