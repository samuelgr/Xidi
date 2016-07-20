/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ImportApiDirectInput.h
 *      Declarations related to importing the API from the DirectInput
 *      library.
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
        // -------- TYPE DEFINITIONS ----------------------------------------------- //
        
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
            HRESULT (STDMETHODCALLTYPE* DllGetClassObject)(_In_ REFCLSID, _In_ REFIID, _Out_ LPVOID*);
        };
        

        // -------- CONSTANTS ------------------------------------------------------ //

        // Holds the name of the library to load from the system directory.
        static const TCHAR* const kDirectInputLibraryName;

        // Holds the length, in characters, of the name of the library.
        static const DWORD kDirectInputLibraryLength;


    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Holds the imported DirectInput API function addresses.
        static SImportTable importTable;

        // Specifies whether or not the import table has been initialized.
        static BOOL importTableIsInitialized;

        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        ImportApiDirectInput();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Dynamically loads the DirectInput library and sets up all imported function calls.
        // Returns S_OK on success and E_FAIL on failure.
        static HRESULT Initialize(void);

#if DIRECTINPUT_VERSION >= 0x0800
        // Calls the imported function DirectInput8Create.
        static HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
        // Calls the imported function DirectInputCreateA.
        static HRESULT DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);

        // Calls the imported function DirectInputCreateW.
        static HRESULT DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);

        // Calls the imported function DirectInputCreateEx.
        static HRESULT DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#endif

        // Calls the imported function DllRegisterServer.
        static HRESULT DllRegisterServer(void);

        // Calls the imported function DllUnregisterServer.
        static HRESULT DllUnregisterServer(void);

        // Calls the imported function DllCanUnloadNow.
        static HRESULT DllCanUnloadNow(void);

        // Calls the imported function DllGetClassObject.
        static HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    };
}
