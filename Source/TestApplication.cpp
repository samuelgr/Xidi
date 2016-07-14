/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * TestApplication.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Dinput8ExportApi.h"
#include "Dinput8ImportApi.h"
#include "XinputControllerIdentification.h"

#include <cstdlib>
#include <iostream>

using namespace XinputControllerDirectInput;
using namespace std;


// -------- MACROS --------------------------------------------------------- //

// Helper for iostream input and output when using unicode.
#ifdef UNICODE
#define terr                                    wcerr
#define tin                                     wcin
#define tout                                    wcout
#else
#define terr                                    cerr
#define tin                                     cin
#define tout                                    tout
#endif


// -------- LOCALS --------------------------------------------------------- //

// Holds the instance GUID of the device to test. Filled during enumeration.
static GUID instanceGuidToTest;

// Specifies if a device to test was successfully enumerated.
static BOOL flagInstanceGuidToTestFound = FALSE;

// A test value for pointer passing.
// Used to verify that application-specific callback parameters are passed successfully.
static const DWORD testValue = 0xfeedf00d;

// A test counter used while testing.
static DWORD testCounter = 0;

// Flag that specifies whether or not a callback is expected to be executed.
// Used to verify that callbacks are invoked only the number of times required, no more and no less.
static BOOL flagCallbackExpected = FALSE;


// -------- HELPERS -------------------------------------------------------- //

LPTSTR DirectInputDeviceTypeToString(BYTE type)
{
    switch (type)
    {
    case DI8DEVTYPE_DEVICE:
        return _T("DEVICE");

    case DI8DEVTYPE_MOUSE:
        return _T("MOUSE");

    case DI8DEVTYPE_KEYBOARD:
        return _T("KEYBOARD");

    case DI8DEVTYPE_JOYSTICK:
        return _T("JOYSTICK");

    case DI8DEVTYPE_GAMEPAD:
        return _T("GAMEPAD");

    case DI8DEVTYPE_DRIVING:
        return _T("DRIVING");

    case DI8DEVTYPE_FLIGHT:
        return _T("FLIGHT");

    case DI8DEVTYPE_1STPERSON:
        return _T("1STPERSON");

    case DI8DEVTYPE_DEVICECTRL:
        return _T("DEVICECTRL");

    case DI8DEVTYPE_SCREENPOINTER:
        return _T("SCREENPOINTER");

    case DI8DEVTYPE_REMOTE:
        return _T("REMOTE");

    case DI8DEVTYPE_SUPPLEMENTAL:
        return _T("SUPPLEMENTAL");

    default:
        return _T("UNKNOWN");
    }
}

LPTSTR DirectInputAxisTypeToString(REFGUID axisTypeGUID)
{
    if (axisTypeGUID == GUID_XAxis)
        return _T("X");
    
    if (axisTypeGUID == GUID_YAxis)
        return _T("Y");

    if (axisTypeGUID == GUID_ZAxis)
        return _T("Z");

    if (axisTypeGUID == GUID_RxAxis)
        return _T("RotX");

    if (axisTypeGUID == GUID_RyAxis)
        return _T("RotY");

    if (axisTypeGUID == GUID_RzAxis)
        return _T("RotZ");
    
    return NULL;
}


// -------- CALLBACKS ------------------------------------------------------ //

// Callback for enumerating DirectInput devices via the IDirectInput8 interface.
BOOL STDMETHODCALLTYPE EnumDevicesTestCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    tout << _T("    ");
    
    if (*testValuePtr != testValue)
        tout << _T("[pvRef fail] ");

    if (!flagCallbackExpected)
        tout << _T("[flagCallbackExpected fail] ");
    
    tout << _T("Found ") << DirectInputDeviceTypeToString(GET_DIDEVICE_TYPE(lpddi->dwDevType)) << ": " << lpddi->tszProductName;

    if (DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(lpddi->dwDevType) && XinputControllerIdentification::IsControllerTypeKnown(lpddi->guidProduct))
    {
        instanceGuidToTest = lpddi->guidInstance;
        flagInstanceGuidToTestFound = TRUE;
        flagCallbackExpected = FALSE;

        tout << _T(", supported") << endl;
    }
    else
        tout << endl;
    
    return flagCallbackExpected ? DIENUM_CONTINUE : DIENUM_STOP;
}

