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


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "VirtualDirectInputDevice.h" for documentation.

VirtualDirectInputDevice::VirtualDirectInputDevice(BOOL useUnicode, XInputController* controller, Mapper::Base* mapper) : controller(controller), mapper(mapper), polledSinceLastGetDeviceState(FALSE), refcount(0), useUnicode(useUnicode) {}

// ---------

VirtualDirectInputDevice::~VirtualDirectInputDevice(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XDI_WRAPPERIDIRECTINPUTDEVICE_DESTROYED_FORMAT, controller->GetPlayerIndex() + 1);
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

    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_FORCE_FEEDBACK_OPERATION_UNSUPPORTED);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_FORCE_FEEDBACK_OPERATION_UNSUPPORTED);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);

    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_FORCE_FEEDBACK_OPERATION_UNSUPPORTED);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumEffectsInFile(LPCTSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;

    Log::WriteLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_FORCE_FEEDBACK_OPERATION_UNSUPPORTED);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    const HRESULT result = mapper->EnumerateMappedObjects(useUnicode, lpCallback, pvRef, dwFlags);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Escape(LPDIEFFESCAPE pesc)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    if (sizeof(*lpDIDevCaps) != lpDIDevCaps->dwSize)
    {
        const HRESULT result = DIERR_INVALIDPARAM;
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    controller->FillDeviceCapabilities(lpDIDevCaps);
    mapper->FillDeviceCapabilities(lpDIDevCaps);

    const HRESULT result = DI_OK;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    // Verify the correct sizes of each structure.
    if (sizeof(DIDEVICEOBJECTDATA) != cbObjectData)
    {
        const HRESULT result = DIERR_INVALIDPARAM;
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    // Verify that the controller has been acquired.
    // This avoids allocating memory in the face of a known error case.
    if (!controller->IsAcquired())
    {
        const HRESULT result = DIERR_NOTACQUIRED;
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    // Verify provided count. Cannot be NULL.
    if (NULL == pdwInOut)
    {
        const HRESULT result = DIERR_INVALIDPARAM;
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
        return result;
    }
    
    // Cause the mapper to read events from the controller and map them to application events.
    const HRESULT result = mapper->WriteApplicationBufferedEvents(controller, rgdod, *pdwInOut, (0 != (dwFlags & DIGDD_PEEK)));
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetDeviceInfo(LPDIDEVICEINSTANCE pdidi)
{
    // Not yet implemented.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
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
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // Submit the state to the mapper, which will in turn map XInput device state to application device state and fill in the application's data structure.
    result = mapper->WriteApplicationControllerState(currentControllerState.Gamepad, lpvData, cbData);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetForceFeedbackState(LPDWORD pdwOut)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
    const HRESULT result = mapper->GetMappedObjectInfo(useUnicode, pdidoi, dwObj, dwHow);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    HRESULT result = DI_OK;

    if (mapper->IsPropertyHandledByMapper(rguidProp))
        result = mapper->GetMappedProperty(rguidProp, pdiph);
    else
        result = controller->GetControllerProperty(rguidProp, pdiph);

    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
    // Operation not necessary.
    const HRESULT result = S_FALSE;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Poll(void)
{
    const HRESULT result = controller->RefreshControllerState();

    if (S_OK == result)
        polledSinceLastGetDeviceState = TRUE;

    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SendForceFeedbackCommand(DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    // Ineffective at present, but this may change.
    const HRESULT result = DI_OK;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
    HRESULT result = mapper->SetApplicationDataFormat(lpdf);

    if (S_OK == result)
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_DATA_FORMAT_ACCEPTED_FORMAT, controller->GetPlayerIndex() + 1);
    else
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_DATA_FORMAT_REJECTED_FORMAT, controller->GetPlayerIndex() + 1);

    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetEventNotification(HANDLE hEvent)
{
    const HRESULT result = controller->SetControllerStateChangedEvent(hEvent);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    HRESULT result = DI_OK;
    
    if (mapper->IsPropertyHandledByMapper(rguidProp))
        result = mapper->SetMappedProperty(rguidProp, pdiph);
    else
        result = controller->SetControllerProperty(rguidProp, pdiph);

    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::Unacquire(void)
{
    const HRESULT result = controller->UnacquireController();
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::WriteEffectToFile(LPCTSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}


#if DIRECTINPUT_VERSION >= 0x0800
// -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCTSTR lpszUserName, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice::SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lptszUserName, DWORD dwFlags)
{
    // Operation not supported.
    const HRESULT result = DIERR_UNSUPPORTED;
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1, result);
    return result;
}
#endif