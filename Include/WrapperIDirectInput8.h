/*****************************************************************************
 * XInputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperIDirectInput8.h
 *      Declaration of the wrapper class for IDirectInput8.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"


namespace XInputControllerDirectInput
{
    // Wraps the IDirectInput8 interface to hook into all calls to it.
    // Holds an underlying instance of an IDirectInput8 object but wraps all method invocations.
    struct WrapperIDirectInput8 : IDirectInput8
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //
        
        // The underlying IDirectInput8 object that this instance wraps.
        IDirectInput8* underlyingDIObject;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        WrapperIDirectInput8();

    public:
        // Constructs an WrapperIDirectInput8 object, given an underlying IDirectInput8 object to wrap.
        WrapperIDirectInput8(IDirectInput8* underlyingDIObject);
        
        
        // -------- METHODS: IUnknown ---------------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
        virtual ULONG STDMETHODCALLTYPE AddRef(void);
        virtual ULONG STDMETHODCALLTYPE Release(void);


        // -------- METHODS: IDirectInput8 ----------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8* lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
        virtual HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);
        virtual HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance);
        virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion);
        virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags);


        // -------- CALLBACKS: IDirectInput8 --------------------------------------- //
        
        // Intercepts callbacks invoked as part of a call to EnumDevices.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

        // Intercepts callbacks invoked as part of a call to EnumDevicesBySemantics.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevicesBySemantics(LPCDIDEVICEINSTANCE lpddi, LPDIRECTINPUTDEVICE8 lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef);
    };
}
