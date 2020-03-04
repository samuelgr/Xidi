/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file VirtualDirectInputDevice.cpp
 *   Implementation of a virtual device that supports IDirectInputDevice but
 *   communicates with an XInput-based controller.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "Log.h"
#include "VirtualDirectInputDevice.h"
#include "Mapper/Base.h"

using namespace Xidi;



// -------- MACROS --------------------------------------------------------- //

/// Logs a DirectInput interface method invocation.
#define LOG_INVOCATION(player, result)      Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, _T("Invoked %s on XInput player %u, result = 0x%08x."), XIDI_LOG_FORMATTED_FUNCTION_NAME, player, result);


// -------- INTERNAL FUNCTIONS --------------------------------------------- //

/// Compares the specified GUID with the known list of property GUIDs.
/// Returns a string that represents the specified GUID.
/// @param [in] pguid GUID to check.
/// @return String representation of the GUID's semantics, even if unknown.
static const TCHAR* StringFromPropertyUniqueIdentifier(REFGUID rguidProp)
{
#if DIRECTINPUT_VERSION >= 0x0800
    if (&DIPROP_APPDATA == &rguidProp)
        return _T("DIPROP_APPDATA");
    if (&DIPROP_CPOINTS == &rguidProp)
        return _T("DIPROP_CPOINTS");
    if (&DIPROP_KEYNAME == &rguidProp)
        return _T("DIPROP_KEYNAME");
    if (&DIPROP_SCANCODE == &rguidProp)
        return _T("DIPROP_SCANCODE");
    if (&DIPROP_TYPENAME == &rguidProp)
        return _T("DIPROP_TYPENAME");
    if (&DIPROP_USERNAME == &rguidProp)
        return _T("DIPROP_USERNAME");
    if (&DIPROP_VIDPID == &rguidProp)
        return _T("DIPROP_VIDPID");
#endif

    if (&DIPROP_AUTOCENTER == &rguidProp)
        return _T("DIPROP_AUTOCENTER");
    if (&DIPROP_AXISMODE == &rguidProp)
        return _T("DIPROP_AXISMODE");
    if (&DIPROP_BUFFERSIZE == &rguidProp)
        return _T("DIPROP_BUFFERSIZE");
    if (&DIPROP_CALIBRATION == &rguidProp)
        return _T("DIPROP_CALIBRATION");
    if (&DIPROP_CALIBRATIONMODE == &rguidProp)
        return _T("DIPROP_CALIBRATIONMODE");
    if (&DIPROP_DEADZONE == &rguidProp)
        return _T("DIPROP_DEADZONE");
    if (&DIPROP_FFGAIN == &rguidProp)
        return _T("DIPROP_FFGAIN");
    if (&DIPROP_INSTANCENAME == &rguidProp)
        return _T("DIPROP_INSTANCENAME");
    if (&DIPROP_PRODUCTNAME == &rguidProp)
        return _T("DIPROP_PRODUCTNAME");
    if (&DIPROP_RANGE == &rguidProp)
        return _T("DIPROP_RANGE");
    if (&DIPROP_SATURATION == &rguidProp)
        return _T("DIPROP_SATURATION");

    return _T("(unknown)");
}


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "VirtualDirectInputDevice.h" for documentation.

VirtualDirectInputDevice::VirtualDirectInputDevice(BOOL useUnicode, XInputController* controller, Mapper::Base* mapper) : controller(controller), mapper(mapper), polledSinceLastGetDeviceState(FALSE), refcount(0), useUnicode(useUnicode) {}

// ---------

VirtualDirectInputDevice::~VirtualDirectInputDevice(void)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, _T("Destroying controller object for XInput player %u."), controller->GetPlayerIndex() + 1);
    delete controller;
    delete mapper;
}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if (NULL == ppvObj)
        return E_INVALIDARG;
    
    HRESULT result = S_OK;
    *ppvObj = NULL;

#if DIRECTINPUT_VERSION >= 0x0800
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice8A) || IsEqualIID(riid, IID_IDirectInputDevice8W))
#else
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice7A) || IsEqualIID(riid, IID_IDirectInputDevice7W) || IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDeviceA) || IsEqualIID(riid, IID_IDirectInputDeviceW))
#endif
    {
        AddRef();
        *ppvObj = this;
    }
    else
        result = E_NOINTERFACE;

    return result;
}

// ---------

ULONG STDMETHODCALLTYPE VirtualDirectInputDevice::AddRef(void)
{
    InterlockedIncrement(&refcount);
    return refcount;
}

// ---------

ULONG STDMETHODCALLTYPE VirtualDirectInputDevice::Release(void)
{
    ULONG numRemainingRefs = InterlockedDecrement(&refcount);

    if (0 == numRemainingRefs)
        delete this;

    return numRemainingRefs;
}


