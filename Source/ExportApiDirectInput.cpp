/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * ExportApiDirectInput.cpp
 *      Implementation of primary exported functions for the DirectInput
 *      library.
 *****************************************************************************/

#include "ImportApiDirectInput.h"
#include "Log.h"
#include "WrapperIDirectInput.h"

using namespace Xidi;


// -------- HELPERS -------------------------------------------------------- //

// Logs a debug event indicating that an instance of IDirectInput(8) could not be created due to a version out-of-range error.
// Requires minimum and maximum versions, as well as the actual version received.
// Returns E_FAIL unconditionally.
static HRESULT LogVersionOutOfRange(DWORD minVersion, DWORD maxVersion, DWORD receivedVersion)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_EXPORTAPIDIRECTINPUT_CREATE_FAILED_VERSION_FORMAT, minVersion, maxVersion, receivedVersion);
    return E_FAIL;
}

// Logs a debug event indicating that an instance of IDirectInput(8) could not be created due to an error having been returned by the system.
static void LogSystemCreateError(HRESULT errorCode)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_EXPORTAPIDIRECTINPUT_CREATE_FAILED_SYSTEM_FORMAT, errorCode);
}

// Logs a debug event indicating that an instance of IDirectInput(8) was created successfully.
static void LogSystemCreateSuccess(void)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_EXPORTAPIDIRECTINPUT_CREATE_SUCCEEDED);
}


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT WINAPI ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput8* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);

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
        return LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);

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
        return LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);

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
        return LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);

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
