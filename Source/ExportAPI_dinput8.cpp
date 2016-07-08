/*****************************************************************************
 * XboxOneDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ExportAPI_dinput8.cpp
 *      Implementation of primary exported functions for "dinput8.dll".
 *****************************************************************************/

#include "API_DirectInput8.h"


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

HRESULT ExportAPI_dinput8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
    return S_FALSE;
}

// ---------

HRESULT __stdcall ExportAPI_dinput8_DllRegisterServer(void)
{
    return S_FALSE;
}

// ---------

HRESULT __stdcall ExportAPI_dinput8_DllUnregisterServer(void)
{
    return S_FALSE;
}

// ---------

HRESULT __stdcall ExportAPI_dinput8_DllCanUnloadNow(void)
{
    return S_FALSE;
}

// ---------

HRESULT __stdcall ExportAPI_dinput8_DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID *ppv)
{
    return S_FALSE;
}
