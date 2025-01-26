/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file DllFunctions.h
 *   Declaration of types, functions, and macros for importing functions from a DLL, exporting
 *   functions from a DLL, and exporting functions that are forwarded perfectly to another one.
 **************************************************************************************************/

#pragma once

#include <string_view>

#include "ApiWindows.h"

/// Defines a destination DLL for forwarding exported API calls.
#define DLL_EXPORT_FORWARD_DEFINE_DLL(libraryName)                                                 \
  static std::wstring_view _Xidi_DllFunctionsInternal_GetLibraryPath_##libraryName(void)           \
  {                                                                                                \
    return _CRT_WIDE(#libraryName);                                                                \
  }

/// Defines a destination DLL for forwarding exported API calls whose path is returned as a
/// wide-character string view from a function implemented immediately below this macro. The macro
/// serves as the function signature line.
#define DLL_EXPORT_FORWARD_DEFINE_DLL_WITH_CUSTOM_PATH(libraryName)                                \
  static std::wstring_view _Xidi_DllFunctionsInternal_GetLibraryPath_##libraryName(void)

/// Defines an exported function to be forwarded to the specified DLL, such that the DLL name was
/// previously defined using one of the preceding macros.
#define DLL_EXPORT_FORWARD(libraryName, funcName)                                                  \
  namespace _Xidi_DllFunctionsInternal                                                             \
  {                                                                                                \
    extern "C" void* _ptr_export_##libraryName##_##funcName = nullptr;                             \
    static ::Xidi::DllFunctions::ForwardedFunction dllForwardedFunctionInstance_##funcName(        \
        &_Xidi_DllFunctionsInternal_GetLibraryPath_##libraryName,                                  \
        #funcName,                                                                                 \
        &_ptr_export_##libraryName##_##funcName);                                                  \
  }

namespace Xidi
{
  namespace DllFunctions
  {
    /// Internal implementation of the high-level language part of the functionality for perfect
    /// forwarding of external API calls to another DLL.
    class ForwardedFunction
    {
    public:

      /// Type alias for representing a function that returns the path of the library to which
      /// function calls should be forwarded.
      using TLibraryPathFunc = std::wstring_view (*)(void);

      /// Constructs a forwarded function object for the specified function whose address is to be
      /// placed at the specified location.
      ForwardedFunction(TLibraryPathFunc libraryPathFunc, std::string_view funcName, void** ptr);

      /// Retrieves the path of the library to which to forward this exported function call.
      /// @return Library path to which to forward this function call.
      inline std::wstring_view GetLibraryPath(void)
      {
        return libraryPathFunc();
      }

      /// Retrieves the name of the exported function itself, also the name of the entry point in
      /// the destination library.
      /// @return Name of the entry point being exported.
      inline std::string_view GetFunctionName(void)
      {
        return funcName;
      }

      /// Sets the destination procedure address in the target library, to be invoked whenever this
      /// export entry point is also invoked.
      /// @param entryPoint Entry point address to set.
      inline void SetProcAddress(void* entryPoint)
      {
        *ptr = entryPoint;
      }

    private:

      /// Function that returns the path of the library to which function calls should be forwarded.
      TLibraryPathFunc libraryPathFunc;

      /// Name of the entry point within the library to which to forward the exported function call.
      /// Also matches the entry point name for the exported function in this module. Must be
      /// null-terminated.
      std::string_view funcName;

      /// Address of a variable to receive the entry point in the destination library.
      void** ptr;
    };

    /// Attempts to load the specified function from the specified library. Logs an error message on
    /// failure.
    /// @param [in] libraryPath Path of the library from which the import is being attempted. Used
    /// for logging only.
    /// @param [in] libraryHandle Handle of the loaded library.
    /// @param [in] functionName Name of the function to be imported.
    /// @param [out] procAddressDestination Address of a pointer to be filled with the address of
    /// the imported function. On failure, will be overwritten with `nullptr`.
    bool TryImport(
        std::wstring_view libraryPath,
        HMODULE libraryHandle,
        const char* functionName,
        const void** procAddressDestination);
  } // namespace DllFunctions
} // namespace Xidi
