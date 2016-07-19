/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DinputExportApi.cpp
 *      Implementation of primary exported functions for the DirectInput
 *      library.
 *****************************************************************************/

#include "DinputImportApi.h"
#include "WrapperIDirectInput.h"

using namespace Xidi;


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT STDMETHODCALLTYPE DinputExportDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput8* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;
    
    HRESULT result = DinputImportApi::ImportedDirectInput8Create(hinst, dwVersion, riidltf, (LPVOID*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new Xidi::WrapperIDirectInput(diObject, (IID_IDirectInput8W == riidltf));
    *ppvOut = (LPVOID)diObject;

    return result;
}
#else
HRESULT WINAPI DinputExportDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;

    HRESULT result = DinputImportApi::ImportedDirectInputCreateA(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTA*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new Xidi::WrapperIDirectInput((LatestIDirectInput*)diObject, FALSE);
    *ppDI = (LPDIRECTINPUTA)diObject;

    return result;
}

// ---------

HRESULT WINAPI DinputExportDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;

    HRESULT result = DinputImportApi::ImportedDirectInputCreateW(hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTW*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new Xidi::WrapperIDirectInput((LatestIDirectInput*)diObject, TRUE);
    *ppDI = (LPDIRECTINPUTW)diObject;

    return result;
}

// ---------

HRESULT WINAPI DinputExportDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput* diObject = NULL;

    if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
        return E_FAIL;

    HRESULT result;
    
    BOOL useUnicode = FALSE;
    if (IID_IDirectInput2W == riidltf || IID_IDirectInput7W == riidltf)
    {
        useUnicode = TRUE;
        result = DinputImportApi::ImportedDirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7W, (LPVOID*)&diObject, punkOuter);
    }
    else
    {
        result = DinputImportApi::ImportedDirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7A, (LPVOID*)&diObject, punkOuter);
    }
    
    if (DI_OK != result) return result;
    
    diObject = new Xidi::WrapperIDirectInput((LatestIDirectInput*)diObject, useUnicode);
    *ppvOut = (LPVOID)diObject;

    return result;
}
#endif

// ---------

HRESULT STDMETHODCALLTYPE DinputExportDllRegisterServer(void)
{
    return DinputImportApi::ImportedDllRegisterServer();
}

// ---------

HRESULT STDMETHODCALLTYPE DinputExportDllUnregisterServer(void)
{
    return DinputImportApi::ImportedDllUnregisterServer();
}

// ---------

HRESULT STDMETHODCALLTYPE DinputExportDllCanUnloadNow(void)
{
    return DinputImportApi::ImportedDllCanUnloadNow();
}

// ---------

HRESULT STDMETHODCALLTYPE DinputExportDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv)
{
    return DinputImportApi::ImportedDllGetClassObject(rclsid, riid, ppv);
}
