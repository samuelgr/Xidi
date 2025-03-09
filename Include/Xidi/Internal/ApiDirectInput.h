/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ApiDirectInput.h
 *   Common header file for the DirectInput API.
 **************************************************************************************************/

#pragma once

// Even for legacy versions of DirectInput the version 8 header is used. This is to allow a binary
// to be built that supports both the version 8 interfaces and the legacy interfaces.
#define DIRECTINPUT_VERSION 0x0800

// DirectInput header files depend on Windows header files, which are are sensitive to include
// order. See "ApiWindows.h" for more information.

// clang-format off

#include "ApiWindows.h"
#include <dinput.h>
#include <hidclass.h>

// clang-format on

/// Missing from `dinput.h`, this constant is used by built-in DirectInput data formats and
/// presumably others. Its intent is to signify that an element of the data format is optional, so
/// setting the data format should not fail if the structure element remains unused. In the absence
/// of this flag, if an element of the requested data format does not have a controller object
/// instance associated with it, setting the data format fails due to invalid parameter.
#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL 0x80000000
#endif

/// Enumerates supported DirectInput interface version classes.
enum class EDirectInputVersion
{
  /// DirectInput 8, with ANSI characters.
  k8A,

  /// DirectInput 8, with wide (Unicode) characters.
  k8W,

  /// DirectInput 7 and below, with ANSI characters.
  kLegacyA,

  /// DirectInput 7 and below, with wide (Unicode) characters.
  kLegacyW
};

/// Determines if the specified DirectInput version enumerator is for version 8.
template <EDirectInputVersion diVersion> concept DirectInputVersionIs8 =
    ((EDirectInputVersion::k8A == diVersion) || (EDirectInputVersion::k8W == diVersion));

/// Determines if the specified DirectInput version enumerator is for a legacy version, 7 or older.
template <EDirectInputVersion diVersion> concept DirectInputVersionIsLegacy =
    ((EDirectInputVersion::kLegacyA == diVersion) || (EDirectInputVersion::kLegacyW == diVersion));

/// Defines helper functions and type aliases specific to a DirectInput version.
/// The unspecialized version is empty and does nothing useful.
/// @tparam diVersion DirectInput version enumerator.
template <EDirectInputVersion diVersion> struct DirectInputTypes
{};

/// Defines helper functions and type aliases specific to DirectInput 8 with ANSI characters.
template <> struct DirectInputTypes<EDirectInputVersion::k8A>
{
  using StringType = LPSTR;
  using ConstStringType = LPCSTR;

  using IDirectInputType = IDirectInput8A;
  using IDirectInputCompatType = IDirectInput8A;
  using IDirectInputDeviceType = IDirectInputDevice8A;
  using IDirectInputDeviceCompatType = IDirectInputDevice8A;

  using DeviceInstanceType = DIDEVICEINSTANCEA;
  using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3A;
  using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCEA;
  using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3A;

  using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACKA;
  using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACKA;
  using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACKA;
  using EnumDevicesBySemanticsCallbackType = LPDIENUMDEVICESBYSEMANTICSCBA;

  using ActionFormatType = DIACTIONFORMATA;
  using ConfigureDevicesParamsType = DICONFIGUREDEVICESPARAMSA;
  using DeviceImageInfoHeaderType = DIDEVICEIMAGEINFOHEADERA;
  using EffectInfoType = DIEFFECTINFOA;

  static inline bool IsCompatibleDirectInputIID(const IID& iid)
  {
    return (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInput8A));
  }

  static inline bool IsCompatibleDirectInputDeviceIID(const IID& iid)
  {
    return (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInputDevice8A));
  }

  static inline DWORD XinputGamepadDeviceType(void)
  {
    return ((DIDEVTYPE_HID) | (DI8DEVTYPE_GAMEPAD) | ((DI8DEVTYPEGAMEPAD_STANDARD) << 8));
  }
};

