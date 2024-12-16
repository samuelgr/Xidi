/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
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
#include "Globals.h"
#include "Strings.h"

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

      void* ptr[sizeof(named) / sizeof(void*)];
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

    /// Logs a warning event related to failure to import a particular function from the import
    /// library.
    /// @param [in] functionName Name of the function whose import attempt failed.
    static void LogImportFailed(LPCWSTR functionName)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Import library is missing DirectInput function \"%s\". Attempts to call it will fail.",
          functionName);
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

    /// Logs an error event related to a missing import function that has been invoked and then
    /// terminates the application.
    /// @param [in] functionName Name of the function that was invoked.
    static void TerminateAndLogMissingFunctionCalled(LPCWSTR functionName)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Application has attempted to call missing DirectInput import function \"%s\".",
          functionName);
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
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
            procAddress = GetProcAddress(loadedLibrary, "DirectInput8Create");
            if (nullptr == procAddress) LogImportFailed(L"DirectInput8Create");
            importTable.named.DirectInput8Create =
                reinterpret_cast<decltype(importTable.named.DirectInput8Create)>(procAddress);
#else
            procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateA");
            if (nullptr == procAddress) LogImportFailed(L"DirectInputCreateA");
            importTable.named.DirectInputCreateA =
                reinterpret_cast<decltype(importTable.named.DirectInputCreateA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateW");
            if (nullptr == procAddress) LogImportFailed(L"DirectInputCreateW");
            importTable.named.DirectInputCreateW =
                reinterpret_cast<decltype(importTable.named.DirectInputCreateW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateEx");
            if (nullptr == procAddress) LogImportFailed(L"DirectInputCreateEx");
            importTable.named.DirectInputCreateEx =
                reinterpret_cast<decltype(importTable.named.DirectInputCreateEx)>(procAddress);
#endif

            procAddress = GetProcAddress(loadedLibrary, "DllRegisterServer");
            if (nullptr == procAddress) LogImportFailed(L"DllRegisterServer");
            importTable.named.DllRegisterServer =
                reinterpret_cast<decltype(importTable.named.DllRegisterServer)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DllUnregisterServer");
            if (nullptr == procAddress) LogImportFailed(L"DllUnregisterServer");
            importTable.named.DllUnregisterServer =
                reinterpret_cast<decltype(importTable.named.DllUnregisterServer)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DllCanUnloadNow");
            if (nullptr == procAddress) LogImportFailed(L"DllCanUnloadNow");
            importTable.named.DllCanUnloadNow =
                reinterpret_cast<decltype(importTable.named.DllCanUnloadNow)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DllGetClassObject");
            if (nullptr == procAddress) LogImportFailed(L"DllGetClassObject");
            importTable.named.DllGetClassObject =
                reinterpret_cast<decltype(importTable.named.DllGetClassObject)>(procAddress);

            // Initialization complete.
            LogInitializeSucceeded();
          });
    }

#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT DirectInput8Create(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      Initialize();

      if (nullptr == importTable.named.DirectInput8Create)
        TerminateAndLogMissingFunctionCalled(L"DirectInput8Create");

      return importTable.named.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
#else
    HRESULT DirectInputCreateA(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
    {
      Initialize();

      if (nullptr == importTable.named.DirectInputCreateA)
        TerminateAndLogMissingFunctionCalled(L"DirectInputCreateA");

      return importTable.named.DirectInputCreateA(hinst, dwVersion, ppDI, punkOuter);
    }

    HRESULT DirectInputCreateW(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
    {
      Initialize();

      if (nullptr == importTable.named.DirectInputCreateW)
        TerminateAndLogMissingFunctionCalled(L"DirectInputCreateW");

      return importTable.named.DirectInputCreateW(hinst, dwVersion, ppDI, punkOuter);
    }

    HRESULT DirectInputCreateEx(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      Initialize();

      if (nullptr == importTable.named.DirectInputCreateEx)
        TerminateAndLogMissingFunctionCalled(L"DirectInputCreateEx");

      return importTable.named.DirectInputCreateEx(hinst, dwVersion, riidltf, ppvOut, punkOuter);
    }
#endif

    HRESULT DllRegisterServer(void)
    {
      Initialize();

      if (nullptr == importTable.named.DllRegisterServer)
        TerminateAndLogMissingFunctionCalled(L"DllRegisterServer");

      return importTable.named.DllRegisterServer();
    }

    HRESULT DllUnregisterServer(void)
    {
      Initialize();

      if (nullptr == importTable.named.DllUnregisterServer)
        TerminateAndLogMissingFunctionCalled(L"DllUnregisterServer");

      return importTable.named.DllUnregisterServer();
    }

    HRESULT DllCanUnloadNow(void)
    {
      Initialize();

      if (nullptr == importTable.named.DllCanUnloadNow)
        TerminateAndLogMissingFunctionCalled(L"DllCanUnloadNow");

      return importTable.named.DllCanUnloadNow();
    }

    HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
      Initialize();

      if (nullptr == importTable.named.DllGetClassObject)
        TerminateAndLogMissingFunctionCalled(L"DllGetClassObject");

      return importTable.named.DllGetClassObject(rclsid, riid, ppv);
    }
  } // namespace ImportApiDirectInput
} // namespace Xidi
