/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file WrapperJoyWinMM.cpp
 *   Implementation of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ApiDirectInput.h"
#include "ApiStdString.h"
#include "ControllerIdentification.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "ImportApiWinMM.h"
#include "Log.h"
#include "MapperFactory.h"
#include "WrapperJoyWinMM.h"
#include "XInputController.h"

#include <climits>
#include <RegStr.h>
#include <utility>
#include <vector>

using namespace Xidi;


// -------- LOCAL TYPES ---------------------------------------------------- //

namespace Xidi
{
    // Used to provide all information needed to get a list of XInput devices exposed by WinMM.
    struct SWinMMEnumCallbackInfo
    {
        std::vector<std::pair<StdString, bool>>* systemDeviceInfo;
        IDirectInput8* directInputInterface;
    };
}


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

std::vector<int> WrapperJoyWinMM::joyIndexMap;

std::vector<std::pair<StdString, bool>> WrapperJoyWinMM::joySystemDeviceInfo;


// -------- CLASS METHODS -------------------------------------------------- //
// See "WrapperJoyWinMM.h" for documentation.

void WrapperJoyWinMM::Initialize(void)
{
    if (FALSE == isInitialized)
    {
        // Create a mapper and set its data format.
        mapper = MapperFactory::CreateMapper();
        if (DI_OK != mapper->SetApplicationDataFormat(&joyStateDataFormat))
            Log::WriteLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_WRAPPERJOYWINMM_INITIALIZE_ERROR_SET_DATA_FORMAT);
        
        // Create controllers, one for each XInput position.
        for (DWORD i = 0; i < _countof(controllers); ++i)
            controllers[i] = new XInputController(i);
        
        // Enumerate all devices exposed by WinMM.
        CreateSystemDeviceInfo();
        
        // Initialize the joystick index map.
        CreateJoyIndexMap();
        
        // Ensure all controllers have their names published in the system registry.
        SetControllerNameRegistryInfo();
        
        // Initialization complete.
        Log::WriteLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_WRAPPERJOYWINMM_INITIALIZE_SUCCESS);
        isInitialized = TRUE;
    }
}

// -------- HELPERS -------------------------------------------------------- //
// See "WrapperJoyWinMM.h" for documentation.

void WrapperJoyWinMM::CreateJoyIndexMap(void)
{
    const size_t numDevicesFromSystem = joySystemDeviceInfo.size();
    const size_t numXInputVirtualDevices = _countof(controllers);
    const size_t numDevicesTotal = numDevicesFromSystem + numXInputVirtualDevices;

    // Initialize the joystick index map with conservative defaults.
    // In the event of an error, it is safest to avoid enabling any Xidi virtual controllers to prevent binding both to the WinMM version and the Xidi version of the same one.
    joyIndexMap.clear();
    joyIndexMap.reserve(numDevicesTotal);
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_MAP_NUM_DEVICES_FORMAT, (unsigned int)numDevicesTotal);
    
    if ((false == joySystemDeviceInfo[0].second) && !(joySystemDeviceInfo[0].first.empty()))
    {
        // Preferred device is present but does not support XInput.
        // Filter out all non-XInput devices, but ensure Xidi virtual devices are mapped to the end.

        for (int i = 0; i < (int)numDevicesFromSystem; ++i)
        {
            if ((false == joySystemDeviceInfo[i].second) && !(joySystemDeviceInfo[i].first.empty()))
            {
                joyIndexMap.push_back(i);
                Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_MAP_DEVICE_SYSTEM_FORMAT, (unsigned int)joyIndexMap.size(), (unsigned int)i);
            }
        }

        for (int i = 0; i < (int)numXInputVirtualDevices; ++i)
        {
            joyIndexMap.push_back(-(i + 1));
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_MAP_DEVICE_XIDI_FORMAT, (unsigned int)joyIndexMap.size(), (unsigned int)i);
        }
    }
    else
    {
        // Preferred device supports XInput or is not present.
        // Filter out all non-XInput devices and present Xidi virtual devices at the start.

        for (int i = 0; i < (int)numXInputVirtualDevices; ++i)
        {
            joyIndexMap.push_back(-(i + 1));
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_MAP_DEVICE_XIDI_FORMAT, (unsigned int)joyIndexMap.size(), (unsigned int)i);
        }

        for (int i = 0; i < (int)numDevicesFromSystem; ++i)
        {
            if ((false == joySystemDeviceInfo[i].second) && !(joySystemDeviceInfo[i].first.empty()))
            {
                joyIndexMap.push_back(i);
                Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_MAP_DEVICE_SYSTEM_FORMAT, (unsigned int)joyIndexMap.size(), (unsigned int)i);
            }
        }
    }
}

