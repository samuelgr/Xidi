/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * TestAppWinMM.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library via WinMM.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ExportApiWinMM.h"
#include "ImportApiWinMM.h"
#include "TestApp.h"


using namespace Xidi;


// -------- MACROS --------------------------------------------------------- //

// Helpers for correct function calls when using Unicode.
#ifdef UNICODE
#define ExportApiWinMMJoyGetDevCaps             ExportApiWinMMJoyGetDevCapsW
#else
#define ExportApiWinMMJoyGetDevCaps             ExportApiWinMMJoyGetDevCapsA
#endif


// -------- FUNCTIONS ------------------------------------------------------ //

// Runs the test application. Effectively acts as its entry point.
int RunTestApp(int argc, char* argv[])
{
    MMRESULT result;

    ////////////////////////////////////
    ////////   Initialization

    // Initialize the imported WinMM API.
    if (MMSYSERR_NOERROR != ImportApiWinMM::Initialize())
    {
        terr << _T("Unable to initialize WinMM API.") << endl;
        return -1;
    }

    
    ////////////////////////////////////
    ////////   Enumeration

    // Check the number of devices to enumerate.
    UINT numJoysticks = ExportApiWinMMJoyGetNumDevs();
    if (0 == numJoysticks)
    {
        terr << _T("No joysticks supported by current driver.") << endl;
        return -1;
    }
    
    // Enumerate all the devices attached to the system.
    tout << _T("Begin enumerating devices via joyGetDevCaps") << endl;
    
    UINT devIdx = numJoysticks;
    for (DWORD i = 0; i < numJoysticks; ++i)
    {
        JOYCAPS joyCaps;

        result = ExportApiWinMMJoyGetDevCaps(i, &joyCaps, sizeof(joyCaps));
        if (JOYERR_NOERROR == result)
        {
            tout << _T("    Joystick detected at ") << i;
            if (i < devIdx)
            {
                devIdx = i;
                tout << _T(", selected");
            }
            tout << endl;
        }
    }

    if (devIdx == numJoysticks)
    {
        tout << _T("No supported devices found. Connect one and try again.") << endl;
        return -1;
    }

    tout << _T("End enumerating devices via joyGetDevCaps") << endl << endl;


    ////////////////////////////////////
    ////////   Device Capabilities

    JOYCAPS joyCaps;
    result = ExportApiWinMMJoyGetDevCaps(devIdx, &joyCaps, sizeof(joyCaps));
    if (JOYERR_NOERROR != result)
    {
        terr << _T("Unable to obtain get device capabilities: code ") << result << _T(".") << endl;
        return -1;
    }

    tout << _T("Device presents ") << joyCaps.wNumAxes << _T(" axes, ") << joyCaps.wNumButtons << _T(" buttons, and ") << (joyCaps.wCaps & JOYCAPS_HASPOV ? 1 : 0) << _T(" POV controllers.") << endl;
    tout << _T("Max axes = ") << joyCaps.wMaxAxes << _T(", max buttons = ") << joyCaps.wMaxButtons << _T(", max period = ") << joyCaps.wPeriodMax << _T(", min period = ") << joyCaps.wPeriodMin << endl;
    tout << _T("X axis: max = ") << joyCaps.wXmax << _T(", min = ") << joyCaps.wXmin << endl;
    tout << _T("Y axis: max = ") << joyCaps.wYmax << _T(", min = ") << joyCaps.wYmin << endl;
    tout << _T("Z axis: max = ") << joyCaps.wZmax << _T(", min = ") << joyCaps.wZmin << endl;
    tout << _T("R axis: max = ") << joyCaps.wRmax << _T(", min = ") << joyCaps.wRmin << endl;
    tout << _T("U axis: max = ") << joyCaps.wUmax << _T(", min = ") << joyCaps.wUmin << endl;
    tout << _T("V axis: max = ") << joyCaps.wVmax << _T(", min = ") << joyCaps.wVmin << endl;
    tout << _T("Product name: ") << joyCaps.szPname << endl;
    tout << _T("OEM driver name: ") << joyCaps.szOEMVxD << endl;
    tout << _T("Registry key: ") << joyCaps.szRegKey << endl;
    tout << endl;


    ////////////////////////////////////
    ////////   Interactive Mode Preparation

    tout << _T("Preparing to launch interactive mode... ");
    tout << _T("DONE") << endl;
    tout << _T("After every character typed, the device's state will be read and reported.") << endl;
    tout << _T("All axes are set to a range of -100 to +100, with 25% each deadzone/saturation.") << endl;
    tout << _T("To quit, type Q and press RETURN.") << endl;
    tout << _T("To re-read the device's state, type any other character and press RETURN.") << endl;
    system("pause");
    system("cls");

    JOYINFOEX testData;
    ZeroMemory(&testData, sizeof(testData));
    testData.dwSize = sizeof(testData);
    testData.dwFlags = JOY_RETURNALL;

    TCHAR inputchar = _T('\0');

    while (_T('Q') != inputchar && _T('q') != inputchar)
    {
        system("cls");
        
        result = ExportApiWinMMJoyGetPosEx(devIdx, &testData);
        if (JOYERR_NOERROR != result)
        {
            tout << _T("Failed to retrieve device state.") << endl;
            return 1;
        }

        tout << _T("Device presents ") << joyCaps.wNumAxes << _T(" axes, ") << joyCaps.wNumButtons << _T(" buttons, and ") << (joyCaps.wCaps & JOYCAPS_HASPOV ? 1 : 0) << _T(" POV controllers.") << endl;
        tout << _T("Max axes = ") << joyCaps.wMaxAxes << _T(", max buttons = ") << joyCaps.wMaxButtons << _T(", max period = ") << joyCaps.wPeriodMax << _T(", min period = ") << joyCaps.wPeriodMin << endl;
        tout << _T("X axis: max = ") << joyCaps.wXmax << _T(", min = ") << joyCaps.wXmin << endl;
        tout << _T("Y axis: max = ") << joyCaps.wYmax << _T(", min = ") << joyCaps.wYmin << endl;
        tout << _T("Z axis: max = ") << joyCaps.wZmax << _T(", min = ") << joyCaps.wZmin << endl;
        tout << _T("R axis: max = ") << joyCaps.wRmax << _T(", min = ") << joyCaps.wRmin << endl;
        tout << _T("U axis: max = ") << joyCaps.wUmax << _T(", min = ") << joyCaps.wUmin << endl;
        tout << _T("V axis: max = ") << joyCaps.wVmax << _T(", min = ") << joyCaps.wVmin << endl;

        tout << endl;

        tout << _T("Device state:") << endl;

        tout << endl;

        tout << _T("   X Axis  = ") << testData.dwXpos << endl;
        tout << _T("   Y Axis  = ") << testData.dwYpos << endl;
        tout << _T("   Z Axis  = ") << testData.dwZpos << endl;

        tout << endl;

        tout << _T("   R Axis  = ") << testData.dwRpos << endl;
        tout << _T("   U Axis  = ") << testData.dwUpos << endl;
        tout << _T("   V Axis  = ") << testData.dwVpos << endl;

        tout << endl;

        tout << _T("   Dpad    = ") << testData.dwPOV << endl;

        tout << endl;

        tout << _T("   Buttons pressed:");
        for (DWORD i = 0; i < joyCaps.wMaxButtons; ++i)
        {
            if (testData.dwButtons & (1 << i))
                tout << _T(" ") << (i + 1);
        }

        tout << endl << endl << _T("Awaiting input (character then RETURN)... ");
        tin >> inputchar;
    }


    ////////////////////////////////////
    ////////   Cleanup and Exit

    tout << _T("Exiting.") << endl;
    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    int result = RunTestApp(argc, argv);

    system("pause");
    return result;
}
