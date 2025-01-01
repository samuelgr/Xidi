/***************************************************************************************************
 * Hookshot
 *   General-purpose library for injecting DLLs and hooking function calls.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2019-2025
 ***********************************************************************************************//**
 * @file HookshotTypes.h
 *   Types definitions used in the public Hookshot interface. External users should include
 *   Hookshot.h instead of this file.
 **************************************************************************************************/

#pragma once

namespace Hookshot
{
  /// Enumeration of possible results from Hookshot functions.
  enum class EResult
  {
    /// Operation was successful.
    Success,
    /// Operation did not generate an error but had no effect.
    NoEffect,

    /// Boundary value between success and failure, not used as an error code.
    BoundaryValue,

    /// Unable to allocate a new hook data structure.
    FailAllocation,

    /// Method was invoked at an inappropriate time. Hook modules may not invoke Hookshot methods
    /// until their entry point.
    FailBadState,

    /// Failed to set the hook.
    FailCannotSetHook,

    /// Specified function is already hooked.
    FailDuplicate,

    /// An argument that was supplied is invalid.
    FailInvalidArgument,

    /// Internal error.
    FailInternal,

    /// Unable to find a hook using the supplied identification.
    FailNotFound,

    /// Upper sentinel value, not used as an error code.
    UpperBoundValue
  };

  /// Convenience function used to determine if a hook operation succeeded.
  /// @param [in] result Result code returned from most Hookshot functions.
  /// @return `true` if the identifier represents success, `false` otherwise.
  inline bool SuccessfulResult(const EResult result)
  {
    return (result < EResult::BoundaryValue);
  }

  /// Main interface used to access all Hookshot functionality. During initialization, Hookshot
  /// creates instances of objects that implement this interface as needed. Any hook modules that
  /// Hookshot loads are provided with an interface pointer when executing their entry point
  /// functions. Alternatively, if loading the Hookshot library directly, an interface pointer is
  /// returned upon completion of library initialization. Interface pointers remain valid throughout
  /// the lifetime of the process and are owned by Hookshot. Its methods can be called at any time
  /// and are completely concurrency-safe. However, it is highly recommended that results be cached
  /// where possible, because most methods require taking some form of a lock.
  class IHookshot
  {
  public:

    /// Causes Hookshot to attempt to install a hook on the specified function.
    /// @param [in] originalFunc Address of the function that should be hooked.
    /// @param [in] hookFunc Hook function that should be invoked instead of the original function.
    /// @return Result of the operation.
    virtual EResult __fastcall CreateHook(void* originalFunc, const void* hookFunc) = 0;

    /// Disables the hook function associated with the specified hook. On success, going forward all
    /// invocations of the original function will execute as if not hooked at all, and Hookshot no
    /// longer associates the hook function with the hook. To re-enable the hook, use
    /// #ReplaceHookFunction and identify the hook by its original function address.
    /// @param [in] originalOrHookFunc Address of either the original function or the current hook
    /// function (it does not matter which) currently associated with the hook.
    /// @return Result of the operation.
    virtual EResult __fastcall DisableHookFunction(const void* originalOrHookFunc) = 0;

    /// Retrieves and returns an address that, when invoked as a function, calls the original (i.e.
    /// un-hooked) version of the hooked function. This is useful for accessing the original
    /// behavior of the function being hooked. It is up to the caller to ensure that invocations to
    /// the returned address satisfy all calling convention and parameter type requirements of the
    /// original function. The returned address is not the original entry point of the hooked
    /// function but rather a trampoline address that Hookshot created when installing the hook.
    /// @param [in] originalOrHookFunc Address of either the original function or the hook function
    /// (it does not matter which) currently associated with the hook.
    /// @return Address that can be invoked to access the functionality of the original function, or
    /// `nullptr` in the event that a hook cannot be found matching the specified function.
    virtual const void* __fastcall GetOriginalFunction(const void* originalOrHookFunc) = 0;

    /// Modifies an existing hook by replacing its hook function. The existing hook is identified
    /// either by the address of the original function or the address of the current hook function.
    /// On success, Hookshot associates the new hook function with the hook and forgets about the
    /// old hook function.
    /// @param [in] originalOrHookFunc Address of either the original function or the hook function
    /// (it does not matter which) currently associated with the hook.
    /// @param [in] newHookFunc Address of the new hook function.
    /// @return Result of the operation.
    virtual EResult __fastcall ReplaceHookFunction(
        const void* originalOrHookFunc, const void* newHookFunc) = 0;
  };
} // namespace Hookshot
