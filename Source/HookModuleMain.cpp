/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file HookModuleMain.cpp
 *   Entry point when injecting Xidi as a hook module.
 *****************************************************************************/

#include "Globals.h"
#include "Message.h"

#include <Hookshot/Hookshot.h>


// -------- ENTRY POINT ---------------------------------------------------- //

/// Hook module entry point. 
HOOKSHOT_HOOK_MODULE_ENTRY(hookshot)
{
    Xidi::Message::Output(Xidi::Message::ESeverity::Info, L"Successfully loaded Xidi as a Hookshot hook module.");
}
