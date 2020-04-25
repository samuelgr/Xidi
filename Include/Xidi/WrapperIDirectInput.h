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
    struct WrapperIDirectInput : LatestIDirectInput
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
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        /// Default constructor. Should never be invoked.
        WrapperIDirectInput(void);

    public:
        /// Constructs an WrapperIDirectInput object, given an underlying IDirectInput8 object to wrap.
        WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode);
        
        
        // -------- METHODS: IUnknown ---------------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
        virtual ULONG STDMETHODCALLTYPE AddRef(void);
        virtual ULONG STDMETHODCALLTYPE Release(void);


        // -------- METHODS: IDirectInput COMMON ----------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, EarliestIDirectInputDevice** lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
        virtual HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCWSTR ptszName, LPGUID pguidInstance);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance);
        virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion);
        virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags);


        // -------- CALLBACKS: IDirectInput COMMON --------------------------------- //
        
        // Callback used to scan for any XInput-compatible game controllers.
        // This is the non-Unicode version.
        static BOOL STDMETHODCALLTYPE CallbackEnumGameControllersXInputScanA(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef);
        
        // Callback used to scan for any XInput-compatible game controllers.
        // This is the Unicode version.
        static BOOL STDMETHODCALLTYPE CallbackEnumGameControllersXInputScanW(LPCDIDEVICEINSTANCEW lpddi, LPVOID pvRef);

        // Callback used to enumerate all devices to the application, filtering out those already seen.
        // This is the non-Unicode version.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevicesFilteredA(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef);
        
        // Callback used to enumerate all devices to the application, filtering out those already seen.
        // This is the Unicode version.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevicesFilteredW(LPCDIDEVICEINSTANCEW lpddi, LPVOID pvRef);
        
#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInput8 ONLY ------------------------------------ //
        virtual HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);
        virtual HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCWSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags);
#else
        // -------- METHODS: IDirectInput LEGACY ----------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
#endif
    };
}
