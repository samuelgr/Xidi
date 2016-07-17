/*****************************************************************************
 * XInputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ApiWindows.h
 *      Common header file for the correct version of the Windows API.
 *****************************************************************************/

#pragma once


// -------- WINDOWS API ---------------------------------------------------- //

#define WIN32_LEAN_AND_MEAN
#include <sdkddkver.h>
#include <windows.h>
#include <tchar.h>
