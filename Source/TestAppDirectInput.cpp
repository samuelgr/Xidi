/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * TestAppDirectInput.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library via DirectInput.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "ExportApiDirectInput.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "TestApp.h"
#include "Mapper/Base.h"


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
#define ExportedDirectInputCreateMethod         ExportApiDirectInputDirectInput8Create
#define Use_IID_IDirectInput                    IID_IDirectInput8
#else
#define ExportedDirectInputCreateMethod         ExportApiDirectInputDirectInputCreateEx
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
#else
LPTSTR DirectInputDeviceTypeToString(BYTE type)
{
    switch (type)
    {
    case DIDEVTYPE_DEVICE:
        return _T("DEVICE");

    case DIDEVTYPE_MOUSE:
        return _T("MOUSE");

    case DIDEVTYPE_KEYBOARD:
        return _T("KEYBOARD");

    case DIDEVTYPE_JOYSTICK:
        return _T("JOYSTICK");

    default:
        return _T("UNKNOWN");
    }
}
#endif

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

// Callback for enumerating DirectInput devices via the IDirectInput interface.
BOOL STDMETHODCALLTYPE EnumDevicesTestCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    DWORD* testValuePtr = (DWORD*)pvRef;
    GUID xProductGUID;
    ControllerIdentification::GetProductGUID(xProductGUID);

    tout << _T("    ");
    
    if (*testValuePtr != testValue)
        tout << _T("[pvRef fail] ");

    if (!flagCallbackExpected)
        tout << _T("[flagCallbackExpected fail] ");
    
    tout << _T("Found ") << DirectInputDeviceTypeToString(GET_DIDEVICE_TYPE(lpddi->dwDevType)) << ": " << lpddi->tszProductName;

#if DIRECTINPUT_VERSION >= 0x0800
    if (DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(lpddi->dwDevType) && (lpddi->guidProduct == xProductGUID))
#else
    if (DIDEVTYPE_JOYSTICK == GET_DIDEVICE_TYPE(lpddi->dwDevType) && (lpddi->guidProduct == xProductGUID))
#endif
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

// Callback for enumerating DirectInput device axes via the IDirectInputDevice interface.
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

// Callback for enumerating DirectInput device buttons via the IDirectInputDevice interface.
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

// Callback for enumerating DirectInput device POVs via the IDirectInputDevice interface.
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

// Callback for enumerating DirectInput device objects via the IDirectInputDevice interface.
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
    DWORD numErrors;
    LatestIDirectInput* directInputIface;
    LatestIDirectInputDevice* directInputDeviceIface;
    
    
    ////////////////////////////////////
    ////////   Initialization
    
    // Initialize the imported DirectInput8 API.
    if (S_OK != ImportApiDirectInput::Initialize())
    {
        terr << _T("Unable to initialize DirectInput8 API.") << endl;
        return -1;
    }

    // Create the main interface to DirectInput.
    result = ExportedDirectInputCreateMethod(GetModuleHandle(NULL), DIRECTINPUT_VERSION, Use_IID_IDirectInput, (LPVOID*)&directInputIface, NULL);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain IDirectInput interface pointer: code ") << result << _T(".") << endl;
        return -1;
    }

    
    ////////////////////////////////////
    ////////   Enumeration
    
    // Enumerate all devices attached to the system.
    flagCallbackExpected = TRUE;
    
    tout << _T("Begin IDirectInput->EnumDevices") << endl;
    
#if DIRECTINPUT_VERSION >= 0x0800
    result = directInputIface->EnumDevices(DI8DEVCLASS_GAMECTRL, &EnumDevicesTestCallback, (LPVOID)&testValue, DIEDFL_ATTACHEDONLY);
#else
    result = directInputIface->EnumDevices(DIDEVTYPE_JOYSTICK, &EnumDevicesTestCallback, (LPVOID)&testValue, DIEDFL_ATTACHEDONLY);
