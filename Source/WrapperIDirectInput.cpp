/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *   Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file WrapperIDirectInput.cpp
 *   Implementation of the wrapper class for IDirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "Mapper.h"
#include "Message.h"
#include "VirtualDirectInputDevice.h"
#include "WrapperIDirectInput.h"
#include "XInputController.h"

#include <cstdlib>
#include <unordered_set>


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Templated helper for printing product names during a device enumeration operation.
    /// @tparam DeviceInstanceType DirectInput device instance type, either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether or not Unicode is desired.
    /// @param [in] severity Desired message severity.
    /// @param [in] baseMessage Base message text, should contain a format specifier for the product name.
    /// @param [in] deviceInstance Device instance whose product name should be printed.
    template <typename DeviceInstanceType> static void EnumDevicesOutputProductName(Message::ESeverity severity, const wchar_t* baseMessage, const DeviceInstanceType* deviceInstance) {}

    template <> static void EnumDevicesOutputProductName<DIDEVICEINSTANCEA>(Message::ESeverity severity, const wchar_t* baseMessage, LPCDIDEVICEINSTANCEA deviceInstance)
    {
        WCHAR productName[_countof(deviceInstance->tszProductName) + 1];
        ZeroMemory(productName, sizeof(productName));
        mbstowcs_s(nullptr, productName, _countof(productName) - 1, deviceInstance->tszProductName, _countof(deviceInstance->tszProductName));
        Message::OutputFormatted(severity, baseMessage, productName);
    }

    template <> static void EnumDevicesOutputProductName<DIDEVICEINSTANCEW>(Message::ESeverity severity, const wchar_t* baseMessage, LPCDIDEVICEINSTANCEW deviceInstance)
    {
        LPCWSTR productName = deviceInstance->tszProductName;
        Message::OutputFormatted(severity, baseMessage, productName);
    }


    // -------- LOCAL TYPES ------------------------------------------------ //

    /// Contains all information required to intercept callbacks to EnumDevices.
    struct SEnumDevicesCallbackInfo
    {
        WrapperIDirectInput* instance;                                          ///< #WrapperIDirectInput instance that invoked the enumeration.
        LPDIENUMDEVICESCALLBACK lpCallback;                                     ///< Application-specified callback that should be invoked.
        LPVOID pvRef;                                                           ///< Application-specified argument to be provided to the application-specified callback.
        BOOL callbackReturnCode;                                                ///< Indicates if the application requested that enumeration continue or stop.
        std::unordered_set<GUID> seenInstanceIdentifiers;                       ///< Holds device identifiers seen during device enumeration.
    };


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "WrapperIDirectInput.h" for documentation.

    WrapperIDirectInput::WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode) : underlyingDIObjectUsesUnicode(underlyingDIObjectUsesUnicode)
    {
        if (underlyingDIObjectUsesUnicode)
            this->underlyingDIObject.a = (LatestIDirectInputA*)underlyingDIObject;
        else
            this->underlyingDIObject.w = (LatestIDirectInputW*)underlyingDIObject;
    }


    // -------- METHODS: IUnknown ------------------------------------------ //
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


    // -------- METHODS: IDirectInput COMMON ------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT STDMETHODCALLTYPE WrapperIDirectInput::CreateDevice(REFGUID rguid, EarliestIDirectInputDevice** lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
    {
        // Check if the specified instance GUID is an XInput GUID.
        LONG xinputIndex = ControllerIdentification::XInputControllerIndexForInstanceGUID(rguid);

        if (-1 == xinputIndex)
        {
            // Not an XInput GUID, so just create the device as requested by the application.
            Message::Output(Message::ESeverity::Info, L"Binding to a non-XInput device. Xidi will not handle communication with it.");

            if (underlyingDIObjectUsesUnicode)
                return underlyingDIObject.w->CreateDevice(rguid, (EarliestIDirectInputDeviceW**)lplpDirectInputDevice, pUnkOuter);
            else
                return underlyingDIObject.a->CreateDevice(rguid, (EarliestIDirectInputDeviceA**)lplpDirectInputDevice, pUnkOuter);
        }
        else
        {
            // Is an XInput GUID, so create a fake device that will communicate with the XInput controller of the specified index.
            Message::OutputFormatted(Message::ESeverity::Info, L"Binding to Xidi virtual XInput device for player %u.", (xinputIndex + 1));

            VirtualDirectInputDevice* newWrappedDevice = new VirtualDirectInputDevice(underlyingDIObjectUsesUnicode, new XInputController(xinputIndex), Mapper::Create());
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
        Message::Output(Message::ESeverity::Debug, L"Starting to enumerate DirectInput devices.");

        // Enumerating game controllers requires some manipulation.
        if (gameControllersRequested)
        {
            // First scan the system for any XInput-compatible game controllers that match the enumeration request.
            if (underlyingDIObjectUsesUnicode)
                enumResult = underlyingDIObject.w->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumGameControllersXInputScan<DIDEVICEINSTANCEW>, (LPVOID)&callbackInfo, dwFlags);
            else
                enumResult = underlyingDIObject.a->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumGameControllersXInputScan<DIDEVICEINSTANCEA>, (LPVOID)&callbackInfo, dwFlags);

            if (DI_OK != enumResult) return enumResult;

            // Second, if the system has XInput controllers, enumerate them.
            // These will be the first controllers seen by the application.
            const BOOL systemHasXInputDevices = (0 != callbackInfo.seenInstanceIdentifiers.size());

            if (systemHasXInputDevices)
            {
                Message::Output(Message::ESeverity::Debug, L"Enumerate: System has XInput devices, so Xidi virtual XInput devices are being presented to the application before other controllers.");

                if (underlyingDIObjectUsesUnicode)
                    callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllers((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
                else
                    callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllers((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);

                if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
                {
                    Message::Output(Message::ESeverity::Debug, L"Application has terminated enumeration.");
                    return enumResult;
                }
            }

            // Third, enumerate all other game controllers, filtering out those that support XInput.
            if (underlyingDIObjectUsesUnicode)
                enumResult = underlyingDIObject.w->EnumDevices(gameControllerDevClass, &WrapperIDirectInput::CallbackEnumDevicesFiltered<DIDEVICEINSTANCEW>, (LPVOID)&callbackInfo, dwFlags);
            else
                enumResult = underlyingDIObject.a->EnumDevices(gameControllerDevClass, &WrapperIDirectInput::CallbackEnumDevicesFiltered<DIDEVICEINSTANCEA>, (LPVOID)&callbackInfo, dwFlags);

            if (DI_OK != enumResult) return enumResult;

            if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
            {
                Message::Output(Message::ESeverity::Debug, L"Application has terminated enumeration.");
                return enumResult;
            }

            // Finally, if the system did not have any XInput controllers, enumerate them anyway.
            // These will be the last controllers seen by the application.
            if (!systemHasXInputDevices)
            {
                Message::Output(Message::ESeverity::Debug, L"Enumerate: System has no XInput devices, so Xidi virtual XInput devices are being presented to the application after other controllers.");

                if (underlyingDIObjectUsesUnicode)
                    callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllers((LPDIENUMDEVICESCALLBACKW)lpCallback, pvRef);
                else
                    callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllers((LPDIENUMDEVICESCALLBACKA)lpCallback, pvRef);

                if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
                {
                    Message::Output(Message::ESeverity::Debug, L"Application has terminated enumeration.");
                    return enumResult;
                }
            }
        }

        // Enumerate anything else the application requested, filtering out game controllers.
        if (underlyingDIObjectUsesUnicode)
            enumResult = underlyingDIObject.w->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumDevicesFiltered<DIDEVICEINSTANCEW>, (LPVOID)&callbackInfo, dwFlags);
        else
            enumResult = underlyingDIObject.a->EnumDevices(dwDevType, &WrapperIDirectInput::CallbackEnumDevicesFiltered<DIDEVICEINSTANCEA>, (LPVOID)&callbackInfo, dwFlags);

        if (DI_OK != enumResult) return enumResult;

        if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
        {
            Message::Output(Message::ESeverity::Debug, L"Application has terminated enumeration.");
            return enumResult;
        }

        Message::Output(Message::ESeverity::Debug, L"Finished enumerating DirectInput devices.");
        return enumResult;
    }

    // ---------

    HRESULT STDMETHODCALLTYPE WrapperIDirectInput::FindDevice(REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance)
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


    // -------- CALLBACKS: IDirectInput COMMON ----------------------------- //
    // See "WrapperIDirectInput.h" for documentation.

    template <typename DeviceInstanceType> BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumGameControllersXInputScan(const DeviceInstanceType* lpddi, LPVOID pvRef)
    {
        SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

        // If the present controller supports XInput, indicate such by adding it to the set of instance identifiers of interest.
        if (ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->instance->underlyingDIObject.t, lpddi->guidInstance))
        {
            callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
            EnumDevicesOutputProductName(Message::ESeverity::Debug, L"Enumerate: DirectInput device \"%s\" supports XInput and will not be presented to the application.", lpddi);
        }

        return DIENUM_CONTINUE;
    }

    // --------

    template <typename DeviceInstanceType> BOOL STDMETHODCALLTYPE WrapperIDirectInput::CallbackEnumDevicesFiltered(const DeviceInstanceType* lpddi, LPVOID pvRef)
    {
        SEnumDevicesCallbackInfo* callbackInfo = (SEnumDevicesCallbackInfo*)pvRef;

        if (0 == callbackInfo->seenInstanceIdentifiers.count(lpddi->guidInstance))
        {
            // If the device has not been seen already, add it to the set and present it to the application.
            callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
            callbackInfo->callbackReturnCode = ((BOOL(FAR PASCAL *)(const DeviceInstanceType*,LPVOID))(callbackInfo->lpCallback))(lpddi, callbackInfo->pvRef);
            EnumDevicesOutputProductName(Message::ESeverity::Debug, L"Enumerate: DirectInput device \"%s\" is being presented to the application.", lpddi);
            return callbackInfo->callbackReturnCode;
        }
        else
        {
            // Otherwise, just skip the device and move onto the next one.
            return DIENUM_CONTINUE;
        }
    }