// Callback for enumerating DirectInput device axes via the IDirectInputDevice8 interface.
BOOL STDMETHODCALLTYPE EnumObjectsAxesTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    tout << _T("    ");

    if (*testValuePtr != testValue)
        tout << _T("[pvRef fail] ");
    if (DIDFT_GETTYPE(lpddoi->dwType) != DIDFT_ABSAXIS)
        tout << _T("[dwType fail] ");

    LPTSTR axisString = DirectInputAxisTypeToString(lpddoi->guidType);
    if (NULL == axisString)
        tout << _T("[guidType fail] ");
    
    tout << _T("Instance ") << DIDFT_GETINSTANCE(lpddoi->dwType) << _T(" @") << lpddoi->dwOfs << _T(": ");
    if (NULL == axisString)
        tout << _T("UNKNOWN") << endl;
    else
        tout << axisString << endl;

    testCounter += 1;
    return DIENUM_CONTINUE;
}

// Callback for enumerating DirectInput device buttons via the IDirectInputDevice8 interface.
BOOL STDMETHODCALLTYPE EnumObjectsButtonsTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    tout << _T("    ");

    if (*testValuePtr != testValue)
        tout << _T("[pvRef fail] ");
    if (DIDFT_GETTYPE(lpddoi->dwType) != DIDFT_PSHBUTTON)
        tout << _T("[dwType fail] ");
    if (lpddoi->guidType != GUID_Button)
        tout << _T("[guidType fail] ");

    tout << _T("Instance ") << DIDFT_GETINSTANCE(lpddoi->dwType) << _T(" @") << lpddoi->dwOfs << endl;

    testCounter += 1;
    return DIENUM_CONTINUE;
}

// Callback for enumerating DirectInput device POVs via the IDirectInputDevice8 interface.
BOOL STDMETHODCALLTYPE EnumObjectsPovTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    tout << _T("    ");

    if (*testValuePtr != testValue)
        tout << _T("[pvRef fail] ");
    if (DIDFT_GETTYPE(lpddoi->dwType) != DIDFT_POV)
        tout << _T("[dwType fail] ");
    if (lpddoi->guidType != GUID_POV)
        tout << _T("[guidType fail] ");

    tout << _T("Instance ") << DIDFT_GETINSTANCE(lpddoi->dwType) << _T(" @") << lpddoi->dwOfs << endl;

    testCounter += 1;
    return DIENUM_CONTINUE;
}

// Callback for enumerating DirectInput device objects via the IDirectInputDevice8 interface.
BOOL STDMETHODCALLTYPE EnumObjectsOverallTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    if (*testValuePtr != testValue)
        tout << _T("[pvRef fail] ");

    testCounter += 1;
    return DIENUM_CONTINUE;
}


// -------- FUNCTIONS ------------------------------------------------------ //

