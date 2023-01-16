/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ExportApiDirectInput.h
 *   Declaration of primary exported functions for the DirectInput
 *   library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


extern "C"
{
    // -------- DLL EXPORT FUNCTIONS --------------------------------------- //
    // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT WINAPI ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
    HRESULT WINAPI ExportApiDirectInputDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
    HRESULT WINAPI ExportApiDirectInputDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
    HRESULT WINAPI ExportApiDirectInputDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#endif

    HRESULT WINAPI ExportApiDirectInputDllRegisterServer(void);
    HRESULT WINAPI ExportApiDirectInputDllUnregisterServer(void);
    HRESULT WINAPI ExportApiDirectInputDllCanUnloadNow(void);
    HRESULT WINAPI ExportApiDirectInputDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
}
