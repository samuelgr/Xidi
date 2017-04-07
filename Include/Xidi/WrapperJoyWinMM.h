/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file WrapperJoyWinMM.h
 *   Declaration of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "XInputController.h"
#include "Mapper/Base.h"


namespace Xidi
{
    // Wraps the WinMM joystick interface.
    // All methods are class methods, because the wrapped interface is not object-oriented.
    class WrapperJoyWinMM
    {
    private:
        // -------- TYPE DEFINITIONS ----------------------------------------------- //
        
        // Holds controller state information retrieved from the mapper.
        struct SJoyStateData
        {
            LONG axisX;
            LONG axisY;
            LONG axisZ;
            LONG axisRx;
            LONG axisRy;
            LONG axisRz;
            LONG pov;
            BYTE buttons[32];
        };
        
        
        // -------- CLASS VARIABLES ------------------------------------------------ //
        
        // Fixed set of four XInput controllers.
        static XInputController* controllers[XInputController::kMaxNumXInputControllers];
        
        // Mapping scheme to be applied to all controllers.
        static Mapper::Base* mapper;
        
        // Specifies if the class is initialized.
        static BOOL isInitialized;
        
        // Specifies the format of each field in SJoyStateData in DirectInput-compatible format.
        static DIOBJECTDATAFORMAT joyStateObjectDataFormat[];
        
        // Specifies the overall data format of SJoyStateData in DirectInput-compatible format.
        static const DIDATAFORMAT joyStateDataFormat;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        WrapperJoyWinMM();
        
        
        // -------- CLASS METHODS -------------------------------------------------- //
        
        // Initializes this class.
        static void Initialize(void);


        // -------- HELPERS -------------------------------------------------------- //
        
        // Communicates with the relevant controller and the mapper to fill the provided structure with device state information.
        static MMRESULT FillDeviceState(UINT joyID, SJoyStateData* joyStateData);

        // Fills in the specified buffer with the name of the registry key to use for referencing controller names.
        // Returns the number of characters written, or negative in the event of an error.
        // This is the non-Unicode version.
        static int FillRegistryKeyStringA(LPSTR buf, const size_t bufcount);

        // Fills in the specified buffer with the name of the registry key to use for referencing controller names.
        // Returns the number of characters written, or negative in the event of an error.
        // This is the Unicode version.
        static int FillRegistryKeyStringW(LPWSTR buf, const size_t bufcount);

        // Places the required keys and values into the registry so that WinMM-based applications can find the correct controller names.
        static void SetControllerNameRegistryInfo(void);
        
        
    public:
        // -------- METHODS: WinMM JOYSTICK ---------------------------------------- //
        static MMRESULT JoyConfigChanged(DWORD dwFlags);
        static MMRESULT JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
        static MMRESULT JoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
        static UINT JoyGetNumDevs(void);
        static MMRESULT JoyGetPos(UINT uJoyID, LPJOYINFO pji);
        static MMRESULT JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        static MMRESULT JoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        static MMRESULT JoyReleaseCapture(UINT uJoyID);
        static MMRESULT JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        static MMRESULT JoySetThreshold(UINT uJoyID, UINT uThreshold);
    };
}
