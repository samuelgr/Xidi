/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * ApiDirectInput.h
 *      Common header file for the DirectInput API.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


// -------- DIRECTINPUT API ------------------------------------------------ //

#include <dinput.h>


// -------- VERSION-SPECIFIC MAPPINGS -------------------------------------- //

#if DIRECTINPUT_VERSION >= 0x0800

#define DINPUT_VER_MIN                          0x0800
#define DINPUT_VER_MAX                          0x08ff

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD           DI8DEVTYPE_GAMEPAD

typedef IDirectInput8                           EarliestIDirectInput;
typedef IDirectInput8                           LatestIDirectInput;
typedef IDirectInputDevice8                     EarliestIDirectInputDevice;
typedef IDirectInputDevice8                     LatestIDirectInputDevice;

#else

#define DINPUT_VER_MIN                          0x0200
#define DINPUT_VER_MAX                          0x07ff

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD           ((DIDEVTYPE_JOYSTICK) | ((DIDEVTYPEJOYSTICK_GAMEPAD) << 8))

typedef IDirectInput                            EarliestIDirectInput;
typedef IDirectInput7                           LatestIDirectInput;
typedef IDirectInputDevice                      EarliestIDirectInputDevice;
typedef IDirectInputDevice7                     LatestIDirectInputDevice;

#endif