/// Defines helper functions and type aliases specific to DirectInput 8 with Unicode characters.
template <> struct DirectInputTypes<EDirectInputVersion::k8W>
{
  using StringType = LPWSTR;
  using ConstStringType = LPCWSTR;

  using IDirectInputType = IDirectInput8W;
  using IDirectInputCompatType = IDirectInput8W;
  using IDirectInputDeviceType = IDirectInputDevice8W;
  using IDirectInputDeviceCompatType = IDirectInputDevice8W;

  using DeviceInstanceType = DIDEVICEINSTANCEW;
  using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3W;
  using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCEW;
  using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3W;

  using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACKW;
  using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACKW;
  using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACKW;
  using EnumDevicesBySemanticsCallbackType = LPDIENUMDEVICESBYSEMANTICSCBW;

  using ActionFormatType = DIACTIONFORMATW;
  using ConfigureDevicesParamsType = DICONFIGUREDEVICESPARAMSW;
  using DeviceImageInfoHeaderType = DIDEVICEIMAGEINFOHEADERW;
  using EffectInfoType = DIEFFECTINFOW;

  static inline bool IsCompatibleDirectInputIID(const IID& iid)
  {
    return (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInput8W));
  }

  static inline bool IsCompatibleDirectInputDeviceIID(const IID& iid)
  {
    return (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInputDevice8W));
  }

  static inline DWORD XinputGamepadDeviceType(void)
  {
    return ((DIDEVTYPE_HID) | (DI8DEVTYPE_GAMEPAD) | ((DI8DEVTYPEGAMEPAD_STANDARD) << 8));
  }
};

/// Defines helper functions and type aliases specific to DirectInput 7 and older with ANSI
/// characters.
template <> struct DirectInputTypes<EDirectInputVersion::kLegacyA>
{
  using StringType = LPSTR;
  using ConstStringType = LPCSTR;

  using IDirectInputType = IDirectInput7A;
  using IDirectInputCompatType = IDirectInputA;
  using IDirectInputDeviceType = IDirectInputDevice7A;
  using IDirectInputDeviceCompatType = IDirectInputDeviceA;

  using DeviceInstanceType = DIDEVICEINSTANCEA;
  using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3A;
  using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCEA;
  using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3A;

  using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACKA;
  using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACKA;
  using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACKA;

  using EffectInfoType = DIEFFECTINFOA;

  static inline bool IsCompatibleDirectInputIID(const IID& iid)
  {
    return (
        IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInput7A) ||
        IsEqualIID(iid, IID_IDirectInput2A) || IsEqualIID(iid, IID_IDirectInputA));
  }

  static inline bool IsCompatibleDirectInputDeviceIID(const IID& iid)
  {
    return (
        IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInputDevice7A) ||
        IsEqualIID(iid, IID_IDirectInputDevice2A) || IsEqualIID(iid, IID_IDirectInputDeviceA));
  }

  static inline DWORD XinputGamepadDeviceType(void)
  {
    return (
        (DIDEVTYPE_HID) | (4 /* DIDEVTYPE_JOYSTICK */) |
        ((4 /* DIDEVTYPEJOYSTICK_GAMEPAD */) << 8));
  }
};

/// Defines helper functions and type aliases specific to DirectInput 7 and older with Unicode
/// characters.
template <> struct DirectInputTypes<EDirectInputVersion::kLegacyW>
{
  using StringType = LPWSTR;
  using ConstStringType = LPCWSTR;

  using IDirectInputType = IDirectInput7W;
  using IDirectInputCompatType = IDirectInputW;
  using IDirectInputDeviceType = IDirectInputDevice7W;
  using IDirectInputDeviceCompatType = IDirectInputDeviceW;

  using DeviceInstanceType = DIDEVICEINSTANCEW;
  using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3W;
  using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCEW;
  using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3W;

  using EnumDevicesCallbackType = LPDIENUMDEVICESCALLBACKW;
  using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACKW;
  using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACKW;

  using EffectInfoType = DIEFFECTINFOW;

  static inline bool IsCompatibleDirectInputIID(const IID& iid)
  {
    return (
        IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInput7W) ||
        IsEqualIID(iid, IID_IDirectInput2W) || IsEqualIID(iid, IID_IDirectInputW));
  }

  static inline bool IsCompatibleDirectInputDeviceIID(const IID& iid)
  {
    return (
        IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IDirectInputDevice7W) ||
        IsEqualIID(iid, IID_IDirectInputDevice2W) || IsEqualIID(iid, IID_IDirectInputDeviceW));
  }

  static inline DWORD XinputGamepadDeviceType(void)
  {
    return (
        (DIDEVTYPE_HID) | (4 /* DIDEVTYPE_JOYSTICK */) |
        ((4 /* DIDEVTYPEJOYSTICK_GAMEPAD */) << 8));
  }
};
