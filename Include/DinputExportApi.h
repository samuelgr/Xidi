/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DinputExportApi.h
 *      Declaration of primary exported functions for the DirectInput
 *      library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


 // -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
 // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT STDMETHODCALLTYPE DinputExportDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
HRESULT WINAPI DinputExportDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter);
HRESULT WINAPI DinputExportDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW *ppDI, LPUNKNOWN punkOuter);
HRESULT WINAPI DinputExportDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
#endif

HRESULT STDMETHODCALLTYPE DinputExportDllRegisterServer(void);

HRESULT STDMETHODCALLTYPE DinputExportDllUnregisterServer(void);

HRESULT STDMETHODCALLTYPE DinputExportDllCanUnloadNow(void);

HRESULT STDMETHODCALLTYPE DinputExportDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv);
