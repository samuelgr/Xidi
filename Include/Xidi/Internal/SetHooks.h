/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file SetHooks.h
 *   Declaration of all Hookshot hooks set by the hook module and all associated functionality.
 **************************************************************************************************/

#pragma once

#include <Hookshot/StaticHook.h>

#include "ApiWindows.h"

namespace Xidi
{
  /// Support function. Outputs the result of a hook setting operation.
  /// @param [in] functionName Name of the function that was intended to be hooked.
  /// @param [in] setHookResult Result returned from Hookshot of the attempt to set the hook.
  void OutputSetHookResult(const wchar_t* functionName, Hookshot::EResult setHookResult);

  /// Top-level function for setting all hooks related to DirectInput.
  /// @param [in] hookshot Hookshot interface pointer.
  void SetHooksDirectInput(Hookshot::IHookshot* hookshot);

  /// Top-level function for setting all hooks related to WinMM.
  /// @param [in] hookshot Hookshot interface pointer.
  void SetHooksWinMM(Hookshot::IHookshot* hookshot);
} // namespace Xidi

// Windows API: CoCreateInstance
// Used to intercept attempts to create DirectInput COM objects.
HOOKSHOT_STATIC_HOOK(CoCreateInstance);
