/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
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
    struct WrapperIDirectInput : VersionedIDirectInput
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //
        
        // The underlying IDirectInput8 object that this instance wraps.
        IDirectInput8* underlyingDIObject;

        // Specifies whether or not the underlying DirectInput object is Unicode-based.
        BOOL underlyingDIObjectUsesUnicode;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        WrapperIDirectInput();

    public:
        // Constructs an WrapperIDirectInput object, given an underlying IDirectInput8 object to wrap.
        WrapperIDirectInput(VersionedIDirectInput* underlyingDIObject, BOOL underlyingDIObjectUsesUnicode);
        
        
        // -------- METHODS: IUnknown ---------------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
        virtual ULONG STDMETHODCALLTYPE AddRef(void);
        virtual ULONG STDMETHODCALLTYPE Release(void);


        // -------- METHODS: IDirectInput ------------------------------------------ //
        virtual HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8* lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
        virtual HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);
        virtual HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance);
        virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion);
        virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags);


        // -------- CALLBACKS: IDirectInput ---------------------------------------- //
        
        // Intercepts callbacks invoked as part of a call to EnumDevices.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevices(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

        // Intercepts callbacks invoked as part of a call to EnumDevicesBySemantics.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevicesBySemantics(LPCDIDEVICEINSTANCE lpddi, LPDIRECTINPUTDEVICE8 lpdid, DWORD dwFlags, DWORD dwRemaining, LPVOID pvRef);
    };
}
