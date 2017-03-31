/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * WrapperJoyWinMM.cpp
 *      Implementation of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "MapperFactory.h"
#include "WrapperJoyWinMM.h"
#include "XInputController.h"

#include <RegStr.h>

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "WrapperJoyWinMM.h" for documentation.

XInputController* WrapperJoyWinMM::controllers[XInputController::kMaxNumXInputControllers];

Mapper::Base* WrapperJoyWinMM::mapper = NULL;

BOOL WrapperJoyWinMM::isInitialized = FALSE;

DIOBJECTDATAFORMAT WrapperJoyWinMM::joyStateObjectDataFormat[] = {
    { &GUID_XAxis,      offsetof(SJoyStateData, axisX),         DIDFT_AXIS   | DIDFT_ANYINSTANCE,   0 },
    { &GUID_YAxis,      offsetof(SJoyStateData, axisY),         DIDFT_AXIS   | DIDFT_ANYINSTANCE,   0 },
    { &GUID_ZAxis,      offsetof(SJoyStateData, axisZ),         DIDFT_AXIS   | DIDFT_ANYINSTANCE,   0 },
    { &GUID_RxAxis,     offsetof(SJoyStateData, axisRx),        DIDFT_AXIS   | DIDFT_ANYINSTANCE,   0 },
    { &GUID_RyAxis,     offsetof(SJoyStateData, axisRy),        DIDFT_AXIS   | DIDFT_ANYINSTANCE,   0 },
    { &GUID_RzAxis,     offsetof(SJoyStateData, axisRz),        DIDFT_AXIS   | DIDFT_ANYINSTANCE,   0 },
    { &GUID_POV,        offsetof(SJoyStateData, pov),           DIDFT_POV    | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 0,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 1,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 2,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 3,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 4,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 5,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 6,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 7,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 8,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 9,   DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 10,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 11,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 12,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 13,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 14,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 15,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 16,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 17,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 18,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 19,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 20,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 21,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 22,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 23,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 24,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 25,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 26,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 27,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 28,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 29,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 30,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
    { &GUID_Button,     offsetof(SJoyStateData, buttons) + 31,  DIDFT_BUTTON | DIDFT_ANYINSTANCE,   0 },
};

const DIDATAFORMAT WrapperJoyWinMM::joyStateDataFormat = { sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), 0, sizeof(SJoyStateData), _countof(joyStateObjectDataFormat), joyStateObjectDataFormat };


// -------- CLASS METHODS -------------------------------------------------- //
// See "WrapperJoyWinMM.h" for documentation.

void WrapperJoyWinMM::Initialize(void)
{
    if (FALSE == isInitialized)
    {
        // Create a mapper and set its data format.
        mapper = MapperFactory::CreateMapper();
        mapper->SetApplicationDataFormat(&joyStateDataFormat);
        
        // Create controllers, one for each XInput position.
        for (DWORD i = 0; i < _countof(controllers); ++i)
            controllers[i] = new XInputController(i);
        
        // Ensure all controllers have their names published in the system registry.
        SetControllerNameRegistryInfo();
        
        // Initialization complete.
        isInitialized = TRUE;
    }
}

// -------- HELPERS -------------------------------------------------------- //
// See "WrapperJoyWinMM.h" for documentation.

// Communicates with the relevant controller and the mapper to fill the provided structure with device state information.
MMRESULT WrapperJoyWinMM::FillDeviceState(UINT joyID, SJoyStateData* joyStateData)
{
    // Ensure the controller number is within bounds.
    if (!(joyID < JoyGetNumDevs()))
        return JOYERR_PARMS;

    // Acquire the controller.
    controllers[joyID]->AcquireController();

    // Get the current state from the controller.
    XINPUT_STATE currentControllerState;

    HRESULT result = controllers[joyID]->RefreshControllerState();
    if (DI_OK != result)
        return JOYERR_NOCANDO;

    result = controllers[joyID]->GetCurrentDeviceState(&currentControllerState);
    if (DI_OK != result)
        return JOYERR_NOCANDO;

    // Get mapped controller state from the mapper.
    return mapper->WriteApplicationControllerState(currentControllerState.Gamepad, (LPVOID)joyStateData, sizeof(*joyStateData));
}

// ---------

int WrapperJoyWinMM::FillRegistryKeyStringA(LPSTR buf, const size_t bufcount)
{
    return LoadStringA(Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, buf, (int)bufcount);
}

// ---------

int WrapperJoyWinMM::FillRegistryKeyStringW(LPWSTR buf, const size_t bufcount)
{
    return LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, buf, (int)bufcount);
}

// ---------