// --------

void WrapperJoyWinMM::CreateSystemDeviceInfo(void)
{
    const size_t numDevicesFromSystem = (size_t)ImportApiWinMM::joyGetNumDevs();
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_ENUM_NUM_DEVS_FORMAT, (unsigned int)numDevicesFromSystem);

    // Initialize the system device information data structure.
    joySystemDeviceInfo.clear();
    joySystemDeviceInfo.reserve(numDevicesFromSystem);

    // Figure out the registry key that needs to be opened and open it.
    JOYCAPS joyCaps;
    HKEY registryKey;
    if (JOYERR_NOERROR != ImportApiWinMM::joyGetDevCaps((UINT_PTR)-1, &joyCaps, sizeof(joyCaps)))
    {
        Log::WriteLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_ENUM_FAILED_NO_REGISTRY);
        return;
    }

    TCHAR registryPath[1024];
    _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG _T("\\%s\\") REGSTR_KEY_JOYCURR, joyCaps.szRegKey);
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_QUERY_VALUE, NULL, &registryKey, NULL))
    {
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_ENUM_FAILED_CANT_OPEN_REGISTRY_FORMAT, registryPath);
        return;
    }
    
    // For each joystick device available in the system, see if it is present and, if so, get its device identifier (vendor ID and product ID string).
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_ENUM_DEVICE_BEGIN);

    for (size_t i = 0; i < numDevicesFromSystem; ++i)
    {
        // Get the device capabilities. If this fails, the device is not present and can be skipped.
        if (JOYERR_NOERROR != ImportApiWinMM::joyGetDevCaps((UINT_PTR)i, &joyCaps, sizeof(joyCaps)))
        {
            joySystemDeviceInfo.push_back({ _T(""), false });
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_ENUM_DEVICE_NOT_PRESENT_FORMAT, (unsigned int)i);
            continue;
        }
        
        // Use the registry to get device vendor ID and product ID string.
        TCHAR registryValueName[64];
        _stprintf_s(registryValueName, _countof(registryValueName), REGSTR_VAL_JOYNOEMNAME, ((int)i + 1));

        TCHAR registryValueData[64];
        DWORD registryValueSize = sizeof(registryValueData);
        if (ERROR_SUCCESS != RegGetValue(registryKey, NULL, registryValueName, RRF_RT_REG_SZ, NULL, registryValueData, &registryValueSize))
        {
            // If the registry value does not exist, this is past the end of the number of devices WinMM sees.
            joySystemDeviceInfo.push_back({ _T(""), false });
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_ENUM_DEVICE_NOT_PRESENT_FORMAT, (unsigned int)i);
            continue;
        }

        // Add the vendor ID and product ID string to the list.
        joySystemDeviceInfo.push_back({ registryValueData, false });
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_ENUM_DEVICE_FORMAT, (unsigned int)i, registryValueData);
    }

    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_ENUM_DEVICE_END);
    RegCloseKey(registryKey);

    // Enumerate all devices using DirectInput8 to find any XInput devices with matching vendor and product identifiers.
    // This will provide information on whether each WinMM device supports XInput.
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_DETECT_XINPUT_BEGIN);
    IDirectInput8* directInputInterface = NULL;
    if (S_OK != ImportApiDirectInput::DirectInput8Create(Globals::GetInstanceHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&directInputInterface, NULL))
    {
        Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_DETECT_XINPUT_FAILED_NO_DINPUT_IFACE);
        return;
    }

    SWinMMEnumCallbackInfo callbackInfo;
    callbackInfo.systemDeviceInfo = &joySystemDeviceInfo;
    callbackInfo.directInputInterface = directInputInterface;
    if (S_OK != directInputInterface->EnumDevices(DI8DEVCLASS_GAMECTRL, CreateSystemDeviceInfoEnumCallback, (LPVOID)&callbackInfo, 0))
    {
        Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_DETECT_XINPUT_FAILED_CANT_ENUM_DINPUT);
        return;
    }

    Log::WriteLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_DETECT_XINPUT_END);
}