#endif
    if (DI_OK != result)
    {
        terr << _T("Unable to enumerate attached devices: code ") << result << _T(".") << endl;
        return -1;
    }
    
    // Test that the callback was invoked the required number of times.
    if (flagCallbackExpected && flagInstanceGuidToTestFound)
        tout << _T("FAIL: IDirectInput->EnumDevices callback test") << endl;
    else
        tout << _T("PASS: IDirectInput->EnumDevices callback test") << endl;

    tout << _T("End IDirectInput->EnumDevices") << endl << endl;

    // Verify that a supported device was found
    if (!flagInstanceGuidToTestFound)
    {
        tout << _T("No supported devices found. Connect one and try again.") << endl;
        return -1;
    }


    ////////////////////////////////////
    ////////   Device Creation

    // Obtain a pointer to the interface of the device.
    result = directInputIface->CreateDevice(instanceGuidToTest, (EarliestIDirectInputDevice**)&directInputDeviceIface, NULL);
    
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain IDirectInputDevice interface pointer: code ") << result << _T(".") << endl;
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

    tout << _T("Device presents ") << deviceCapabilities.dwAxes << _T(" axes, ") << deviceCapabilities.dwButtons << _T(" buttons, and ") << deviceCapabilities.dwPOVs << _T(" POV controllers.") << endl << endl;
    

    ////////////////////////////////////
    ////////   Device Object Enumeration

    tout << _T("Begin IDirectInputDevice->EnumObjects") << endl;
    
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
        tout << _T("PASS: IDirectInputDevice->EnumObjects axis consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice->EnumObjects axis consistency check.") << endl;

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
        tout << _T("PASS: IDirectInputDevice->EnumObjects button consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice->EnumObjects button consistency check.") << endl;

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
        tout << _T("PASS: IDirectInputDevice->EnumObjects POV consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice->EnumObjects POV consistency check.") << endl;

    // Attempt to enumerate everything to verify consistency.
    testCounter = 0;
    result = directInputDeviceIface->EnumObjects(&EnumObjectsOverallTestCallback, (LPVOID)&testValue, DIDFT_ALL);
    if (DI_OK != result)
    {
        terr << _T("Unable to obtain get device objects: code ") << result << _T(".") << endl;
        return -1;
    }
    if (testCounter == (deviceCapabilities.dwAxes + deviceCapabilities.dwButtons + deviceCapabilities.dwPOVs))
        tout << _T("PASS: IDirectInputDevice->EnumObjects overall consistency check.") << endl;
    else
        tout << _T("FAIL: IDirectInputDevice->EnumObjects overall consistency check.") << endl;

    // Finished enumerating objects.
    tout << _T("End IDirectInputDevice->EnumObjects") << endl << endl;


    ////////////////////////////////////
    ////////   Device Object Information

    tout << _T("Begin IDirectInputDevice->GetObjectInfo") << endl;

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
    tout << _T("End IDirectInputDevice->GetObjectInfo") << endl << endl;
    
    
    ////////////////////////////////////
    ////////   Device Properties

    tout << _T("Begin IDirectInputDevice->[Set|Get]Property") << endl;
    
    DIPROPRANGE rangeTest;
    DIPROPDWORD deadzoneTest;

    ZeroMemory(&rangeTest, sizeof(rangeTest));
    ZeroMemory(&deadzoneTest, sizeof(deadzoneTest));

    // First, test range without setting header size properly but setting everything else.
    rangeTest.diph.dwHow = DIPH_BYID;
    rangeTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(0);
    rangeTest.diph.dwSize = sizeof(rangeTest);
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result || Mapper::Base::kDefaultAxisRangeMax == rangeTest.lMax)
        tout << _T("FAIL: Invalid header size test.") << endl;
    else
        tout << _T("PASS: Invalid header size test.") << endl;

    // Same, but now set header size and not overall size
    rangeTest.diph.dwSize = 0;
    rangeTest.diph.dwHeaderSize = sizeof(rangeTest.diph);
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result || Mapper::Base::kDefaultAxisRangeMax == rangeTest.lMax)
        tout << _T("FAIL: Invalid structure size test.") << endl;
    else
        tout << _T("PASS: Invalid structure size test.") << endl;

    // Set sizes and expect to get default values back.
    rangeTest.diph.dwSize = sizeof(rangeTest);
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DI_OK != result || Mapper::Base::kDefaultAxisRangeMax != rangeTest.lMax || Mapper::Base::kDefaultAxisRangeMin != rangeTest.lMin)
        tout << _T("FAIL: Default range test.") << endl;
    else
        tout << _T("PASS: Default range test.") << endl;

    // Set an invalid range and expect it to be rejected.
    rangeTest.lMax = -1000;
    rangeTest.lMin = 1000;
    result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result)
        tout << _T("FAIL: Set invalid range test 1.") << endl;
    else
        tout << _T("PASS: Set invalid range test 1.") << endl;

    // Another invalid range to be rejected.
    rangeTest.lMax = 1000;
    rangeTest.lMin = 1000;
    result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DIERR_INVALIDPARAM != result)
        tout << _T("FAIL: Set invalid range test 2.") << endl;
    else
        tout << _T("PASS: Set invalid range test 2.") << endl;

    // This range is valid and should be accepted.
    rangeTest.lMax = 1000;
    rangeTest.lMin = -1000;
    result = directInputDeviceIface->SetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DI_OK != result)
        tout << _T("FAIL: Set valid range test.") << endl;
    else
        tout << _T("PASS: Set valid range test.") << endl;

    // Expect to read back that range.
    rangeTest.lMax = 0;
    rangeTest.lMin = 0;
    result = directInputDeviceIface->GetProperty(DIPROP_RANGE, &rangeTest.diph);
    if (DI_OK != result || 1000 != rangeTest.lMax || -1000 != rangeTest.lMin)
        tout << _T("FAIL: Get valid range test.") << endl;
    else
        tout << _T("PASS: Get valid range test.") << endl;

    // Get a valid deadzone but targetting a button, should be rejected.
    deadzoneTest.diph.dwHow = DIPH_BYID;
    deadzoneTest.diph.dwObj = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(0);
    deadzoneTest.diph.dwSize = sizeof(deadzoneTest);
    deadzoneTest.diph.dwHeaderSize = sizeof(deadzoneTest.diph);
    deadzoneTest.dwData = 1000;
    result = directInputDeviceIface->GetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DIERR_UNSUPPORTED != result || 1000 != deadzoneTest.dwData)
        tout << _T("FAIL: Bad deadzone target test.") << endl;
    else
        tout << _T("PASS: Bad deadzone target test.") << endl;

    // Set an actual valid deadzone.
    deadzoneTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(0);
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result || 1000 != deadzoneTest.dwData)
        tout << _T("FAIL: Set valid deadzone test.") << endl;
    else
        tout << _T("PASS: Set valid deadzone test.") << endl;

    // Read it back.
    deadzoneTest.dwData = 1000000;
    result = directInputDeviceIface->GetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result || 1000 != deadzoneTest.dwData)
        tout << _T("FAIL: Get valid deadzone test.") << endl;
    else
        tout << _T("PASS: Get valid deadzone test.") << endl;

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
        tout << _T("FAIL: Single axis valid deadzone test.") << endl;
    else
        tout << _T("PASS: Single axis valid deadzone test.") << endl;

    // Write a deadzone out of range.
    deadzoneTest.dwData = Mapper::Base::kMaxAxisDeadzoneSaturation * 2;
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DIERR_INVALIDPARAM != result)
        tout << _T("FAIL: Set out-of-range deadzone test.") << endl;
    else
        tout << _T("PASS: Set out-of-range deadzone test.") << endl;
    
    // Write a deadzone for the whole device, but use an invalid "dwObj".
    deadzoneTest.dwData = 51;
    deadzoneTest.diph.dwHow = DIPH_DEVICE;
    deadzoneTest.diph.dwObj = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(1);
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DIERR_INVALIDPARAM != result)
        tout << _T("FAIL: Set invalid whole device deadzone test.") << endl;
    else
        tout << _T("PASS: Set invalid whole device deadzone test.") << endl;

    // Write a valid deadzone for the whole device.
    deadzoneTest.dwData = 54;
    deadzoneTest.diph.dwObj = 0;
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result)
        tout << _T("FAIL: Set valid whole device deadzone test.") << endl;
    else
        tout << _T("PASS: Set valid whole device deadzone test.") << endl;

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
        tout << _T("FAIL: Whole device valid deadzone test.") << endl;
    else
        tout << _T("PASS: Whole device valid deadzone test.") << endl;

    tout << _T("End IDirectInputDevice->[Set|Get]Property") << endl << endl;


    ////////////////////////////////////
    ////////   Interactive Mode Preparation

    tout << _T("Preparing to launch interactive mode... ");

    
    // Set deadzone and saturation for the whole device.
    deadzoneTest.dwData = 2500;
    deadzoneTest.diph.dwHow = DIPH_DEVICE;
    deadzoneTest.diph.dwObj = 0;
    result = directInputDeviceIface->SetProperty(DIPROP_DEADZONE, &deadzoneTest.diph);
    if (DI_OK != result)
    {
        tout << _T("FAILED") << endl << _T("Unable to set deadzone.") << endl;
        return -1;
    }
    deadzoneTest.dwData = 7500;
    result = directInputDeviceIface->SetProperty(DIPROP_SATURATION, &deadzoneTest.diph);
    if (DI_OK != result)
    {
        tout << _T("FAILED") << endl << _T("Unable to set saturation.") << endl;
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
            tout << _T("FAILED") << endl << _T("Unable to set range.") << endl;
            return -1;
        }
    }

    // Set the application's data format to that specified at the top of this file.
    result = directInputDeviceIface->SetDataFormat(&dataFormat);
    if (DI_OK != result)
    {
        tout << _T("FAILED") << endl << _T("Unable to set data format.") << endl;
        return -1;
    }

    // Acquire the device.
    result = directInputDeviceIface->Acquire();
    if (DI_OK != result)
    {
        tout << _T("FAILED") << endl << _T("Unable to acquire device.") << endl;
        return -1;
    }

    tout << _T("DONE") << endl;
    tout << _T("After every character typed, the device's state will be read and reported.") << endl;
    tout << _T("All axes are set to a range of -100 to +100, with 25% each deadzone/saturation.") << endl;
    tout << _T("To quit, type Q and press RETURN.") << endl;
    tout << _T("To re-read the device's state, type any other character and press RETURN.") << endl;
    system("pause");
    system("cls");

    SInteractiveTestData testData;
    TCHAR inputchar = _T('\0');

    while (_T('Q') != inputchar && _T('q') != inputchar)
    {
        system("cls");
        
        // Poll the device to update its state.
        result = directInputDeviceIface->Poll();
        if (DI_OK != result)
        {
            tout << _T("Failed to poll device.") << endl;
            return -1;
        }

        // Retrieve the device's new state.
        result = directInputDeviceIface->GetDeviceState(sizeof(testData), (LPVOID)&testData);
        if (DI_OK != result)
        {
            tout << _T("Failed to retrieve device state.") << endl;
            return -1;
        }

        // Verify that all POVs that are not present are centered.
        for (DWORD i = deviceCapabilities.dwPOVs; i < _countof(testData.povs); ++i)
        {
            if ((DWORD)-1 != testData.povs[i])
            {
                tout << _T("Invalid POV data: those not present should be centered.") << endl;
                return -1;
            }
        }

        tout << _T("Device presents ") << deviceCapabilities.dwAxes << _T(" axes, ") << deviceCapabilities.dwButtons << _T(" buttons, and ") << deviceCapabilities.dwPOVs << _T(" POV controllers.") << endl;

        tout << endl;
        
        tout << _T("Device state:") << endl;

        tout << endl;

        tout << _T("   X Axis  = ") << testData.axisX << endl;
        tout << _T("   Y Axis  = ") << testData.axisY << endl;
        tout << _T("   Z Axis  = ") << testData.axisZ << endl;
        
        tout << endl;
        
        tout << _T("   Rx Axis = ") << testData.axisRx << endl;
        tout << _T("   Ry Axis = ") << testData.axisRy << endl;
        tout << _T("   Rz Axis = ") << testData.axisRz << endl;
        
        tout << endl;

        tout << _T("   Dpad    = ") << testData.povs[0] << endl;

        tout << endl;

        tout << _T("   Buttons pressed:");
        for (int i = 0; i < _countof(testData.buttons); ++i)
        {
            if (0x80 == testData.buttons[i])
                tout << _T(" ") << (i + 1);
        }
        
        tout << endl << endl << _T("Awaiting input (character then RETURN)... ");
        tin >> inputchar;
    }

    
    ////////////////////////////////////
    ////////   Cleanup and Exit
    
    tout << _T("Exiting.") << endl;
    
    directInputDeviceIface->Release();
    directInputIface->Release();
    
    return 0;
}


// -------- ENTRY POINT ---------------------------------------------------- //

int main(int argc, char* argv[])
{
    Globals::SetInstanceHandle(GetModuleHandle(NULL));
    
    int result = RunTestApp(argc, argv);
    
    system("pause");
    return result;
}
