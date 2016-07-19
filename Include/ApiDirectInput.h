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

typedef IDirectInput8                           VersionedIDirectInput;
typedef IDirectInputDevice8                     VersionedIDirectInputDevice;

#else

typedef IDirectInput7                           VersionedIDirectInput;
typedef IDirectInputDevice7                     VersionedIDirectInputDevice;

#endif
