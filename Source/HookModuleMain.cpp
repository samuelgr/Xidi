/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file HookModuleMain.cpp
 *   Entry point when injecting Xidi as a hook module.
 **************************************************************************************************/

#include <Hookshot/Hookshot.h>

#include "SetHooks.h"

HOOKSHOT_HOOK_MODULE_ENTRY(hookshot)
{
  Xidi::SetHooksDirectInput(hookshot);
  Xidi::SetHooksWinMM(hookshot);
}
