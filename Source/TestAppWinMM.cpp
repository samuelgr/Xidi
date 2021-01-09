/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file TestAppWinMM.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library via WinMM.
 *****************************************************************************/

#include <iostream>
#include <Windows.h>
#include <RegStr.h>

using namespace std;


// -------- HELPERS -------------------------------------------------------- //

// Retrieves the controller name from the registry for the specified controller index.
// Returns the number of characters written, with zero indicating an error.
size_t GetJoystickName(UINT index, wchar_t* buf, size_t count)
{
    // Sanity check.
    if (joyGetNumDevs() <= index)
        return 0;

    // Get the registry key name.
    JOYCAPS joyCaps;
    if (JOYERR_NOERROR != joyGetDevCapsW((UINT_PTR)-1, &joyCaps, sizeof(joyCaps)))
        return 0;

    // Open the correct registry key to determine the location to look for the joystick's actual OEM name.
    HKEY registryKey;
    wchar_t registryPath[1024];
    swprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG L"\\%s\\" REGSTR_KEY_JOYCURR, joyCaps.szRegKey);

    LRESULT result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, nullptr, REG_OPTION_VOLATILE, KEY_QUERY_VALUE, nullptr, &registryKey, nullptr);
    if (ERROR_SUCCESS != result)
        return 0;

    // Figure out the identifying string for the joystick, which specifies where to look for its actual OEM name.
    wchar_t registryValueName[64];
    swprintf_s(registryValueName, _countof(registryValueName), REGSTR_VAL_JOYNOEMNAME, ((int)index + 1));

    wchar_t registryValueData[256];
    DWORD registryValueSize = sizeof(registryValueData);

    result = RegGetValue(registryKey, nullptr, registryValueName, RRF_RT_REG_SZ, nullptr, registryValueData, &registryValueSize);
    RegCloseKey(registryKey);

    if (ERROR_SUCCESS != result)
        return 0;

    // Open the correct registry key to look for the joystick's OEM name.
    swprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYOEM L"\\%s", registryValueData);

    result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, nullptr, REG_OPTION_VOLATILE, KEY_QUERY_VALUE, nullptr, &registryKey, nullptr);
    if (ERROR_SUCCESS != result)
        return 0;

    // Read the joystick's OEM name.
    registryValueSize = (DWORD)count * sizeof(wchar_t);
    result = RegGetValue(registryKey, nullptr, REGSTR_VAL_JOYOEMNAME, RRF_RT_REG_SZ, nullptr, buf, &registryValueSize);

    return (ERROR_SUCCESS == result ? registryValueSize / sizeof(wchar_t) : 0);
}


// -------- FUNCTIONS ------------------------------------------------------ //

