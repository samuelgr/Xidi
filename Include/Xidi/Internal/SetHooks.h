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

  /// Sets all hooks and replaces Xidi import functions for the specified library.
  /// @param [in] hookshot Hookshot interface pointer.
  /// @param [in] apiImportFunctions Xidi API interface pointer for replacing Xidi's imported
  /// functions.
  /// @param [in] xidiLibraryhandle Handle for the main Xidi library.
  /// @param [in] libraryIdentifier Xidi API library identifier for the library for which hooks
  /// should be set and import functions replaced.
  /// @param [in] libraryName Printable name of the library. Used for logging.
  /// @param [in] libraryFilename Path to the library that was already loaded and for which hooks
  /// should be set.
  /// @param [in] replacementFunctionPrefix Prefix to add to the import function name in order to
  /// find it in the main Xidi library.
  void SetHooksForLibrary(
      Hookshot::IHookshot* hookshot,
      Api::IImportFunctions2* apiImportFunctions,
      HMODULE xidiLibraryHandle,
      Api::IImportFunctions2::ELibrary libraryIdentifier,
      const wchar_t* libraryName,
      const wchar_t* libraryFilename,
      const wchar_t* replacementFunctionPrefix);

  /// Sets a hook on the COM function `CoCreateInstance`.
  /// @param [in] hookshot Hookshot interface pointer.
  void SetHookCoCreateInstance(Hookshot::IHookshot* hookshot);
} // namespace Xidi

// Windows API: CoCreateInstance
// Used to intercept attempts to create DirectInput COM objects.
HOOKSHOT_STATIC_HOOK(CoCreateInstance);
