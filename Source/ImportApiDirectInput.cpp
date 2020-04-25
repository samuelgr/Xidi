/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ImportApiDirectInput.cpp
 *   Implementations of functions for accessing the DirectInput API imported
 *   from the native DirectInput library.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "Log.h"

#include <mutex>
#include <string>


namespace Xidi
{
    namespace ImportApiDirectInput
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Holds pointers to all the functions imported from the native DirectInput library.
        struct SImportTable
        {
#if DIRECTINPUT_VERSION >= 0x0800
            HRESULT(STDMETHODCALLTYPE* DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#else
            HRESULT(STDMETHODCALLTYPE* DirectInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);
            HRESULT(STDMETHODCALLTYPE* DirectInputCreateW)(HINSTANCE, DWORD, LPDIRECTINPUTW*, LPUNKNOWN);
            HRESULT(STDMETHODCALLTYPE* DirectInputCreateEx)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#endif
            HRESULT(STDMETHODCALLTYPE* DllRegisterServer)(void);
            HRESULT(STDMETHODCALLTYPE* DllUnregisterServer)(void);
            HRESULT(STDMETHODCALLTYPE* DllCanUnloadNow)(void);
            HRESULT(STDMETHODCALLTYPE* DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //
        
        /// Holds the imported DirectInput API function addresses.
        static SImportTable importTable;


        // -------- INTERNAL FUNCTIONS --------------------------------------------- //

        /// Logs a warning event related to failure to import a particular function from the import library.
        /// @param [in] functionName Name of the function whose import attempt failed.
        static void LogImportFailed(LPCWSTR functionName)
        {
            Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Import library is missing DirectInput function \"%s\". Attempts to call it will fail.", functionName);
        }

        /// Logs a debug event related to attempting to load the system-provided library for importing functions.
        /// @param [in] libraryPath Path of the library that was loaded.
        static void LogInitializeLibraryPath(LPCWSTR libraryPath)
        {
            Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, L"Attempting to import DirectInput functions from \"%s\".", libraryPath);
        }

        /// Logs an error event related to failure to initialize the import table because the import library could not be loaded.
        /// @param [in] libraryPath Path of the library that was loaded.
        static void LogInitializeFailed(LPCWSTR libraryPath)
        {
            Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Failed to load DirectInput import library \"%s\".", libraryPath);
        }

        /// Logs an informational event related to successful initialization of the import table.
        static void LogInitializeSucceeded(void)
        {
            Log::WriteLogMessage(ELogLevel::LogLevelInfo, L"Successfully initialized imported DirectInput functions.");
        }

        /// Logs an error event related to a missing import function that has been invoked.
        /// @param [in] functionName Name of the function that was invoked.
        static void LogMissingFunctionCalled(LPCWSTR functionName)
        {
            Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Application has attempted to call missing DirectInput import function \"%s\".", functionName);
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "ImportApiDirectInput.h" for documentation.

        void Initialize(void)
        {
            static volatile bool isInitialized = false;

            if (false == isInitialized)
            {
                static std::mutex initializeMutex;
                std::lock_guard<std::mutex> lock(initializeMutex);
                
                if (false == isInitialized)
                {
                    // Initialize the import table.
                    ZeroMemory(&importTable, sizeof(importTable));

                    // Obtain the full library path string.
                    std::wstring libraryPath;

#if DIRECTINPUT_VERSION >= 0x0800
                    Globals::FillDirectInput8LibraryPath(libraryPath);
#else
                    Globals::FillDirectInputLibraryPath(libraryPath);
#endif

                    // Attempt to load the library.
                    LogInitializeLibraryPath(libraryPath.c_str());
                    HMODULE loadedLibrary = LoadLibraryEx(libraryPath.c_str(), nullptr, 0);
                    if (nullptr == loadedLibrary)
                    {
                        LogInitializeFailed(libraryPath.c_str());
                        return;
                    }

                    // Attempt to obtain the addresses of all imported API functions.
                    FARPROC procAddress = nullptr;

#if DIRECTINPUT_VERSION >= 0x0800
                    procAddress = GetProcAddress(loadedLibrary, "DirectInput8Create");
                    if (nullptr == procAddress) LogImportFailed(L"DirectInput8Create");
                    importTable.DirectInput8Create = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))procAddress;
#else
                    procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateA");
                    if (nullptr == procAddress) LogImportFailed(L"DirectInputCreateA");
                    importTable.DirectInputCreateA = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateW");
                    if (nullptr == procAddress) LogImportFailed(L"DirectInputCreateW");
                    importTable.DirectInputCreateW = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, LPDIRECTINPUTW*, LPUNKNOWN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateEx");
                    if (nullptr == procAddress) LogImportFailed(L"DirectInputCreateEx");
                    importTable.DirectInputCreateEx = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))procAddress;
