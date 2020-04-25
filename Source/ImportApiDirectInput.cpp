/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ImportApiDirectInput.cpp
 *   Implementation of importing the API from the DirectInput library.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "Log.h"

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "ImportApiDirectInput.h" for documentation.

ImportApiDirectInput::SImportTable ImportApiDirectInput::importTable;
BOOL ImportApiDirectInput::importTableIsInitialized = FALSE;


// -------- CLASS METHODS -------------------------------------------------- //
// See "ImportApiDirectInput.h" for documentation.

void ImportApiDirectInput::Initialize(void)
{
    if (FALSE == importTableIsInitialized)
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
        HMODULE loadedLibrary = LoadLibraryEx(libraryPath.c_str(), NULL, 0);
        if (NULL == loadedLibrary)
        {
            LogInitializeFailed(libraryPath.c_str());
            return;
        }

        // Attempt to obtain the addresses of all imported API functions.
        FARPROC procAddress = NULL;

#if DIRECTINPUT_VERSION >= 0x0800
        procAddress = GetProcAddress(loadedLibrary, "DirectInput8Create");
        if (NULL == procAddress) LogImportFailed(L"DirectInput8Create");
        importTable.DirectInput8Create = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))procAddress;
#else
        procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateA");
        if (NULL == procAddress) LogImportFailed(L"DirectInputCreateA");
        importTable.DirectInputCreateA = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateW");
        if (NULL == procAddress) LogImportFailed(L"DirectInputCreateW");
        importTable.DirectInputCreateW = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, LPDIRECTINPUTW*, LPUNKNOWN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DirectInputCreateEx");
        if (NULL == procAddress) LogImportFailed(L"DirectInputCreateEx");
        importTable.DirectInputCreateEx = (HRESULT(STDMETHODCALLTYPE*)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN))procAddress;
#endif

        procAddress = GetProcAddress(loadedLibrary, "DllRegisterServer");
        if (NULL == procAddress) LogImportFailed(L"DllRegisterServer");
        importTable.DllRegisterServer = (HRESULT(STDMETHODCALLTYPE*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllUnregisterServer");
        if (NULL == procAddress) LogImportFailed(L"DllUnregisterServer");
        importTable.DllUnregisterServer = (HRESULT(STDMETHODCALLTYPE*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllCanUnloadNow");
        if (NULL == procAddress) LogImportFailed(L"DllCanUnloadNow");
        importTable.DllCanUnloadNow = (HRESULT(STDMETHODCALLTYPE*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DllGetClassObject");
        if (NULL == procAddress) LogImportFailed(L"DllGetClassObject");
        importTable.DllGetClassObject = (HRESULT(STDMETHODCALLTYPE*)(REFCLSID, REFIID, LPVOID*))procAddress;

        // Initialization complete.
        importTableIsInitialized = TRUE;
        LogInitializeSucceeded();
    }
}

// ---------

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT ImportApiDirectInput::DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    Initialize();

    if (NULL == importTable.DirectInput8Create)
        LogMissingFunctionCalled(L"DirectInput8Create");

    return importTable.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
#else
HRESULT ImportApiDirectInput::DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA*ppDI, LPUNKNOWN punkOuter)
{
    Initialize();

    if (NULL == importTable.DirectInputCreateA)
        LogMissingFunctionCalled(L"DirectInputCreateA");

    return importTable.DirectInputCreateA(hinst, dwVersion, ppDI, punkOuter);
}

// ---------

HRESULT ImportApiDirectInput::DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW*ppDI, LPUNKNOWN punkOuter)
{
    Initialize();

    if (NULL == importTable.DirectInputCreateW)
        LogMissingFunctionCalled(L"DirectInputCreateW");

    return importTable.DirectInputCreateW(hinst, dwVersion, ppDI, punkOuter);
}

// ---------

HRESULT ImportApiDirectInput::DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    Initialize();

    if (NULL == importTable.DirectInputCreateEx)
        LogMissingFunctionCalled(L"DirectInputCreateEx");

    return importTable.DirectInputCreateEx(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
#endif

// ---------

HRESULT ImportApiDirectInput::DllRegisterServer(void)
{
    Initialize();

    if (NULL == importTable.DllRegisterServer)
        LogMissingFunctionCalled(L"DllRegisterServer");

    return importTable.DllRegisterServer();
}

// ---------

HRESULT ImportApiDirectInput::DllUnregisterServer(void)
{
    Initialize();

    if (NULL == importTable.DllUnregisterServer)
        LogMissingFunctionCalled(L"DllUnregisterServer");

    return importTable.DllUnregisterServer();
}

// ---------

HRESULT ImportApiDirectInput::DllCanUnloadNow(void)
{
    Initialize();

    if (NULL == importTable.DllCanUnloadNow)
        LogMissingFunctionCalled(L"DllCanUnloadNow");

    return importTable.DllCanUnloadNow();
}

// ---------

HRESULT ImportApiDirectInput::DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    Initialize();

    if (NULL == importTable.DllGetClassObject)
        LogMissingFunctionCalled(L"DllGetClassObject");

    return importTable.DllGetClassObject(rclsid, riid, ppv);
}


// -------- HELPERS -------------------------------------------------------- //
// See "ImportApiDirectInput.h" for documentation.

void ImportApiDirectInput::LogImportFailed(LPCWSTR functionName)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Import library is missing DirectInput function \"%s\". Attempts to call it will fail.", functionName);
}

// ---------

void ImportApiDirectInput::LogInitializeLibraryPath(LPCWSTR libraryPath)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, L"Attempting to import DirectInput functions from \"%s\".", libraryPath);
}

// ---------

void ImportApiDirectInput::LogInitializeFailed(LPCWSTR libraryPath)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Failed to load DirectInput import library \"%s\".", libraryPath);
}

// ---------

void ImportApiDirectInput::LogInitializeSucceeded(void)
{
    Log::WriteLogMessage(ELogLevel::LogLevelInfo, L"Successfully initialized imported DirectInput functions.");
}

// ---------

void ImportApiDirectInput::LogMissingFunctionCalled(LPCWSTR functionName)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Application has attempted to call missing DirectInput import function \"%s\".", functionName);
}
