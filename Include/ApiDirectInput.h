/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
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

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD           DI8DEVTYPE_GAMEPAD

typedef IDirectInput8                           VersionedIDirectInput;
typedef IDirectInputDevice8                     VersionedIDirectInputDevice;

#else

#define DINPUT_DEVTYPE_XINPUT_GAMEPAD           ((DIDEVTYPE_JOYSTICK) | ((DIDEVTYPEJOYSTICK_GAMEPAD) << 8))

typedef IDirectInput7                           VersionedIDirectInput;
typedef IDirectInputDevice7                     VersionedIDirectInputDevice;

#endif
