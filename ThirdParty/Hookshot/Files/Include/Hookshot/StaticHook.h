/***************************************************************************************************
 * Hookshot
 *   General-purpose library for injecting DLLs and hooking function calls.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2019-2024
 ***********************************************************************************************//**
 * @file StaticHook.h
 *   Convenience wrapper types and definitions for creating static hooks. A static hook is one
 *   whose original function address is available at compile or link time. Examples include API
 *   functions declared in header files and defined in libraries against which a hook module
 *   links. Windows API functions, exported by system-supplied libraries, can generally be hooked
 *   this way. A key advantage of using static hooks, as opposed to calling Hookshot functions
 *   directly, is type safety: return type, calling convention, and argument types are
 *   automatically detected from the declaration of the function being hooked, and any accidental
 *   mismatches trigger compiler errors. This file is intended to be included externally.
 **************************************************************************************************/

#pragma once

#include <type_traits>
#include <utility>

#include "Hookshot.h"

/// Declares a static hook. Defines a type to represent a hook for the specified function. Parameter
/// is the name of the function being hooked. Type name is of the "StaticHook_[function name]" and
/// is created in whatever namespace encloses the invocation of this macro. Relevant static members
/// of the created type are `Hook` (the hook function, which must be implemented) and `Original`
/// (automatically implemented to provide access to the original un-hooked functionality of the
/// specified function). To activate the static hook, the `SetHook` method must be invoked at
/// runtime. Function prototypes for both `Hook` and `Original` are automatically set to match that
/// of the specified function, including calling convention. To define the hook function, simply
/// provide a funciton body for `StaticHook_[function name]::Hook`. The invocation of this macro
/// should be placed in a location visible wherever access to the underlying type is needed. It is
/// safe to place in a header file that is included in multiple places. Note that Hookshot might
/// fail to create the requested hook. Therefore, the return code from `SetHook` should be checked.
/// Once `SetHook` has been invoked successfully, further invocations have no effect and simply
/// return `EResult::NoEffect`.
#define HOOKSHOT_STATIC_HOOK(func)                                                                 \
  namespace _HookshotInternal                                                                      \
  {                                                                                                \
    inline constexpr wchar_t kHookName__##func[] = _CRT_WIDE(#func);                               \
  }                                                                                                \
  using StaticHook_##func = ::Hookshot::                                                           \
      StaticHook<_HookshotInternal::kHookName__##func, (void*)(&(func)), decltype(func)>

/// Retrieves a proxy object for the specified static hook. Proxy objects can be used to manipulate
/// static hooks using an object-oriented interface.
#define HOOKSHOT_STATIC_HOOK_PROXY(name) StaticHook_##name::GetProxy()

namespace Hookshot
{
  /// Proxy object for manipulating a static hook using an object-oriented interface.
  class StaticHookProxy
  {
  public:

    using TIsHookSetFunc = bool (*)(void);
    using TSetHookFunc = EResult (*)(IHookshot* const);
    using TDisableHookFunc = EResult (*)(IHookshot* const);
    using TEnableHookFunc = EResult (*)(IHookshot* const);
    using TGetFunctionNameFunc = const wchar_t* (*)(void);

    /// Not intended for external invocation. Objects of this class should be constructed using the
    /// appropriate proxy macro above.
    inline StaticHookProxy(
        TIsHookSetFunc funcIsHookSet,
        TSetHookFunc funcSetHook,
        TDisableHookFunc funcDisableHook,
        TEnableHookFunc funcEnableHook,
        TGetFunctionNameFunc funcGetFunctionName)
        : funcIsHookSet(funcIsHookSet),
          funcSetHook(funcSetHook),
          funcDisableHook(funcDisableHook),
          funcEnableHook(funcEnableHook),
          funcGetFunctionName(funcGetFunctionName)
    {}

    /// Determines if the hook has already been set for the associated static hook.
    /// @return `true` if the hook has already been set successfully, `false` otherwise.
    inline bool IsHookSet(void) const
    {
      return funcIsHookSet();
    }

    /// Attempts to set the associated static hook. If this function completes successfully, then
    /// the original function is effectively "replaced" by the associated static hook's hook
    /// function.
    /// @param [in] hookshot Interface pointer through which all Hookshot functionality is accessed.
    /// @return Result of the operation. See "HookshotTypes.h" for possible enumerators.
    inline EResult SetHook(IHookshot* const hookshot) const
    {
      return funcSetHook(hookshot);
    }

    /// Disables the associated static hook. Bypasses the hook function and redirects everything to
    /// the original function.
    /// @param [in] hookshot Interface pointer through which all Hookshot functionality is accessed.
    /// @return Result of the operation. See "HookshotTypes.h" for possible enumerators.
    inline EResult DisableHook(IHookshot* const hookshot) const
    {
      return funcDisableHook(hookshot);
    }

    /// Enables the associated static hook. Reinstates the hook function such that it once again
    /// replaces the original function.
    /// @param [in] hookshot Interface pointer through which all Hookshot functionality is accessed.
    /// @return Result of the operation. See "HookshotTypes.h" for possible enumerators.
    inline EResult EnableHook(IHookshot* const hookshot) const
    {
      return funcEnableHook(hookshot);
    }