// Runs the test application. Effectively acts as its entry point.
int RunTestApp(int argc, char* argv[])
{
    MMRESULT result;

    ////////////////////////////////////
    ////////   Initialization

    // Nothing to do here.


    ////////////////////////////////////
    ////////   Enumeration

    // Check the number of devices to enumerate.
    UINT numJoysticks = joyGetNumDevs();
    if (0 == numJoysticks)
    {
        wcerr << L"No joysticks supported by current driver." << endl;
        return -1;
    }

    // Enumerate all the devices attached to the system.
    wcout << L"Driver reports " << numJoysticks << L" joysticks are available." << endl << endl;
    wcout << L"Begin enumerating devices via joyGetDevCaps" << endl;

    UINT devIdx = numJoysticks;
    for (DWORD i = 0; i < numJoysticks; ++i)
    {
        JOYCAPS joyCaps;

        result = joyGetDevCapsW(i, &joyCaps, sizeof(joyCaps));
        if (JOYERR_NOERROR == result)
        {
            wchar_t joystickName[1024];

            if (0 == GetJoystickName(i, joystickName, _countof(joystickName)))
                wcout << L"    Joystick \"(unknown)\" detected at " << i;
            else
            {
                wcout << L"    Joystick \"" << joystickName << L"\" detected at " << i;

                if ((i < devIdx) && (nullptr != wcsstr(joystickName, L"Xidi: ")))
                {
                    devIdx = i;
                    wcout << L", selected";
                }
            }

            wcout << endl;
        }
    }

    if (devIdx == numJoysticks)
    {
        wcout << L"No supported devices found. Connect one and try again." << endl;
        return -1;
    }

    wcout << L"End enumerating devices via joyGetDevCaps" << endl << endl;


    ////////////////////////////////////
    ////////   Device Capabilities

    JOYCAPS joyCaps;
    result = joyGetDevCapsW(devIdx, &joyCaps, sizeof(joyCaps));
    if (JOYERR_NOERROR != result)
    {
        wcerr << L"Unable to obtain get device capabilities: code " << result << L"." << endl;
        return -1;
    }

    wcout << L"Device presents " << joyCaps.wNumAxes << L" axes, " << joyCaps.wNumButtons << L" buttons, and " << (joyCaps.wCaps & JOYCAPS_HASPOV ? 1 : 0) << L" POV controllers." << endl;
    wcout << L"Axes present: X Y";
    if (joyCaps.wCaps & JOYCAPS_HASZ) wcout << L" Z";
    if (joyCaps.wCaps & JOYCAPS_HASR) wcout << L" R";
    if (joyCaps.wCaps & JOYCAPS_HASU) wcout << L" U";
    if (joyCaps.wCaps & JOYCAPS_HASV) wcout << L" V";
    wcout << endl;
    wcout << L"Max axes = " << joyCaps.wMaxAxes << L", max buttons = " << joyCaps.wMaxButtons << L", max period = " << joyCaps.wPeriodMax << L", min period = " << joyCaps.wPeriodMin << endl;
    wcout << L"X axis: max = " << joyCaps.wXmax << L", min = " << joyCaps.wXmin << endl;
    wcout << L"Y axis: max = " << joyCaps.wYmax << L", min = " << joyCaps.wYmin << endl;
    wcout << L"Z axis: max = " << joyCaps.wZmax << L", min = " << joyCaps.wZmin << endl;
    wcout << L"R axis: max = " << joyCaps.wRmax << L", min = " << joyCaps.wRmin << endl;
    wcout << L"U axis: max = " << joyCaps.wUmax << L", min = " << joyCaps.wUmin << endl;
    wcout << L"V axis: max = " << joyCaps.wVmax << L", min = " << joyCaps.wVmin << endl;
    wcout << L"Manufacturer ID = " << joyCaps.wMid << L", product ID = " << joyCaps.wPid << endl;
    wcout << L"Product name: " << joyCaps.szPname << endl;
    wcout << L"OEM driver name: " << joyCaps.szOEMVxD << endl;
    wcout << L"Registry key: " << joyCaps.szRegKey << endl;
    wcout << endl;


    ////////////////////////////////////
    ////////   Interactive Mode Preparation

    wcout << L"Preparing to launch interactive mode... ";
    wcout << L"DONE" << endl;
    wcout << L"Device state is updated twice per second." << endl;
    wcout << L"Quits automatically after 50 updates. To quit early, use CTRL+C." << endl;
    system("pause");
    system("cls");

    JOYINFOEX testData;
    ZeroMemory(&testData, sizeof(testData));
    testData.dwSize = sizeof(testData);
    testData.dwFlags = JOY_RETURNALL;

    for (unsigned int i = 0; i < 50; ++i)
    {
        system("cls");
        wcout << L"Update #" << (i+1) << endl;

        result = joyGetPosEx(devIdx, &testData);
        if (JOYERR_NOERROR != result)
        {
            wcout << L"Failed to retrieve device state." << endl;
            return 1;
        }

        wcout << L"Device presents " << joyCaps.wNumAxes << L" axes, " << joyCaps.wNumButtons << L" buttons, and " << (joyCaps.wCaps & JOYCAPS_HASPOV ? 1 : 0) << L" POV controllers." << endl;
        wcout << L"Max axes = " << joyCaps.wMaxAxes << L", max buttons = " << joyCaps.wMaxButtons << L", max period = " << joyCaps.wPeriodMax << L", min period = " << joyCaps.wPeriodMin << endl;
        wcout << L"X axis: max = " << joyCaps.wXmax << L", min = " << joyCaps.wXmin << endl;
        wcout << L"Y axis: max = " << joyCaps.wYmax << L", min = " << joyCaps.wYmin << endl;
        wcout << L"Z axis: max = " << joyCaps.wZmax << L", min = " << joyCaps.wZmin << endl;
        wcout << L"R axis: max = " << joyCaps.wRmax << L", min = " << joyCaps.wRmin << endl;
        wcout << L"U axis: max = " << joyCaps.wUmax << L", min = " << joyCaps.wUmin << endl;
        wcout << L"V axis: max = " << joyCaps.wVmax << L", min = " << joyCaps.wVmin << endl;

        wcout << endl;

        wcout << L"Device state:" << endl;

        wcout << endl;

        wcout << L"   X Axis  = " << testData.dwXpos << endl;
        wcout << L"   Y Axis  = " << testData.dwYpos << endl;
        wcout << L"   Z Axis  = " << testData.dwZpos << endl;

        wcout << endl;

        wcout << L"   R Axis  = " << testData.dwRpos << endl;
        wcout << L"   U Axis  = " << testData.dwUpos << endl;
        wcout << L"   V Axis  = " << testData.dwVpos << endl;

        wcout << endl;

        wcout << L"   Dpad    = " << testData.dwPOV << endl;

        wcout << endl;

        wcout << L"   Buttons pressed:";
        for (DWORD i = 0; i < joyCaps.wMaxButtons; ++i)
        {
            if (testData.dwButtons & (1 << i))
                wcout << L" " << (i + 1);
        }

        Sleep(500);
    }


    ////////////////////////////////////
    ////////   Cleanup and Exit

    wcout << L"\nExiting." << endl;
    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    int result = RunTestApp(argc, argv);
    system("pause");
    return result;
}
