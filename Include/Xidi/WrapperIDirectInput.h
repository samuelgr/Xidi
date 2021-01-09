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
    /// Helper types for differentiating between Unicode and ASCII interface versions.
    template <ECharMode charMode> struct DirectInputType
    {
        typedef LPCTSTR ConstStringType;
        typedef DIDEVICEINSTANCE DeviceInstanceType;
        typedef EarliestIDirectInput EarliestIDirectInputType;
        typedef EarliestIDirectInputDevice EarliestIDirectInputDeviceType;
        typedef LPDIENUMDEVICESCALLBACK EnumDevicesCallbackType;
        typedef LatestIDirectInput LatestIDirectInputType;
#if DIRECTINPUT_VERSION >= 0x0800
        typedef LPDIACTIONFORMAT ActionFormatType;
        typedef LPDICONFIGUREDEVICESPARAMS ConfigureDevicesParamsType;
        typedef LPDIENUMDEVICESBYSEMANTICSCB EnumDevicesBySemanticsCallbackType;
#endif
    };

    template <> struct DirectInputType<ECharMode::A> : public LatestIDirectInputA
    {
        typedef LPCSTR ConstStringType;
        typedef DIDEVICEINSTANCEA DeviceInstanceType;
        typedef EarliestIDirectInputA EarliestIDirectInputType;
        typedef EarliestIDirectInputDeviceA EarliestIDirectInputDeviceType;
        typedef LPDIENUMDEVICESCALLBACKA EnumDevicesCallbackType;
        typedef LatestIDirectInputA LatestIDirectInputType;
#if DIRECTINPUT_VERSION >= 0x0800
        typedef LPDIACTIONFORMATA ActionFormatType;
        typedef LPDICONFIGUREDEVICESPARAMSA ConfigureDevicesParamsType;
        typedef LPDIENUMDEVICESBYSEMANTICSCBA EnumDevicesBySemanticsCallbackType;
#endif
    };

    template <> struct DirectInputType<ECharMode::W> : public LatestIDirectInputW
    {
        typedef LPCWSTR ConstStringType;
        typedef DIDEVICEINSTANCEW DeviceInstanceType;
        typedef EarliestIDirectInputW EarliestIDirectInputType;
        typedef EarliestIDirectInputDeviceW EarliestIDirectInputDeviceType;
        typedef LPDIENUMDEVICESCALLBACKW EnumDevicesCallbackType;
        typedef LatestIDirectInputW LatestIDirectInputType;
#if DIRECTINPUT_VERSION >= 0x0800
        typedef LPDIACTIONFORMATW ActionFormatType;
        typedef LPDICONFIGUREDEVICESPARAMSW ConfigureDevicesParamsType;
        typedef LPDIENUMDEVICESBYSEMANTICSCBW EnumDevicesBySemanticsCallbackType;
#endif
    };

    /// Wraps the IDirectInput8 interface to hook into all calls to it.
    /// Holds an underlying instance of an IDirectInput object but wraps all method invocations.
    /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types and interfaces.
    template <ECharMode charMode> class WrapperIDirectInput : public DirectInputType<charMode>
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        /// The underlying IDirectInput8 object that this instance wraps.
        DirectInputType<charMode>::LatestIDirectInputType* underlyingDIObject;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        WrapperIDirectInput(void) = delete;

        /// Constructs an WrapperIDirectInput object, given an underlying IDirectInput8 object to wrap.
        WrapperIDirectInput(DirectInputType<charMode>::LatestIDirectInputType* underlyingDIObject);


        // -------- METHODS: IUnknown ---------------------------------------------- //
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;


        // -------- METHODS: IDirectInput COMMON ----------------------------------- //
        HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, DirectInputType<charMode>::EarliestIDirectInputDeviceType** lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
        HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, DirectInputType<charMode>::EnumDevicesCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, DirectInputType<charMode>::ConstStringType ptszName, LPGUID pguidInstance) override;
        HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance) override;
        HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) override;
        HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;


        // -------- CALLBACKS: IDirectInput COMMON --------------------------------- //

        // Callback used to scan for any XInput-compatible game controllers.
        static BOOL STDMETHODCALLTYPE CallbackEnumGameControllersXInputScan(const DirectInputType<charMode>::DeviceInstanceType* lpddi, LPVOID pvRef);

        // Callback used to enumerate all devices to the application, filtering out those already seen.
        static BOOL STDMETHODCALLTYPE CallbackEnumDevicesFiltered(const DirectInputType<charMode>::DeviceInstanceType* lpddi, LPVOID pvRef);

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInput8 ONLY ------------------------------------ //
        HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, DirectInputType<charMode>::ConfigureDevicesParamsType lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) override;
        HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(DirectInputType<charMode>::ConstStringType ptszUserName, DirectInputType<charMode>::ActionFormatType lpdiActionFormat, DirectInputType<charMode>::EnumDevicesBySemanticsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags) override;
#else
        // -------- METHODS: IDirectInput LEGACY ----------------------------------- //
        HRESULT STDMETHODCALLTYPE CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
#endif
    };
}
