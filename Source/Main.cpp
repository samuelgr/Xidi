/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Main.cpp
 *      Entry point when loading or unloading this dynamic library.
 *****************************************************************************/

#include "API_Windows.h"
#include "ImportAPI_dinput8.h"


// -------- FUNCTIONS ------------------------------------------------------ //

// Main DLL entry and exit point.
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    BOOL result = TRUE;
    
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            if (S_OK != ImportAPI_dinput8::Initialize()) result = FALSE;
            break;

        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return result;
}