void WrapperJoyWinMM::SetControllerNameRegistryInfo(void)
{
    HKEY registryKey;
    TCHAR registryKeyName[32];
    TCHAR registryPath[256];

    // First, add OEM string references to HKCU\System\CurrentControlSet\Control\MediaResources\Joystick\Xidi.
    // These will point a WinMM-based application to another part of the registry, by reference, which will actually contain the names.
    FillRegistryKeyStringW(registryKeyName, _countof(registryKeyName));
    _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG _T("\\%s\\") REGSTR_KEY_JOYCURR, registryKeyName);
    
    LSTATUS result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &registryKey, NULL);
    if (ERROR_SUCCESS != result) return;

    for (DWORD i = 0; i < _countof(controllers); ++i)
    {
        TCHAR valueName[64];
        TCHAR valueData[64];

        const int valueNameCount = _stprintf_s(valueName, _countof(valueName), REGSTR_VAL_JOYNOEMNAME, (i + 1));
        const int valueDataCount = _stprintf_s(valueData, _countof(valueData), _T("%s%u"), registryKeyName, (i + 1));

        result = RegSetValueEx(registryKey, valueName, 0, REG_SZ, (const BYTE*)valueData, (sizeof(TCHAR) * (valueDataCount + 1)));
        if (ERROR_SUCCESS != result)
        {
            RegCloseKey(registryKey);
            return;
        }
    }

    RegCloseKey(registryKey);

    // Next, place the names into the correct spots for the application to read.
    // These will be in HKCU\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\[valueData from above loop] and contain the name of the controller.
    for (DWORD i = 0; i < _countof(controllers); ++i)
    {
        TCHAR valueData[64];
        const int valueDataCount = ControllerIdentification::FillXInputControllerNameW(valueData, _countof(valueData), i);

        _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYOEM _T("\\%s%u"), registryKeyName, i + 1);
        result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &registryKey, NULL);
        if (ERROR_SUCCESS != result) return;

        result = RegSetValueEx(registryKey, REGSTR_VAL_JOYOEMNAME, 0, REG_SZ, (const BYTE*)valueData, (sizeof(TCHAR) * (valueDataCount + 1)));
        RegCloseKey(registryKey);

        if (ERROR_SUCCESS != result) return;
    }
}


// -------- METHODS: WinMM JOYSTICK ---------------------------------------- //
// See WinMM documentation for more information.

MMRESULT WrapperJoyWinMM::JoyConfigChanged(DWORD dwFlags)
{
    Initialize();
    
    // Redirect to the imported API.
    return ImportApiWinMM::joyConfigChanged(dwFlags);
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Check for the correct structure size.
        if (sizeof(*pjc) != cbjc)
            return JOYERR_PARMS;

        // Ensure the controller number is within bounds.
        if (!(uJoyID < JoyGetNumDevs()))
            return JOYERR_PARMS;

        // Get information from the mapper on the mapped device's capabilities.
        DIDEVCAPS mappedDeviceCaps;
        mapper->FillDeviceCapabilities(&mappedDeviceCaps);

        // Fill in the provided structure.
        ZeroMemory(pjc, sizeof(*pjc));
        pjc->wMaxAxes = 6;
        pjc->wMaxButtons = _countof(SJoyStateData::buttons);
        pjc->wNumAxes = (WORD)mappedDeviceCaps.dwAxes;
        pjc->wNumButtons = (WORD)mappedDeviceCaps.dwButtons;
        pjc->wXmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wXmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wYmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wYmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wZmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wZmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wRmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wRmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wUmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wUmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wVmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wVmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;

        if (mappedDeviceCaps.dwPOVs > 0)
            pjc->wCaps = JOYCAPS_HASPOV | JOYCAPS_POV4DIR;

        if (mappedDeviceCaps.dwAxes > 2)
            pjc->wCaps |= JOYCAPS_HASZ;

        if (mappedDeviceCaps.dwAxes > 3)
            pjc->wCaps |= JOYCAPS_HASR;

        if (mappedDeviceCaps.dwAxes > 4)
            pjc->wCaps |= JOYCAPS_HASU;

        if (mappedDeviceCaps.dwAxes > 5)
            pjc->wCaps |= JOYCAPS_HASV;

        FillRegistryKeyStringA(pjc->szRegKey, _countof(pjc->szRegKey));
        ControllerIdentification::FillXInputControllerNameA(pjc->szPname, _countof(pjc->szPname), (DWORD)uJoyID);

        return JOYERR_NOERROR;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyGetDevCapsA(uJoyID - _countof(controllers), pjc, cbjc);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Check for the correct structure size.
        if (sizeof(*pjc) != cbjc)
            return JOYERR_PARMS;

        // Ensure the controller number is within bounds.
        if (!(uJoyID < JoyGetNumDevs()))
            return JOYERR_PARMS;

        // Get information from the mapper on the mapped device's capabilities.
        DIDEVCAPS mappedDeviceCaps;
        mapper->FillDeviceCapabilities(&mappedDeviceCaps);

        // Fill in the provided structure.
        ZeroMemory(pjc, sizeof(*pjc));
        pjc->wMaxAxes = 6;
        pjc->wMaxButtons = _countof(SJoyStateData::buttons);
        pjc->wNumAxes = (WORD)mappedDeviceCaps.dwAxes;
        pjc->wNumButtons = (WORD)mappedDeviceCaps.dwButtons;
        pjc->wXmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wXmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wYmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wYmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wZmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wZmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wRmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wRmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wUmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wUmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;
        pjc->wVmin = (WORD)Mapper::Base::kDefaultAxisRangeMin;
        pjc->wVmax = (WORD)Mapper::Base::kDefaultAxisRangeMax;

        if (mappedDeviceCaps.dwPOVs > 0)
            pjc->wCaps = JOYCAPS_HASPOV | JOYCAPS_POV4DIR;

        if (mapper->AxisTypeCount(GUID_ZAxis) > 0)
            pjc->wCaps |= JOYCAPS_HASZ;

        if (mapper->AxisTypeCount(GUID_RzAxis) > 0)
            pjc->wCaps |= JOYCAPS_HASR;

        if (mapper->AxisTypeCount(GUID_RyAxis) > 0)
            pjc->wCaps |= JOYCAPS_HASU;

        if (mapper->AxisTypeCount(GUID_RxAxis) > 0)
            pjc->wCaps |= JOYCAPS_HASV;

        FillRegistryKeyStringW(pjc->szRegKey, _countof(pjc->szRegKey));
        ControllerIdentification::FillXInputControllerNameW(pjc->szPname, _countof(pjc->szPname), (DWORD)uJoyID);

        return JOYERR_NOERROR;
    }
    else
    {
        // Querying a non-XInput controller.
        HRESULT result = ImportApiWinMM::joyGetDevCapsW(uJoyID - _countof(controllers), pjc, cbjc);
        return result;
    }
}