// Runs the test application. Effectively acts as its entry point.
int RunTestApp(int argc, char* argv[])
{
    HRESULT result;
    IDirectInput8* directInputIface;
    IDirectInputDevice8* directInputDeviceIface;
    
    
    ////////////////////////////////////
    ////////   Initialization
    
    // Initialize the imported DirectInput8 API.
    if (S_OK != Dinput8ImportApi::Initialize())
    {
        terr << _T("Unable to initialize DirectInput8 API.") << endl;
        return -1;
    }

    // Create the main interface to DirectInput8.
    result = Dinput8ExportDirectInput8Create(GetModuleHandle(NULL), 0x0800, IID_IDirectInput8, (LPVOID*)&directInputIface, NULL);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain IDirectInput8 interface pointer: code ") << result << _T(".") << endl;
        return -1;
    }

    
    ////////////////////////////////////
    ////////   Enumeration
    
    // Enumerate all devices attached to the system.
    flagCallbackExpected = TRUE;
    
    tout << _T("Begin IDirectInput8->EnumDevices") << endl;
    
    result = directInputIface->EnumDevices(DI8DEVCLASS_ALL, &EnumDevicesTestCallback, (LPVOID)&testValue, DIEDFL_ATTACHEDONLY);
    if (DI_OK != result)
    {
        terr << _T("Unable to enumerate attached devices: code ") << result << _T(".") << endl;
        return -1;
    }
    
    // Test that the callback was invoked the required number of times.
    if (flagCallbackExpected && flagInstanceGuidToTestFound)
        tout << _T("FAIL: IDirectInput8->EnumDevices callback test") << endl;
    else
        tout << _T("PASS: IDirectInput8->EnumDevices callback test") << endl;

    tout << _T("End IDirectInput8->EnumDevices") << endl << endl;

    // Verify that a supported device was found
    if (!flagInstanceGuidToTestFound)
    {
        tout << _T("No supported devices found. Connect one and try again.") << endl;
        return -1;
    }


    ////////////////////////////////////
    ////////   Device Creation

    // Obtain a pointer to the interface of the device.
    result = directInputIface->CreateDevice(instanceGuidToTest, &directInputDeviceIface, NULL);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain IDirectInputDevice8 interface pointer: code ") << result << _T(".") << endl;
        return -1;
    }


    ////////////////////////////////////
    ////////   Device Capabilities

    // Attempt to get the capabilities of the device.
    DIDEVCAPS deviceCapabilities;
    deviceCapabilities.dwSize = sizeof(deviceCapabilities);

    result = directInputDeviceIface->GetCapabilities(&deviceCapabilities);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain get device capabilities: code ") << result << _T(".") << endl;
        return -1;
    }

    terr << _T("Device presents ") << deviceCapabilities.dwAxes << _T(" axes, ") << deviceCapabilities.dwButtons << _T(" buttons, and ") << deviceCapabilities.dwPOVs << _T(" POV controllers.") << endl << endl;
    

    ////////////////////////////////////
    ////////   Device Object Enumeration

    tout << _T("Begin IDirectInputDevice8->EnumObjects") << endl;
    
    // Attempt to enumerate axes.
    tout << _T("  Axes...") << endl;
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsAxesTestCallback, (LPVOID)&testValue, DIDFT_AXIS);
    
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain get device axes: code ") << result << _T(".") << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwAxes))
        tout << _T("PASS: IDirectInputDevice8->EnumObjects axis consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice8->EnumObjects axis consistency check.") << endl;

    // Attempt to enumerate buttons.
    tout << _T("  Buttons...") << endl;
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsButtonsTestCallback, (LPVOID)&testValue, DIDFT_BUTTON);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain get device buttons: code ") << result << _T(".") << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwButtons))
        tout << _T("PASS: IDirectInputDevice8->EnumObjects button consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice8->EnumObjects button consistency check.") << endl;

    // Attempt to enumerate POVs.
    tout << _T("  POVs...") << endl;
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsPovTestCallback, (LPVOID)&testValue, DIDFT_POV);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain get device POVs: code ") << result << _T(".") << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwPOVs))
        tout << _T("PASS: IDirectInputDevice8->EnumObjects POV consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice8->EnumObjects POV consistency check.") << endl;

    // Attempt to enumerate everything to verify consistency.
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsOverallTestCallback, (LPVOID)&testValue, DIDFT_ALL);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain get device objects: code ") << result << _T(".") << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwAxes + deviceCapabilities.dwButtons + deviceCapabilities.dwPOVs))
        tout << _T("PASS: IDirectInputDevice8->EnumObjects overall consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice8->EnumObjects overall consistency check.") << endl;

    // Finished enumerating objects.
    tout << _T("End IDirectInputDevice8->EnumObjects") << endl << endl;


    ////////////////////////////////////
    ////////   Device Object Information

    tout << _T("Begin IDirectInputDevice8->GetObjectInfo") << endl;

    // Attempt to iterate over axes.
    tout << _T("  Axes...") << endl;
    for (DWORD i = 0; i < deviceCapabilities.dwAxes; ++i)
    {
        tout << _T("    ") << i << _T(": ");

        DIDEVICEOBJECTINSTANCE objectInfo;
        objectInfo.dwSize = sizeof(objectInfo);

        result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i), DIPH_BYID);
        if (DI_OK != result)
        {
            tout << _T("FAILED") << endl;
            continue;
        }

        tout << _T("OK: ") << objectInfo.tszName << _T(" (") << DirectInputAxisTypeToString(objectInfo.guidType) << _T(" @") << objectInfo.dwOfs << _T(")") << endl;
    }

    // Attempt to iterate over buttons.
    tout << _T("  Buttons...") << endl;
    for (DWORD i = 0; i < deviceCapabilities.dwButtons; ++i)
    {
        tout << _T("    ") << i << _T(": ");

        DIDEVICEOBJECTINSTANCE objectInfo;
        objectInfo.dwSize = sizeof(objectInfo);

        result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(i), DIPH_BYID);
        if (DI_OK != result)
        {
            tout << _T("FAILED") << endl;
            continue;
        }

        tout << _T("OK: ") << objectInfo.tszName << _T(" (@") << objectInfo.dwOfs << _T(")") << endl;
    }

    // Attempt to iterate over POVs.
    tout << _T("  POVs...") << endl;
    for (DWORD i = 0; i < deviceCapabilities.dwPOVs; ++i)
    {
        tout << _T("    ") << i << _T(": ");

        DIDEVICEOBJECTINSTANCE objectInfo;
        objectInfo.dwSize = sizeof(objectInfo);

        result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_POV | DIDFT_MAKEINSTANCE(i), DIPH_BYID);
        if (DI_OK != result)
        {
            tout << _T("FAILED") << endl;
            continue;
        }

        tout << _T("OK: ") << objectInfo.tszName << _T(" (@") << objectInfo.dwOfs << _T(")") << endl;
    }

    // Attempt to request information on objects that should not be available or are otherwise invalid requests.
    DIDEVICEOBJECTINSTANCE objectInfo;
    ZeroMemory(&objectInfo, sizeof(objectInfo));

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(0), DIPH_BYID);
    if (DI_OK == result)
        tout << _T("FAIL: Invalid DIDEVICEOBJECTINSTANCE dwSize test.") << endl;
    else
        tout << _T("PASS: Invalid DIDEVICEOBJECTINSTANCE dwSize test.") << endl;

    objectInfo.dwSize = sizeof(objectInfo);
    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(deviceCapabilities.dwAxes), DIPH_BYID);
    if (DI_OK == result)
        tout << _T("FAIL: Invalid axis object info test.") << endl;
    else
        tout << _T("PASS: Invalid axis object info test.") << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(deviceCapabilities.dwButtons), DIPH_BYID);
    if (DI_OK == result)
        tout << _T("FAIL: Invalid button object info test.") << endl;
    else
        tout << _T("PASS: Invalid button object info test.") << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_POV | DIDFT_MAKEINSTANCE(deviceCapabilities.dwPOVs), DIPH_BYID);
    if (DI_OK == result)
        tout << _T("FAIL: Invalid POV object info test.") << endl;
    else
        tout << _T("PASS: Invalid POV object info test.") << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, 0, DIPH_BYOFFSET);
    if (DI_OK == result)
        tout << _T("FAIL: Uninitialized data format object info test.") << endl;
    else
        tout << _T("PASS: Uninitialized data format object info test.") << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, 0, DIPH_BYUSAGE);
    if (DI_OK == result)
        tout << _T("FAIL: Unsupported request type object info test.") << endl;
    else
        tout << _T("PASS: Unsupported request type object info test.") << endl;
    
    // Finished checking objects.
    tout << _T("End IDirectInputDevice8->GetObjectInfo") << endl << endl;
    
    
    ////////////////////////////////////
    ////////   Cleanup and Exit
    
    directInputDeviceIface->Release();
    directInputIface->Release();
    
    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    int testAppResult = RunTestApp(argc, argv);

    system("pause");
    return testAppResult;
}
