/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ImportAPI_dinput8.cpp
 *      Implementation of importing the API from "dinput8.dll".
 *****************************************************************************/

#include "API_Windows.h"
#include "ImportAPI_dinput8.h"


// -------- LOCALS --------------------------------------------------------- //

// Holds the addresses of all imported API functions.
static struct importTable
{
    HRESULT (__stdcall *DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
    HRESULT (__stdcall *DllRegisterServer)(void);
    HRESULT (__stdcall *DllUnregisterServer)(void);
    HRESULT (__stdcall *DllCanUnloadNow)(void);
    HRESULT (__stdcall *DllGetClassObject)(_In_ REFCLSID, _In_ REFIID, _Out_ LPVOID*);
} importTable;

// Specifies whether or not the import table has been initialized.
static BOOL importTableIsInitialized = FALSE;


// -------- CLASS METHODS -------------------------------------------------- //
// See "ImportAPI_dinput8.h" for documentation.

HRESULT ImportAPI_dinput8::Initialize(void)
{
    if (FALSE == importTableIsInitialized)
    {
        // Obtain the %windows%\system32\dinput8.dll string.
        // A path must be specified directly since the system has already loaded this DLL of the same name.
        TCHAR libraryName[1024];
        GetSystemDirectory(libraryName, 512);
        _tcsncat_s(libraryName, _countof(libraryName), _T("\\dinput8.dll"), 12);
        
        // Attempt to load the library.
        HMODULE loadedLibrary = LoadLibraryEx(libraryName, NULL, 0);
        if (NULL == loadedLibrary) return E_FAIL;

        // Attempt to obtain the addresses of all imported API functions.
        FARPROC procAddress = GetProcAddress(loadedLibrary, "DirectInput8Create");
        if (NULL == procAddress) return E_FAIL;
        importTable.DirectInput8Create = (HRESULT(__stdcall *)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllRegisterServer");
        if (NULL == procAddress) return E_FAIL;
        importTable.DllRegisterServer = (HRESULT(__stdcall *)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllUnregisterServer");
        if (NULL == procAddress) return E_FAIL;
        importTable.DllUnregisterServer = (HRESULT(__stdcall *)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllCanUnloadNow");
        if (NULL == procAddress) return E_FAIL;
        importTable.DllCanUnloadNow = (HRESULT(__stdcall *)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllGetClassObject");
        if (NULL == procAddress) return E_FAIL;
        importTable.DllGetClassObject = (HRESULT(__stdcall *)(_In_ REFCLSID, _In_ REFIID, _Out_ LPVOID*))procAddress;
        
        // Initialization complete.
        importTableIsInitialized = TRUE;
    }

    return S_OK;
}

// ---------

HRESULT ImportAPI_dinput8::ImportedDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    return importTable.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

// ---------

HRESULT ImportAPI_dinput8::ImportedDllRegisterServer(void)
{
    return importTable.DllRegisterServer();
}

// ---------

HRESULT ImportAPI_dinput8::ImportedDllUnregisterServer(void)
{
    return importTable.DllUnregisterServer();
}

// ---------

HRESULT ImportAPI_dinput8::ImportedDllCanUnloadNow(void)
{
    return importTable.DllCanUnloadNow();
}

// ---------

HRESULT ImportAPI_dinput8::ImportedDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv)
{
    return importTable.DllGetClassObject(rclsid, riid, ppv);
}
