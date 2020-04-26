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
    class VirtualDirectInputDevice : public LatestIDirectInputDevice
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
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;


        // -------- METHODS: IDirectInputDevice COMMON ----------------------------- //
        HRESULT STDMETHODCALLTYPE Acquire(void) override;
        HRESULT STDMETHODCALLTYPE CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter) override;
        HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl) override;
        HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType) override;
        HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCWSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc) override;
        HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) override;
        HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCE pdidi) override;
        HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) override;
        HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid) override;
        HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut) override;
        HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow) override;
        HRESULT STDMETHODCALLTYPE GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph) override;
        HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) override;
        HRESULT STDMETHODCALLTYPE Poll(void) override;
        HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) override;
        HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) override;
        HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) override;
        HRESULT STDMETHODCALLTYPE SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) override;
        HRESULT STDMETHODCALLTYPE Unacquire(void) override;
        HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCWSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) override;

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
        HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader) override;
        HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCWSTR lptszUserName, DWORD dwFlags) override;
#endif
    };
}
