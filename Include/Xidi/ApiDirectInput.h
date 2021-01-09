/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ApiDirectInput.h
 *   Common header file for the DirectInput API.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


// -------- DIRECTINPUT API ------------------------------------------------ //

#include <dinput.h>


// -------- CONSTANTS ------------------------------------------------------ //

/// Missing from `dinput.h`, this constant is used by built-in DirectInput data formats and presumably others.
/// Its intent is to signify that an element of the data format is optional, so setting the data format should not fail if the structure element remains unused.
/// In the absence of this flag, if an element of the requested data format does not have a controller object instance associated with it, setting the data format fails due to invalid parameter.
#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL                          0x80000000
#endif


// -------- TYPE DEFINITIONS ----------------------------------------------- //

/// Enumerates supported character type modes for DirectInput interfaces.
enum class ECharMode
{
    A,                                                                      ///< ASCII mode, denoted with an "A" suffix in Microsoft documentation.
    W                                                                       ///< Wide-character (Unicode) mode, denoted with a "W" suffix in Microsoft documentation.
};


// -------- VERSION-SPECIFIC MAPPINGS -------------------------------------- //

#if DIRECTINPUT_VERSION >= 0x0800

#define DINPUT_VER_MIN                          0x0800
#define DINPUT_VER_MAX                          0x08ff

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD           DI8DEVTYPE_GAMEPAD

typedef IDirectInput8                           EarliestIDirectInput;
typedef IDirectInput8                           LatestIDirectInput;
typedef IDirectInput8A                          EarliestIDirectInputA;
typedef IDirectInput8A                          LatestIDirectInputA;
typedef IDirectInput8W                          EarliestIDirectInputW;
typedef IDirectInput8W                          LatestIDirectInputW;
typedef IDirectInputDevice8                     EarliestIDirectInputDevice;
typedef IDirectInputDevice8                     LatestIDirectInputDevice;
typedef IDirectInputDevice8A                    EarliestIDirectInputDeviceA;
typedef IDirectInputDevice8A                    LatestIDirectInputDeviceA;
typedef IDirectInputDevice8W                    EarliestIDirectInputDeviceW;
typedef IDirectInputDevice8W                    LatestIDirectInputDeviceW;

#define IID_LatestIDirectInput                  IID_IDirectInput8
#define IID_LatestIDirectInputA                 IID_IDirectInput8A
#define IID_LatestIDirectInputW                 IID_IDirectInput8W
#define IID_LatestIDirectInputDevice            IID_IDirectInputDevice8
#define IID_LatestIDirectInputDeviceA           IID_IDirectInputDevice8A
#define IID_LatestIDirectInputDeviceW           IID_IDirectInputDevice8W

#else

#define DINPUT_VER_MIN                          0x0200
#define DINPUT_VER_MAX                          0x07ff

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD           ((DIDEVTYPE_JOYSTICK) | ((DIDEVTYPEJOYSTICK_GAMEPAD) << 8))

typedef IDirectInput                            EarliestIDirectInput;
typedef IDirectInput7                           LatestIDirectInput;
typedef IDirectInputA                           EarliestIDirectInputA;
typedef IDirectInput7A                          LatestIDirectInputA;
typedef IDirectInputW                           EarliestIDirectInputW;
typedef IDirectInput7W                          LatestIDirectInputW;
typedef IDirectInputDevice                      EarliestIDirectInputDevice;
typedef IDirectInputDevice7                     LatestIDirectInputDevice;
typedef IDirectInputDeviceA                     EarliestIDirectInputDeviceA;
typedef IDirectInputDevice7A                    LatestIDirectInputDeviceA;
typedef IDirectInputDeviceW                     EarliestIDirectInputDeviceW;
typedef IDirectInputDevice7W                    LatestIDirectInputDeviceW;

#define IID_LatestIDirectInput                  IID_IDirectInput7
#define IID_LatestIDirectInputA                 IID_IDirectInput7A
#define IID_LatestIDirectInputW                 IID_IDirectInput7W
#define IID_LatestIDirectInputDevice            IID_IDirectInputDevice7
#define IID_LatestIDirectInputDeviceA           IID_IDirectInputDevice7A
#define IID_LatestIDirectInputDeviceW           IID_IDirectInputDevice7W

#endif
