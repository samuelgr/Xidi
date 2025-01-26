/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
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
    extern "C" MMRESULT __stdcall joyConfigChanged(DWORD dwFlags);
    extern "C" MMRESULT __stdcall joyGetDevCapsA(UINT_PTR uJoyID, JOYCAPSA* pjc, UINT cbjc);
    extern "C" MMRESULT __stdcall joyGetDevCapsW(UINT_PTR uJoyID, JOYCAPSW* pjc, UINT cbjc);
    extern "C" UINT __stdcall joyGetNumDevs(void);
    extern "C" MMRESULT __stdcall joyGetPos(UINT uJoyID, LPJOYINFO pji);
    extern "C" MMRESULT __stdcall joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
    extern "C" MMRESULT __stdcall joyGetThreshold(UINT uJoyID, LPUINT puThreshold);
    extern "C" MMRESULT __stdcall joyReleaseCapture(UINT uJoyID);
    extern "C" MMRESULT __stdcall joySetCapture(
        HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
    extern "C" MMRESULT __stdcall joySetThreshold(UINT uJoyID, UINT uThreshold);
  } // namespace WrapperJoyWinMM
} // namespace Xidi
