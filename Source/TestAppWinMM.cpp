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

#include <iostream>
#include <RegStr.h>

using namespace std;
using namespace Xidi;


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
    if (JOYERR_NOERROR != ExportApiWinMMJoyGetDevCapsW((UINT_PTR)-1, &joyCaps, sizeof(joyCaps)))
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
        wcerr << _T("Unable to initialize WinMM API.") << endl;
        return -1;
    }

    
    ////////////////////////////////////
    ////////   Enumeration

    // Check the number of devices to enumerate.
    UINT numJoysticks = ExportApiWinMMJoyGetNumDevs();
    if (0 == numJoysticks)
    {
        wcerr << _T("No joysticks supported by current driver.") << endl;
        return -1;
    }
    
    // Enumerate all the devices attached to the system.
    wcout << _T("Driver reports ") << numJoysticks << _T(" joysticks are available.") << endl << endl;
    wcout << _T("Begin enumerating devices via joyGetDevCaps") << endl;
    
    UINT devIdx = numJoysticks;
    for (DWORD i = 0; i < numJoysticks; ++i)
    {
        JOYCAPS joyCaps;

        result = ExportApiWinMMJoyGetDevCapsW(i, &joyCaps, sizeof(joyCaps));
        if (JOYERR_NOERROR == result)
        {
            TCHAR joystickName[1024];
            
            if (0 == GetJoystickName(i, joystickName, _countof(joystickName)))
                wcout << _T("    Joystick \"(unknown)\" detected at ") << i;
            else
            {
                wcout << _T("    Joystick \"") << joystickName << _T("\" detected at ") << i;

                if ((i < devIdx) && (NULL != _tcsstr(joystickName, _T("Xidi: "))))
                {
                    devIdx = i;
                    wcout << _T(", selected");
                }
            }
            
            wcout << endl;
        }
    }

    if (devIdx == numJoysticks)
    {
        wcout << _T("No supported devices found. Connect one and try again.") << endl;
        return -1;
    }

    wcout << _T("End enumerating devices via joyGetDevCaps") << endl << endl;


    ////////////////////////////////////
    ////////   Device Capabilities

    JOYCAPS joyCaps;
    result = ExportApiWinMMJoyGetDevCapsW(devIdx, &joyCaps, sizeof(joyCaps));
    if (JOYERR_NOERROR != result)
    {
        wcerr << _T("Unable to obtain get device capabilities: code ") << result << _T(".") << endl;
        return -1;
    }

    wcout << _T("Device presents ") << joyCaps.wNumAxes << _T(" axes, ") << joyCaps.wNumButtons << _T(" buttons, and ") << (joyCaps.wCaps & JOYCAPS_HASPOV ? 1 : 0) << _T(" POV controllers.") << endl;
    wcout << _T("Axes present: X Y");
    if (joyCaps.wCaps & JOYCAPS_HASZ) wcout << _T(" Z");
    if (joyCaps.wCaps & JOYCAPS_HASR) wcout << _T(" R");
    if (joyCaps.wCaps & JOYCAPS_HASU) wcout << _T(" U");
    if (joyCaps.wCaps & JOYCAPS_HASV) wcout << _T(" V");
    wcout << endl;
    wcout << _T("Max axes = ") << joyCaps.wMaxAxes << _T(", max buttons = ") << joyCaps.wMaxButtons << _T(", max period = ") << joyCaps.wPeriodMax << _T(", min period = ") << joyCaps.wPeriodMin << endl;
    wcout << _T("X axis: max = ") << joyCaps.wXmax << _T(", min = ") << joyCaps.wXmin << endl;
    wcout << _T("Y axis: max = ") << joyCaps.wYmax << _T(", min = ") << joyCaps.wYmin << endl;
    wcout << _T("Z axis: max = ") << joyCaps.wZmax << _T(", min = ") << joyCaps.wZmin << endl;
    wcout << _T("R axis: max = ") << joyCaps.wRmax << _T(", min = ") << joyCaps.wRmin << endl;
    wcout << _T("U axis: max = ") << joyCaps.wUmax << _T(", min = ") << joyCaps.wUmin << endl;
    wcout << _T("V axis: max = ") << joyCaps.wVmax << _T(", min = ") << joyCaps.wVmin << endl;
    wcout << _T("Manufacturer ID = ") << joyCaps.wMid << _T(", product ID = ") << joyCaps.wPid << endl;
    wcout << _T("Product name: ") << joyCaps.szPname << endl;
    wcout << _T("OEM driver name: ") << joyCaps.szOEMVxD << endl;
    wcout << _T("Registry key: ") << joyCaps.szRegKey << endl;
    wcout << endl;


    ////////////////////////////////////
    ////////   Interactive Mode Preparation

    wcout << _T("Preparing to launch interactive mode... ");
    wcout << _T("DONE") << endl;
    wcout << _T("Device state is updated twice per second.") << endl;
    wcout << _T("Quits automatically after 50 updates. To quit early, use CTRL+C.") << endl;
    system("pause");
    system("cls");

    JOYINFOEX testData;
    ZeroMemory(&testData, sizeof(testData));
    testData.dwSize = sizeof(testData);
    testData.dwFlags = JOY_RETURNALL;

    for (unsigned int i = 0; i < 50; ++i)
    {
        system("cls");
        wcout << _T("Update #") << (i+1) << endl;
        
        result = ExportApiWinMMJoyGetPosEx(devIdx, &testData);
        if (JOYERR_NOERROR != result)
        {
            wcout << _T("Failed to retrieve device state.") << endl;
            return 1;
        }

        wcout << _T("Device presents ") << joyCaps.wNumAxes << _T(" axes, ") << joyCaps.wNumButtons << _T(" buttons, and ") << (joyCaps.wCaps & JOYCAPS_HASPOV ? 1 : 0) << _T(" POV controllers.") << endl;
        wcout << _T("Max axes = ") << joyCaps.wMaxAxes << _T(", max buttons = ") << joyCaps.wMaxButtons << _T(", max period = ") << joyCaps.wPeriodMax << _T(", min period = ") << joyCaps.wPeriodMin << endl;
        wcout << _T("X axis: max = ") << joyCaps.wXmax << _T(", min = ") << joyCaps.wXmin << endl;
        wcout << _T("Y axis: max = ") << joyCaps.wYmax << _T(", min = ") << joyCaps.wYmin << endl;
        wcout << _T("Z axis: max = ") << joyCaps.wZmax << _T(", min = ") << joyCaps.wZmin << endl;
        wcout << _T("R axis: max = ") << joyCaps.wRmax << _T(", min = ") << joyCaps.wRmin << endl;
        wcout << _T("U axis: max = ") << joyCaps.wUmax << _T(", min = ") << joyCaps.wUmin << endl;
        wcout << _T("V axis: max = ") << joyCaps.wVmax << _T(", min = ") << joyCaps.wVmin << endl;

        wcout << endl;

        wcout << _T("Device state:") << endl;

        wcout << endl;

        wcout << _T("   X Axis  = ") << testData.dwXpos << endl;
        wcout << _T("   Y Axis  = ") << testData.dwYpos << endl;
        wcout << _T("   Z Axis  = ") << testData.dwZpos << endl;

        wcout << endl;

        wcout << _T("   R Axis  = ") << testData.dwRpos << endl;
        wcout << _T("   U Axis  = ") << testData.dwUpos << endl;
        wcout << _T("   V Axis  = ") << testData.dwVpos << endl;

        wcout << endl;

        wcout << _T("   Dpad    = ") << testData.dwPOV << endl;

        wcout << endl;

        wcout << _T("   Buttons pressed:");
        for (DWORD i = 0; i < joyCaps.wMaxButtons; ++i)
        {
            if (testData.dwButtons & (1 << i))
                wcout << _T(" ") << (i + 1);
        }

        Sleep(500);
    }


    ////////////////////////////////////
    ////////   Cleanup and Exit

    wcout << _T("\nExiting.") << endl;
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
