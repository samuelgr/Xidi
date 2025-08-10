/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file DllFunctions.cpp
 *   Implementation of functionality for importing functions from a DLL, exporting functions from
 *   a DLL, and exporting functions that are forwarded perfectly to another one.
 **************************************************************************************************/

#include "DllFunctions.h"

#include <map>
#include <mutex>
#include <string_view>
#include <unordered_map>

#include "ApiWindows.h"

#include "Infra/Core/Message.h"
#include "Infra/Core/Strings.h"
#include "Infra/Core/TemporaryBuffer.h"

namespace Xidi
{
  namespace DllFunctions
  {
    /// Type alias for mapping from a library name to a library handle. Library names are
    /// case-insensitive.
    using TLibraryNameToHandleMap = std::unordered_map<
        std::wstring_view,
        HMODULE,
        Infra::Strings::CaseInsensitiveHasher<wchar_t>,
        Infra::Strings::CaseInsensitiveEqualityComparator<wchar_t>>;

    /// Type alias for mapping from a function name to a forwarded function object. Function names
    /// are case-sensitive.
    using TForwardedFunctionNameToObjectMap = std::map<std::string_view, ForwardedFunction*>;

    /// Holds handles for all libraries to which functions are being forwarded.
    static TLibraryNameToHandleMap libraryHandles;

    /// Holds an index of all registered forwarded functions. Maps from forwarded function name to
    /// associated object.
    static TForwardedFunctionNameToObjectMap allForwardedFunctions;

    ForwardedFunction::ForwardedFunction(
        ForwardedFunction::TLibraryPathFunc libraryPathFunc, std::string_view funcName, void** ptr)
        : libraryPathFunc(libraryPathFunc), funcName(funcName), ptr(ptr)
    {
      allForwardedFunctions.emplace(funcName, this);
    }

    /// Writes to the log a failure to import a specific function from a library.
    /// @param [in] libraryPath Path of the library from which the function was attempted to be
    /// imported.
    /// @param [in] functionName Name of the exported function that was attempted to be imported.
    static void LogImportFailed(std::wstring_view libraryPath, std::string_view functionName)
    {
      Infra::TemporaryString functionNameWide =
          Infra::Strings::ConvertNarrowToWide(functionName.data());
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Library %.*s is missing entry point %s. Attempts to call it will fail.",
          static_cast<int>(libraryPath.length()),
          libraryPath.data(),
          functionNameWide.AsCString());
    }

    /// Shows an error and terminates the process in the event of failure to load a particular
    /// library dependency from which functions are to be imported.
    /// @param [in] libraryName Name of the library from which the import is being attempted.
    static void TerminateProcessBecauseLibraryNotLoaded(std::wstring_view libraryName)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Library %.*s could not be loaded.\n\nXidi is terminating the application because calls to this library will otherwise cause it to crash.",
          static_cast<int>(libraryName.length()),
          libraryName.data());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }

    /// Shows an error and terminates the process in the event of failure to import a particular
    /// function from a library dependency.
    /// @param [in] libraryName Name of the library from which the import is being attempted.
    /// @param [in] functionName Name of the function whose import attempt failed.
    static void TerminateProcessBecauseImportFailed(
        std::wstring_view libraryName, std::string_view functionName)
    {
      Infra::TemporaryString functionNameWideCharacterString = functionName;
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Library %.*s is missing entry point %.*s.\n\nXidi is terminating the application because calls to this entry point will otherwise cause it to crash.",
          static_cast<int>(libraryName.length()),
          libraryName.data(),
          static_cast<int>(functionNameWideCharacterString.AsStringView().length()),
          functionNameWideCharacterString.AsStringView().data());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }

    bool TryImport(
        std::wstring_view libraryPath,
        HMODULE libraryHandle,
        const char* functionName,
        const void** procAddressDestination)
    {
      void* procAddress = GetProcAddress(libraryHandle, functionName);
      if (nullptr == procAddress)
      {
        LogImportFailed(libraryPath, functionName);
        return false;
      }

      *procAddressDestination = procAddress;
      return true;
    }
  } // namespace DllFunctions
} // namespace Xidi

/// Initializes the destination addresses of all forwarded functions. Invoked automatically from the
/// assembly implementation of the individual exported functions.
extern "C" void DllForwardedFunctionsInitialize(void)
{
  using namespace Xidi::DllFunctions;

  static std::once_flag initializationFlag;
  std::call_once(
      initializationFlag,
      []() -> void
      {
        while (false == allForwardedFunctions.empty())
        {
          auto dllForwardedFunction = allForwardedFunctions.extract(allForwardedFunctions.cbegin());
          std::wstring_view libraryPath = dllForwardedFunction.mapped()->GetLibraryPath();

          auto libraryHandleIter = libraryHandles.find(libraryPath);
          if (libraryHandles.end() == libraryHandleIter)
          {
            HMODULE loadedLibraryHandle =
                LoadLibrary(dllForwardedFunction.mapped()->GetLibraryPath().data());
            if (nullptr == loadedLibraryHandle)
            {
              TerminateProcessBecauseLibraryNotLoaded(
                  dllForwardedFunction.mapped()->GetLibraryPath());
              return;
            }
            libraryHandleIter = libraryHandles.emplace(libraryPath, loadedLibraryHandle).first;
          }

          HMODULE libraryHandle = libraryHandleIter->second;
          void* procAddress = GetProcAddress(
              libraryHandle, dllForwardedFunction.mapped()->GetFunctionName().data());
          if (nullptr == procAddress)
          {
            TerminateProcessBecauseImportFailed(
                dllForwardedFunction.mapped()->GetLibraryPath(),
                dllForwardedFunction.mapped()->GetFunctionName());
            return;
          }

          dllForwardedFunction.mapped()->SetProcAddress(procAddress);
        }
      });
}
