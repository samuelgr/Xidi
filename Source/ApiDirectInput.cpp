/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ApiDirectInput.cpp
 *   Pulls in all DirectInput GUID definitions to avoid linking with the DirectInput library.
 **************************************************************************************************/

// Defining GUIDs in this file depends on the below header file inclusion order. First the
// preprocessor symbols need to be set so that GUID variables are defined rather than referenced
// externally, and then the actual DirectInput-related GUID definitions need to be included.

// clang-format off

#include <initguid.h>
#include "ApiDirectInput.h"

// clang-format on
