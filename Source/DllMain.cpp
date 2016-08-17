/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DllMain.cpp
 *      Entry point when loading or unloading this dynamic library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"


// -------- ENTRY POINT ---------------------------------------------------- //

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    BOOL result = TRUE;
    
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            Xidi::Globals::SetInstanceHandle(hModule);
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return result;
}
