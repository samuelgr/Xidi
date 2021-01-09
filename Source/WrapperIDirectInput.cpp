/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *   Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file WrapperIDirectInput.cpp
 *   Implementation of the wrapper class for IDirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "Mapper.h"
#include "Message.h"
#include "VirtualController.h"
#include "VirtualDirectInputDevice.h"
#include "WrapperIDirectInput.h"

#include <cstdlib>
#include <unordered_set>


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Templated helper for printing product names during a device enumeration operation.
    /// @tparam charMode Specifies whether to use underlying Unicode or not.
    /// @param [in] severity Desired message severity.
    /// @param [in] baseMessage Base message text, should contain a format specifier for the product name.
    /// @param [in] deviceInstance Device instance whose product name should be printed.
    template <ECharMode charMode> static void EnumDevicesOutputProductName(Message::ESeverity severity, const wchar_t* baseMessage, const typename DirectInputType<charMode>::DeviceInstanceType* deviceInstance) {}

    template <> static void EnumDevicesOutputProductName<ECharMode::A>(Message::ESeverity severity, const wchar_t* baseMessage, typename const DirectInputType<ECharMode::A>::DeviceInstanceType* deviceInstance)
    {
        WCHAR productName[_countof(deviceInstance->tszProductName) + 1];
        ZeroMemory(productName, sizeof(productName));
        mbstowcs_s(nullptr, productName, _countof(productName) - 1, deviceInstance->tszProductName, _countof(deviceInstance->tszProductName));
        Message::OutputFormatted(severity, baseMessage, productName);
    }

    template <> static void EnumDevicesOutputProductName<ECharMode::W>(Message::ESeverity severity, const wchar_t* baseMessage, const typename DirectInputType<ECharMode::W>::DeviceInstanceType* deviceInstance)
    {
        LPCWSTR productName = deviceInstance->tszProductName;
        Message::OutputFormatted(severity, baseMessage, productName);
    }


    // -------- LOCAL TYPES ------------------------------------------------ //

    /// Contains all information required to intercept callbacks to EnumDevices.
    template <ECharMode charMode> struct SEnumDevicesCallbackInfo
    {
        WrapperIDirectInput<charMode>* instance;                                      ///< #WrapperIDirectInput instance that invoked the enumeration.
        typename DirectInputType<charMode>::EnumDevicesCallbackType lpCallback;     ///< Application-specified callback that should be invoked.
        LPVOID pvRef;                                                                   ///< Application-specified argument to be provided to the application-specified callback.
        BOOL callbackReturnCode;                                                        ///< Indicates if the application requested that enumeration continue or stop.
        std::unordered_set<GUID> seenInstanceIdentifiers;                               ///< Holds device identifiers seen during device enumeration.
    };


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "WrapperIDirectInput.h" for documentation.

    template <ECharMode charMode> WrapperIDirectInput<charMode>::WrapperIDirectInput(DirectInputType<charMode>::LatestIDirectInputType* underlyingDIObject) : underlyingDIObject(underlyingDIObject) {}


    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        void* interfacePtr = nullptr;
        const HRESULT result = underlyingDIObject->QueryInterface(riid, &interfacePtr);

        if (S_OK == result)
        {
            bool shouldWrapInterface = false;

            if (ECharMode::W == charMode)
            {
#if DIRECTINPUT_VERSION >= 0x0800
                if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput8W))
#else
                if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput7W) || IsEqualIID(riid, IID_IDirectInput2W) || IsEqualIID(riid, IID_IDirectInputW))
#endif
                {
                    shouldWrapInterface = true;
                }
            }
            else
            {
#if DIRECTINPUT_VERSION >= 0x0800
                if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput8A))
#else
                if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInput7A) || IsEqualIID(riid, IID_IDirectInput2A) || IsEqualIID(riid, IID_IDirectInputA))
