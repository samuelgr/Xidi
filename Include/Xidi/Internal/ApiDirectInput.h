/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ApiDirectInput.h
 *   Common header file for the DirectInput API.
 **************************************************************************************************/

#pragma once

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

/// Enumerates supported character type modes for DirectInput interfaces.
enum class ECharMode
{
  /// ASCII mode, denoted with an "A" suffix in Microsoft documentation.
  A,

  /// Wide-character (Unicode) mode, denoted with a "W" suffix in Microsoft documentation.
  W
};

#if DIRECTINPUT_VERSION >= 0x0800

#define DINPUT_VER_MIN 0x0800
#define DINPUT_VER_MAX 0x08ff

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD                                                              \
  ((DIDEVTYPE_HID) | (DI8DEVTYPE_GAMEPAD) | ((DI8DEVTYPEGAMEPAD_STANDARD) << 8))

using EarliestIDirectInput = IDirectInput8;
using LatestIDirectInput = IDirectInput8;
using EarliestIDirectInputA = IDirectInput8A;
using LatestIDirectInputA = IDirectInput8A;
using EarliestIDirectInputW = IDirectInput8W;
using LatestIDirectInputW = IDirectInput8W;
using EarliestIDirectInputDevice = IDirectInputDevice8;
using LatestIDirectInputDevice = IDirectInputDevice8;
using EarliestIDirectInputDeviceA = IDirectInputDevice8A;
using LatestIDirectInputDeviceA = IDirectInputDevice8A;
using EarliestIDirectInputDeviceW = IDirectInputDevice8W;
using LatestIDirectInputDeviceW = IDirectInputDevice8W;

#define IID_LatestIDirectInput        IID_IDirectInput8
#define IID_LatestIDirectInputA       IID_IDirectInput8A
#define IID_LatestIDirectInputW       IID_IDirectInput8W
#define IID_LatestIDirectInputDevice  IID_IDirectInputDevice8
#define IID_LatestIDirectInputDeviceA IID_IDirectInputDevice8A
#define IID_LatestIDirectInputDeviceW IID_IDirectInputDevice8W

#else

#define DINPUT_VER_MIN 0x0200
#define DINPUT_VER_MAX 0x07ff

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD                                                              \
  ((DIDEVTYPE_HID) | (DIDEVTYPE_JOYSTICK) | ((DIDEVTYPEJOYSTICK_GAMEPAD) << 8))

using EarliestIDirectInput = IDirectInput;
using LatestIDirectInput = IDirectInput7;
using EarliestIDirectInputA = IDirectInputA;
using LatestIDirectInputA = IDirectInput7A;
using EarliestIDirectInputW = IDirectInputW;
using LatestIDirectInputW = IDirectInput7W;
using EarliestIDirectInputDevice = IDirectInputDevice;
using LatestIDirectInputDevice = IDirectInputDevice7;
using EarliestIDirectInputDeviceA = IDirectInputDeviceA;
using LatestIDirectInputDeviceA = IDirectInputDevice7A;
using EarliestIDirectInputDeviceW = IDirectInputDeviceW;
using LatestIDirectInputDeviceW = IDirectInputDevice7W;

#define IID_LatestIDirectInput        IID_IDirectInput7
#define IID_LatestIDirectInputA       IID_IDirectInput7A
#define IID_LatestIDirectInputW       IID_IDirectInput7W
#define IID_LatestIDirectInputDevice  IID_IDirectInputDevice7
#define IID_LatestIDirectInputDeviceA IID_IDirectInputDevice7A
#define IID_LatestIDirectInputDeviceW IID_IDirectInputDevice7W

#endif
