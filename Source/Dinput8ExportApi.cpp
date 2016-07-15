/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Dinput8ExportApi.cpp
 *      Implementation of primary exported functions for "dinput8.dll".
 *****************************************************************************/

#include "Dinput8ImportApi.h"
#include "WrapperIDirectInput8.h"

using namespace XinputControllerDirectInput;


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See DirectInput and COM documentation for more information.

HRESULT STDMETHODCALLTYPE Dinput8ExportDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    IDirectInput8* diObject = NULL;
    
    HRESULT result = Dinput8ImportApi::ImportedDirectInput8Create(hinst, dwVersion, riidltf, (LPVOID*)&diObject, punkOuter);
    if (DI_OK != result) return result;
    
    diObject = new XinputControllerDirectInput::WrapperIDirectInput8(diObject);
    *ppvOut = (LPVOID)diObject;
    
    return result;
}

// ---------

HRESULT STDMETHODCALLTYPE Dinput8ExportDllRegisterServer(void)
{
    return Dinput8ImportApi::ImportedDllRegisterServer();
}

// ---------

HRESULT STDMETHODCALLTYPE Dinput8ExportDllUnregisterServer(void)
{
    return Dinput8ImportApi::ImportedDllUnregisterServer();
}

// ---------

HRESULT STDMETHODCALLTYPE Dinput8ExportDllCanUnloadNow(void)
{
    return Dinput8ImportApi::ImportedDllCanUnloadNow();
}

// ---------

HRESULT STDMETHODCALLTYPE Dinput8ExportDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv)
{
    return Dinput8ImportApi::ImportedDllGetClassObject(rclsid, riid, ppv);
}
