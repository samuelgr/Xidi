/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * WrapperIDirectInput.h
 *      Declaration of the wrapper class for IDirectInput.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


namespace Xidi
{
    // Wraps the IDirectInput8 interface to hook into all calls to it.
    // Holds an underlying instance of an IDirectInput object but wraps all method invocations.
    struct WrapperIDirectInput : LatestIDirectInput
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //
        
        // The underlying IDirectInput8 object that this instance wraps.
        LatestIDirectInput* underlyingDIObject;

        // Specifies whether or not the underlying DirectInput object is Unicode-based.
        BOOL underlyingDIObjectUsesUnicode;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        WrapperIDirectInput();

    public:
        // Constructs an WrapperIDirectInput object, given an underlying IDirectInput8 object to wrap.
        WrapperIDirectInput(LatestIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode);
        
        
    private:
        // -------- HELPERS -------------------------------------------------------- //
        
        // Logs an informational event related to creating a non-XInput device.
        static void LogCreateDeviceNonXInput(void);

        // Logs an informational event related to creating an XInput device.
        static void LogCreateDeviceXInput(unsigned int index);
        
        // Logs a debug event related to enumerating a device to the application.
        static void LogEnumDevice(LPCTSTR deviceName);

        // Logs a debug event related to stopping enumeration early.
        static void LogEnumFinishEarly(void);

        // Logs a debug event related to skipping over enumeration of an XInput-compatible device.
        static void LogEnumSkipDevice(LPCTSTR deviceName);

        // Logs a debug event related to enumerating Xidi-created virtual devices.
        static void LogEnumXidiDevices(void);
        
        // Logs a debug event related to starting to enumerate devices.
        static void LogStartEnumDevices(void);
        
        // Logs a debug event related to finishing to enumerate devices.
        static void LogFinishEnumDevices(void);
        

    public:
        // -------- METHODS: IUnknown ---------------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
        virtual ULONG STDMETHODCALLTYPE AddRef(void);
        virtual ULONG STDMETHODCALLTYPE Release(void);


        // -------- METHODS: IDirectInput COMMON ----------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, EarliestIDirectInputDevice** lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
        virtual HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance);
        virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion);
        virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags);


        // -------- CALLBACKS: IDirectInput COMMON --------------------------------- //
        
        // Intercepts callbacks invoked as part of a call to EnumDevices.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);


#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInput8 ONLY ------------------------------------ //
        virtual HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);
        virtual HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags);
#else
        // -------- METHODS: IDirectInput LEGACY ----------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
#endif
    };
}
