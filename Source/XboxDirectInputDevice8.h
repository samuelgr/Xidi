/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XboxDirectInputDevice8.h
 *      Declaration of the wrapper class for IDirectInputDevice8.
 *****************************************************************************/

#pragma once

#include "API_DirectInput8.h"


namespace XboxControllerDirectInput
{
    // Wraps the IDirectInput8 interface to hook into all calls to it.
    // Holds an underlying instance of an IDirectInput8 object but wraps all method invocations.
    struct XboxDirectInputDevice8 : IDirectInputDevice8
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        // The underlying IDirectInputDevice8 object that this instance wraps.
        IDirectInputDevice8* underlyingDIObject;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Constructs an XboxDirectInput8 object, given an underlying IDirectInput8 object to wrap.
        XboxDirectInputDevice8(IDirectInputDevice8* underlyingDIObject);


        // -------- METHODS: IUnknown ---------------------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj);
        virtual ULONG STDMETHODCALLTYPE AddRef(void);
        virtual ULONG STDMETHODCALLTYPE Release(void);


        // -------- METHODS: IDirectInputDevice8 ----------------------------------- //
        virtual HRESULT STDMETHODCALLTYPE Acquire(void);
        virtual HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCTSTR lpszUserName, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter);
        virtual HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl);
        virtual HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType);
        virtual HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCTSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc);
        virtual HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCE pdidi);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData);
        virtual HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid);
        virtual HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut);
        virtual HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader);
        virtual HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow);
        virtual HRESULT STDMETHODCALLTYPE GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);
        virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid);
        virtual HRESULT STDMETHODCALLTYPE Poll(void);
        virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl);
        virtual HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lptszUserName, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags);
        virtual HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf);
        virtual HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent);
        virtual HRESULT STDMETHODCALLTYPE SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);
        virtual HRESULT STDMETHODCALLTYPE Unacquire(void);
        virtual HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCTSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags);
    };
}
