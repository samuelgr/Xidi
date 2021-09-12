/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file HookModuleMain.cpp
 *   Entry point when injecting Xidi as a hook module.
 *****************************************************************************/

#include "SetHooks.h"

#include <Hookshot/Hookshot.h>


// -------- ENTRY POINT ---------------------------------------------------- //

/// Hook module entry point. 
HOOKSHOT_HOOK_MODULE_ENTRY(hookshot)
{
    Xidi::SetHooksDirectInput(hookshot);
    Xidi::SetHooksWinMM(hookshot);
}
