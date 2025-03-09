/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ImportApiDirectInput.h
 *   Declarations of functions for accessing the DirectInput API imported from
 *   the native DirectInput library.
 **************************************************************************************************/

#pragma once

#include "ApiDirectInput.h"

namespace Xidi
{
  namespace ImportApiDirectInput
  {

    namespace Version8
    {
      HRESULT DirectInput8Create(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
      HRESULT DllRegisterServer(void);
      HRESULT DllUnregisterServer(void);
      HRESULT DllCanUnloadNow(void);
      HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    } // namespace Version8

    namespace VersionLegacy
    {
      HRESULT DirectInputCreateA(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
      HRESULT DirectInputCreateW(
          HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
      HRESULT DirectInputCreateEx(
          HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
      HRESULT DllRegisterServer(void);
      HRESULT DllUnregisterServer(void);
      HRESULT DllCanUnloadNow(void);
      HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    } // namespace VersionLegacy

    // clang-format on
  } // namespace ImportApiDirectInput
} // namespace Xidi
