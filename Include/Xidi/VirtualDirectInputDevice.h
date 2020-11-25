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
    /// Helper types for differentiating between Unicode and ASCII interface versions.
    template <bool useUnicode> struct DirectInputDeviceHelper
    {
        typedef LPCTSTR ConstStringType;
        typedef LPDIDEVICEINSTANCE DeviceInstanceType;
        typedef LPDIDEVICEOBJECTINSTANCE DeviceObjectInstanceType;
        typedef LPDIEFFECTINFO EffectInfoType;
        typedef LPDIENUMEFFECTSCALLBACK EnumEffectsCallbackType;
        typedef LPDIENUMDEVICEOBJECTSCALLBACK EnumObjectsCallbackType;
#if DIRECTINPUT_VERSION >= 0x0800
        typedef LPDIACTIONFORMAT ActionFormatType;
        typedef LPDIDEVICEIMAGEINFOHEADER DeviceImageInfoHeaderType;
#endif
    };
    template <> struct DirectInputDeviceHelper<false> : public LatestIDirectInputDeviceA
    {
        typedef LPCSTR ConstStringType;
        typedef LPDIDEVICEINSTANCEA DeviceInstanceType;
        typedef LPDIDEVICEOBJECTINSTANCEA DeviceObjectInstanceType;
        typedef LPDIEFFECTINFOA EffectInfoType;
        typedef LPDIENUMEFFECTSCALLBACKA EnumEffectsCallbackType;
        typedef LPDIENUMDEVICEOBJECTSCALLBACKA EnumObjectsCallbackType;
#if DIRECTINPUT_VERSION >= 0x0800
        typedef LPDIACTIONFORMATA ActionFormatType;
        typedef LPDIDEVICEIMAGEINFOHEADERA DeviceImageInfoHeaderType;
#endif
    };
    template <> struct DirectInputDeviceHelper<true> : public LatestIDirectInputDeviceW
    {
        typedef LPCWSTR ConstStringType;
        typedef LPDIDEVICEINSTANCEW DeviceInstanceType;
        typedef LPDIDEVICEOBJECTINSTANCEW DeviceObjectInstanceType;
        typedef LPDIEFFECTINFOW EffectInfoType;
        typedef LPDIENUMEFFECTSCALLBACKW EnumEffectsCallbackType;
        typedef LPDIENUMDEVICEOBJECTSCALLBACKW EnumObjectsCallbackType;
#if DIRECTINPUT_VERSION >= 0x0800
        typedef LPDIACTIONFORMATW ActionFormatType;
        typedef LPDIDEVICEIMAGEINFOHEADERW DeviceImageInfoHeaderType;
#endif
    };

    /// Inherits from whatever IDirectInputDevice version is appropriate.
    template <bool useUnicode> class VirtualDirectInputDevice : public DirectInputDeviceHelper<useUnicode>
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


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        VirtualDirectInputDevice(void) = delete;

        /// Constructs a WrapperIDirectInput object, given a mapper and a controller.
        /// @param [in] controller XInput controller object to associate with this object.
        /// @param [in] mapper Mapper object to associate with this object.
        VirtualDirectInputDevice(XInputController* controller, Mapper* mapper);

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
        HRESULT STDMETHODCALLTYPE EnumEffects(DirectInputDeviceHelper<useUnicode>::EnumEffectsCallbackType lpCallback, LPVOID pvRef, DWORD dwEffType) override;
        HRESULT STDMETHODCALLTYPE EnumEffectsInFile(DirectInputDeviceHelper<useUnicode>::ConstStringType lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE EnumObjects(DirectInputDeviceHelper<useUnicode>::EnumObjectsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc) override;
        HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) override;
        HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE GetDeviceInfo(DirectInputDeviceHelper<useUnicode>::DeviceInstanceType pdidi) override;
        HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) override;
        HRESULT STDMETHODCALLTYPE GetEffectInfo(DirectInputDeviceHelper<useUnicode>::EffectInfoType pdei, REFGUID rguid) override;
        HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut) override;
        HRESULT STDMETHODCALLTYPE GetObjectInfo(DirectInputDeviceHelper<useUnicode>::DeviceObjectInstanceType pdidoi, DWORD dwObj, DWORD dwHow) override;
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
        HRESULT STDMETHODCALLTYPE WriteEffectToFile(DirectInputDeviceHelper<useUnicode>::ConstStringType lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) override;

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInputDevice8 ONLY ------------------------------ //
        HRESULT STDMETHODCALLTYPE BuildActionMap(DirectInputDeviceHelper<useUnicode>::ActionFormatType lpdiaf, DirectInputDeviceHelper<useUnicode>::ConstStringType lpszUserName, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE GetImageInfo(DirectInputDeviceHelper<useUnicode>::DeviceImageInfoHeaderType lpdiDevImageInfoHeader) override;
        HRESULT STDMETHODCALLTYPE SetActionMap(DirectInputDeviceHelper<useUnicode>::ActionFormatType lpdiActionFormat, DirectInputDeviceHelper<useUnicode>::ConstStringType lptszUserName, DWORD dwFlags) override;
#endif
    };
}
