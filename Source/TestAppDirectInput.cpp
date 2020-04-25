/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file TestAppDirectInput.cpp
 *   Entry point and other implementation for a simple console application
 *   for testing the functionality of this library via DirectInput.
 *****************************************************************************/

#include <initguid.h>
#include <dinput.h>

#include "ControllerIdentification.h"
#include "Mapper/Base.h"

#include <iostream>

using namespace std;
using namespace Xidi;


// -------- TYPE DEFINITIONS ----------------------------------------------- //

struct SInteractiveTestData
{
    LONG axisX;
    LONG axisY;
    LONG axisZ;
    LONG axisRx;
    LONG axisRy;
    LONG axisRz;
    LONG povs[4];
    BYTE buttons[16];
};


// -------- MACROS --------------------------------------------------------- //

// Routes calls to exported methods based on the DirectInput version.
#if DIRECTINPUT_VERSION >= 0x0800
#define ExportedDirectInputCreateMethod         DirectInput8Create
#define Use_IID_IDirectInput                    IID_IDirectInput8
#else
#define ExportedDirectInputCreateMethod         DirectInputCreateEx
#define Use_IID_IDirectInput                    IID_IDirectInput7
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

// Object format definition for interactive test mode.
static DIOBJECTDATAFORMAT objectFormats[] = {
    { &GUID_XAxis, offsetof(SInteractiveTestData, axisX), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_YAxis, offsetof(SInteractiveTestData, axisY), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_ZAxis, offsetof(SInteractiveTestData, axisZ), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RxAxis, offsetof(SInteractiveTestData, axisRx), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RyAxis, offsetof(SInteractiveTestData, axisRy), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RzAxis, offsetof(SInteractiveTestData, axisRz), DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, offsetof(SInteractiveTestData, povs) + (0 * sizeof(SInteractiveTestData::povs[0])), DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, offsetof(SInteractiveTestData, povs) + (1 * sizeof(SInteractiveTestData::povs[0])), DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, offsetof(SInteractiveTestData, povs) + (2 * sizeof(SInteractiveTestData::povs[0])), DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, offsetof(SInteractiveTestData, povs) + (3 * sizeof(SInteractiveTestData::povs[0])), DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (0 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (1 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (2 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (3 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (4 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (5 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (6 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (7 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (8 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (9 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (10 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (11 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (12 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (13 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (14 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Button, offsetof(SInteractiveTestData, buttons) + (15 * sizeof(SInteractiveTestData::buttons[0])), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
};

// Overall format definition for interactive test mode.
static const DIDATAFORMAT dataFormat = {sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), 0, sizeof(SInteractiveTestData), _countof(objectFormats), objectFormats};


// -------- HELPERS -------------------------------------------------------- //

#if DIRECTINPUT_VERSION >= 0x0800
LPCWSTR DirectInputDeviceTypeToString(BYTE type)
{
    switch (type)
    {
    case DI8DEVTYPE_DEVICE:
        return L"DEVICE";

    case DI8DEVTYPE_MOUSE:
        return L"MOUSE";

    case DI8DEVTYPE_KEYBOARD:
        return L"KEYBOARD";

    case DI8DEVTYPE_JOYSTICK:
        return L"JOYSTICK";

    case DI8DEVTYPE_GAMEPAD:
        return L"GAMEPAD";

    case DI8DEVTYPE_DRIVING:
        return L"DRIVING";

    case DI8DEVTYPE_FLIGHT:
        return L"FLIGHT";

    case DI8DEVTYPE_1STPERSON:
        return L"1STPERSON";

    case DI8DEVTYPE_DEVICECTRL:
        return L"DEVICECTRL";

    case DI8DEVTYPE_SCREENPOINTER:
        return L"SCREENPOINTER";

    case DI8DEVTYPE_REMOTE:
        return L"REMOTE";

    case DI8DEVTYPE_SUPPLEMENTAL:
        return L"SUPPLEMENTAL";

    default:
        return L"UNKNOWN";
    }
}
#else
LPCWSTR DirectInputDeviceTypeToString(BYTE type)
{
    switch (type)
    {
    case DIDEVTYPE_DEVICE:
        return L"DEVICE";

    case DIDEVTYPE_MOUSE:
        return L"MOUSE";

    case DIDEVTYPE_KEYBOARD:
        return L"KEYBOARD";

    case DIDEVTYPE_JOYSTICK:
        return L"JOYSTICK";

    default:
        return L"UNKNOWN";
    }
}
#endif

LPCWSTR DirectInputAxisTypeToString(REFGUID axisTypeGUID)
{
    if (axisTypeGUID == GUID_XAxis)
        return L"X";

    if (axisTypeGUID == GUID_YAxis)
        return L"Y";

    if (axisTypeGUID == GUID_ZAxis)
        return L"Z";

    if (axisTypeGUID == GUID_RxAxis)
        return L"RotX";

    if (axisTypeGUID == GUID_RyAxis)
        return L"RotY";

    if (axisTypeGUID == GUID_RzAxis)
        return L"RotZ";

    return nullptr;
}


// -------- CALLBACKS ------------------------------------------------------ //

// Callback for enumerating DirectInput devices via the IDirectInput interface.
BOOL STDMETHODCALLTYPE EnumDevicesTestCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;
    GUID xProductGUID = ControllerIdentification::kXInputProductGUID;

    wcout << L"    ";

    if (*testValuePtr != testValue)
        wcout << L"[pvRef fail] ";

    if (!flagCallbackExpected)
        wcout << L"[flagCallbackExpected fail] ";

    wcout << L"Found " << DirectInputDeviceTypeToString(GET_DIDEVICE_TYPE(lpddi->dwDevType)) << ": " << lpddi->tszProductName;

#if DIRECTINPUT_VERSION >= 0x0800
    if (DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(lpddi->dwDevType) && (lpddi->guidProduct == xProductGUID))
#else
    if (DIDEVTYPE_JOYSTICK == GET_DIDEVICE_TYPE(lpddi->dwDevType) && (lpddi->guidProduct == xProductGUID))
#endif
    {
        instanceGuidToTest = lpddi->guidInstance;
        flagInstanceGuidToTestFound = TRUE;
        flagCallbackExpected = FALSE;

        wcout << L", supported" << endl;
    }
    else
        wcout << endl;

    return flagCallbackExpected ? DIENUM_CONTINUE : DIENUM_STOP;
}

// Callback for enumerating DirectInput device axes via the IDirectInputDevice interface.
BOOL STDMETHODCALLTYPE EnumObjectsAxesTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    wcout << L"    ";

    if (*testValuePtr != testValue)
        wcout << L"[pvRef fail] ";
    if (DIDFT_GETTYPE(lpddoi->dwType) != DIDFT_ABSAXIS)
        wcout << L"[dwType fail] ";

    LPCWSTR axisString = DirectInputAxisTypeToString(lpddoi->guidType);
    if (nullptr == axisString)
        wcout << L"[guidType fail] ";

    wcout << L"Instance " << DIDFT_GETINSTANCE(lpddoi->dwType) << L" @" << lpddoi->dwOfs << L": ";
    if (nullptr == axisString)
        wcout << L"UNKNOWN" << endl;
    else
        wcout << axisString << endl;

    testCounter += 1;
    return DIENUM_CONTINUE;
}

// Callback for enumerating DirectInput device buttons via the IDirectInputDevice interface.
BOOL STDMETHODCALLTYPE EnumObjectsButtonsTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    wcout << L"    ";

    if (*testValuePtr != testValue)
        wcout << L"[pvRef fail] ";
    if (DIDFT_GETTYPE(lpddoi->dwType) != DIDFT_PSHBUTTON)
        wcout << L"[dwType fail] ";
    if (lpddoi->guidType != GUID_Button)
        wcout << L"[guidType fail] ";

    wcout << L"Instance " << DIDFT_GETINSTANCE(lpddoi->dwType) << L" @" << lpddoi->dwOfs << endl;

    testCounter += 1;
    return DIENUM_CONTINUE;
}

// Callback for enumerating DirectInput device POVs via the IDirectInputDevice interface.
BOOL STDMETHODCALLTYPE EnumObjectsPovTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    wcout << L"    ";

    if (*testValuePtr != testValue)
        wcout << L"[pvRef fail] ";
    if (DIDFT_GETTYPE(lpddoi->dwType) != DIDFT_POV)
        wcout << L"[dwType fail] ";
    if (lpddoi->guidType != GUID_POV)
        wcout << L"[guidType fail] ";

    wcout << L"Instance " << DIDFT_GETINSTANCE(lpddoi->dwType) << L" @" << lpddoi->dwOfs << endl;

    testCounter += 1;
    return DIENUM_CONTINUE;
}

// Callback for enumerating DirectInput device objects via the IDirectInputDevice interface.
BOOL STDMETHODCALLTYPE EnumObjectsOverallTestCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;

    if (*testValuePtr != testValue)
        wcout << L"[pvRef fail] ";

    testCounter += 1;
    return DIENUM_CONTINUE;
}


// -------- FUNCTIONS ------------------------------------------------------ //

// Runs the test application. Effectively acts as its entry point.
int RunTestApp(int argc, char* argv[])
{
    HRESULT result;
    DWORD numErrors;
    LatestIDirectInput* directInputIface;
    LatestIDirectInputDevice* directInputDeviceIface;


    ////////////////////////////////////
    ////////   Initialization

    // Create the main interface to DirectInput.
    result = ExportedDirectInputCreateMethod(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, Use_IID_IDirectInput, (LPVOID*)&directInputIface, nullptr);
    if (DI_OK != result)
    {
        wcerr << L"Unable to obtain IDirectInput interface pointer: code " << result << L"." << endl;
        return -1;
    }


    ////////////////////////////////////
    ////////   Enumeration

    // Enumerate all devices attached to the system.
    flagCallbackExpected = TRUE;

    wcout << L"Begin IDirectInput->EnumDevices" << endl;

#if DIRECTINPUT_VERSION >= 0x0800
    result = directInputIface->EnumDevices(DI8DEVCLASS_ALL, &EnumDevicesTestCallback, (LPVOID)&testValue, DIEDFL_ATTACHEDONLY);
#else
    result = directInputIface->EnumDevices(0, &EnumDevicesTestCallback, (LPVOID)&testValue, DIEDFL_ATTACHEDONLY);
#endif
    if (DI_OK != result)
    {
        wcerr << L"Unable to enumerate attached devices: code " << result << L"." << endl;
        return -1;
    }

    // Test that the callback was invoked the required number of times.
    if (flagCallbackExpected && flagInstanceGuidToTestFound)
        wcout << L"FAIL: IDirectInput->EnumDevices callback test" << endl;
    else
        wcout << L"PASS: IDirectInput->EnumDevices callback test" << endl;

    wcout << L"End IDirectInput->EnumDevices" << endl << endl;

    // Verify that a supported device was found
    if (!flagInstanceGuidToTestFound)
    {
        wcout << L"No supported devices found. Connect one and try again." << endl;
        return -1;
    }


    ////////////////////////////////////
    ////////   Device Creation

    // Obtain a pointer to the interface of the device.
    result = directInputIface->CreateDevice(instanceGuidToTest, (EarliestIDirectInputDevice**)&directInputDeviceIface, nullptr);

    if (DI_OK != result)
    {
        wcerr << L"Unable to obtain IDirectInputDevice interface pointer: code " << result << L"." << endl;
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
        wcerr << L"Unable to obtain get device capabilities: code " << result << L"." << endl;
        return -1;
    }

    wcout << L"Device presents " << deviceCapabilities.dwAxes << L" axes, " << deviceCapabilities.dwButtons << L" buttons, and " << deviceCapabilities.dwPOVs << L" POV controllers." << endl << endl;


    ////////////////////////////////////
    ////////   Device Object Enumeration

    wcout << L"Begin IDirectInputDevice->EnumObjects" << endl;

    // Attempt to enumerate axes.
    wcout << L"  Axes..." << endl;
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsAxesTestCallback, (LPVOID)&testValue, DIDFT_AXIS);

    if (DI_OK != result)
    {
        wcerr << L"Unable to obtain get device axes: code " << result << L"." << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwAxes))
        wcout << L"PASS: IDirectInputDevice->EnumObjects axis consistency check." << endl;
    else
        wcout << L"FAIL: IDirectInputDevice->EnumObjects axis consistency check." << endl;

    // Attempt to enumerate buttons.
    wcout << L"  Buttons..." << endl;
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsButtonsTestCallback, (LPVOID)&testValue, DIDFT_BUTTON);
    if (DI_OK != result)
    {
        wcerr << L"Unable to obtain get device buttons: code " << result << L"." << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwButtons))
        wcout << L"PASS: IDirectInputDevice->EnumObjects button consistency check." << endl;
    else
        wcout << L"FAIL: IDirectInputDevice->EnumObjects button consistency check." << endl;

    // Attempt to enumerate POVs.
    wcout << L"  POVs..." << endl;
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsPovTestCallback, (LPVOID)&testValue, DIDFT_POV);
    if (DI_OK != result)
    {
        wcerr << L"Unable to obtain get device POVs: code " << result << L"." << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwPOVs))
        wcout << L"PASS: IDirectInputDevice->EnumObjects POV consistency check." << endl;
    else
        wcout << L"FAIL: IDirectInputDevice->EnumObjects POV consistency check." << endl;

    // Attempt to enumerate everything to verify consistency.
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsOverallTestCallback, (LPVOID)&testValue, DIDFT_ALL);
    if (DI_OK != result)
    {
        wcerr << L"Unable to obtain get device objects: code " << result << L"." << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwAxes + deviceCapabilities.dwButtons + deviceCapabilities.dwPOVs))
        wcout << L"PASS: IDirectInputDevice->EnumObjects overall consistency check." << endl;
    else
        wcout << L"FAIL: IDirectInputDevice->EnumObjects overall consistency check." << endl;

    // Finished enumerating objects.
    wcout << L"End IDirectInputDevice->EnumObjects" << endl << endl;


    ////////////////////////////////////
    ////////   Device Object Information

    wcout << L"Begin IDirectInputDevice->GetObjectInfo" << endl;

    // Attempt to iterate over axes.
    wcout << L"  Axes..." << endl;
    for (DWORD i = 0; i < deviceCapabilities.dwAxes; ++i)
    {
        wcout << L"    " << i << L": ";

        DIDEVICEOBJECTINSTANCE objectInfo;
        objectInfo.dwSize = sizeof(objectInfo);

        result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i), DIPH_BYID);
        if (DI_OK != result)
        {
            wcout << L"FAILED" << endl;
            continue;
        }

        wcout << L"OK: " << objectInfo.tszName << L" (" << DirectInputAxisTypeToString(objectInfo.guidType) << L" @" << objectInfo.dwOfs << L")" << endl;
    }

    // Attempt to iterate over buttons.
    wcout << L"  Buttons..." << endl;
    for (DWORD i = 0; i < deviceCapabilities.dwButtons; ++i)
    {
        wcout << L"    " << i << L": ";

        DIDEVICEOBJECTINSTANCE objectInfo;
        objectInfo.dwSize = sizeof(objectInfo);

        result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(i), DIPH_BYID);
        if (DI_OK != result)
        {
            wcout << L"FAILED" << endl;
            continue;
        }

        wcout << L"OK: " << objectInfo.tszName << L" (@" << objectInfo.dwOfs << L")" << endl;
    }

    // Attempt to iterate over POVs.
    wcout << L"  POVs..." << endl;
    for (DWORD i = 0; i < deviceCapabilities.dwPOVs; ++i)
    {
        wcout << L"    " << i << L": ";

        DIDEVICEOBJECTINSTANCE objectInfo;
        objectInfo.dwSize = sizeof(objectInfo);

        result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_POV | DIDFT_MAKEINSTANCE(i), DIPH_BYID);
        if (DI_OK != result)
        {
            wcout << L"FAILED" << endl;
            continue;
        }

        wcout << L"OK: " << objectInfo.tszName << L" (@" << objectInfo.dwOfs << L")" << endl;
    }

    // Attempt to request information on objects that should not be available or are otherwise invalid requests.
    DIDEVICEOBJECTINSTANCE objectInfo;
    ZeroMemory(&objectInfo, sizeof(objectInfo));

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(0), DIPH_BYID);
    if (DI_OK == result)
        wcout << L"FAIL: Invalid DIDEVICEOBJECTINSTANCE dwSize test." << endl;
    else
        wcout << L"PASS: Invalid DIDEVICEOBJECTINSTANCE dwSize test." << endl;

    objectInfo.dwSize = sizeof(objectInfo);
    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(deviceCapabilities.dwAxes), DIPH_BYID);
    if (DI_OK == result)
        wcout << L"FAIL: Invalid axis object info test." << endl;
    else
        wcout << L"PASS: Invalid axis object info test." << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(deviceCapabilities.dwButtons), DIPH_BYID);
    if (DI_OK == result)
        wcout << L"FAIL: Invalid button object info test." << endl;
    else
        wcout << L"PASS: Invalid button object info test." << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, DIDFT_POV | DIDFT_MAKEINSTANCE(deviceCapabilities.dwPOVs), DIPH_BYID);
    if (DI_OK == result)
        wcout << L"FAIL: Invalid POV object info test." << endl;
    else
        wcout << L"PASS: Invalid POV object info test." << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, 0, DIPH_BYOFFSET);
    if (DI_OK == result)
        wcout << L"FAIL: Uninitialized data format object info test." << endl;
    else
        wcout << L"PASS: Uninitialized data format object info test." << endl;

    result = directInputDeviceIface->GetObjectInfo(&objectInfo, 0, DIPH_BYUSAGE);
    if (DI_OK == result)
        wcout << L"FAIL: Unsupported request type object info test." << endl;
    else
        wcout << L"PASS: Unsupported request type object info test." << endl;

    // Finished checking objects.
    wcout << L"End IDirectInputDevice->GetObjectInfo" << endl << endl;


    ////////////////////////////////////
    ////////   Device Properties

    wcout << L"Begin IDirectInputDevice->[Set|Get]Property" << endl;

    DIPROPRANGE rangeTest;
    DIPROPDWORD deadzoneTest;
    DIPROPDWORD bufferSize;

    ZeroMemory(&rangeTest, sizeof(rangeTest));
    ZeroMemory(&deadzoneTest, sizeof(deadzoneTest));
    ZeroMemory(&bufferSize, sizeof(bufferSize));

    // First, test range without setting header size properly but setting everything else.
    rangeTest.diph.dwHow = DIPH_BYID;
    rangeTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(0);
    rangeTest.diph.dwSize = sizeof(rangeTest);
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result || Mapper::Base::kDefaultAxisRangeMax == rangeTest.lMax)
        wcout << L"FAIL: Invalid header size test." << endl;
    else
        wcout << L"PASS: Invalid header size test." << endl;

    // Same, but now set header size and not overall size
    rangeTest.diph.dwSize = 0;
    rangeTest.diph.dwHeaderSize = sizeof(rangeTest.diph);
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result || Mapper::Base::kDefaultAxisRangeMax == rangeTest.lMax)
        wcout << L"FAIL: Invalid structure size test." << endl;
    else
        wcout << L"PASS: Invalid structure size test." << endl;

    // Set sizes and expect to get default values back.
    rangeTest.diph.dwSize = sizeof(rangeTest);
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DI_OK != result || Mapper::Base::kDefaultAxisRangeMax != rangeTest.lMax || Mapper::Base::kDefaultAxisRangeMin != rangeTest.lMin)
        wcout << L"FAIL: Default range test." << endl;
    else
        wcout << L"PASS: Default range test." << endl;

    // Set an invalid range and expect it to be rejected.
    rangeTest.lMax = -1000;
    rangeTest.lMin = 1000;
    result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result)
        wcout << L"FAIL: Set invalid range test 1." << endl;
    else
        wcout << L"PASS: Set invalid range test 1." << endl;

    // Another invalid range to be rejected.
    rangeTest.lMax = 1000;
    rangeTest.lMin = 1000;
    result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result)
        wcout << L"FAIL: Set invalid range test 2." << endl;
    else
        wcout << L"PASS: Set invalid range test 2." << endl;

    // This range is valid and should be accepted.
    rangeTest.lMax = 1000;
    rangeTest.lMin = -1000;
    result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DI_OK != result)
        wcout << L"FAIL: Set valid range test." << endl;
    else
        wcout << L"PASS: Set valid range test." << endl;

    // Expect to read back that range.
    rangeTest.lMax = 0;
    rangeTest.lMin = 0;
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DI_OK != result || 1000 != rangeTest.lMax || -1000 != rangeTest.lMin)
        wcout << L"FAIL: Get valid range test." << endl;
    else
        wcout << L"PASS: Get valid range test." << endl;

    // Get a valid deadzone but targetting a button, should be rejected.
    deadzoneTest.diph.dwHow = DIPH_BYID;
    deadzoneTest.diph.dwObj = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(0);
    deadzoneTest.diph.dwSize = sizeof(deadzoneTest);
    deadzoneTest.diph.dwHeaderSize = sizeof(deadzoneTest.diph);
    deadzoneTest.dwData = 1000;
    result = directInputDeviceIface->GetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DIERR_UNSUPPORTED != result || 1000 != deadzoneTest.dwData)
        wcout << L"FAIL: Bad deadzone target test." << endl;
    else
        wcout << L"PASS: Bad deadzone target test." << endl;

    // Set an actual valid deadzone.
    deadzoneTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(0);
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result || 1000 != deadzoneTest.dwData)
        wcout << L"FAIL: Set valid deadzone test." << endl;
    else
        wcout << L"PASS: Set valid deadzone test." << endl;

    // Read it back.
    deadzoneTest.dwData = 1000000;
    result = directInputDeviceIface->GetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result || 1000 != deadzoneTest.dwData)
        wcout << L"FAIL: Get valid deadzone test." << endl;
    else
        wcout << L"PASS: Get valid deadzone test." << endl;

    // Make sure the scope was limited to just that axis and it didn't go elsewhere.
    numErrors = 0;
    for (WORD i = 1; i < deviceCapabilities.dwAxes; ++i)
    {
        deadzoneTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i);
        result = directInputDeviceIface->GetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
        if (DI_OK != result || 1000 == deadzoneTest.dwData)
            numErrors += 1;
    }
    if (0 != numErrors)
        wcout << L"FAIL: Single axis valid deadzone test." << endl;
    else
        wcout << L"PASS: Single axis valid deadzone test." << endl;

    // Write a deadzone out of range.
    deadzoneTest.dwData = Mapper::Base::kMaxAxisDeadzoneSaturation * 2;
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DIERR_INVALIDPARAM != result)
        wcout << L"FAIL: Set out-of-range deadzone test." << endl;
    else
        wcout << L"PASS: Set out-of-range deadzone test." << endl;

    // Write a deadzone for the whole device, but use an invalid "dwObj".
    deadzoneTest.dwData = 51;
    deadzoneTest.diph.dwHow = DIPH_DEVICE;
    deadzoneTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(1);
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DIERR_INVALIDPARAM != result)
        wcout << L"FAIL: Set invalid whole device deadzone test." << endl;
    else
        wcout << L"PASS: Set invalid whole device deadzone test." << endl;

    // Write a valid deadzone for the whole device.
    deadzoneTest.dwData = 54;
    deadzoneTest.diph.dwObj = 0;
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result)
        wcout << L"FAIL: Set valid whole device deadzone test." << endl;
    else
        wcout << L"PASS: Set valid whole device deadzone test." << endl;

    // Read back the deadzone from the whole device.
    numErrors = 0;
    deadzoneTest.diph.dwHow = DIPH_BYID;
    for (WORD i = 0; i < deviceCapabilities.dwAxes; ++i)
    {
        deadzoneTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i);
        result = directInputDeviceIface->GetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
        if (DI_OK != result || 54 != deadzoneTest.dwData)
            numErrors += 1;
    }
    if (0 != numErrors)
        wcout << L"FAIL: Whole device valid deadzone test." << endl;
    else
        wcout << L"PASS: Whole device valid deadzone test." << endl;

    wcout << L"End IDirectInputDevice->[Set|Get]Property" << endl << endl;

    // Set the input buffer size to something huge (1GB).
    bufferSize.diph.dwHow = DIPH_DEVICE;
    bufferSize.diph.dwObj = 0;
    bufferSize.diph.dwSize = sizeof(bufferSize);
    bufferSize.diph.dwHeaderSize = sizeof(bufferSize.diph);
    bufferSize.dwData = 1 * 1024 * 1024 * 1024;
    result = directInputDeviceIface->SetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph);
    if (DI_OK != result)
        wcout << L"FAIL: Set huge buffer size test." << endl;
    else
        wcout << L"PASS: Set huge buffer size test." << endl;

    // Read back the input buffer size and make sure the value set is the value obtained.
    bufferSize.dwData = 0;
    result = directInputDeviceIface->GetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph);
    if ((DI_OK != result) || (1 * 1024 * 1024 * 1024 != bufferSize.dwData))
        wcout << L"FAIL: Get huge buffer size test." << endl;
    else
        wcout << L"PASS: Get huge buffer size test." << endl;

    // Set the input buffer size to something reasonable (1kB).
    bufferSize.dwData = 1024;
    result = directInputDeviceIface->SetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph);
    if (DI_OK != result)
        wcout << L"FAIL: Set reasonable buffer size test." << endl;
    else
        wcout << L"PASS: Set reasonable buffer size test." << endl;

    // Expect to get that same value back.
    bufferSize.dwData = 0;
    result = directInputDeviceIface->GetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph);
    if ((DI_OK != result) || (1024 != bufferSize.dwData))
        wcout << L"FAIL: Get reasonable buffer size test." << endl;
    else
        wcout << L"PASS: Get reasonable buffer size test." << endl;


    ////////////////////////////////////
    ////////   Interactive Mode Preparation

    wcout << L"Preparing to launch interactive mode... ";


    // Set the input buffer size to 128kB to avoid possible overflows during interactive testing.
    bufferSize.dwData = 128 * 1024;
    result = directInputDeviceIface->SetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph);
    if (DI_OK != result)
    {
        wcout << L"FAILED" << endl << L"Unable to set input buffer size." << endl;
        return -1;
    }

    // Set deadzone and saturation for the whole device.
    deadzoneTest.dwData = 2500;
    deadzoneTest.diph.dwHow = DIPH_DEVICE;
    deadzoneTest.diph.dwObj = 0;
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result)
    {
        wcout << L"FAILED" << endl << L"Unable to set deadzone." << endl;
        return -1;
    }
    deadzoneTest.dwData = 7500;
    result = directInputDeviceIface->SetProperty(DIPROP_SATURATION, &deadzoneTest.diph);
    if (DI_OK != result)
    {
        wcout << L"FAILED" << endl << L"Unable to set saturation." << endl;
        return -1;
    }

    // Set range for all axes to between -100 and 100 for easy reading.
    rangeTest.diph.dwHeaderSize = sizeof(rangeTest.diph);
    rangeTest.diph.dwSize = sizeof(rangeTest);
    rangeTest.diph.dwHow = DIPH_BYID;
    rangeTest.lMax = 100;
    rangeTest.lMin = -100;

    for (DWORD i = 0; i < deviceCapabilities.dwAxes; ++i)
    {
        rangeTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i);
        result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
        if (DI_OK != result)
        {
            wcout << L"FAILED" << endl << L"Unable to set range." << endl;
            return -1;
        }
    }

    // Set the application's data format to that specified at the top of this file.
    result = directInputDeviceIface->SetDataFormat(&dataFormat);
    if (DI_OK != result)
    {
        wcout << L"FAILED" << endl << L"Unable to set data format." << endl;
        return -1;
    }

    // Acquire the device.
    result = directInputDeviceIface->Acquire();
    if (DI_OK != result)
    {
        wcout << L"FAILED" << endl << L"Unable to acquire device." << endl;
        return -1;
    }

    // Initialize test data structures.
    SInteractiveTestData testData;
    SInteractiveTestData testBufferedData;
    DIDEVICEOBJECTDATA bufferedData[1024];
    DWORD bufferedDataCount = _countof(bufferedData);
    ZeroMemory(&testData, sizeof(testData));
    ZeroMemory(&testBufferedData, sizeof(testBufferedData));
    ZeroMemory(bufferedData, sizeof(bufferedData));

    // Perform an initial poll to set the correct state for both buffers.
    result = directInputDeviceIface->Poll();
    if (DI_OK != result)
    {
        wcout << L"Failed to poll device." << endl;
        return -1;
    }

    result = directInputDeviceIface->GetDeviceState(sizeof(testData), (LPVOID)&testData);
    if (DI_OK != result)
    {
        wcout << L"Failed to retrieve device initial state." << endl;
        return -1;
    }

    CopyMemory(&testBufferedData, &testData, sizeof(testData));

    wcout << L"DONE" << endl;
    wcout << L"Device state is updated twice per second, with multiple polls in between." << endl;
    wcout << L"All axes are set to a range of -100 to +100, with 25% each deadzone/saturation." << endl;
    wcout << L"Quits automatically after 50 updates. To quit early, use CTRL+C." << endl;
    system("pause");
    system("cls");

    for (unsigned int i = 0; i < 50; ++i)
    {
        system("cls");
        wcout << L"Update #" << (i+1) << endl;

        // Retrieve the device's buffered input events.
        bufferedDataCount = _countof(bufferedData);
        result = directInputDeviceIface->GetDeviceData(sizeof(bufferedData[0]), bufferedData, &bufferedDataCount, 0);
        if (DI_BUFFEROVERFLOW == result)
        {
            wcout << L"Device event buffer has overflowed." << endl;
            return -1;
        }
        else if (DI_OK != result)
        {
            wcout << L"Failed to retrieve device buffered events." << endl;
            return -1;
        }

        // Apply the buffered input events to the test buffer.
        BYTE* bufptr = (BYTE*)&testBufferedData;
        for (DWORD i = 0; i < bufferedDataCount; ++i)
        {
            if (bufferedData[i].dwOfs >= offsetof(SInteractiveTestData, buttons))
            {
                // Data element is a single byte
                bufptr[bufferedData[i].dwOfs] = (BYTE)bufferedData[i].dwData;
            }
            else
            {
                // Data element is 4 bytes
                *((DWORD*)(&bufptr[bufferedData[i].dwOfs])) = (DWORD)bufferedData[i].dwData;
            }
        }

        // Retrieve the device's new state.
        result = directInputDeviceIface->GetDeviceState(sizeof(testData), (LPVOID)&testData);
        if (DI_OK != result)
        {
            wcout << L"Failed to retrieve device state." << endl;
            return -1;
        }

        // Compare buffer that results from device buffered event versus device state retrieval.
        if (0 != memcmp(&testData, &testBufferedData, sizeof(testData)))
        {
            wcout << L"GetDeviceData() and GetDeviceState() consistency check failed." << endl;
            return -1;
        }

        // Verify that all POVs that are not present are centered.
        for (DWORD i = deviceCapabilities.dwPOVs; i < _countof(testData.povs); ++i)
        {
            if ((DWORD)-1 != testData.povs[i])
            {
                wcout << L"Invalid POV data: those not present should be centered." << endl;
                return -1;
            }
        }

        wcout << L"Device presents " << deviceCapabilities.dwAxes << L" axes, " << deviceCapabilities.dwButtons << L" buttons, and " << deviceCapabilities.dwPOVs << L" POV controllers." << endl;

        wcout << endl;

        wcout << L"Device state:" << endl;

        wcout << endl;

        wcout << L"   X Axis  = " << testData.axisX << endl;
        wcout << L"   Y Axis  = " << testData.axisY << endl;
        wcout << L"   Z Axis  = " << testData.axisZ << endl;

        wcout << endl;

        wcout << L"   Rx Axis = " << testData.axisRx << endl;
        wcout << L"   Ry Axis = " << testData.axisRy << endl;
        wcout << L"   Rz Axis = " << testData.axisRz << endl;

        wcout << endl;

        wcout << L"   Dpad    = " << testData.povs[0] << endl;

        wcout << endl;

        wcout << L"   Buttons pressed:";
        for (int i = 0; i < _countof(testData.buttons); ++i)
        {
            if (0x80 == testData.buttons[i])
                wcout << L" " << (i + 1);
        }

        for (int i = 0; i < 10; ++i)
        {
            // Poll the device to update its state.
            result = directInputDeviceIface->Poll();
            if (DI_OK != result)
            {
                wcout << L"Failed to poll device." << endl;
                return -1;
            }

            Sleep(50);
        }
    }


    ////////////////////////////////////
    ////////   Cleanup and Exit

    wcout << L"\nExiting." << endl;

    directInputDeviceIface->Release();
    directInputIface->Release();

    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    int result = RunTestApp(argc, argv);
    system("pause");
    return result;
}