// ---------

UINT WrapperJoyWinMM::JoyGetNumDevs(void)
{
    Initialize();

    // Number of controllers = number of XInput controllers + number of driver-reported controllers.
    return _countof(controllers) + ImportApiWinMM::joyGetNumDevs();
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        SJoyStateData joyStateData;
        MMRESULT result = FillDeviceState(uJoyID, &joyStateData);
        if (JOYERR_NOERROR != result)
            return result;

        // Fill in the provided structure.
        pji->wXpos = (WORD)joyStateData.axisX;
        pji->wYpos = (WORD)joyStateData.axisY;
        pji->wZpos = (WORD)joyStateData.axisZ;
        pji->wButtons = 0;
        if (joyStateData.buttons[0])
            pji->wButtons |= JOY_BUTTON1;
        if (joyStateData.buttons[1])
            pji->wButtons |= JOY_BUTTON2;
        if (joyStateData.buttons[2])
            pji->wButtons |= JOY_BUTTON3;
        if (joyStateData.buttons[3])
            pji->wButtons |= JOY_BUTTON4;

        // Operation complete.
        return JOYERR_NOERROR;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyGetPos(uJoyID - _countof(controllers), pji);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Check for the correct structure size.
        if (sizeof(*pji) != pji->dwSize)
            return JOYERR_PARMS;

        SJoyStateData joyStateData;
        MMRESULT result = FillDeviceState(uJoyID, &joyStateData);
        if (JOYERR_NOERROR != result)
            return result;

        // Fill in the provided structure.
        // WinMM uses only 16 bits to indicate that the dpad is centered, whereas it is safe to use all 32 in DirectInput, hence the conversion (forgetting this can introduce bugs into games).
        pji->dwPOV = ((DWORD)-1 == joyStateData.pov ? (DWORD)(JOY_POVCENTERED) : joyStateData.pov);
        pji->dwXpos = joyStateData.axisX;
        pji->dwYpos = joyStateData.axisY;
        pji->dwZpos = joyStateData.axisZ;
        pji->dwRpos = joyStateData.axisRz;
        pji->dwUpos = joyStateData.axisRy;
        pji->dwVpos = joyStateData.axisRx;
        pji->dwButtons = 0;
        for (DWORD i = 0; i < _countof(SJoyStateData::buttons); ++i)
        {
            if (joyStateData.buttons[i])
                pji->dwButtons |= (1 << i);
        }

        // Operation complete.
        return JOYERR_NOERROR;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyGetPosEx(uJoyID - _countof(controllers), pji);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Operation not supported.
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyGetThreshold(uJoyID - _countof(controllers), puThreshold);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyReleaseCapture(UINT uJoyID)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Operation not supported.
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyReleaseCapture(uJoyID - _countof(controllers));
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Operation not supported.
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joySetCapture(hwnd, uJoyID - _countof(controllers), uPeriod, fChanged);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoySetThreshold(UINT uJoyID, UINT uThreshold)
{
    if (uJoyID < _countof(controllers))
    {
        // Querying an XInput controller.
        Initialize();

        // Operation not supported.
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joySetThreshold(uJoyID - _countof(controllers), uThreshold);
    }
}
