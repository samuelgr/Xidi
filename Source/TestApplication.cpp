/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * TestApplication.cpp
 *      Entry point and other implementation for a simple console application
 *      for testing the functionality of this library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerIdentification.h"
#include "Dinput8ExportApi.h"
#include "Dinput8ImportApi.h"
#include "Mapper/Base.h"

#include <cstdlib>
#include <iostream>

using namespace Xidi;
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

    if (DI8DEVTYPE_GAMEPAD == GET_DIDEVICE_TYPE(lpddi->dwDevType) && IsEqualGUID(lpddi->guidProduct, ControllerIdentification::kXInputProductGUID))
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
    DWORD numErrors;
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
    ////////   Device Properties

    tout << _T("Begin IDirectInputDevice8->[Set|Get]Property") << endl;
    
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

    tout << _T("End IDirectInputDevice8->[Set|Get]Property") << endl << endl;


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
