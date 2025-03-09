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
  /// Wraps the IDirectInput interface of all supported versions to hook into all calls to it. Holds
  /// an underlying instance of an IDirectInput object but wraps all method invocations. This base
  /// class only contains methods common to all supported versions of DirectInput.
  /// @tparam diVersion DirectInput version enumerator.
  template <EDirectInputVersion diVersion> class WrapperIDirectInputBase
      : public DirectInputTypes<diVersion>::IDirectInputType
  {
  public:

    WrapperIDirectInputBase(DirectInputTypes<diVersion>::IDirectInputType* underlyingDIObject);

    /// Callback used to scan for any XInput-compatible game controllers.
    static BOOL __stdcall CallbackEnumGameControllersXInputScan(
        const DirectInputTypes<diVersion>::DeviceInstanceType* lpddi, LPVOID pvRef);

    /// Callback used to enumerate all devices to the application, filtering out those already seen.
    static BOOL __stdcall CallbackEnumDevicesFiltered(
        const DirectInputTypes<diVersion>::DeviceInstanceType* lpddi, LPVOID pvRef);

    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    // IDirectInput (legacy and 8)
    HRESULT __stdcall CreateDevice(
        REFGUID rguid,
        DirectInputTypes<diVersion>::IDirectInputDeviceCompatType** lplpDirectInputDevice,
        LPUNKNOWN pUnkOuter) override;
    HRESULT __stdcall EnumDevices(
        DWORD dwDevType,
        DirectInputTypes<diVersion>::EnumDevicesCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwFlags) override;
    HRESULT __stdcall FindDevice(
        REFGUID rguidClass,
        DirectInputTypes<diVersion>::ConstStringType ptszName,
        LPGUID pguidInstance) override;
    HRESULT __stdcall GetDeviceStatus(REFGUID rguidInstance) override;
    HRESULT __stdcall Initialize(HINSTANCE hinst, DWORD dwVersion) override;
    HRESULT __stdcall RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;

  protected:

    /// The underlying IDirectInput8 object that this instance wraps.
    DirectInputTypes<diVersion>::IDirectInputType* underlyingDIObject;
  };

  /// Subclass for methods only present in version 8 of the IDirectInput interface.
  /// @tparam diVersion DirectInput version enumerator. Must identify version 8.
  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIs8<diVersion>)
  class WrapperIDirectInputVersion8Only : public WrapperIDirectInputBase<diVersion>
  {
  public:

    inline WrapperIDirectInputVersion8Only(
        DirectInputTypes<diVersion>::IDirectInputType* underlyingDIObject)
        : WrapperIDirectInputBase<diVersion>(underlyingDIObject)
    {}

    // IDirectInput8
    HRESULT __stdcall ConfigureDevices(
        LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
        DirectInputTypes<diVersion>::ConfigureDevicesParamsType* lpdiCDParams,
        DWORD dwFlags,
        LPVOID pvRefData) override;
    HRESULT __stdcall EnumDevicesBySemantics(
        DirectInputTypes<diVersion>::ConstStringType ptszUserName,
        DirectInputTypes<diVersion>::ActionFormatType* lpdiActionFormat,
        DirectInputTypes<diVersion>::EnumDevicesBySemanticsCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwFlags) override;
  };

  /// Subclass for methods only present in legacy versions of the IDirectInput interface.
  /// @tparam diVersion DirectInput version enumerator. Must identify a legacy version.
  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIsLegacy<diVersion>)
  class WrapperIDirectInputVersionLegacyOnly : public WrapperIDirectInputBase<diVersion>
  {
  public:

    inline WrapperIDirectInputVersionLegacyOnly(
        DirectInputTypes<diVersion>::IDirectInputType* underlyingDIObject)
        : WrapperIDirectInputBase<diVersion>(underlyingDIObject)
    {}

    // IDirectInput7
    HRESULT __stdcall CreateDeviceEx(
        REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
  };

  /// Templated wrapper for all supported versions of the the IDirectInput interface. The
  /// unspecialized version does nothing, but individual specialized versions exist for all possible
  /// enumerators.
  /// @tparam diVersion DirectInput version enumerator.
  template <EDirectInputVersion diVersion> class WrapperIDirectInput
  {};

  template <> class WrapperIDirectInput<EDirectInputVersion::k8A>
      : public WrapperIDirectInputVersion8Only<EDirectInputVersion::k8A>
  {
  public:

    inline WrapperIDirectInput(
        DirectInputTypes<EDirectInputVersion::k8A>::IDirectInputType* underlyingDIObject)
        : WrapperIDirectInputVersion8Only(underlyingDIObject)
    {}
  };

  template <> class WrapperIDirectInput<EDirectInputVersion::k8W>
      : public WrapperIDirectInputVersion8Only<EDirectInputVersion::k8W>
  {
  public:

    inline WrapperIDirectInput(
        DirectInputTypes<EDirectInputVersion::k8W>::IDirectInputType* underlyingDIObject)
        : WrapperIDirectInputVersion8Only(underlyingDIObject)
    {}
  };

  template <> class WrapperIDirectInput<EDirectInputVersion::kLegacyA>
      : public WrapperIDirectInputVersionLegacyOnly<EDirectInputVersion::kLegacyA>
  {
  public:

    inline WrapperIDirectInput(
        DirectInputTypes<EDirectInputVersion::kLegacyA>::IDirectInputType* underlyingDIObject)
        : WrapperIDirectInputVersionLegacyOnly(underlyingDIObject)
    {}
  };

  template <> class WrapperIDirectInput<EDirectInputVersion::kLegacyW>
      : public WrapperIDirectInputVersionLegacyOnly<EDirectInputVersion::kLegacyW>
  {
  public:

    inline WrapperIDirectInput(
        DirectInputTypes<EDirectInputVersion::kLegacyW>::IDirectInputType* underlyingDIObject)
        : WrapperIDirectInputVersionLegacyOnly(underlyingDIObject)
    {}
  };
} // namespace Xidi
