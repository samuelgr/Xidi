/*****************************************************************************
 * XInputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DllMain.cpp
 *      Entry point when loading or unloading this dynamic library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Dinput8ImportApi.h"

using namespace XInputControllerDirectInput;


// -------- ENTRY POINT ---------------------------------------------------- //

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    BOOL result = TRUE;
    
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            if (S_OK != Dinput8ImportApi::Initialize())
                result = FALSE;
            break;

        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return result;
}
