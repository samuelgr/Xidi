/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Dinput8ImportApi.cpp
 *      Implementation of importing the API from "dinput8.dll".
 *****************************************************************************/

#include "ApiWindows.h"
#include "Dinput8ImportApi.h"

using namespace XinputControllerDirectInput;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Dinput8ImportApi.h" for documentation.

SImportTable Dinput8ImportApi::importTable = {NULL, NULL, NULL, NULL, NULL};
BOOL Dinput8ImportApi::importTableIsInitialized = FALSE;


// -------- CLASS METHODS -------------------------------------------------- //
// See "Dinput8ImportApi.h" for documentation.

HRESULT Dinput8ImportApi::Initialize(void)
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

HRESULT Dinput8ImportApi::ImportedDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    if (FALSE == importTableIsInitialized) return E_NOT_VALID_STATE;
    return importTable.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

// ---------

HRESULT Dinput8ImportApi::ImportedDllRegisterServer(void)
{
    if (FALSE == importTableIsInitialized) return E_NOT_VALID_STATE;
    return importTable.DllRegisterServer();
}

// ---------

HRESULT Dinput8ImportApi::ImportedDllUnregisterServer(void)
{
    if (FALSE == importTableIsInitialized) return E_NOT_VALID_STATE;
    return importTable.DllUnregisterServer();
}

// ---------

HRESULT Dinput8ImportApi::ImportedDllCanUnloadNow(void)
{
    if (FALSE == importTableIsInitialized) return E_NOT_VALID_STATE;
    return importTable.DllCanUnloadNow();
}

// ---------

HRESULT Dinput8ImportApi::ImportedDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (FALSE == importTableIsInitialized) return E_NOT_VALID_STATE;
    return importTable.DllGetClassObject(rclsid, riid, ppv);
}
