/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperJoyWinMM.cpp
 *      Implementation of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ImportApiWinMM.h"
#include "WrapperJoyWinMM.h"
#include "XInputController.h"
#include "Mapper/Base.h"
#include "Mapper/StandardGamepad.h"
#include "Mapper/NativeXInput.h"
#include "Mapper/NativeXInputSharedTriggers.h"

using namespace Xidi;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperJoyWinMM.h" for documentation.

WrapperJoyWinMM::WrapperJoyWinMM()
{
    // Create controllers, each with fixed user index.
    for (DWORD i = 0; i < _countof(controllers); ++i)
        controllers[i] = new XInputController(i);

    mapper = new Mapper::StandardGamepad();
}

// ---------

WrapperJoyWinMM::~WrapperJoyWinMM()
{
    // Delete the created controllers.
    for (DWORD i = 0; i < _countof(controllers); ++i)
        delete controllers[i];

    // Delete the created mapper.
    delete mapper;
}


// -------- METHODS: WinMM JOYSTICK ---------------------------------------- //
// See WinMM documentation for more information.

MMRESULT WrapperJoyWinMM::JoyConfigChanged(DWORD dwFlags)
{
    return ImportApiWinMM::joyConfigChanged(dwFlags);
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    // Check for the correct structure size.
    if (sizeof(*pjc) != cbjc)
        return JOYERR_PARMS;

    return JOYERR_NOCANDO;
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    // Check for the correct structure size.
    if (sizeof(*pjc) != cbjc)
        return JOYERR_PARMS;
    
    return JOYERR_NOCANDO;
}

// ---------

UINT WrapperJoyWinMM::JoyGetNumDevs(void)
{
    // Number of controllers is fixed.
    return _countof(controllers);
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    // Ensure the controller number is within bounds.
    if (!(uJoyID < JoyGetNumDevs()))
        return JOYERR_PARMS;

    return JOYERR_NOCANDO;
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    // Check for the correct structure size.
    if (sizeof(*pji) != pji->dwSize)
        return JOYERR_PARMS;

    // Ensure the controller number is within bounds.
    if (!(uJoyID < JoyGetNumDevs()))
        return JOYERR_PARMS;

    return JOYERR_NOCANDO;
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    // Operation not supported.
    return JOYERR_NOCANDO;
}

// ---------

MMRESULT WrapperJoyWinMM::JoyReleaseCapture(UINT uJoyID)
{
    // Operation not supported.
    return JOYERR_NOCANDO;
}

// ---------

MMRESULT WrapperJoyWinMM::JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    // Operation not supported.
    return JOYERR_NOCANDO;
}

// ---------

MMRESULT WrapperJoyWinMM::JoySetThreshold(UINT uJoyID, UINT uThreshold)
{
    // Operation not supported.
    return JOYERR_NOCANDO;
}
