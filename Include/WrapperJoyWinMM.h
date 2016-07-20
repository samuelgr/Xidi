/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperJoyWinMM.h
 *      Declaration of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "XInputController.h"
#include "Mapper/Base.h"


namespace Xidi
{
    // Wraps the IDirectInput8 interface to hook into all calls to it.
    // Holds an underlying instance of an IDirectInput object but wraps all method invocations.
    class WrapperJoyWinMM
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //
        
        // Fixed set of four XInput controllers.
        XInputController* controllers[4];
        
        // Mapping scheme to be applied to all controllers.
        Mapper::Base* mapper;
        
    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor.
        WrapperJoyWinMM();
        
        // Default destructor.
        ~WrapperJoyWinMM();

        
        // -------- METHODS: WinMM JOYSTICK ---------------------------------------- //
        MMRESULT JoyConfigChanged(DWORD dwFlags);
        MMRESULT JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
        MMRESULT JoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
        UINT JoyGetNumDevs(void);
        MMRESULT JoyGetPos(UINT uJoyID, LPJOYINFO pji);
        MMRESULT JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        MMRESULT JoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        MMRESULT JoyReleaseCapture(UINT uJoyID);
        MMRESULT JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        MMRESULT JoySetThreshold(UINT uJoyID, UINT uThreshold);
    };
}
