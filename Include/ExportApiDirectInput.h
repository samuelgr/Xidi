/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ExportApiDirectInput.h
 *      Declaration of primary exported functions for the DirectInput
 *      library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


 // -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
 // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
HRESULT STDMETHODCALLTYPE ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
HRESULT WINAPI ExportApiDirectInputDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter);
HRESULT WINAPI ExportApiDirectInputDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW *ppDI, LPUNKNOWN punkOuter);
HRESULT WINAPI ExportApiDirectInputDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
#endif

HRESULT STDMETHODCALLTYPE ExportApiDirectInputDllRegisterServer(void);
HRESULT STDMETHODCALLTYPE ExportApiDirectInputDllUnregisterServer(void);
HRESULT STDMETHODCALLTYPE ExportApiDirectInputDllCanUnloadNow(void);
HRESULT STDMETHODCALLTYPE ExportApiDirectInputDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv);
