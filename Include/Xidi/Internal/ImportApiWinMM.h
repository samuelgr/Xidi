/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ImportApiWinMM.h
 *   Declarations of functions for accessing the WinMM API imported from the
 *   native WinMM library.
 **************************************************************************************************/

#pragma once

#include <string_view>
#include <unordered_map>

#include "ApiWindows.h"
#include "ApiXidi.h"

namespace Xidi
{
  namespace ImportApiWinMM
  {
    /// Retrieves a pointer to the interface that allows some imported functions to be replaced.
    Api::IMutableImportTable* GetMutableImportTable(void);

    /// Dynamically loads the WinMM library and sets up all imported function calls.
    void Initialize(void);

    // clang-format off

    MMRESULT     joyConfigChanged(DWORD dwFlags);
    MMRESULT     joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
    MMRESULT     joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
    UINT         joyGetNumDevs(void);
    MMRESULT     joyGetPos(UINT uJoyID, LPJOYINFO pji);
    MMRESULT     joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
    MMRESULT     joyGetThreshold(UINT uJoyID, LPUINT puThreshold);
    MMRESULT     joyReleaseCapture(UINT uJoyID);
    MMRESULT     joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
    MMRESULT     joySetThreshold(UINT uJoyID, UINT uThreshold);

    MMRESULT     timeBeginPeriod(UINT uPeriod);
    MMRESULT     timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
    DWORD        timeGetTime(void);

    // clang-format on
  } // namespace ImportApiWinMM
} // namespace Xidi
