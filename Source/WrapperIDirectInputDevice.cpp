/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file WrapperIDirectInputDevice.cpp
 *   Implementation of the wrapper class for IDirectInputDevice.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "Log.h"
#include "WrapperIDirectInputDevice.h"
#include "Mapper/Base.h"

using namespace Xidi;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInputDevice.h" for documentation.

WrapperIDirectInputDevice::WrapperIDirectInputDevice(BOOL useUnicode, XInputController* controller, Mapper::Base* mapper) : controller(controller), mapper(mapper), polledSinceLastGetDeviceState(FALSE), refcount(0), useUnicode(useUnicode) {}

// ---------

WrapperIDirectInputDevice::~WrapperIDirectInputDevice(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XDI_WRAPPERIDIRECTINPUTDEVICE_DESTROYED_FORMAT, controller->GetPlayerIndex() + 1);
    delete controller;
    delete mapper;
}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT result = S_OK;

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

ULONG STDMETHODCALLTYPE WrapperIDirectInputDevice::AddRef(void)
{
    refcount += 1;
    return refcount;
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInputDevice::Release(void)
{
    ULONG numRemainingRefs = refcount - 1;

    if (0 == numRemainingRefs)
        delete this;
    else
        refcount = numRemainingRefs;

    return numRemainingRefs;
}


// -------- METHODS: IDirectInputDevice COMMON ----------------------------- //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::Acquire(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Can only acquire the device once the data format has been set.
    if (mapper->IsApplicationDataFormatSet())
        return controller->AcquireController();

    return DIERR_INVALIDPARAM;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::EnumEffectsInFile(LPCTSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    return mapper->EnumerateMappedObjects(useUnicode, lpCallback, pvRef, dwFlags);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::Escape(LPDIEFFESCAPE pesc)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    if (sizeof(*lpDIDevCaps) != lpDIDevCaps->dwSize)
        return DIERR_INVALIDPARAM;
    
    controller->FillDeviceCapabilities(lpDIDevCaps);
    mapper->FillDeviceCapabilities(lpDIDevCaps);

    return DI_OK;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Verify the correct sizes of each structure.
    if (sizeof(DIDEVICEOBJECTDATA) != cbObjectData)
        return DIERR_INVALIDPARAM;
    
    // Verify that the controller has been acquired.
    // This avoids allocating memory in the face of a known error case.
    if (!controller->IsAcquired())
        return DIERR_NOTACQUIRED;
    
    // Verify provided count. Cannot be NULL.
    if (NULL == pdwInOut)
        return DIERR_INVALIDPARAM;
    
    // Cause the mapper to read events from the controller and map them to application events.
    return mapper->WriteApplicationBufferedEvents(controller, rgdod, *pdwInOut, (0 != (dwFlags & DIGDD_PEEK)));
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetDeviceInfo(LPDIDEVICEINSTANCE pdidi)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Not yet implemented.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Handle games that forget to poll the device.
    // Don't bother buffering any changes, since this method has the effect of clearing the buffer anyway.
    if (FALSE == polledSinceLastGetDeviceState)
        controller->RefreshControllerState();

    polledSinceLastGetDeviceState = FALSE;
    
    // Get the current state from the controller.
    XINPUT_STATE currentControllerState;
    HRESULT result = controller->GetCurrentDeviceState(&currentControllerState);
    if (DI_OK != result)
        return result;

    // Submit the state to the mapper, which will in turn map XInput device state to application device state and fill in the application's data structure.
    return mapper->WriteApplicationControllerState(currentControllerState.Gamepad, lpvData, cbData);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetForceFeedbackState(LPDWORD pdwOut)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    return mapper->GetMappedObjectInfo(useUnicode, pdidoi, dwObj, dwHow);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    if (mapper->IsPropertyHandledByMapper(rguidProp))
        return mapper->GetMappedProperty(rguidProp, pdiph);
    else
        return controller->GetControllerProperty(rguidProp, pdiph);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not necessary.
    return S_FALSE;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::Poll(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    HRESULT refreshResult = controller->RefreshControllerState();

    if (S_OK == refreshResult)
        polledSinceLastGetDeviceState = TRUE;

    return refreshResult;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SendForceFeedbackCommand(DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Ineffective at present, but this may change.
    return DI_OK;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    HRESULT result = mapper->SetApplicationDataFormat(lpdf);

    if (S_OK == result)
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_DATA_FORMAT_ACCEPTED_FORMAT, controller->GetPlayerIndex() + 1);
    else
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_DATA_FORMAT_REJECTED_FORMAT, controller->GetPlayerIndex() + 1);

    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SetEventNotification(HANDLE hEvent)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    return controller->SetControllerStateChangedEvent(hEvent);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);

    if (mapper->IsPropertyHandledByMapper(rguidProp))
        return mapper->SetMappedProperty(rguidProp, pdiph);
    else
        return controller->SetControllerProperty(rguidProp, pdiph);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::Unacquire(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    return controller->UnacquireController();
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::WriteEffectToFile(LPCTSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}


#if DIRECTINPUT_VERSION >= 0x0800
// -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCTSTR lpszUserName, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice::SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lptszUserName, DWORD dwFlags)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERIDIRECTINPUTDEVICE_OPERATION_FORMAT, XIDI_LOG_FORMATTED_FUNCTION_NAME, controller->GetPlayerIndex() + 1);
    
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}
#endif
