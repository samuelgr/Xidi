/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file WrapperIDirectInput.h
 *   Declaration of the wrapper class for IDirectInput.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


namespace Xidi
{
    /// Wraps the IDirectInput8 interface to hook into all calls to it.
    /// Holds an underlying instance of an IDirectInput object but wraps all method invocations.
    class WrapperIDirectInput : public LatestIDirectInput
    {
    private:
        // -------- TYPE DEFINITIONS ----------------------------------------------- //

        /// Internal type, used to select between Unicode and non-Unicode representations of the underlying DirectInput interface.
        union UIDirectInput
        {
            LatestIDirectInputA* a;
            LatestIDirectInputW* w;
            LatestIDirectInput* t;
        };


        // -------- INSTANCE VARIABLES --------------------------------------------- //

        /// The underlying IDirectInput8 object that this instance wraps.
        /// Represented both in Unicode and non-Unicode form, with the correct version to be specified by the application.
        UIDirectInput underlyingDIObject;

        /// Specifies whether or not the underlying DirectInput object is Unicode-based.
        BOOL underlyingDIObjectUsesUnicode;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        WrapperIDirectInput(void) = delete;

        /// Constructs an WrapperIDirectInput object, given an underlying IDirectInput8 object to wrap.
        WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode);


        // -------- METHODS: IUnknown ---------------------------------------------- //
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;


        // -------- METHODS: IDirectInput COMMON ----------------------------------- //
        HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, EarliestIDirectInputDevice** lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
        HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance) override;
        HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance) override;
        HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) override;
        HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;


        // -------- CALLBACKS: IDirectInput COMMON --------------------------------- //

        // Callback used to scan for any XInput-compatible game controllers.
        /// @tparam DeviceInstanceType DirectInput device instance type, either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether or not Unicode is desired.
        template <typename DeviceInstanceType> static BOOL STDMETHODCALLTYPE CallbackEnumGameControllersXInputScan(const DeviceInstanceType* lpddi, LPVOID pvRef);

        // Callback used to enumerate all devices to the application, filtering out those already seen.
        /// @tparam DeviceInstanceType DirectInput device instance type, either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether or not Unicode is desired.
        template <typename DeviceInstanceType> static BOOL STDMETHODCALLTYPE CallbackEnumDevicesFiltered(const DeviceInstanceType* lpddi, LPVOID pvRef);

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInput8 ONLY ------------------------------------ //
        HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) override;
        HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCWSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags) override;
#else
        // -------- METHODS: IDirectInput LEGACY ----------------------------------- //
        HRESULT STDMETHODCALLTYPE CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
#endif
    };
}