    /// Retrieves a C string representation of the name of the original function. This was the
    /// `func` parameter passed into the #HOOKSHOT_STATIC_HOOK macro.
    /// @return C string representation of the name of the original function.
    inline const wchar_t* GetFunctionName(void) const
    {
      return funcGetFunctionName();
    }

  private:

    TIsHookSetFunc funcIsHookSet;
    TSetHookFunc funcSetHook;
    TDisableHookFunc funcDisableHook;
    TEnableHookFunc funcEnableHook;
    TGetFunctionNameFunc funcGetFunctionName;
  };
} // namespace Hookshot

// Everything below this point is internal to the implementation of static hooks. External users
// should only make use of the macro above.

/// Implements static hook template specialization so that function prototypes and calling
/// conventions are automatically extracted based on the supplied function. Parameters are just
/// different syntactic representations of calling conventions, which are used to create one
/// template specialization for calling convention.
#define HOOKSHOT_STATIC_HOOK_TEMPLATE(callingConvention, callingConventionInBrackets)              \
  template <                                                                                       \
      const wchar_t* kOriginalFunctionName,                                                        \
      void* const kOriginalFunctionAddress,                                                        \
      typename ReturnType,                                                                         \
      typename... ArgumentTypes>                                                                   \
  class StaticHook<                                                                                \
      kOriginalFunctionName,                                                                       \
      kOriginalFunctionAddress,                                                                    \
      ReturnType callingConventionInBrackets(ArgumentTypes...)>                                    \
      : public StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>                     \
  {                                                                                                \
  public:                                                                                          \
                                                                                                   \
    typedef ReturnType callingConvention TFunction(ArgumentTypes...);                              \
    typedef ReturnType(callingConvention* TFunctionPtr)(ArgumentTypes...);                         \
    static ReturnType callingConvention Hook(ArgumentTypes...);                                    \
    static ReturnType callingConvention Original(ArgumentTypes... args)                            \
    {                                                                                              \
      return ((ReturnType(callingConvention*)(                                                     \
          ArgumentTypes...))StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>::      \
                  GetOriginalFunction())(std::forward<ArgumentTypes>(args)...);                    \
    }                                                                                              \
                                                                                                   \
    static bool IsHookSet(void)                                                                    \
    {                                                                                              \
      return StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>::IsHookSet();         \
    }                                                                                              \
                                                                                                   \
    static EResult SetHook(IHookshot* const hookshot)                                              \
    {                                                                                              \
      return StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>::SetHook(             \
          hookshot, &Hook);                                                                        \
    }                                                                                              \
                                                                                                   \
    static EResult DisableHook(IHookshot* const hookshot)                                          \
    {                                                                                              \
      return hookshot->DisableHookFunction(&Hook);                                                 \
    }                                                                                              \
                                                                                                   \
    static EResult EnableHook(IHookshot* const hookshot)                                           \
    {                                                                                              \
      return hookshot->ReplaceHookFunction(kOriginalFunctionAddress, &Hook);                       \
    }                                                                                              \
                                                                                                   \
    static const wchar_t* GetFunctionName(void)                                                    \
    {                                                                                              \
      return kOriginalFunctionName;                                                                \
    }                                                                                              \
                                                                                                   \
    static StaticHookProxy GetProxy(void)                                                          \
    {                                                                                              \
      return StaticHookProxy(&IsHookSet, &SetHook, &DisableHook, &EnableHook, &GetFunctionName);   \
    }                                                                                              \
  };

namespace Hookshot
{
  /// Base class for all static hooks. Used to hide implementation details from external users.
  template <const wchar_t* kOriginalFunctionName, void* const kOriginalFunctionAddress>
  class StaticHookBase
  {
  public:

    StaticHookBase(void) = delete;
    StaticHookBase(const StaticHookBase& other) = delete;
    StaticHookBase(StaticHookBase&& other) = delete;

  protected:

    static inline const void* GetOriginalFunction(void)
    {
      return originalFunction;
    }

    static inline bool IsHookSet(void)
    {
      return (nullptr != originalFunction);
    }

    static inline EResult SetHook(IHookshot* const hookshot, const void* hookFunc)
    {
      if (true == IsHookSet()) return EResult::NoEffect;

      const EResult result = hookshot->CreateHook(kOriginalFunctionAddress, hookFunc);

      if (SuccessfulResult(result))
        originalFunction = hookshot->GetOriginalFunction(kOriginalFunctionAddress);

      return result;
    }

  private:

    inline static const void* originalFunction = nullptr;
  };

  /// Primary static hook template. Specialized using #HOOKSHOT_STATIC_HOOK_TEMPLATE.
  template <const wchar_t* kOriginalFunctionName, void* const kOriginalFunctionAddress, typename T>
  class StaticHook
  {
    static_assert(
        std::is_function<T>::value,
        "Supplied argument in StaticHook declaration must map to a function type.");
  };

#ifdef _WIN64
  HOOKSHOT_STATIC_HOOK_TEMPLATE(, );
#else
  HOOKSHOT_STATIC_HOOK_TEMPLATE(__cdecl, (__cdecl));
  HOOKSHOT_STATIC_HOOK_TEMPLATE(__fastcall, (__fastcall));
  HOOKSHOT_STATIC_HOOK_TEMPLATE(__stdcall, (__stdcall));
  HOOKSHOT_STATIC_HOOK_TEMPLATE(__vectorcall, (__vectorcall));
#endif
} // namespace Hookshot
