/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file TestAppWinMM.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library via WinMM.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Configuration.h"
#include "ExportApiWinMM.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "Log.h"
#include "TestApp.h"

#include <RegStr.h>

using namespace Xidi;


// -------- MACROS --------------------------------------------------------- //

// Helpers for correct function calls when using Unicode.
#ifdef UNICODE
#define ExportApiWinMMJoyGetDevCaps             ExportApiWinMMJoyGetDevCapsW
#else
#define ExportApiWinMMJoyGetDevCaps             ExportApiWinMMJoyGetDevCapsA
#endif


// -------- HELPERS -------------------------------------------------------- //

// Retrieves the controller name from the registry for the specified controller index.
// Returns the number of characters written, with zero indicating an error.
size_t GetJoystickName(UINT index, TCHAR* buf, size_t count)
{
    // Sanity check.
    if (ExportApiWinMMJoyGetNumDevs() <= index)
        return 0;

    // Get the registry key name.
    JOYCAPS joyCaps;
    if (JOYERR_NOERROR != ExportApiWinMMJoyGetDevCaps((UINT_PTR)-1, &joyCaps, sizeof(joyCaps)))
        return 0;

    // Open the correct registry key to determine the location to look for the joystick's actual OEM name.
    HKEY registryKey;
    TCHAR registryPath[1024];
    _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG _T("\\%s\\") REGSTR_KEY_JOYCURR, joyCaps.szRegKey);
    
    LRESULT result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_QUERY_VALUE, NULL, &registryKey, NULL);
    if (ERROR_SUCCESS != result)
        return 0;

    // Figure out the identifying string for the joystick, which specifies where to look for its actual OEM name.
    TCHAR registryValueName[64];
    _stprintf_s(registryValueName, _countof(registryValueName), REGSTR_VAL_JOYNOEMNAME, ((int)index + 1));

    TCHAR registryValueData[256];
    DWORD registryValueSize = sizeof(registryValueData);
    
    result = RegGetValue(registryKey, NULL, registryValueName, RRF_RT_REG_SZ, NULL, registryValueData, &registryValueSize);
    RegCloseKey(registryKey);

    if (ERROR_SUCCESS != result)
        return 0;

    // Open the correct registry key to look for the joystick's OEM name.
    _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYOEM _T("\\%s"), registryValueData);
    
    result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_QUERY_VALUE, NULL, &registryKey, NULL);
    if (ERROR_SUCCESS != result)
        return 0;

    // Read the joystick's OEM name.
    registryValueSize = (DWORD)count * sizeof(TCHAR);
    result = RegGetValue(registryKey, NULL, REGSTR_VAL_JOYOEMNAME, RRF_RT_REG_SZ, NULL, buf, &registryValueSize);

    return (ERROR_SUCCESS == result ? registryValueSize / sizeof(TCHAR) : 0);
}


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
    tout << _T("Driver reports ") << numJoysticks << _T(" joysticks are available.") << endl << endl;
    tout << _T("Begin enumerating devices via joyGetDevCaps") << endl;
    
    UINT devIdx = numJoysticks;
    for (DWORD i = 0; i < numJoysticks; ++i)
    {
        JOYCAPS joyCaps;

        result = ExportApiWinMMJoyGetDevCaps(i, &joyCaps, sizeof(joyCaps));
        if (JOYERR_NOERROR == result)
        {
            TCHAR joystickName[1024];
            
            if (0 == GetJoystickName(i, joystickName, _countof(joystickName)))
                tout << _T("    Joystick \"(unknown)\" detected at ") << i;
            else
            {
                tout << _T("    Joystick \"") << joystickName << _T("\" detected at ") << i;

                if ((i < devIdx) && (NULL != _tcsstr(joystickName, _T("Xidi: "))))
                {
                    devIdx = i;
                    tout << _T(", selected");
                }
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
    tout << _T("Axes present: X Y");
    if (joyCaps.wCaps & JOYCAPS_HASZ) tout << _T(" Z");
    if (joyCaps.wCaps & JOYCAPS_HASR) tout << _T(" R");
    if (joyCaps.wCaps & JOYCAPS_HASU) tout << _T(" U");
    if (joyCaps.wCaps & JOYCAPS_HASV) tout << _T(" V");
    tout << endl;
    tout << _T("Max axes = ") << joyCaps.wMaxAxes << _T(", max buttons = ") << joyCaps.wMaxButtons << _T(", max period = ") << joyCaps.wPeriodMax << _T(", min period = ") << joyCaps.wPeriodMin << endl;
    tout << _T("X axis: max = ") << joyCaps.wXmax << _T(", min = ") << joyCaps.wXmin << endl;
    tout << _T("Y axis: max = ") << joyCaps.wYmax << _T(", min = ") << joyCaps.wYmin << endl;
    tout << _T("Z axis: max = ") << joyCaps.wZmax << _T(", min = ") << joyCaps.wZmin << endl;
    tout << _T("R axis: max = ") << joyCaps.wRmax << _T(", min = ") << joyCaps.wRmin << endl;
    tout << _T("U axis: max = ") << joyCaps.wUmax << _T(", min = ") << joyCaps.wUmin << endl;
    tout << _T("V axis: max = ") << joyCaps.wVmax << _T(", min = ") << joyCaps.wVmin << endl;
    tout << _T("Manufacturer ID = ") << joyCaps.wMid << _T(", product ID = ") << joyCaps.wPid << endl;
    tout << _T("Product name: ") << joyCaps.szPname << endl;
    tout << _T("OEM driver name: ") << joyCaps.szOEMVxD << endl;
    tout << _T("Registry key: ") << joyCaps.szRegKey << endl;
    tout << endl;


    ////////////////////////////////////
    ////////   Interactive Mode Preparation

    tout << _T("Preparing to launch interactive mode... ");
    tout << _T("DONE") << endl;
    tout << _T("Device state is updated twice per second.") << endl;
    tout << _T("Quits automatically after 50 updates. To quit early, use CTRL+C.") << endl;
    system("pause");
    system("cls");

    JOYINFOEX testData;
    ZeroMemory(&testData, sizeof(testData));
    testData.dwSize = sizeof(testData);
    testData.dwFlags = JOY_RETURNALL;

    for (unsigned int i = 0; i < 50; ++i)
    {
        system("cls");
        tout << _T("Update #") << (i+1) << endl;
        
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

        Sleep(500);
    }


    ////////////////////////////////////
    ////////   Cleanup and Exit

    tout << _T("\nExiting.") << endl;
    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    Globals::SetInstanceHandle(GetModuleHandle(NULL));
    Configuration::ParseAndApplyConfigurationFile();
    
    int result = RunTestApp(argc, argv);

    Log::FinalizeLog();
    system("pause");
    return result;
}
