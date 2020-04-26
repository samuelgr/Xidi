/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file VirtualDirectInputDevice.h
 *   Declaration of a virtual device that supports IDirectInputDevice but
 *   communicates with an XInput-based controller.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "Mapper.h"
#include "XInputController.h"


namespace Xidi
{
    /// Inherits from whatever IDirectInputDevice version is appropriate.
    struct VirtualDirectInputDevice : LatestIDirectInputDevice
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        /// Controller with which to interface.
        XInputController* controller;

        /// Mapping scheme to be applied to the wrapped DirectInput device.
        Mapper* mapper;

        /// Specifies whether or not the device was polled since the last time its state was obtained.
        BOOL polledSinceLastGetDeviceState;

        /// Reference count.
        ULONG refcount;

        /// Specifies whether or not to use Unicode (this depends on the application configuration).
        BOOL useUnicode;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        VirtualDirectInputDevice(void);

    public:
        /// Constructs a WrapperIDirectInput object, given a mapper and a controller.
        /// @param [in] useUnicode Specifies if the object should use Unicode versions of methods.
        /// @param [in] controller XInput controller object to associate with this object.
        /// @param [in] mapper Mapper object to associate with this object.
        VirtualDirectInputDevice(BOOL useUnicode, XInputController* controller, Mapper* mapper);

        /// Default destructor.
        virtual ~VirtualDirectInputDevice(void);


    public:
        // -------- METHODS: IUnknown ---------------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
        virtual ULONG STDMETHODCALLTYPE AddRef(void);
        virtual ULONG STDMETHODCALLTYPE Release(void);


        // -------- METHODS: IDirectInputDevice COMMON ----------------------------- //
        virtual HRESULT STDMETHODCALLTYPE Acquire(void);
        virtual HRESULT STDMETHODCALLTYPE CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter);
        virtual HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl);
        virtual HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType);
        virtual HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCWSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc);
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCE pdidi);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData);
        virtual HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid);
        virtual HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut);
        virtual HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow);
        virtual HRESULT STDMETHODCALLTYPE GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);
        virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid);
        virtual HRESULT STDMETHODCALLTYPE Poll(void);
        virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl);
        virtual HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf);
        virtual HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent);
        virtual HRESULT STDMETHODCALLTYPE SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);
        virtual HRESULT STDMETHODCALLTYPE Unacquire(void);
        virtual HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCWSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags);

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
        virtual HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader);
        virtual HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCWSTR lptszUserName, DWORD dwFlags);
#endif
    };
}
