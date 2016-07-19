/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DinputImportApi.h
 *      Declarations related to importing the API from the DirectInput
 *      library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


namespace Xidi
{
    // Fields specify the addresses of the imported DirectInput API functions.
    struct SImportTable
    {
#if DIRECTINPUT_VERSION >= 0x0800
        HRESULT(__stdcall *DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#else
        HRESULT(__stdcall *DirectInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA, LPUNKNOWN);
        HRESULT(__stdcall *DirectInputCreateW)(HINSTANCE, DWORD, LPDIRECTINPUTW, LPUNKNOWN);
        HRESULT(__stdcall *DirectInputCreateEx)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
#endif
        HRESULT(__stdcall *DllRegisterServer)(void);
        HRESULT(__stdcall *DllUnregisterServer)(void);
        HRESULT(__stdcall *DllCanUnloadNow)(void);
        HRESULT(__stdcall *DllGetClassObject)(_In_ REFCLSID, _In_ REFIID, _Out_ LPVOID*);
    };
    
    
    // Enables access to the underlying system's DirectInput API.
    // Dynamically loads the library and holds pointers to all of its methods.
    // Methods are intended to be called directly rather than through an instance.
    class DinputImportApi
    {
    public:
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
        DinputImportApi();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Dynamically loads the DirectInput library and sets up all imported function calls.
        // Returns S_OK on success and E_FAIL on failure.
        static HRESULT Initialize(void);

        // Calls the imported function DirectInput8Create.
        static HRESULT ImportedDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);

        // Calls the imported function DllRegisterServer.
        static HRESULT ImportedDllRegisterServer(void);

        // Calls the imported function DllUnregisterServer.
        static HRESULT ImportedDllUnregisterServer(void);

        // Calls the imported function DllCanUnloadNow.
        static HRESULT ImportedDllCanUnloadNow(void);

        // Calls the imported function DllGetClassObject.
        static HRESULT ImportedDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    };
}