// --------

BOOL STDMETHODCALLTYPE WrapperJoyWinMM::CreateSystemDeviceInfoEnumCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    SWinMMEnumCallbackInfo* callbackInfo = (SWinMMEnumCallbackInfo*)pvRef;

    std::wstring devicePath;
    BOOL deviceSupportsXInput = ControllerIdentification::DoesDirectInputControllerSupportXInput(callbackInfo->directInputInterface, lpddi->guidInstance, &devicePath);

    if (deviceSupportsXInput)
    {
#ifdef UNICODE
        const WCHAR* devicePathString = devicePath.c_str();
#else
        CHAR devicePathString[(_countof(DIPROPGUIDANDPATH::wszPath) + 1) * sizeof(WCHAR) / sizeof(CHAR)];
        ZeroMemory(devicePathString, sizeof(devicePathString));
        wcstombs_s(NULL, devicePathString, _countof(devicePathString) - 1, devicePath.c_str(), devicePath.length());
#endif

        // Skip to the part of the path string that identifies vendor and product.
        const TCHAR* devicePathSubstring = _tcsstr(devicePathString, _T("VID_"));
        if (NULL == devicePathSubstring) devicePathSubstring = _tcsstr(devicePathString, _T("vid_"));
        if (NULL != devicePathSubstring)
        {
            // For each element of the WinMM devices list, see if the vendor and product IDs match the one DirectInput presented as being compatible with XInput.
            // If so, mark it in the list as being an XInput controller.
            for (size_t i = 0; i < callbackInfo->systemDeviceInfo->size(); ++i)
            {
                // Already seen this device, skip.
                if (true == callbackInfo->systemDeviceInfo->at(i).second)
                    continue;

                // No device at that position, skip.
                if (callbackInfo->systemDeviceInfo->at(i).first.empty())
                    continue;

                // Check for a matching vendor and product ID. If so, mark the device as supporting XInput.
                if (0 == _tcsnicmp(callbackInfo->systemDeviceInfo->at(i).first.c_str(), devicePathSubstring, callbackInfo->systemDeviceInfo->at(i).first.length()))
                {
                    callbackInfo->systemDeviceInfo->at(i).second = true;
                    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_DETECT_XINPUT_DEVICE_SUPPORTS_FORMAT, (unsigned int)i);
                }
            }
        }
    }

    return DIENUM_CONTINUE;
}

// --------

