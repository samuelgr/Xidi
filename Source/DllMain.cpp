/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file DllMain.cpp
 *   Entry point when loading or unloading this dynamic library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"


// -------- ENTRY POINT ---------------------------------------------------- //

/// Performs library initialization and teardown functions.
/// Invoked automatically by the operating system.
/// Refer to Windows documentation for more information.
/// @param [in] hModule Instance handle for this library.
/// @param [in] ulReasonForCall Specifies the event that caused this function to be invoked.
/// @param [in] lpReserved Reserved.
/// @return `TRUE` if this function successfully initialized or uninitialized this library, `FALSE` otherwise.
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved)
{
    switch (ulReasonForCall)
    {
        case DLL_PROCESS_ATTACH:
            Xidi::Globals::Initialize();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
