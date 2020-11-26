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

#include "Hooks.h"
#include "Message.h"

#include <Hookshot/Hookshot.h>
#include <string_view>


// -------- INTERNAL FUNCTIONS --------------------------------------------- //

namespace Xidi
{
    static void OutputSetHookResult(const wchar_t* functionName, Hookshot::EResult setHookResult)
    {
        if (Hookshot::SuccessfulResult(setHookResult))
            Message::OutputFormatted(Message::ESeverity::Info, L"Successfully set hook for %s.", functionName);
        else
            Message::OutputFormatted(Message::ESeverity::Error, L"Failed (Hookshot::EResult = %u) to set hook for %s.", (unsigned int)setHookResult, functionName);
    }
}


// -------- ENTRY POINT ---------------------------------------------------- //

/// Hook module entry point. 
HOOKSHOT_HOOK_MODULE_ENTRY(hookshot)
{
    Xidi::OutputSetHookResult(L"CoCreateInstance", StaticHook_CoCreateInstance::SetHook(hookshot));
}
