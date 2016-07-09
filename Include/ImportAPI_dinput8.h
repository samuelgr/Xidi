/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ImportAPI_dinput8.cpp
 *      Declarations related to importing the API from "dinput8.dll".
 *****************************************************************************/

#pragma once

#include "API_DirectInput8.h"


// Enables access to the underlying system's "dinput8.dll" API.
// Dynamically loads the library and holds pointers to all of its methods.
// Methods are intended to be called directly rather than through an instance.
class ImportAPI_dinput8
{
private:
// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

    // Default constructor. Should never be invoked.
    ImportAPI_dinput8();

public:
// -------- CLASS METHODS -------------------------------------------------- //

    // Dynamically loads the "dinput8.dll" library and sets up all imported function calls.
    // Returns S_OK on success and E_FAIL on failure.
    static HRESULT Initialize(void);

    // Calls the imported function DirectInput8Create.
    static HRESULT ImportedDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
    
    // Calls the imported function DllRegisterServer.
    static HRESULT ImportedDllRegisterServer(void);

    // Calls the imported function DllUnregisterServer.
    static HRESULT ImportedDllUnregisterServer(void);
    
    // Calls the imported function DllCanUnloadNow.
    static HRESULT ImportedDllCanUnloadNow(void);

    // Calls the imported function DllGetClassObject.
    static HRESULT ImportedDllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Out_ LPVOID* ppv);
};
