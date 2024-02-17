/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file WrapperJoyWinMM.h
 *   Declaration of the wrapper for all WinMM joystick functions.
 **************************************************************************************************/

#pragma once

#include "ApiWindows.h"

namespace Xidi
{
  namespace WrapperJoyWinMM
  {
    MMRESULT JoyConfigChanged(DWORD dwFlags);
    template <typename JoyCapsType> MMRESULT JoyGetDevCaps(
        UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc);
    UINT JoyGetNumDevs(void);
    MMRESULT JoyGetPos(UINT uJoyID, LPJOYINFO pji);
    MMRESULT JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
    MMRESULT JoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
    MMRESULT JoyReleaseCapture(UINT uJoyID);
    MMRESULT JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
    MMRESULT JoySetThreshold(UINT uJoyID, UINT uThreshold);
  }; // namespace WrapperJoyWinMM
} // namespace Xidi
