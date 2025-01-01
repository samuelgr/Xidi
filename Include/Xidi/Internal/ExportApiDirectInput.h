/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ExportApiDirectInput.h
 *   Declaration of primary exported functions for the DirectInput library.
 **************************************************************************************************/

#pragma once

#include "ApiDirectInput.h"

extern "C"
{
  // clang-format off

#if DIRECTINPUT_VERSION >= 0x0800
  HRESULT __stdcall ExportApiDirectInputDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
  HRESULT __stdcall ExportApiDirectInputDirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
  HRESULT __stdcall ExportApiDirectInputDirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
  HRESULT __stdcall ExportApiDirectInputDirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#endif

  HRESULT __stdcall ExportApiDirectInputDllRegisterServer(void);
  HRESULT __stdcall ExportApiDirectInputDllUnregisterServer(void);
  HRESULT __stdcall ExportApiDirectInputDllCanUnloadNow(void);
  HRESULT __stdcall ExportApiDirectInputDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

  // clang-format on
}
