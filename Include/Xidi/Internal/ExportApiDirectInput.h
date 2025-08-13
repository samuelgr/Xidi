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
      HRESULT __stdcall Version8DirectInput8Create(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
      HRESULT __stdcall VersionLegacyDirectInputCreateA(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
      HRESULT __stdcall VersionLegacyDirectInputCreateW(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
      HRESULT __stdcall VersionLegacyDirectInputCreateEx(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
      HRESULT __stdcall Version8DllRegisterServer(void);
      HRESULT __stdcall VersionLegacyDllRegisterServer(void);
      HRESULT __stdcall Version8DllUnregisterServer(void);
      HRESULT __stdcall VersionLegacyDllUnregisterServer(void);
      HRESULT __stdcall Version8DllCanUnloadNow(void);
      HRESULT __stdcall VersionLegacyDllCanUnloadNow(void);
      HRESULT __stdcall Version8DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
      
      HRESULT __stdcall VersionLegacyDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    }
  } // namespace ExportApiDirectInput
} // namespace Xidi
