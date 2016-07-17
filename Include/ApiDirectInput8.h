/*****************************************************************************
 * XInputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ApiDirectInput8.h
 *      Common header file for the DirectInput8 API.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


// -------- DIRECTINPUT 8 API ---------------------------------------------- //

#define DIRECTINPUT_VERSION                     0x0800
#include <dinput.h>
