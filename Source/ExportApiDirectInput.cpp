/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ExportApiDirectInput.cpp
 *   Implementation of primary exported functions for the DirectInput
 *   library.
 *****************************************************************************/

#include "ImportApiDirectInput.h"
#include "Log.h"
#include "WrapperIDirectInput.h"

using namespace Xidi;


// -------- HELPERS -------------------------------------------------------- //

/// Logs an error event indicating that an instance of IDirectInput(8) could not be created due to a version out-of-range error.
/// @param [in] minVersion Minimum allowed version.
/// @param [in] maxVersion Maximum allowed version.
/// @param [in] receivedVersion Actual version received.
inline static void LogVersionOutOfRange(DWORD minVersion, DWORD maxVersion, DWORD receivedVersion)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, _T("Failed to create a DirectInput interface object because the version is out of range (expected 0x%04x to 0x%04x, got 0x%04x)."), minVersion, maxVersion, receivedVersion);
}

/// Logs an error event indicating that an instance of IDirectInput(8) could not be created due to an error having been returned by the system.
/// @param [in] errorCode Error code returned by the system.
inline static void LogSystemCreateError(HRESULT errorCode)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, _T("Failed to create a DirectInput interface object because the imported function returned code 0x%08x."), errorCode);
}

/// Logs an informational event indicating that an instance of IDirectInput(8) was created successfully.
inline static void LogSystemCreateSuccess(void)
{
    Log::WriteLogMessage(ELogLevel::LogLevelInfo, _T("Successfully created a DirectInput interface object."));
}


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT WINAPI ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput8* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
    {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return E_FAIL;
    }

    HRESULT result = ImportApiDirectInput::DirectInput8Create(hinst, dwVersion, riidltf, (LPVOID*)&diObject, punkOuter);
    if (DI_OK != result)
    {
        LogSystemCreateError(result);
        return result;
    }

    diObject = new WrapperIDirectInput(diObject, (IID_IDirectInput8W == riidltf));
    *ppvOut = (LPVOID)diObject;

    LogSystemCreateSuccess();
    return result;
}
#else
HRESULT WINAPI ExportApiDirectInputDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
    {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return E_FAIL;
    }

    HRESULT result = ImportApiDirectInput::DirectInputCreateA(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTA*)&diObject, punkOuter);
    if (DI_OK != result)
    {
        LogSystemCreateError(result);
        return result;
    }

    diObject = new WrapperIDirectInput((LatestIDirectInput*)diObject, FALSE);
    *ppDI = (LPDIRECTINPUTA)diObject;

    LogSystemCreateSuccess();
    return result;
}

// ---------

HRESULT WINAPI ExportApiDirectInputDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
    {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return E_FAIL;
    }

    HRESULT result = ImportApiDirectInput::DirectInputCreateW(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTW*)&diObject, punkOuter);
    if (DI_OK != result)
    {
        LogSystemCreateError(result);
        return result;
    }

    diObject = new WrapperIDirectInput((LatestIDirectInput*)diObject, TRUE);
    *ppDI = (LPDIRECTINPUTW)diObject;

    LogSystemCreateSuccess();
    return result;
}

// ---------

HRESULT WINAPI ExportApiDirectInputDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
    {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return E_FAIL;
    }

    HRESULT result;
    BOOL useUnicode = FALSE;
    
    if (IID_IDirectInput2W == riidltf || IID_IDirectInput7W == riidltf)
    {
        useUnicode = TRUE;
        result = ImportApiDirectInput::DirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7W, (LPVOID*)&diObject, punkOuter);
    }
    else
    {
        result = ImportApiDirectInput::DirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7A, (LPVOID*)&diObject, punkOuter);
    }
    
    if (DI_OK != result)
    {
        LogSystemCreateError(result);
        return result;
    }

    diObject = new WrapperIDirectInput((LatestIDirectInput*)diObject, useUnicode);
    *ppvOut = (LPVOID)diObject;

    LogSystemCreateSuccess();
    return result;
}
#endif

// ---------

HRESULT WINAPI ExportApiDirectInputDllRegisterServer(void)
{
    return ImportApiDirectInput::DllRegisterServer();
}

// ---------

HRESULT WINAPI ExportApiDirectInputDllUnregisterServer(void)
{
    return ImportApiDirectInput::DllUnregisterServer();
}

// ---------

HRESULT WINAPI ExportApiDirectInputDllCanUnloadNow(void)
{
    return ImportApiDirectInput::DllCanUnloadNow();
}

// ---------

HRESULT WINAPI ExportApiDirectInputDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return ImportApiDirectInput::DllGetClassObject(rclsid, riid, ppv);
}