#if DIRECTINPUT_VERSION >= 0x0800
    // -------- METHODS: IDirectInput8 ONLY -------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT STDMETHODCALLTYPE WrapperIDirectInput::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
    {
        if (underlyingDIObjectUsesUnicode)
            return underlyingDIObject.w->ConfigureDevices(lpdiCallback, (LPDICONFIGUREDEVICESPARAMSW)lpdiCDParams, dwFlags, pvRefData);
        else
            return underlyingDIObject.a->ConfigureDevices(lpdiCallback, (LPDICONFIGUREDEVICESPARAMSA)lpdiCDParams, dwFlags, pvRefData);
    }

    // ---------

    HRESULT STDMETHODCALLTYPE WrapperIDirectInput::EnumDevicesBySemantics(LPCWSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
        // Operation not supported.
        return DIERR_UNSUPPORTED;
    }
#else
    // -------- METHODS: IDirectInput LEGACY ------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT STDMETHODCALLTYPE WrapperIDirectInput::CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
    {
        // Make sure the supplied IID is valid.
        if (underlyingDIObjectUsesUnicode)
        {
            if (!(IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDevice7W)))
            {
                Message::Output(Message::ESeverity::Warning, L"CreateDeviceEx failed due to an invalid IID.");
                return E_INVALIDARG;
            }
        }
        else
        {
            if (!(IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDevice7A)))
            {
                Message::Output(Message::ESeverity::Warning, L"CreateDeviceEx failed due to an invalid IID.");
                return E_INVALIDARG;
            }
        }

        // Create a device the normal way.
        return CreateDevice(rguid, (EarliestIDirectInputDevice**)lplpDirectInputDevice, pUnkOuter);

    }
#endif
}
