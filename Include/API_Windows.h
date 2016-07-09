/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * API_Windows.h
 *      Common header file for the correct version of the Windows API.
 *****************************************************************************/

#pragma once


// -------- WINDOWS API ---------------------------------------------------- //

#define WIN32_LEAN_AND_MEAN
#include <sdkddkver.h>
#include <windows.h>
#include <tchar.h>
