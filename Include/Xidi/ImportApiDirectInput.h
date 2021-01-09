/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ImportApiDirectInput.h
 *   Declarations of functions for accessing the DirectInput API imported from
 *   the native DirectInput library.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"


namespace Xidi
{
    namespace ImportApiDirectInput
    {
        // -------- FUNCTIONS ---------------------------------------------- //

        /// Dynamically loads the DirectInput library and sets up all imported function calls.
        void Initialize(void);


        // -------- IMPORTED FUNCTIONS ------------------------------------- //
        // See DirectInput and COM documentation for more information.

#if DIRECTINPUT_VERSION >= 0x0800
        HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#else
        HRESULT DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter);
        HRESULT DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter);
        HRESULT DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
#endif
        HRESULT DllRegisterServer(void);
        HRESULT DllUnregisterServer(void);
        HRESULT DllCanUnloadNow(void);
        HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
    }
}