// -------- METHODS: IDirectInputDevice COMMON ----------------------------- //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Acquire(void)
{
    HRESULT result = DIERR_INVALIDPARAM;
    
    // Can only acquire the device once the data format has been set.
    if (mapper->IsApplicationDataFormatSet())
        result = controller->AcquireController();

    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessage(ELogLevel::LogLevelWarning, _T("Application attempted a force-feedback operation, which is not currently supported."));
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessage(ELogLevel::LogLevelWarning, _T("Application attempted a force-feedback operation, which is not currently supported."));
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);

    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessage(ELogLevel::LogLevelWarning, _T("Application attempted a force-feedback operation, which is not currently supported."));
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumEffectsInFile(LPCTSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessage(ELogLevel::LogLevelWarning, _T("Application attempted a force-feedback operation, which is not currently supported."));
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    const HRESULT result = mapper->EnumerateMappedObjects(useUnicode, lpCallback, pvRef, dwFlags);
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Escape(LPDIEFFESCAPE pesc)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    if (sizeof(*lpDIDevCaps) != lpDIDevCaps->dwSize)
    {
        const HRESULT result = DIERR_INVALIDPARAM;
        LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    controller->FillDeviceCapabilities(lpDIDevCaps);
    mapper->FillDeviceCapabilities(lpDIDevCaps);

    const HRESULT result = DI_OK;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    // Verify the correct sizes of each structure.
    if (sizeof(DIDEVICEOBJECTDATA) != cbObjectData)
    {
        const HRESULT result = DIERR_INVALIDPARAM;
        LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    // Verify that the controller has been acquired.
    // This avoids allocating memory in the face of a known error case.
    if (!controller->IsAcquired())
    {
        const HRESULT result = DIERR_NOTACQUIRED;
        LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    // Verify provided count. Cannot be NULL.
    if (NULL == pdwInOut)
    {
        const HRESULT result = DIERR_INVALIDPARAM;
        LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    // Cause the mapper to read events from the controller and map them to application events.
    const HRESULT result = mapper->WriteApplicationBufferedEvents(controller, rgdod, *pdwInOut, (0 != (dwFlags & DIGDD_PEEK)));
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetDeviceInfo(LPDIDEVICEINSTANCE pdidi)
{
    // Not yet implemented.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
    // Handle games that forget to poll the device.
    // Don't bother buffering any changes, since this method has the effect of clearing the buffer anyway.
    if (FALSE == polledSinceLastGetDeviceState)
        controller->RefreshControllerState();

    polledSinceLastGetDeviceState = FALSE;
    
    // Get the current state from the controller.
    XINPUT_STATE currentControllerState;
    HRESULT result = controller->GetCurrentDeviceState(&currentControllerState);
    if (DI_OK != result)
    {
        LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // Submit the state to the mapper, which will in turn map XInput device state to application device state and fill in the application's data structure.
    result = mapper->WriteApplicationControllerState(currentControllerState.Gamepad, lpvData, cbData);
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetForceFeedbackState(LPDWORD pdwOut)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
    const HRESULT result = mapper->GetMappedObjectInfo(useUnicode, pdidoi, dwObj, dwHow);
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    HRESULT result = DI_OK;

    Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Received a request to GET property %s, handled by the %s."), StringFromPropertyUniqueIdentifier(rguidProp), (mapper->IsPropertyHandledByMapper(rguidProp) ? _T("MAPPER") : _T("CONTROLLER")));

    if (mapper->IsPropertyHandledByMapper(rguidProp))
        result = mapper->GetMappedProperty(rguidProp, pdiph);
    else
        result = controller->GetControllerProperty(rguidProp, pdiph);

    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
    // Operation not necessary.
    const HRESULT result = S_FALSE;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Poll(void)
{
    const HRESULT result = controller->RefreshControllerState();

    if (S_OK == result)
        polledSinceLastGetDeviceState = TRUE;

    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SendForceFeedbackCommand(DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    // Ineffective at present, but this may change.
    const HRESULT result = DI_OK;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
    HRESULT result = mapper->SetApplicationDataFormat(lpdf);

    if (S_OK == result)
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, _T("Accepted application-supplied data format for XInput player %u."), controller->GetPlayerIndex() + 1);
    else
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, _T("Rejected application-supplied data format for XInput player %u."), controller->GetPlayerIndex() + 1);

    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetEventNotification(HANDLE hEvent)
{
    const HRESULT result = controller->SetControllerStateChangedEvent(hEvent);
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    HRESULT result = DI_OK;

    Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Received a request to SET property %s, handled by the %s."), StringFromPropertyUniqueIdentifier(rguidProp), (mapper->IsPropertyHandledByMapper(rguidProp) ? _T("MAPPER") : _T("CONTROLLER")));
    
    if (mapper->IsPropertyHandledByMapper(rguidProp))
        result = mapper->SetMappedProperty(rguidProp, pdiph);
    else
        result = controller->SetControllerProperty(rguidProp, pdiph);

    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Unacquire(void)
{
    const HRESULT result = controller->UnacquireController();
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::WriteEffectToFile(LPCTSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}


#if DIRECTINPUT_VERSION >= 0x0800
// -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCTSTR lpszUserName, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lptszUserName, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    LOG_INVOCATION(controller->GetPlayerIndex() + 1, result);
    return result;
}
#endif