#endif

                    procAddress = GetProcAddress(loadedLibrary, "DllRegisterServer");
                    if (nullptr == procAddress) LogImportFailed(L"DllRegisterServer");
                    importTable.DllRegisterServer = (HRESULT(STDMETHODCALLTYPE*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DllUnregisterServer");
                    if (nullptr == procAddress) LogImportFailed(L"DllUnregisterServer");
                    importTable.DllUnregisterServer = (HRESULT(STDMETHODCALLTYPE*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DllCanUnloadNow");
                    if (nullptr == procAddress) LogImportFailed(L"DllCanUnloadNow");
                    importTable.DllCanUnloadNow = (HRESULT(STDMETHODCALLTYPE*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DllGetClassObject");
                    if (nullptr == procAddress) LogImportFailed(L"DllGetClassObject");
                    importTable.DllGetClassObject = (HRESULT(STDMETHODCALLTYPE*)(REFCLSID, REFIID, LPVOID*))procAddress;

                    // Initialization complete.
                    isInitialized = true;
                    LogInitializeSucceeded();
                }
            }
        }


        // -------- IMPORTED FUNCTIONS ------------------------------------- //
        // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
        HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
        {
            Initialize();

            if (nullptr == importTable.DirectInput8Create)
                LogMissingFunctionCalled(L"DirectInput8Create");

            return importTable.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
        }
#else
        HRESULT DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
        {
            Initialize();

            if (nullptr == importTable.DirectInputCreateA)
                LogMissingFunctionCalled(L"DirectInputCreateA");

            return importTable.DirectInputCreateA(hinst, dwVersion, ppDI, punkOuter);
        }

        // ---------

        HRESULT DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
        {
            Initialize();

            if (nullptr == importTable.DirectInputCreateW)
                LogMissingFunctionCalled(L"DirectInputCreateW");

            return importTable.DirectInputCreateW(hinst, dwVersion, ppDI, punkOuter);
        }

        // ---------

        HRESULT DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
        {
            Initialize();

            if (nullptr == importTable.DirectInputCreateEx)
                LogMissingFunctionCalled(L"DirectInputCreateEx");

            return importTable.DirectInputCreateEx(hinst, dwVersion, riidltf, ppvOut, punkOuter);
        }
#endif

        // ---------

        HRESULT DllRegisterServer(void)
        {
            Initialize();

            if (nullptr == importTable.DllRegisterServer)
                LogMissingFunctionCalled(L"DllRegisterServer");

            return importTable.DllRegisterServer();
        }

        // ---------

        HRESULT DllUnregisterServer(void)
        {
            Initialize();

            if (nullptr == importTable.DllUnregisterServer)
                LogMissingFunctionCalled(L"DllUnregisterServer");

            return importTable.DllUnregisterServer();
        }

        // ---------

        HRESULT DllCanUnloadNow(void)
        {
            Initialize();

            if (nullptr == importTable.DllCanUnloadNow)
                LogMissingFunctionCalled(L"DllCanUnloadNow");

            return importTable.DllCanUnloadNow();
        }

        // ---------

        HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
        {
            Initialize();

            if (nullptr == importTable.DllGetClassObject)
                LogMissingFunctionCalled(L"DllGetClassObject");

            return importTable.DllGetClassObject(rclsid, riid, ppv);
        }
    }
}
