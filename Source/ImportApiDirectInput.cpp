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
#define IMPORT_TABLE_INDEX_OF(importTable, name)                                                   \
  (offsetof(decltype(importTable), named.##name) / sizeof(decltype(importTable)::ptr[0]))

/// Attempts to import a single function and save it into the import table.
#define TRY_IMPORT(importTable, libraryPath, libraryHandle, functionName)                          \
  DllFunctions::TryImport(                                                                         \
      libraryPath,                                                                                 \
      loadedLibrary,                                                                               \
      #functionName,                                                                               \
      &importTable.ptr[IMPORT_TABLE_INDEX_OF(importTable, functionName)])

namespace Xidi
{
  namespace ImportApiDirectInput
  {
    /// Holds pointers to all the functions imported from the native DirectInput 8 library.
    /// Exposes them as both an array of typeless pointers and a named structure of type-specific
    /// pointers.
    union UImportTableVersion8
    {
      struct
      {
        HRESULT(__stdcall* DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
        HRESULT(__stdcall* DllRegisterServer)(void);
        HRESULT(__stdcall* DllUnregisterServer)(void);
        HRESULT(__stdcall* DllCanUnloadNow)(void);
        HRESULT(__stdcall* DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
      } named;

      const void* ptr[sizeof(named) / sizeof(void*)];
    };

    /// Holds pointers to all the functions imported from the native DirectInput legacy library.
    /// Exposes them as both an array of typeless pointers and a named structure of type-specific
    /// pointers.
    union UImportTableVersionLegacy
    {
      struct
      {
        HRESULT(__stdcall* DirectInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);
        HRESULT(__stdcall* DirectInputCreateW)(HINSTANCE, DWORD, LPDIRECTINPUTW*, LPUNKNOWN);
        HRESULT(__stdcall* DirectInputCreateEx)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
        HRESULT(__stdcall* DllRegisterServer)(void);
        HRESULT(__stdcall* DllUnregisterServer)(void);
        HRESULT(__stdcall* DllCanUnloadNow)(void);
        HRESULT(__stdcall* DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
      } named;

      const void* ptr[sizeof(named) / sizeof(void*)];
    };

    /// Holds the imported DirectInput 8 API function addresses.
    static UImportTableVersion8 importTableVersion8;

    /// Holds the imported DirectInput legacy API function addresses.
    static UImportTableVersionLegacy importTableVersionLegacy;

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

    /// Dynamically loads the DirectInput 8 and sets up all imported function calls.
    static void InitializeVersion8(void)
    {
      static std::once_flag initializeFlag;
      std::call_once(
          initializeFlag,
          []() -> void
          {
            ZeroMemory(&importTableVersion8, sizeof(importTableVersion8));

            std::wstring_view libraryPath = Strings::GetSystemLibraryFilenameDirectInput8();
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"Attempting to import DirectInput 8 functions from %s.",
                libraryPath.data());

            HMODULE loadedLibrary = LoadLibraryEx(libraryPath.data(), nullptr, 0);
            if (nullptr == loadedLibrary)
            {
              LogInitializeFailed(libraryPath.data());
              return;
            }

            TRY_IMPORT(importTableVersion8, libraryPath, loadedLibrary, DirectInput8Create);
            TRY_IMPORT(importTableVersion8, libraryPath, loadedLibrary, DllRegisterServer);
            TRY_IMPORT(importTableVersion8, libraryPath, loadedLibrary, DllUnregisterServer);
            TRY_IMPORT(importTableVersion8, libraryPath, loadedLibrary, DllCanUnloadNow);
            TRY_IMPORT(importTableVersion8, libraryPath, loadedLibrary, DllGetClassObject);

            Infra::Message::Output(
                Infra::Message::ESeverity::Info,
                L"Finished importing DirectInput 8 functions.");
          });
    }

    /// Dynamically loads the DirectInput 8 and sets up all imported function calls.
    static void InitializeVersionLegacy(void)
    {
      static std::once_flag initializeFlag;
      std::call_once(
          initializeFlag,
          []() -> void
          {
            ZeroMemory(&importTableVersionLegacy, sizeof(importTableVersionLegacy));

            std::wstring_view libraryPath = Strings::GetSystemLibraryFilenameDirectInput();
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"Attempting to import DirectInput legacy functions from %s.",
                libraryPath.data());

            HMODULE loadedLibrary = LoadLibraryEx(libraryPath.data(), nullptr, 0);
            if (nullptr == loadedLibrary)
            {
              LogInitializeFailed(libraryPath.data());
              return;
            }

            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DirectInputCreateA);
            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DirectInputCreateW);
            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DirectInputCreateEx);
            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DllRegisterServer);
            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DllUnregisterServer);
            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DllCanUnloadNow);
            TRY_IMPORT(importTableVersionLegacy, libraryPath, loadedLibrary, DllGetClassObject);

            Infra::Message::Output(
                Infra::Message::ESeverity::Info,
                L"Finished importing DirectInput legacy functions.");
          });
    }

    namespace Version8
    {
      HRESULT DirectInput8Create(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
      {
        if (nullptr == importTableVersion8.named.DirectInput8Create) InitializeVersion8();
        return importTableVersion8.named.DirectInput8Create(
            hinst, dwVersion, riidltf, ppvOut, punkOuter);
      }

      HRESULT DllRegisterServer(void)
      {
        if (nullptr == importTableVersion8.named.DllRegisterServer) InitializeVersion8();
        return importTableVersion8.named.DllRegisterServer();
      }

      HRESULT DllUnregisterServer(void)
      {
        if (nullptr == importTableVersion8.named.DllUnregisterServer) InitializeVersion8();
        return importTableVersion8.named.DllUnregisterServer();
      }

      HRESULT DllCanUnloadNow(void)
      {
        if (nullptr == importTableVersion8.named.DllCanUnloadNow) InitializeVersion8();
        return importTableVersion8.named.DllCanUnloadNow();
      }

      HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
      {
        if (nullptr == importTableVersion8.named.DllGetClassObject) InitializeVersion8();
        return importTableVersion8.named.DllGetClassObject(rclsid, riid, ppv);
      }
    } // namespace Version8

    namespace VersionLegacy
    {
      HRESULT DirectInputCreateA(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
      {
        if (nullptr == importTableVersionLegacy.named.DirectInputCreateA) InitializeVersionLegacy();
        return importTableVersionLegacy.named.DirectInputCreateA(hinst, dwVersion, ppDI, punkOuter);
      }

      HRESULT DirectInputCreateW(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
      {
        if (nullptr == importTableVersionLegacy.named.DirectInputCreateW) InitializeVersionLegacy();
        return importTableVersionLegacy.named.DirectInputCreateW(hinst, dwVersion, ppDI, punkOuter);
      }

      HRESULT DirectInputCreateEx(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
      {
        if (nullptr == importTableVersionLegacy.named.DirectInputCreateEx)
          InitializeVersionLegacy();
        return importTableVersionLegacy.named.DirectInputCreateEx(
            hinst, dwVersion, riidltf, ppvOut, punkOuter);
      }

      HRESULT DllRegisterServer(void)
      {
        if (nullptr == importTableVersionLegacy.named.DllRegisterServer) InitializeVersionLegacy();
        return importTableVersionLegacy.named.DllRegisterServer();
      }

      HRESULT DllUnregisterServer(void)
      {
        if (nullptr == importTableVersionLegacy.named.DllUnregisterServer)
          InitializeVersionLegacy();
        return importTableVersionLegacy.named.DllUnregisterServer();
      }

      HRESULT DllCanUnloadNow(void)
      {
        if (nullptr == importTableVersionLegacy.named.DllCanUnloadNow) InitializeVersionLegacy();
        return importTableVersionLegacy.named.DllCanUnloadNow();
      }

      HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
      {
        if (nullptr == importTableVersionLegacy.named.DllGetClassObject) InitializeVersionLegacy();
        return importTableVersionLegacy.named.DllGetClassObject(rclsid, riid, ppv);
      }
    } // namespace VersionLegacy
  } // namespace ImportApiDirectInput
} // namespace Xidi
