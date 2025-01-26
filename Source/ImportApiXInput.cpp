/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ImportApiXInput.cpp
 *   Implementations of functions for accessing the XInput API imported from the native XInput
 *   library.
 **************************************************************************************************/

#include "ImportApiXInput.h"

#include <array>
#include <mutex>

#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiWindows.h"
#include "DllFunctions.h"

/// Computes the index of the specified named function in the pointer array of the import table.
#define IMPORT_TABLE_INDEX_OF(name)                                                                \
  (offsetof(UImportTable, named.##name) / sizeof(UImportTable::ptr[0]))

/// Attempts to import a single function and save it into the import table. Terminates the process
/// on failure.
#define IMPORT_OR_TERMINATE(libraryPath, libraryHandle, functionName)                              \
  if (false ==                                                                                     \
      DllFunctions::TryImport(                                                                     \
          libraryPath,                                                                             \
          loadedLibrary,                                                                           \
          #functionName,                                                                           \
          &importTable.ptr[IMPORT_TABLE_INDEX_OF(functionName)]))                                  \
    TerminateProcessBecauseImportFailed(libraryPath, _CRT_WIDE(#functionName));

namespace Xidi
{
  namespace ImportApiXInput
  {
    /// Holds pointers to all the functions imported from the native XInput library.
    /// Exposes them as both an array of typeless pointers and a named structure of type-specific
    /// pointers.

    union UImportTable
    {
      struct
      {
        DWORD(__stdcall* XInputGetState)(DWORD, XINPUT_STATE*);
        DWORD(__stdcall* XInputSetState)(DWORD, XINPUT_VIBRATION*);
      } named;

      const void* ptr[sizeof(named) / sizeof(const void*)];
    };

    static_assert(
        sizeof(UImportTable::named) == sizeof(UImportTable::ptr), "Element size mismatch.");

    /// Holds the imported WinMM API function addresses.
    static UImportTable importTable;

    /// Shows an error and terminates the process in the event of failure to import a particular
    /// function from the import library.
    /// @param [in] libraryName Name of the library from which the import is being attempted.
    /// @param [in] functionName Name of the function whose import attempt failed.
    static void TerminateProcessBecauseImportFailed(LPCWSTR libraryName, LPCWSTR functionName)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Import library %s is missing XInput function %s.\n\nXidi cannot function without it.",
          libraryName,
          functionName);
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }

    /// Shows an error and terminates the process in the event of failure to load any XInput
    /// library.
    static void TerminateProcessBecauseNoXInputLibraryLoaded(void)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Failed to load an XInput library.\n\nXidi cannot function without it.");
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }

    void Initialize(void)
    {
      static std::once_flag initializeFlag;
      std::call_once(
          initializeFlag,
          []() -> void
          {
            // Try loading each possible DLL, in order from most preferred to least preferred.
            constexpr std::array kXInputLibraryNamesOrdered = {
                L"xinput1_4.dll",
                L"xinput1_3.dll",
                L"xinput1_2.dll",
                L"xinput1_1.dll",
                L"xinput9_1_0.dll"};

            for (const auto& xinputLibraryName : kXInputLibraryNamesOrdered)
            {
              // Initialize the import table.
              ZeroMemory(&importTable, sizeof(importTable));

              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Attempting to import XInput functions from %s.",
                  xinputLibraryName);
              HMODULE loadedLibrary = LoadLibraryEx(xinputLibraryName, nullptr, 0);
              if (nullptr == loadedLibrary)
              {
                Infra::Message::OutputFormatted(
                    Infra::Message::ESeverity::Warning,
                    L"Failed to import XInput functions from %s.",
                    xinputLibraryName);
                continue;
              }

              // Attempt to obtain the addresses of all imported API functions.
              FARPROC procAddress = nullptr;

              IMPORT_OR_TERMINATE(xinputLibraryName, loadedLibrary, XInputGetState);
              IMPORT_OR_TERMINATE(xinputLibraryName, loadedLibrary, XInputSetState);

              // Initialization complete.
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Successfully initialized imported XInput functions.");
              return;
            }

            TerminateProcessBecauseNoXInputLibraryLoaded();
          });
    }

    DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
    {
      if (nullptr == importTable.named.XInputGetState) Initialize();
      return importTable.named.XInputGetState(dwUserIndex, pState);
    }

    DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
    {
      if (nullptr == importTable.named.XInputSetState) Initialize();
      return importTable.named.XInputSetState(dwUserIndex, pVibration);
    }
  } // namespace ImportApiXInput
} // namespace Xidi
