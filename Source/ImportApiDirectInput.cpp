/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ImportApiDirectInput.cpp
 *   Implementations of functions for accessing the DirectInput API imported
 *   from the native DirectInput library.
 **************************************************************************************************/

#include "ImportApiDirectInput.h"

#include <exception>
#include <mutex>
#include <string_view>

#include <Infra/Core/Configuration.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiDirectInput.h"
#include "DllFunctions.h"
#include "Globals.h"
#include "Strings.h"

/// Computes the index of the specified named function in the pointer array of the import table.
#define IMPORT_TABLE_INDEX_OF(name)                                                                \
  (offsetof(UImportTable, named.##name) / sizeof(UImportTable::ptr[0]))

/// Attempts to import a single function and save it into the import table.
#define TRY_IMPORT(libraryPath, libraryHandle, functionName)                                       \
  DllFunctions::TryImport(                                                                         \
      libraryPath,                                                                                 \
      loadedLibrary,                                                                               \
      #functionName,                                                                               \
      &importTable.ptr[IMPORT_TABLE_INDEX_OF(functionName)])

namespace Xidi
{
  namespace ImportApiDirectInput
  {
    /// Holds pointers to all the functions imported from the native DirectInput library.
    /// Exposes them as both an array of typeless pointers and a named structure of type-specific
    /// pointers.
    union UImportTable
    {
      struct
      {
#if DIRECTINPUT_VERSION >= 0x0800
        HRESULT(__stdcall* DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#else
        HRESULT(__stdcall* DirectInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);
        HRESULT(__stdcall* DirectInputCreateW)(HINSTANCE, DWORD, LPDIRECTINPUTW*, LPUNKNOWN);
        HRESULT(__stdcall* DirectInputCreateEx)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#endif
        HRESULT(__stdcall* DllRegisterServer)(void);
        HRESULT(__stdcall* DllUnregisterServer)(void);
        HRESULT(__stdcall* DllCanUnloadNow)(void);
        HRESULT(__stdcall* DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
      } named;

      const void* ptr[sizeof(named) / sizeof(void*)];
    };

    /// Holds the imported DirectInput API function addresses.
    static UImportTable importTable;

    /// Retrieves the library path for the DirectInput library that should be used for importing
    /// functions.
    /// @return Library path.
    static std::wstring_view GetImportLibraryPathDirectInput(void)
    {
      return Globals::GetConfigurationData()[Strings::kStrConfigurationSectionImport]
                                            [Strings::kStrConfigurationSettingImportDirectInput]
                                                .ValueOr(
                                                    Strings::GetSystemLibraryFilenameDirectInput());
    }

    /// Retrieves the library path for the DirectInput8 library that should be used for importing
    /// functions.
    /// @return Library path.
    static std::wstring_view GetImportLibraryPathDirectInput8(void)
    {
      return Globals::GetConfigurationData()
          [Strings::kStrConfigurationSectionImport]
          [Strings::kStrConfigurationSettingImportDirectInput8]
              .ValueOr(Strings::GetSystemLibraryFilenameDirectInput8());
    }

    /// Logs a debug event related to attempting to load the system-provided library for importing
    /// functions.
    /// @param [in] libraryPath Path of the library that was loaded.
    static void LogInitializeLibraryPath(LPCWSTR libraryPath)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Debug,
          L"Attempting to import DirectInput functions from %s.",
          libraryPath);
    }

    /// Logs an error event related to failure to initialize the import table because the import
    /// library could not be loaded.
    /// @param [in] libraryPath Path of the library that was loaded.
    static void LogInitializeFailed(LPCWSTR libraryPath)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to load DirectInput import library %s.",
          libraryPath);
    }

    /// Logs an informational event related to successful initialization of the import table.
    static void LogInitializeSucceeded(void)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Info,
          L"Successfully initialized imported DirectInput functions.");
    }

    void Initialize(void)
    {
      static std::once_flag initializeFlag;
      std::call_once(
          initializeFlag,
          []() -> void
          {
            // Initialize the import table.
            ZeroMemory(&importTable, sizeof(importTable));

        // Obtain the full library path string.
#if DIRECTINPUT_VERSION >= 0x0800
            std::wstring_view libraryPath = GetImportLibraryPathDirectInput8();
#else
            std::wstring_view libraryPath = GetImportLibraryPathDirectInput();
#endif

            // Attempt to load the library.
            LogInitializeLibraryPath(libraryPath.data());
            HMODULE loadedLibrary = LoadLibraryEx(libraryPath.data(), nullptr, 0);
            if (nullptr == loadedLibrary)
            {
              LogInitializeFailed(libraryPath.data());
              return;
            }

            // Attempt to obtain the addresses of all imported API functions.
            FARPROC procAddress = nullptr;

#if DIRECTINPUT_VERSION >= 0x0800
            TRY_IMPORT(libraryPath, loadedLibrary, DirectInput8Create);
#else
            TRY_IMPORT(libraryPath, loadedLibrary, DirectInputCreateA);
            TRY_IMPORT(libraryPath, loadedLibrary, DirectInputCreateW);
            TRY_IMPORT(libraryPath, loadedLibrary, DirectInputCreateEx);
#endif

            TRY_IMPORT(libraryPath, loadedLibrary, DllRegisterServer);
            TRY_IMPORT(libraryPath, loadedLibrary, DllUnregisterServer);
            TRY_IMPORT(libraryPath, loadedLibrary, DllCanUnloadNow);
            TRY_IMPORT(libraryPath, loadedLibrary, DllGetClassObject);

            // Initialization complete.
            LogInitializeSucceeded();
          });
    }

#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT DirectInput8Create(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      if (nullptr == importTable.named.DirectInput8Create) Initialize();
      return importTable.named.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
#else
    HRESULT DirectInputCreateA(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
    {
      if (nullptr == importTable.named.DirectInputCreateA) Initialize();
      return importTable.named.DirectInputCreateA(hinst, dwVersion, ppDI, punkOuter);
    }

    HRESULT DirectInputCreateW(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
    {
      if (nullptr == importTable.named.DirectInputCreateW) Initialize();
      return importTable.named.DirectInputCreateW(hinst, dwVersion, ppDI, punkOuter);
    }

    HRESULT DirectInputCreateEx(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      if (nullptr == importTable.named.DirectInputCreateEx) Initialize();
      return importTable.named.DirectInputCreateEx(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
#endif

    HRESULT DllRegisterServer(void)
    {
      if (nullptr == importTable.named.DllRegisterServer) Initialize();
      return importTable.named.DllRegisterServer();
    }

    HRESULT DllUnregisterServer(void)
    {
      if (nullptr == importTable.named.DllUnregisterServer) Initialize();
      return importTable.named.DllUnregisterServer();
    }

    HRESULT DllCanUnloadNow(void)
    {
      if (nullptr == importTable.named.DllCanUnloadNow) Initialize();
      return importTable.named.DllCanUnloadNow();
    }

    HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
      if (nullptr == importTable.named.DllGetClassObject) Initialize();
      return importTable.named.DllGetClassObject(rclsid, riid, ppv);
    }
  } // namespace ImportApiDirectInput
} // namespace Xidi
