/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file WrapperIDirectInput.h
 *   Declaration of the wrapper class for IDirectInput.
 **************************************************************************************************/

#pragma once

#include "ApiDirectInput.h"

namespace Xidi
{
  /// Helper types for differentiating between Unicode and ASCII interface versions.
  template <ECharMode charMode> struct DirectInputType
  {
    using ConstStringType = LPCTSTR;
    using DeviceInstanceType = DIDEVICEINSTANCE;
    using EarliestIDirectInputType = EarliestIDirectInput;
    using EarliestIDirectInputDeviceType = EarliestIDirectInputDevice;
    using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACK;
    using LatestIDirectInputType = LatestIDirectInput;
#if DIRECTINPUT_VERSION >= 0x0800
    using ActionFormatType = LPDIACTIONFORMAT;
    using ConfigureDevicesParamsType = LPDICONFIGUREDEVICESPARAMS;
    using EnumDevicesBySemanticsCallbackType = LPDIENUMDEVICESBYSEMANTICSCB;
#endif
  };

  template <> struct DirectInputType<ECharMode::A> : public LatestIDirectInputA
  {
    using ConstStringType = LPCSTR;
    using DeviceInstanceType = DIDEVICEINSTANCEA;
    using EarliestIDirectInputType = EarliestIDirectInputA;
    using EarliestIDirectInputDeviceType = EarliestIDirectInputDeviceA;
    using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACKA;
    using LatestIDirectInputType = LatestIDirectInputA;
#if DIRECTINPUT_VERSION >= 0x0800
    using ActionFormatType = LPDIACTIONFORMATA;
    using ConfigureDevicesParamsType = LPDICONFIGUREDEVICESPARAMSA;
    using EnumDevicesBySemanticsCallbackType = LPDIENUMDEVICESBYSEMANTICSCBA;
#endif
  };

  template <> struct DirectInputType<ECharMode::W> : public LatestIDirectInputW
  {
    using ConstStringType = LPCWSTR;
    using DeviceInstanceType = DIDEVICEINSTANCEW;
    using EarliestIDirectInputType = EarliestIDirectInputW;
    using EarliestIDirectInputDeviceType = EarliestIDirectInputDeviceW;
    using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACKW;
    using LatestIDirectInputType = LatestIDirectInputW;
#if DIRECTINPUT_VERSION >= 0x0800
    using ActionFormatType = LPDIACTIONFORMATW;
    using ConfigureDevicesParamsType = LPDICONFIGUREDEVICESPARAMSW;
    using EnumDevicesBySemanticsCallbackType = LPDIENUMDEVICESBYSEMANTICSCBW;
#endif
  };

  /// Wraps the IDirectInput8 interface to hook into all calls to it. Holds an underlying instance
  /// of an IDirectInput object but wraps all method invocations.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  template <ECharMode charMode> class WrapperIDirectInput : public DirectInputType<charMode>
  {
  public:

    WrapperIDirectInput(DirectInputType<charMode>::LatestIDirectInputType* underlyingDIObject);

    /// Callback used to scan for any XInput-compatible game controllers.
    static BOOL __stdcall CallbackEnumGameControllersXInputScan(
        const DirectInputType<charMode>::DeviceInstanceType* lpddi, LPVOID pvRef);

    /// Callback used to enumerate all devices to the application, filtering out those already seen.
    static BOOL __stdcall CallbackEnumDevicesFiltered(
        const DirectInputType<charMode>::DeviceInstanceType* lpddi, LPVOID pvRef);

    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    // IDirectInput
    HRESULT __stdcall CreateDevice(
        REFGUID rguid,
        DirectInputType<charMode>::EarliestIDirectInputDeviceType** lplpDirectInputDevice,
        LPUNKNOWN pUnkOuter) override;
    HRESULT __stdcall EnumDevices(
        DWORD dwDevType,
        DirectInputType<charMode>::EnumDevicesCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwFlags) override;
    HRESULT __stdcall FindDevice(
        REFGUID rguidClass,
        DirectInputType<charMode>::ConstStringType ptszName,
        LPGUID pguidInstance) override;
    HRESULT __stdcall GetDeviceStatus(REFGUID rguidInstance) override;
    HRESULT __stdcall Initialize(HINSTANCE hinst, DWORD dwVersion) override;
    HRESULT __stdcall RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;
#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT __stdcall ConfigureDevices(
        LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
        DirectInputType<charMode>::ConfigureDevicesParamsType lpdiCDParams,
        DWORD dwFlags,
        LPVOID pvRefData) override;
    HRESULT __stdcall EnumDevicesBySemantics(
        DirectInputType<charMode>::ConstStringType ptszUserName,
        DirectInputType<charMode>::ActionFormatType lpdiActionFormat,
        DirectInputType<charMode>::EnumDevicesBySemanticsCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwFlags) override;
#else
    HRESULT __stdcall CreateDeviceEx(
        REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
#endif

  private:

    /// The underlying IDirectInput8 object that this instance wraps.
    DirectInputType<charMode>::LatestIDirectInputType* underlyingDIObject;
  };
} // namespace Xidi
