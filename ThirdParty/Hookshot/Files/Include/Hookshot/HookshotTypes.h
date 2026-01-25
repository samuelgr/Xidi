/***************************************************************************************************
 * Hookshot
 *   General-purpose library for injecting DLLs and hooking function calls.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2019-2025
 ***********************************************************************************************//**
 * @file HookshotTypes.h
 *   Common types definitions and associated helper functions used in the public Hookshot
 *   interface. External users should include Hookshot.h instead of this file.
 **************************************************************************************************/

#pragma once

#include <string_view>

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

    /// Operation was already performed and cannot be performed again.
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

  /// Convenience function used to determine if a Hookshot operation succeeded.
  /// @param [in] result Result code returned from most Hookshot functions.
  /// @return `true` if the identifier represents success, `false` otherwise.
  inline bool SuccessfulResult(const EResult result)
  {
    return (result < EResult::BoundaryValue);
  }

  /// Convenience function used to determine if a Hookshot operation succeeded, overloaded for those
  /// operations that return an error message rather than a result enumerator.
  /// @param [in] result Result error message returned from certain Hookshot functions.
  /// @return `true` if the result indicates success, `false` otherwise.
  inline bool SuccessfulResult(std::wstring_view result)
  {
    return (true == result.empty());
  }
} // namespace Hookshot
