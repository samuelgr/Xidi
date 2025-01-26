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

namespace Xidi
{
  namespace ExportApiDirectInput
  {
    extern "C"
    {
#if DIRECTINPUT_VERSION >= 0x0800
      HRESULT __stdcall DirectInput8Create(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
      HRESULT __stdcall DirectInputCreateA(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
      HRESULT __stdcall DirectInputCreateW(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
      HRESULT __stdcall DirectInputCreateEx(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#endif

      HRESULT __stdcall DllRegisterServer(void);
      HRESULT __stdcall DllUnregisterServer(void);
      HRESULT __stdcall DllCanUnloadNow(void);
      HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    }
  } // namespace ExportApiDirectInput
} // namespace Xidi