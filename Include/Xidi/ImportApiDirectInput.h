/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ImportApiDirectInput.h
 *   Declarations related to importing the API from the DirectInput
 *   library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


namespace Xidi
{
    // Enables access to the underlying system's DirectInput API.
    // Dynamically loads the library and holds pointers to all of its methods.
    // Methods are intended to be called directly rather than through an instance.
    class ImportApiDirectInput
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        // Fields specify the addresses of the imported DirectInput API functions.
        struct SImportTable
        {
#if DIRECTINPUT_VERSION >= 0x0800
            HRESULT (STDMETHODCALLTYPE* DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#else
            HRESULT (STDMETHODCALLTYPE* DirectInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);
            HRESULT (STDMETHODCALLTYPE* DirectInputCreateW)(HINSTANCE, DWORD, LPDIRECTINPUTW*, LPUNKNOWN);
            HRESULT (STDMETHODCALLTYPE* DirectInputCreateEx)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#endif
            HRESULT (STDMETHODCALLTYPE* DllRegisterServer)(void);
            HRESULT (STDMETHODCALLTYPE* DllUnregisterServer)(void);
            HRESULT (STDMETHODCALLTYPE* DllCanUnloadNow)(void);
            HRESULT (STDMETHODCALLTYPE* DllGetClassObject)(REFCLSID, REFIID, LPVOID*);
        };


    private:
        // -------- CLASS VARIABLES ---------------------------------------- //

        // Holds the imported DirectInput API function addresses.
        static SImportTable importTable;

        // Specifies whether or not the import table has been initialized.
        static BOOL importTableIsInitialized;


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        // Default constructor. Should never be invoked.
        ImportApiDirectInput(void);


    public:
        // -------- CLASS METHODS ------------------------------------------ //

        // Dynamically loads the DirectInput library and sets up all imported function calls.
        static void Initialize(void);


        // -------- CLASS METHODS: IMPORTED FUNCTIONS ---------------------- //
        // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
        static HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
        static HRESULT DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
        static HRESULT DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
        static HRESULT DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#endif
        static HRESULT DllRegisterServer(void);
        static HRESULT DllUnregisterServer(void);
        static HRESULT DllCanUnloadNow(void);
        static HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);


    private:
        // -------- HELPERS ------------------------------------------------ //

        // Logs a warning event related to failure to import a particular function from the import library.
        static void LogImportFailed(LPCWSTR functionName);

        // Logs a debug event related to attempting to load the system-provided library for importing functions.
        static void LogInitializeLibraryPath(LPCWSTR libraryPath);

        // Logs an error event related to failure to initialize the import table because the import library could not be loaded.
        static void LogInitializeFailed(LPCWSTR libraryPath);

        // Logs an informational event related to successful initialization of the import table.
        static void LogInitializeSucceeded(void);

        // Logs an error event related to a missing import function that has been invoked.
        static void LogMissingFunctionCalled(LPCWSTR functionName);
    };
}