MMRESULT WrapperJoyWinMM::FillDeviceState(UINT joyID, SJoyStateData* joyStateData)
{
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
    LSTATUS result;
    TCHAR registryKeyName[128];
    TCHAR registryPath[1024];

#ifdef UNICODE
    FillRegistryKeyStringW(registryKeyName, _countof(registryKeyName));
#else
    FillRegistryKeyStringA(registryKeyName, _countof(registryKeyName));
#endif
    
    // Place the names into the correct spots for the application to read.
    // These will be in HKCU\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\Xidi# and contain the name of the controller.
    for (DWORD i = 0; i < _countof(controllers); ++i)
    {
        TCHAR valueData[64];
#ifdef UNICODE
        const int valueDataCount = ControllerIdentification::FillXInputControllerNameW(valueData, _countof(valueData), i);
#else
        const int valueDataCount = ControllerIdentification::FillXInputControllerNameA(valueData, _countof(valueData), i);
#endif

        _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYOEM _T("\\%s%u"), registryKeyName, i + 1);
        result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &registryKey, NULL);
        if (ERROR_SUCCESS != result) return;

        result = RegSetValueEx(registryKey, REGSTR_VAL_JOYOEMNAME, 0, REG_SZ, (const BYTE*)valueData, (sizeof(TCHAR) * (valueDataCount + 1)));
        RegCloseKey(registryKey);

        if (ERROR_SUCCESS != result) return;
    }

    // Next, add OEM string references to HKCU\System\CurrentControlSet\Control\MediaResources\Joystick\Xidi.
    // These will point a WinMM-based application to another part of the registry, by reference, which actually contain the names.
    // Do this by consuming the joystick index map and system device information data structure.
    _stprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG _T("\\%s\\") REGSTR_KEY_JOYCURR, registryKeyName);

    result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &registryKey, NULL);
    if (ERROR_SUCCESS != result) return;
    
    for (DWORD i = 0; i < joyIndexMap.size(); ++i)
    {
        TCHAR valueName[64];
        const int valueNameCount = _stprintf_s(valueName, _countof(valueName), REGSTR_VAL_JOYNOEMNAME, (i + 1));
        
        if (joyIndexMap[i] < 0)
        {
            // Map points to a Xidi virtual device.

            // Index is just -1 * the value in the map.
            // Use this value to create the correct string to write to the registry.
            TCHAR valueData[64];
            const int valueDataCount = _stprintf_s(valueData, _countof(valueData), _T("%s%u"), registryKeyName, ((UINT)(-joyIndexMap[i])));

            // Write the value to the registry.
            RegSetValueEx(registryKey, valueName, 0, REG_SZ, (const BYTE*)valueData, (sizeof(TCHAR) * (valueDataCount + 1)));
        }
        else
        {
            // Map points to a non-Xidi device.

            // Just reference the string directly.
            const TCHAR* valueData = joySystemDeviceInfo[joyIndexMap[i]].first.c_str();
            const int valueDataCount = (int)joySystemDeviceInfo[joyIndexMap[i]].first.length();

            // Write the value to the registry.
             RegSetValueEx(registryKey, valueName, 0, REG_SZ, (const BYTE*)valueData, (sizeof(joySystemDeviceInfo[joyIndexMap[i]].first[0]) * (valueDataCount + 1)));
        }
    }
}

// --------

int WrapperJoyWinMM::TranslateApplicationJoyIndex(UINT uJoyID)
{
    if (joyIndexMap.size() <= (size_t)uJoyID)
        return INT_MAX;
    else
        return joyIndexMap[uJoyID];
}


// -------- METHODS: WinMM JOYSTICK ---------------------------------------- //
// See WinMM documentation for more information.

MMRESULT WrapperJoyWinMM::JoyConfigChanged(DWORD dwFlags)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_WRAPPERJOYWINMM_REFRESH_JOY_STATE);
    Initialize();

    // Redirect to the imported API so that its view of the registry can be updated.
    HRESULT result = ImportApiWinMM::joyConfigChanged(dwFlags);

    // Update Xidi's view of devices.
    CreateSystemDeviceInfo();
    CreateJoyIndexMap();
    SetControllerNameRegistryInfo();

    return result;
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joyGetDevCapsA", (unsigned int)uJoyID);
    
    // Special case: index is specified as -1, which the API says just means fill in the registry key.
    if ((UINT_PTR)-1 == uJoyID)
    {
        FillRegistryKeyStringA(pjc->szRegKey, _countof(pjc->szRegKey));
        return JOYERR_NOERROR;
    }
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex((UINT)uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

        // Check for the correct structure size.
        if (sizeof(*pjc) != cbjc)
        {
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_FAILED_XINPUT_OPERATION_PARAMS_FORMAT, "joyGetDevCapsA");
            return JOYERR_PARMS;
        }

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

        FillRegistryKeyStringA(pjc->szRegKey, _countof(pjc->szRegKey));
        ControllerIdentification::FillXInputControllerNameA(pjc->szPname, _countof(pjc->szPname), xJoyID);

        return JOYERR_NOERROR;
    }
    else
    {
        // Querying a non-XInput controller.
        // Replace the registry key but otherwise leave the response unchanged.
        HRESULT result = ImportApiWinMM::joyGetDevCapsA((UINT_PTR)realJoyID, pjc, cbjc);
        
        if (JOYERR_NOERROR == result)
            FillRegistryKeyStringA(pjc->szRegKey, _countof(pjc->szRegKey));

        return result;
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joyGetDevCapsW", (unsigned int)uJoyID);
    
    // Special case: index is specified as -1, which the API says just means fill in the registry key.
    if ((UINT_PTR)-1 == uJoyID)
    {
        FillRegistryKeyStringW(pjc->szRegKey, _countof(pjc->szRegKey));
        return JOYERR_NOERROR;
    }
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex((UINT)uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

        // Check for the correct structure size.
        if (sizeof(*pjc) != cbjc)
        {
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_FAILED_XINPUT_OPERATION_PARAMS_FORMAT, "joyGetDevCapsW");
            return JOYERR_PARMS;
        }

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
        ControllerIdentification::FillXInputControllerNameW(pjc->szPname, _countof(pjc->szPname), xJoyID);

        return JOYERR_NOERROR;
    }
    else
    {
        // Querying a non-XInput controller.
        // Replace the registry key with ours but otherwise leave the response unchanged.
        HRESULT result = ImportApiWinMM::joyGetDevCapsW((UINT_PTR)realJoyID, pjc, cbjc);

        if (JOYERR_NOERROR == result)
            FillRegistryKeyStringW(pjc->szRegKey, _countof(pjc->szRegKey));
        
        return result;
    }
}

