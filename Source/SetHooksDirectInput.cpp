/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file SetHooksDirectInput.cpp
 *   Implementation of all functionality for setting DirectInput hooks.
 **************************************************************************************************/

#include <Hookshot/Hookshot.h>

#include "Message.h"
#include "SetHooks.h"

namespace Xidi
{
  void SetHooksDirectInput(Hookshot::IHookshot* hookshot)
  {
    Message::Output(Message::ESeverity::Info, L"Beginning to set hooks for DirectInput.");
    OutputSetHookResult(L"CoCreateInstance", StaticHook_CoCreateInstance::SetHook(hookshot));
  }
} // namespace Xidi
