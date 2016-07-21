/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ExportApiDirectInput.cpp
 *      Implementation of primary exported functions for the DirectInput
 *      library.
 *****************************************************************************/

#include "ImportApiDirectInput.h"
#include "WrapperIDirectInput.h"

using namespace Xidi;


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT WINAPI ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput8* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;
    
    HRESULT result = ImportApiDirectInput::DirectInput8Create(hinst, dwVersion, riidltf, (LPVOID*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new WrapperIDirectInput(diObject, (IID_IDirectInput8W == riidltf));
    *ppvOut = (LPVOID)diObject;

    return result;
}
#else
HRESULT WINAPI ExportApiDirectInputDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;

    HRESULT result = ImportApiDirectInput::DirectInputCreateA(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTA*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new WrapperIDirectInput((LatestIDirectInput*)diObject, FALSE);
    *ppDI = (LPDIRECTINPUTA)diObject;

    return result;
}

// ---------

HRESULT WINAPI ExportApiDirectInputDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;

    HRESULT result = ImportApiDirectInput::DirectInputCreateW(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTW*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new WrapperIDirectInput((LatestIDirectInput*)diObject, TRUE);
    *ppDI = (LPDIRECTINPUTW)diObject;

    return result;
}

// ---------

HRESULT WINAPI ExportApiDirectInputDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;

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
    
    if (DI_OK != result) return result;
    
    diObject = new WrapperIDirectInput((LatestIDirectInput*)diObject, useUnicode);
    *ppvOut = (LPVOID)diObject;

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

HRESULT WINAPI ExportApiDirectInputDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv)
{
    return ImportApiDirectInput::DllGetClassObject(rclsid, riid, ppv);
}
