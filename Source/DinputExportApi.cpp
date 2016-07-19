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

    HRESULT result = DinputImportApi::ImportedDirectInput8Create(hinst, dwVersion, riidltf, (LPVOID*)&diObject, punkOuter);
    if (DI_OK != result) return result;

    diObject = new Xidi::WrapperIDirectInput(diObject, (IID_IDirectInput8W == riidltf));
    *ppvOut = (LPVOID)diObject;

    return result;
}
#else
HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter)
{
    return E_FAIL;
}

// ---------

HRESULT WINAPI DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW *ppDI, LPUNKNOWN punkOuter)
{
    return E_FAIL;
}

// ---------

HRESULT WINAPI DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    return E_FAIL;
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
