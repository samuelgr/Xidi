/******************************************************************************
 * Hookshot
 *   General-purpose library for injecting DLLs and hooking function calls.
 ******************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2019-2023
 **************************************************************************//**
 * @file StaticHook.h
 *   Convenience wrapper types and definitions for creating static hooks. A
 *   static hook is one whose original function address is available at
 *   compile or link time. Examples include API functions declared in header
 *   files and defined in libraries against which a hook module links.
 *   Windows API functions, exported by system-supplied libraries, can
 *   generally be hooked this way. A key advantage of using static hooks, as
 *   opposed to calling Hookshot functions directly, is type safety: return
 *   type, calling convention, and argument types are automatically detected
 *   from the declaration of the function being hooked, and any accidental
 *   mismatches trigger compiler errors. This file is intended to be
 *   included externally.
 *****************************************************************************/

#pragma once

#include "Hookshot.h"

#include <type_traits>


// -------- MACROS --------------------------------------------------------- //
// These provide the interface to static hooks.

/// Declares a static hook. Defines a type to represent a hook for the specified function. Parameter is the name of the function being hooked.
/// Type name is of the format "StaticHook_[function name]" and is created in whatever namespace encloses the invocation of this macro.
/// Relevant static members of the created type are `Hook` (the hook function, which must be implemented) and `Original` (automatically implemented to provide access to the original un-hooked functionality of the specified function).
/// To activate the static hook, the `SetHook` method must be invoked at runtime.
/// Function prototypes for both `Hook` and `Original` are automatically set to match that of the specified function, including calling convention.
/// To define the hook function, simply provide a funciton body for `StaticHook_[function name]::Hook`.
/// The invocation of this macro should be placed in a location visible wherever access to the underlying type is needed. It is safe to place in a header file that is included in multiple places.
/// Note that Hookshot might fail to create the requested hook. Therefore, the return code from `SetHook` should be checked.
/// Once `SetHook` has been invoked successfully, further invocations have no effect and simply return EResult::NoEffect.
#define HOOKSHOT_STATIC_HOOK(func) \
    inline constexpr wchar_t kHookName__##func[] = _CRT_WIDE(#func); \
    using StaticHook_##func = ::Hookshot::StaticHook<kHookName__##func, (void*)(&(func)), decltype(func)>


// -------- IMPLEMENTATION DETAILS ----------------------------------------- //
// Everything below this point is internal to the implementation of static hooks.

/// Implements static hook template specialization so that function prototypes and calling conventions are automatically extracted based on the supplied function.
/// Parameters are just different syntactic representations of calling conventions, which are used to create one template specialization for calling convention.
#define HOOKSHOT_STATIC_HOOK_TEMPLATE(callingConvention, callingConventionInBrackets) \
    template <const wchar_t* kOriginalFunctionName, void* const kOriginalFunctionAddress, typename ReturnType, typename... ArgumentTypes> class StaticHook<kOriginalFunctionName, kOriginalFunctionAddress, ReturnType callingConventionInBrackets (ArgumentTypes...)> : public StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress> \
    { \
    public: \
        static ReturnType callingConvention Hook(ArgumentTypes...); \
        static ReturnType callingConvention Original(ArgumentTypes... args) { return ((ReturnType(callingConvention *)(ArgumentTypes...))StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>::GetOriginalFunction())(args...); } \
        static EResult SetHook(IHookshot* const hookshot) { return StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>::SetHook(hookshot, &Hook); } \
        static EResult DisableHook(IHookshot* const hookshot) { return hookshot->DisableHookFunction(&Hook); } \
        static EResult EnableHook(IHookshot* const hookshot) { return hookshot->ReplaceHookFunction(kOriginalFunctionAddress, &Hook); } \
    };

namespace Hookshot
{
    /// Base class for all static hooks.
    /// Used to hide implementation details from external users.
    template <const wchar_t* kOriginalFunctionName, void* const kOriginalFunctionAddress> class StaticHookBase
    {
    private:
        static const void* originalFunction;

    protected:
        static inline const void* GetOriginalFunction(void)
        {
            return originalFunction;
        }

        static inline EResult SetHook(IHookshot* const hookshot, const void* hookFunc)
        {
            if (nullptr != originalFunction)
                return EResult::NoEffect;

            const EResult result = hookshot->CreateHook(kOriginalFunctionAddress, hookFunc);

            if (SuccessfulResult(result))
                originalFunction = hookshot->GetOriginalFunction(kOriginalFunctionAddress);

            return result;
        }
    };
    template <const wchar_t* kOriginalFunctionName, void* const kOriginalFunctionAddress> const void* StaticHookBase<kOriginalFunctionName, kOriginalFunctionAddress>::originalFunction = nullptr;
    
    /// Primary static hook template. Specialized using #HOOKSHOT_STATIC_HOOK_TEMPLATE.
    template <const wchar_t* kOriginalFunctionName, void* const kOriginalFunctionAddress, typename T> class StaticHook
    {
        static_assert(std::is_function<T>::value, "Supplied argument in StaticHook declaration must map to a function type.");
    };

#ifdef _WIN64
    HOOKSHOT_STATIC_HOOK_TEMPLATE( , );
#else
    HOOKSHOT_STATIC_HOOK_TEMPLATE(__cdecl, (__cdecl));
    HOOKSHOT_STATIC_HOOK_TEMPLATE(__fastcall, (__fastcall));
    HOOKSHOT_STATIC_HOOK_TEMPLATE(__stdcall, (__stdcall));
    HOOKSHOT_STATIC_HOOK_TEMPLATE(__vectorcall, (__vectorcall));
#endif
}