#endif
                {
                    shouldWrapInterface = true;
                }
            }

            if (true == shouldWrapInterface)
                *ppvObj = this;
            else
                *ppvObj = interfacePtr;
        }

        return result;
    }

    // ---------

    template <ECharMode charMode> ULONG STDMETHODCALLTYPE WrapperIDirectInput<charMode>::AddRef(void)
    {
        return underlyingDIObject->AddRef();
    }

    // ---------

    template <ECharMode charMode> ULONG STDMETHODCALLTYPE WrapperIDirectInput<charMode>::Release(void)
    {
        const ULONG numRemainingRefs = underlyingDIObject->Release();

        if (0 == numRemainingRefs)
            delete this;

        return numRemainingRefs;
    }


    // -------- METHODS: IDirectInput COMMON ------------------------------- //
    // See DirectInput documentation for more information.

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::CreateDevice(REFGUID rguid, DirectInputType<charMode>::EarliestIDirectInputDeviceType** lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
    {
        // Check if the specified instance GUID is an XInput GUID.
        LONG xinputIndex = ControllerIdentification::XInputControllerIndexForInstanceGUID(rguid);

        if (-1 == xinputIndex)
        {
            // Not an XInput GUID, so just create the device as requested by the application.
            Message::Output(Message::ESeverity::Info, L"Binding to a non-XInput device. Xidi will not handle communication with it.");

            return underlyingDIObject->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
        }
        else
        {
            // Is an XInput GUID, so create a virtual controller wrapped with a DirectInput interface..
            Message::OutputFormatted(Message::ESeverity::Info, L"Binding to Xidi virtual controller %u.", (xinputIndex + 1));

            if (nullptr != pUnkOuter)
                Message::Output(Message::ESeverity::Warning, L"Application requested COM aggregation, which is not implemented, while binding to a Xidi virtual device.");

            const Controller::Mapper* mapper = Controller::Mapper::GetConfigured();
            if (nullptr == mapper)
            {
                Message::Output(Message::ESeverity::Error, L"Failed to create a Xidi virtual controller because no mapper could be located.");
                return DIERR_NOINTERFACE;
            }
            
            *lplpDirectInputDevice = new VirtualDirectInputDevice<charMode>(std::make_unique<Controller::VirtualController>(xinputIndex, *mapper));
            return DI_OK;
        }
    }

    // ---------

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::EnumDevices(DWORD dwDevType, DirectInputType<charMode>::EnumDevicesCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
#if DIRECTINPUT_VERSION >= 0x0800
        const BOOL gameControllersRequested = (DI8DEVCLASS_ALL == dwDevType || DI8DEVCLASS_GAMECTRL == dwDevType);
        const DWORD gameControllerDevClass = DI8DEVCLASS_GAMECTRL;
#else
        const BOOL gameControllersRequested = (0 == dwDevType || DIDEVTYPE_JOYSTICK == dwDevType);
        const DWORD gameControllerDevClass = DIDEVTYPE_JOYSTICK;
#endif

        SEnumDevicesCallbackInfo<charMode> callbackInfo;
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
            enumResult = underlyingDIObject->EnumDevices(dwDevType, &WrapperIDirectInput<charMode>::CallbackEnumGameControllersXInputScan, (LPVOID)&callbackInfo, dwFlags);
            if (DI_OK != enumResult) return enumResult;

            // Second, if the system has XInput controllers, enumerate them.
            // These will be the first controllers seen by the application.
            const BOOL systemHasXInputDevices = (0 != callbackInfo.seenInstanceIdentifiers.size());

            if (systemHasXInputDevices)
            {
                Message::Output(Message::ESeverity::Debug, L"Enumerate: System has XInput devices, so Xidi virtual XInput devices are being presented to the application before other controllers.");

                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllers(lpCallback, pvRef);

                if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
                {
                    Message::Output(Message::ESeverity::Debug, L"Application has terminated enumeration.");
                    return enumResult;
                }
            }

            // Third, enumerate all other game controllers, filtering out those that support XInput.
            enumResult = underlyingDIObject->EnumDevices(gameControllerDevClass, &CallbackEnumDevicesFiltered, (LPVOID)&callbackInfo, dwFlags);

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

                callbackInfo.callbackReturnCode = ControllerIdentification::EnumerateXInputControllers(lpCallback, pvRef);

                if (DIENUM_CONTINUE != callbackInfo.callbackReturnCode)
                {
                    Message::Output(Message::ESeverity::Debug, L"Application has terminated enumeration.");
                    return enumResult;
                }
            }
        }

        // Enumerate anything else the application requested, filtering out game controllers.
        enumResult = underlyingDIObject->EnumDevices(dwDevType, &CallbackEnumDevicesFiltered, (LPVOID)&callbackInfo, dwFlags);

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

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::FindDevice(REFGUID rguidClass, DirectInputType<charMode>::ConstStringType ptszName, LPGUID pguidInstance)
    {
        return underlyingDIObject->FindDevice(rguidClass, ptszName, pguidInstance);
    }

    // ---------

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::GetDeviceStatus(REFGUID rguidInstance)
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

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::Initialize(HINSTANCE hinst, DWORD dwVersion)
    {
        return underlyingDIObject->Initialize(hinst, dwVersion);
    }

    // ---------

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
    {
        return underlyingDIObject->RunControlPanel(hwndOwner, dwFlags);
    }


    // -------- CALLBACKS: IDirectInput COMMON ----------------------------- //
    // See "WrapperIDirectInput.h" for documentation.

    template <ECharMode charMode> BOOL STDMETHODCALLTYPE WrapperIDirectInput<charMode>::CallbackEnumGameControllersXInputScan(const DirectInputType<charMode>::DeviceInstanceType* lpddi, LPVOID pvRef)
    {
        SEnumDevicesCallbackInfo<charMode>* callbackInfo = (SEnumDevicesCallbackInfo<charMode>*)pvRef;

        // If the present controller supports XInput, indicate such by adding it to the set of instance identifiers of interest.
        if (ControllerIdentification::DoesDirectInputControllerSupportXInput<DirectInputType<charMode>::EarliestIDirectInputType, DirectInputType<charMode>::EarliestIDirectInputDeviceType>(callbackInfo->instance->underlyingDIObject, lpddi->guidInstance))
        {
            callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
            EnumDevicesOutputProductName<charMode>(Message::ESeverity::Debug, L"Enumerate: DirectInput device \"%s\" supports XInput and will not be presented to the application.", lpddi);
        }

        return DIENUM_CONTINUE;
    }

    // --------

    template <ECharMode charMode> BOOL STDMETHODCALLTYPE WrapperIDirectInput<charMode>::CallbackEnumDevicesFiltered(const DirectInputType<charMode>::DeviceInstanceType* lpddi, LPVOID pvRef)
    {
        SEnumDevicesCallbackInfo<charMode>* callbackInfo = (SEnumDevicesCallbackInfo<charMode>*)pvRef;

        if (0 == callbackInfo->seenInstanceIdentifiers.count(lpddi->guidInstance))
        {
            // If the device has not been seen already, add it to the set and present it to the application.
            callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
            callbackInfo->callbackReturnCode = ((BOOL(FAR PASCAL *)(const DirectInputType<charMode>::DeviceInstanceType*,LPVOID))(callbackInfo->lpCallback))(lpddi, callbackInfo->pvRef);
            EnumDevicesOutputProductName<charMode>(Message::ESeverity::Debug, L"Enumerate: DirectInput device \"%s\" is being presented to the application.", lpddi);
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

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, DirectInputType<charMode>::ConfigureDevicesParamsType lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
    {
        return underlyingDIObject->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
    }

    // ---------

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::EnumDevicesBySemantics(DirectInputType<charMode>::ConstStringType ptszUserName, DirectInputType<charMode>::ActionFormatType lpdiActionFormat, DirectInputType<charMode>::EnumDevicesBySemanticsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
        // Operation not supported.
        return DIERR_UNSUPPORTED;
    }
#else
    // -------- METHODS: IDirectInput LEGACY ------------------------------- //
    // See DirectInput documentation for more information.

    template <ECharMode charMode> HRESULT STDMETHODCALLTYPE WrapperIDirectInput<charMode>::CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
    {
        // Make sure the supplied IID is valid.
        if (ECharMode::W == charMode)
        {
            if (!(IsEqualIID(riid, IID_IDirectInputDeviceW) || IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDevice7W)))
            {
                Message::Output(Message::ESeverity::Warning, L"CreateDeviceEx Unicode failed due to an invalid IID.");
                return DIERR_NOINTERFACE;
            }
        }
        else
        {
            if (!(IsEqualIID(riid, IID_IDirectInputDeviceA) || IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDevice7A)))
            {
                Message::Output(Message::ESeverity::Warning, L"CreateDeviceEx ASCII failed due to an invalid IID.");
                return DIERR_NOINTERFACE;
            }
        }

        // Create a device the normal way.
        return CreateDevice(rguid, (typename DirectInputType<charMode>::EarliestIDirectInputDeviceType**)lplpDirectInputDevice, pUnkOuter);

    }
#endif

    // -------- EXPLICIT TEMPLATE INSTANTIATION ---------------------------- //
    // Instantiates both the ASCII and Unicode versions of this class.

    template class WrapperIDirectInput<ECharMode::A>;
    template class WrapperIDirectInput<ECharMode::W>;
}