// ---------

UINT WrapperJoyWinMM::JoyGetNumDevs(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_GLOBAL_OPERATION_FORMAT, "joyGetNumDevs");
    
    Initialize();

    // Number of controllers = number of XInput controllers + number of driver-reported controllers.
    return (UINT)joyIndexMap.size();
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joyGetPos", (unsigned int)uJoyID);

    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex(uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

        SJoyStateData joyStateData;
        MMRESULT result = FillDeviceState((UINT)xJoyID, &joyStateData);
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
        return ImportApiWinMM::joyGetPos((UINT)realJoyID, pji);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joyGetPosEx", (unsigned int)uJoyID);
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex(uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

        // Check for the correct structure size.
        if (sizeof(*pji) != pji->dwSize)
        {
            Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_FAILED_XINPUT_OPERATION_PARAMS_FORMAT, "joyGetPosEx");
            return JOYERR_PARMS;
        }

        SJoyStateData joyStateData;
        MMRESULT result = FillDeviceState((UINT)xJoyID, &joyStateData);
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
        return ImportApiWinMM::joyGetPosEx((UINT)realJoyID, pji);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joyGetThreshold", (unsigned int)uJoyID);
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex(uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        
        // Operation not supported.
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_UNSUPPORTED_XINPUT_OPERATION_FORMAT, "joyGetThreshold");
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyGetThreshold((UINT)realJoyID, puThreshold);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoyReleaseCapture(UINT uJoyID)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joyReleaseCapture", (unsigned int)uJoyID);
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex(uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        
        // Operation not supported.
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_UNSUPPORTED_XINPUT_OPERATION_FORMAT, "joyReleaseCapture");
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joyReleaseCapture((UINT)realJoyID);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joySetCapture", (unsigned int)uJoyID);
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex(uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        
        // Operation not supported.
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_UNSUPPORTED_XINPUT_OPERATION_FORMAT, "joySetCapture");
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joySetCapture(hwnd, (UINT)realJoyID, uPeriod, fChanged);
    }
}

// ---------

MMRESULT WrapperJoyWinMM::JoySetThreshold(UINT uJoyID, UINT uThreshold)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_WRAPPERJOYWINMM_LOG_OPERATION_FORMAT, "joySetThreshold", (unsigned int)uJoyID);
    
    Initialize();
    const int realJoyID = TranslateApplicationJoyIndex(uJoyID);
    
    if (realJoyID < 0)
    {
        // Querying an XInput controller.
        
        // Operation not supported.
        Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_WRAPPERJOYWINMM_UNSUPPORTED_XINPUT_OPERATION_FORMAT, "joySetThreshold");
        return JOYERR_NOCANDO;
    }
    else
    {
        // Querying a non-XInput controller.
        return ImportApiWinMM::joySetThreshold((UINT)realJoyID, uThreshold);
    }
}
