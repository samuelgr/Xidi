/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ExportAPI_dinput8.cpp
 *      Implementation of primary exported functions for "dinput8.dll".
 *****************************************************************************/

#include "ImportAPI_dinput8.h"
#include "XboxDirectInput8.h"


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

HRESULT STDMETHODCALLTYPE ExportAPI_dinput8_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput8* diObject = NULL;
    HRESULT result = ImportAPI_dinput8::ImportedDirectInput8Create(hinst, dwVersion, riidltf, (LPVOID*)&diObject, punkOuter);

    diObject = new XboxControllerDirectInput::XboxDirectInput8(diObject);
    *ppvOut = (LPVOID)diObject;

    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE ExportAPI_dinput8_DllRegisterServer(void)
{
    return ImportAPI_dinput8::ImportedDllRegisterServer();
}

// ---------

HRESULT STDMETHODCALLTYPE ExportAPI_dinput8_DllUnregisterServer(void)
{
    return ImportAPI_dinput8::ImportedDllUnregisterServer();
}

// ---------

HRESULT STDMETHODCALLTYPE ExportAPI_dinput8_DllCanUnloadNow(void)
{
    return ImportAPI_dinput8::ImportedDllCanUnloadNow();
}

// ---------

HRESULT STDMETHODCALLTYPE ExportAPI_dinput8_DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv)
{
    return ImportAPI_dinput8::ImportedDllGetClassObject(rclsid, riid, ppv);
}
