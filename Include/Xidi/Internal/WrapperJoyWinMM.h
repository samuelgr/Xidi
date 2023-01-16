/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file WrapperJoyWinMM.h
 *   Declaration of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "VirtualController.h"

#include <string>
#include <utility>
#include <vector>


namespace Xidi
{
    namespace WrapperJoyWinMM
    {
        // -------- FUNCTIONS: WinMM JOYSTICK -------------------------------------- //

        MMRESULT JoyConfigChanged(DWORD dwFlags);
        template <typename JoyCapsType> MMRESULT JoyGetDevCaps(UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc);
        UINT JoyGetNumDevs(void);
        MMRESULT JoyGetPos(UINT uJoyID, LPJOYINFO pji);
        MMRESULT JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        MMRESULT JoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        MMRESULT JoyReleaseCapture(UINT uJoyID);
        MMRESULT JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        MMRESULT JoySetThreshold(UINT uJoyID, UINT uThreshold);
    };
}
