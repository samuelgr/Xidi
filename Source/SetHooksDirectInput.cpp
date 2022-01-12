/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file SetHooksDirectInput.cpp
 *   Implementation of all functionality for setting DirectInput hooks.
 *****************************************************************************/

#include "Message.h"
#include "SetHooks.h"

#include <Hookshot/Hookshot.h>


namespace Xidi
{
    // -------- FUNCTIONS -------------------------------------------------- //
    // See "SetHooks.h" for documentation.

    void SetHooksDirectInput(Hookshot::IHookshot* hookshot)
    {
        Message::Output(Message::ESeverity::Info, L"Beginning to set hooks for DirectInput.");
        OutputSetHookResult(L"CoCreateInstance", StaticHook_CoCreateInstance::SetHook(hookshot));
    }
}
