/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file SetHooks.h
 *   Declaration of all Hookshot hooks set by the hook module and all associated functionality.
 **************************************************************************************************/

#pragma once

#include <Hookshot/StaticHook.h>

#include "ApiWindows.h"
#include "ApiXidi.h"

namespace Xidi
{
  /// Support function. Outputs the result of a hook setting operation.
  /// @param [in] functionName Name of the function that was intended to be hooked.
  /// @param [in] setHookResult Result returned from Hookshot of the attempt to set the hook.
  void OutputSetHookResult(const wchar_t* functionName, Hookshot::EResult setHookResult);

  /// Top-level function for setting all hooks related to DirectInput.
  /// @param [in] hookshot Hookshot interface pointer.
  void SetHookCoCreateInstance(Hookshot::IHookshot* hookshot);

  /// Top-level function for setting all hooks related to WinMM.
  /// @param [in] hookshot Hookshot interface pointer.
  /// @param [in] apiImportFunctions Xidi API interface pointer for replacing Xidi's imported
  /// functions.
  /// @param [in] xidiLibraryhandle Handle for the main Xidi library.
  /// @param [in] winmmLibraryPath Path to the WinMM library that was already loaded and for which
  /// hooks should be set.
  void SetHooksWinMM(
      Hookshot::IHookshot* hookshot,
      Api::IImportFunctions2* apiImportFunctions,
      HMODULE xidiLibraryHandle,
      const wchar_t* winmmLibraryFilename);
} // namespace Xidi

// Windows API: CoCreateInstance
// Used to intercept attempts to create DirectInput COM objects.
HOOKSHOT_STATIC_HOOK(CoCreateInstance);
