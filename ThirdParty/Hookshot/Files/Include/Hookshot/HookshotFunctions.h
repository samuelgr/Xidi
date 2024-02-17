/***************************************************************************************************
 * Hookshot
 *   General-purpose library for injecting DLLs and hooking function calls.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2019-2024
 ***********************************************************************************************//**
 * @file HookshotFunctions.h
 *   Function prototypes and macros for the public Hookshot interface. External users should
 *   include Hookshot.h instead of this file.
 **************************************************************************************************/

#pragma once

#include "HookshotTypes.h"

// Define this preprocessor symbol if linking directly with the Hookshot library and loading it by
// normal means. Projects that build hook modules and projects that load the Hookshot library at
// runtime (i.e. via LoadLibrary) should leave this preprocessor symbol undefined.
#ifdef HOOKSHOT_LINK_WITH_LIBRARY

/// Initializes the Hookshot library.
/// Hook modules must not invoke this function because Hookshot initializes itself before loading
/// them. If they do, this function will emit a warning message and fail with `nullptr`. When
/// linking directly against the Hookshot library, this function must be invoked once to obtain the
/// Hookshot interface pointer. If invoked multiple times, this function will emit a warning message
/// and fail with `nullptr` beginning with the second invocation. The returned Hookshot interface
/// pointer remains valid throughout the lifetime of the process and owned by Hookshot. It can only
/// be obtained once and therefore should be cached by the caller.
/// @return Hookshot interface pointer on success, or `nullptr` on failure.
extern "C" __declspec(dllimport) Hookshot::IHookshot* __fastcall HookshotLibraryInitialize(void);

#else

/// Convenient definition for the entry point of a Hookshot hook module.
/// If building a hook module, use this macro to create a hook module entry point.
/// Macro parameter is the desired name of the entry point's function parameter, namely the Hookshot
/// interface object pointer. The Hookshot interface object pointer can only be obtained this way
/// and therefore should be cached by the hook module.
#define HOOKSHOT_HOOK_MODULE_ENTRY(param)                                                          \
  extern "C" __declspec(dllexport) void __fastcall HookshotMain(Hookshot::IHookshot* param)

namespace Hookshot
{
  /// Type definition for a pointer to the Hookshot library initialization function, whose address
  /// can be retrieved via a call to a function like `GetProcAddress`.
  using TLibraryInitializeProc = Hookshot::IHookshot*(__fastcall*)(void);

#ifdef _WIN64
  /// Name of the Hookshot library initialization function, which can be passed directly to a
  /// function like `GetProcAddress`. Valid in 64-bit mode.
  inline constexpr char kLibraryInitializeProcName[] = "HookshotLibraryInitialize";
#else
  /// Name of the Hookshot library initialization function, which can be passed directly to a
  /// function like `GetProcAddress`. Valid in 32-bit mode.
  inline constexpr char kLibraryInitializeProcName[] = "@HookshotLibraryInitialize@0";
#endif
} // namespace Hookshot

#endif
