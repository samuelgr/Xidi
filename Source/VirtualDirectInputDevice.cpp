/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file VirtualDirectInputDevice.cpp
 *   Implementation of a virtual device that supports IDirectInputDevice but
 *   communicates with an XInput-based controller.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "Message.h"
#include "VirtualDirectInputDevice.h"
#include "Mapper.h"


// -------- MACROS --------------------------------------------------------- //

/// Logs a DirectInput interface method invocation.
#define LOG_INVOCATION(lvl, player, result) Message::OutputFormatted(lvl, L"Invoked %s on XInput player %u, result = 0x%08x.", __FUNCTIONW__ L"()", player, result);

/// Produces and returns a human-readable string from a given DirectInput property GUID.
#define DI_PROPERTY_STRING(rguid, diprop) if (&diprop == &rguid) return _CRT_WIDE(#diprop);


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Compares the specified GUID with the known list of property GUIDs.
    /// Returns a string that represents the specified GUID.
    /// @param [in] pguid GUID to check.
    /// @return String representation of the GUID's semantics, even if unknown.
    static const wchar_t* StringFromPropertyUniqueIdentifier(REFGUID rguidProp)
    {
#if DIRECTINPUT_VERSION >= 0x0800
        DI_PROPERTY_STRING(rguidProp, DIPROP_KEYNAME);
        DI_PROPERTY_STRING(rguidProp, DIPROP_CPOINTS);
        DI_PROPERTY_STRING(rguidProp, DIPROP_APPDATA);
        DI_PROPERTY_STRING(rguidProp, DIPROP_SCANCODE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_VIDPID);
        DI_PROPERTY_STRING(rguidProp, DIPROP_USERNAME);
        DI_PROPERTY_STRING(rguidProp, DIPROP_TYPENAME);
#endif

        DI_PROPERTY_STRING(rguidProp, DIPROP_BUFFERSIZE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_AXISMODE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_GRANULARITY);
        DI_PROPERTY_STRING(rguidProp, DIPROP_RANGE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_DEADZONE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_SATURATION);
        DI_PROPERTY_STRING(rguidProp, DIPROP_FFGAIN);
        DI_PROPERTY_STRING(rguidProp, DIPROP_FFLOAD);
        DI_PROPERTY_STRING(rguidProp, DIPROP_AUTOCENTER);
        DI_PROPERTY_STRING(rguidProp, DIPROP_CALIBRATIONMODE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_CALIBRATION);
        DI_PROPERTY_STRING(rguidProp, DIPROP_GUIDANDPATH);
        DI_PROPERTY_STRING(rguidProp, DIPROP_INSTANCENAME);
        DI_PROPERTY_STRING(rguidProp, DIPROP_PRODUCTNAME);
        DI_PROPERTY_STRING(rguidProp, DIPROP_JOYSTICKID);
        DI_PROPERTY_STRING(rguidProp, DIPROP_GETPORTDISPLAYNAME);
        DI_PROPERTY_STRING(rguidProp, DIPROP_PHYSICALRANGE);
        DI_PROPERTY_STRING(rguidProp, DIPROP_LOGICALRANGE);

        return L"(unknown)";
    }


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "VirtualDirectInputDevice.h" for documentation.

    template <bool useUnicode> VirtualDirectInputDevice<useUnicode>::VirtualDirectInputDevice(XInputController* controller, Mapper* mapper) : controller(controller), mapper(mapper), polledSinceLastGetDeviceState(FALSE), refcount(1) {}

    // ---------

    template <bool useUnicode> VirtualDirectInputDevice<useUnicode>::~VirtualDirectInputDevice(void)
    {
        Message::OutputFormatted(Message::ESeverity::Info, L"Destroying controller object for XInput player %u.", controller->GetPlayerIndex() + 1);
        delete controller;
        delete mapper;
    }


    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        if (nullptr == ppvObj)
            return E_POINTER;

        bool validInterfaceRequested = false;

        if (true == useUnicode)
        {
#if DIRECTINPUT_VERSION >= 0x0800
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice8W))
#else
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice7W) || IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDeviceW))
#endif
                validInterfaceRequested = true;
        }
        else
        {
#if DIRECTINPUT_VERSION >= 0x0800
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice8A))
#else
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice7A) || IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDeviceA))
#endif
                validInterfaceRequested = true;
        }

        if (true == validInterfaceRequested)
        {
            AddRef();
            *ppvObj = this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    // ---------

    template <bool useUnicode> ULONG STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::AddRef(void)
    {
        InterlockedIncrement(&refcount);
        return refcount;
    }

    // ---------

    template <bool useUnicode> ULONG STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::Release(void)
    {
        ULONG numRemainingRefs = InterlockedDecrement(&refcount);

        if (0 == numRemainingRefs)
            delete this;

        return numRemainingRefs;
    }


    // -------- METHODS: IDirectInputDevice COMMON ------------------------- //
    // See DirectInput documentation for more information.

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::Acquire(void)
    {
        HRESULT result = DIERR_INVALIDPARAM;

        // Can only acquire the device once the data format has been set.
        if (mapper->IsApplicationDataFormatSet())
            result = controller->AcquireController();

        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;

        Message::Output(Message::ESeverity::Warning, L"Application attempted a force-feedback operation, which is not currently supported.");
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);

        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;

        Message::Output(Message::ESeverity::Warning, L"Application attempted a force-feedback operation, which is not currently supported.");
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);

        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::EnumEffects(DirectInputDeviceHelper<useUnicode>::EnumEffectsCallbackType lpCallback, LPVOID pvRef, DWORD dwEffType)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;

        Message::Output(Message::ESeverity::Warning, L"Application attempted a force-feedback operation, which is not currently supported.");
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);

        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::EnumEffectsInFile(DirectInputDeviceHelper<useUnicode>::ConstStringType lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;

        Message::Output(Message::ESeverity::Warning, L"Application attempted a force-feedback operation, which is not currently supported.");
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);

        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::EnumObjects(DirectInputDeviceHelper<useUnicode>::EnumObjectsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
        const HRESULT result = mapper->EnumerateMappedObjects(lpCallback, pvRef, dwFlags);
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::Escape(LPDIEFFESCAPE pesc)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
    {
        if (sizeof(*lpDIDevCaps) != lpDIDevCaps->dwSize)
        {
            const HRESULT result = DIERR_INVALIDPARAM;
            LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
            return result;
        }

        controller->FillDeviceCapabilities(lpDIDevCaps);
        mapper->FillDeviceCapabilities(lpDIDevCaps);

        const HRESULT result = DI_OK;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
    {
        // Verify the correct sizes of each structure.
        if (sizeof(DIDEVICEOBJECTDATA) != cbObjectData)
        {
            const HRESULT result = DIERR_INVALIDPARAM;
            LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
            return result;
        }

        // Verify that the controller has been acquired.
        // This avoids allocating memory in the face of a known error case.
        if (!controller->IsAcquired())
        {
            const HRESULT result = DIERR_NOTACQUIRED;
            LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
            return result;
        }

        // Verify provided count. Cannot be nullptr.
        if (nullptr == pdwInOut)
        {
            const HRESULT result = DIERR_INVALIDPARAM;
            LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
            return result;
        }

        // Cause the mapper to read events from the controller and map them to application events.
        const HRESULT result = mapper->WriteApplicationBufferedEvents(controller, rgdod, *pdwInOut, (0 != (dwFlags & DIGDD_PEEK)));
        LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetDeviceInfo(DirectInputDeviceHelper<useUnicode>::DeviceInstanceType pdidi)
    {
        // Not yet implemented.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetDeviceState(DWORD cbData, LPVOID lpvData)
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
            LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
            return result;
        }

        // Submit the state to the mapper, which will in turn map XInput device state to application device state and fill in the application's data structure.
        result = mapper->WriteApplicationControllerState(currentControllerState.Gamepad, lpvData, cbData);
        LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetEffectInfo(DirectInputDeviceHelper<useUnicode>::EffectInfoType pdei, REFGUID rguid)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetForceFeedbackState(LPDWORD pdwOut)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetObjectInfo(DirectInputDeviceHelper<useUnicode>::DeviceObjectInstanceType pdidoi, DWORD dwObj, DWORD dwHow)
    {
        const HRESULT result = mapper->GetMappedObjectInfo(pdidoi, dwObj, dwHow);
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
    {
        HRESULT result = DI_OK;

        Message::OutputFormatted(Message::ESeverity::Debug, L"Received a request to GET property %s on XInput player %u, handled by the %s.", StringFromPropertyUniqueIdentifier(rguidProp), controller->GetPlayerIndex() + 1, (mapper->IsPropertyHandledByMapper(rguidProp) ? L"MAPPER" : L"CONTROLLER"));

        if (mapper->IsPropertyHandledByMapper(rguidProp))
            result = mapper->GetMappedProperty(rguidProp, pdiph);
        else
            result = controller->GetControllerProperty(rguidProp, pdiph);

        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
    {
        // Operation not necessary.
        const HRESULT result = S_FALSE;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::Poll(void)
    {
        const HRESULT result = controller->RefreshControllerState();

        if (S_OK == result)
            polledSinceLastGetDeviceState = TRUE;

        LOG_INVOCATION(Message::ESeverity::SuperDebug, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SendForceFeedbackCommand(DWORD dwFlags)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
    {
        // Ineffective at present, but this may change.
        const HRESULT result = DI_OK;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SetDataFormat(LPCDIDATAFORMAT lpdf)
    {
        HRESULT result = mapper->SetApplicationDataFormat(lpdf);

        if (S_OK == result)
            Message::OutputFormatted(Message::ESeverity::Info, L"Accepted application-supplied data format for XInput player %u.", controller->GetPlayerIndex() + 1);
        else
            Message::OutputFormatted(Message::ESeverity::Error, L"Rejected application-supplied data format for XInput player %u.", controller->GetPlayerIndex() + 1);

        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SetEventNotification(HANDLE hEvent)
    {
        const HRESULT result = controller->SetControllerStateChangedEvent(hEvent);
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
    {
        HRESULT result = DI_OK;

        Message::OutputFormatted(Message::ESeverity::Debug, L"Received a request to SET property %s on XInput player %u, handled by the %s.", StringFromPropertyUniqueIdentifier(rguidProp), controller->GetPlayerIndex() + 1, (mapper->IsPropertyHandledByMapper(rguidProp) ? L"MAPPER" : L"CONTROLLER"));

        if (mapper->IsPropertyHandledByMapper(rguidProp))
            result = mapper->SetMappedProperty(rguidProp, pdiph);
        else
            result = controller->SetControllerProperty(rguidProp, pdiph);

        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::Unacquire(void)
    {
        const HRESULT result = controller->UnacquireController();
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::WriteEffectToFile(DirectInputDeviceHelper<useUnicode>::ConstStringType lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }


#if DIRECTINPUT_VERSION >= 0x0800
    // -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
    // See DirectInput documentation for more information.

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::BuildActionMap(DirectInputDeviceHelper<useUnicode>::ActionFormatType lpdiaf, DirectInputDeviceHelper<useUnicode>::ConstStringType lpszUserName, DWORD dwFlags)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::GetImageInfo(DirectInputDeviceHelper<useUnicode>::DeviceImageInfoHeaderType lpdiDevImageInfoHeader)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }

    // ---------

    template <bool useUnicode> HRESULT STDMETHODCALLTYPE VirtualDirectInputDevice<useUnicode>::SetActionMap(DirectInputDeviceHelper<useUnicode>::ActionFormatType lpdiActionFormat, DirectInputDeviceHelper<useUnicode>::ConstStringType lptszUserName, DWORD dwFlags)
    {
        // Operation not supported.
        const HRESULT result = DIERR_UNSUPPORTED;
        LOG_INVOCATION(Message::ESeverity::Info, controller->GetPlayerIndex() + 1, result);
        return result;
    }
#endif


    // -------- EXPLICIT TEMPLATE INSTANTIATION ---------------------------- //
    // Instantiates both the ASCII and Unicode versions of this class.

    template class VirtualDirectInputDevice<false>;
    template class VirtualDirectInputDevice<true>;
}
