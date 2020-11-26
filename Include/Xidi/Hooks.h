/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Hooks.h
 *   Declaration of all Hookshot hooks set by the hook module.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <Hookshot/StaticHook.h>


// -------- HOOKS ---------------------------------------------------------- //

// Windows API: CoCreateInstance
// Used to intercept attempts to create 
HOOKSHOT_STATIC_HOOK(CoCreateInstance);
