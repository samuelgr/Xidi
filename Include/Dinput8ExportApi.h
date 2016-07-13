/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Dinput8ExportApi.h
 *      Declaration of primary exported functions for "dinput8.dll".
 *****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"


 // -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
 // See DirectInput and COM documentation for more information.

HRESULT STDMETHODCALLTYPE Dinput8ExportDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);

HRESULT STDMETHODCALLTYPE Dinput8ExportDllRegisterServer(void);

HRESULT STDMETHODCALLTYPE Dinput8ExportDllUnregisterServer(void);

HRESULT STDMETHODCALLTYPE Dinput8ExportDllCanUnloadNow(void);

HRESULT STDMETHODCALLTYPE Dinput8ExportDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv);
