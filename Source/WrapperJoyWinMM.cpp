/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file WrapperJoyWinMM.cpp
 *   Implementation of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ApiDirectInput.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "ImportApiWinMM.h"
#include "Message.h"
#include "VirtualController.h"
#include "WrapperJoyWinMM.h"

#include <climits>
#include <RegStr.h>
#include <string>
#include <utility>
#include <vector>


// -------- MACROS --------------------------------------------------------- //

/// Logs a WinMM device-specific function invocation.
#define LOG_INVOCATION(joyID, result)       Message::OutputFormatted(Message::ESeverity::Info, L"Invoked %s on device %d, result = %u.", __FUNCTIONW__ L"()", joyID, result);

/// Logs invocation of an unsupported WinMM operation.
#define LOG_UNSUPPORTED_OPERATION()         Message::OutputFormatted(Message::ESeverity::Warning, L"Application invoked %s on a Xidi virtual device, which is not supported.", __FUNCTIONW__ L"()");

/// Logs invocation of a WinMM operation with invalid parameters.
#define LOG_INVALID_PARAMS()                Message::OutputFormatted(Message::ESeverity::Warning, L"Application invoked %s on a Xidi virtual device, which failed due to invalid parameters.", __FUNCTIONW__ L"()");


namespace Xidi
{
    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Templated wrapper around the imported `joyGetDevCaps` WinMM function, which ordinarily exists in a Unicode and non-Unicode version separately.
    template <typename JoyCapsType> static inline MMRESULT ImportedJoyGetDevCaps(UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc)
    {
        return JOYERR_NOCANDO;
    }

    template <> static inline MMRESULT ImportedJoyGetDevCaps<JOYCAPSA>(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
    {
        return ImportApiWinMM::joyGetDevCapsA(uJoyID, pjc, cbjc);
    }

    template <> static inline MMRESULT ImportedJoyGetDevCaps<JOYCAPSW>(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
    {
        return ImportApiWinMM::joyGetDevCapsW(uJoyID, pjc, cbjc);
    }


    // -------- LOCAL TYPES ------------------------------------------------ //

    // Used to provide all information needed to get a list of XInput devices exposed by WinMM.
    struct SWinMMEnumCallbackInfo
    {
        std::vector<std::pair<std::wstring, bool>>* systemDeviceInfo;
        IDirectInput8* directInputInterface;
    };


    // -------- CLASS VARIABLES -------------------------------------------- //
    // See "WrapperJoyWinMM.h" for documentation.

    VirtualController* WrapperJoyWinMM::controllers[4];

    BOOL WrapperJoyWinMM::isInitialized = FALSE;

    std::vector<int> WrapperJoyWinMM::joyIndexMap;

    std::vector<std::pair<std::wstring, bool>> WrapperJoyWinMM::joySystemDeviceInfo;


    // -------- CLASS METHODS ---------------------------------------------- //
    // See "WrapperJoyWinMM.h" for documentation.

    void WrapperJoyWinMM::Initialize(void)
    {
        if (FALSE == isInitialized)
        {
            const Controller::Mapper* mapper = Controller::Mapper::GetConfigured();
            if (nullptr == mapper)
            {
                Message::Output(Message::ESeverity::Error, L"Failed to locate a mapper. Xidi virtual controllers will not function.");
                return;
            }

            for (int i = 0; i < _countof(controllers); ++i)
            {
                controllers[i] = new VirtualController(i, *mapper);
                
                //controllers[i]->SetAllAxisDeadzone(1000);
                //controllers[i]->SetAllAxisSaturation(9000);
                controllers[i]->SetAllAxisRange(0, 65535);
            }

            // Enumerate all devices exposed by WinMM.
            CreateSystemDeviceInfo();

            // Initialize the joystick index map.
            CreateJoyIndexMap();

            // Ensure all controllers have their names published in the system registry.
            SetControllerNameRegistryInfo();

            // Initialization complete.
            Message::Output(Message::ESeverity::Info, L"Completed initialization of WinMM joystick wrapper.");
            isInitialized = TRUE;
        }
    }

    // -------- HELPERS ---------------------------------------------------- //
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
        Message::OutputFormatted(Message::ESeverity::Debug, L"Presenting the system with these WinMM devices:");

        if ((false == joySystemDeviceInfo[0].second) && !(joySystemDeviceInfo[0].first.empty()))
        {
            // Preferred device is present but does not support XInput.
            // Filter out all XInput devices, but ensure Xidi virtual devices are mapped to the end.

            for (int i = 0; i < (int)numDevicesFromSystem; ++i)
            {
                if ((false == joySystemDeviceInfo[i].second) && !(joySystemDeviceInfo[i].first.empty()))
                {
                    Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: System-supplied WinMM device %u", (unsigned int)joyIndexMap.size(), (unsigned int)i);
                    joyIndexMap.push_back(i);
                }
            }

            for (int i = 0; i < (int)numXInputVirtualDevices; ++i)
            {
                Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: Xidi virtual device for player %u", (unsigned int)joyIndexMap.size(), (unsigned int)(i + 1));
                joyIndexMap.push_back(-(i + 1));
            }
        }
        else
        {
            // Preferred device supports XInput or is not present.
            // Filter out all XInput devices and present Xidi virtual devices at the start.

            for (int i = 0; i < (int)numXInputVirtualDevices; ++i)
            {
                Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: Xidi virtual device for player %u", (unsigned int)joyIndexMap.size(), (unsigned int)(i + 1));
                joyIndexMap.push_back(-(i + 1));
            }

            for (int i = 0; i < (int)numDevicesFromSystem; ++i)
            {
                if ((false == joySystemDeviceInfo[i].second) && !(joySystemDeviceInfo[i].first.empty()))
                {
                    Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: System-supplied WinMM device %u", (unsigned int)joyIndexMap.size(), (unsigned int)i);
                    joyIndexMap.push_back(i);
                }
            }
        }
    }

    // --------

    void WrapperJoyWinMM::CreateSystemDeviceInfo(void)
    {
        const size_t numDevicesFromSystem = (size_t)ImportApiWinMM::joyGetNumDevs();
        Message::OutputFormatted(Message::ESeverity::Debug, L"System provides %u WinMM devices.", (unsigned int)numDevicesFromSystem);

        // Initialize the system device information data structure.
        joySystemDeviceInfo.clear();
        joySystemDeviceInfo.reserve(numDevicesFromSystem);

        // Figure out the registry key that needs to be opened and open it.
        JOYCAPS joyCaps;
        HKEY registryKey;
        if (JOYERR_NOERROR != ImportApiWinMM::joyGetDevCaps((UINT_PTR)-1, &joyCaps, sizeof(joyCaps)))
        {
            Message::Output(Message::ESeverity::Warning, L"Unable to enumerate system WinMM devices because the correct registry key could not be identified by the system.");
            return;
        }

        wchar_t registryPath[1024];
        swprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG L"\\%s\\" REGSTR_KEY_JOYCURR, joyCaps.szRegKey);
        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, nullptr, REG_OPTION_VOLATILE, KEY_QUERY_VALUE, nullptr, &registryKey, nullptr))
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Unable to enumerate system WinMM devices because the registry key \"%s\" could not be opened.", registryPath);
            return;
        }

        // For each joystick device available in the system, see if it is present and, if so, get its device identifier (vendor ID and product ID string).
        Message::Output(Message::ESeverity::Debug, L"Enumerating system WinMM devices...");

        for (size_t i = 0; i < numDevicesFromSystem; ++i)
        {
            // Get the device capabilities. If this fails, the device is not present and can be skipped.
            if (JOYERR_NOERROR != ImportApiWinMM::joyGetDevCaps((UINT_PTR)i, &joyCaps, sizeof(joyCaps)))
            {
                joySystemDeviceInfo.push_back({ L"", false });
                Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: (not present - failed to get capabilities)", (unsigned int)i);
                continue;
            }

            // Use the registry to get device vendor ID and product ID string.
            wchar_t registryValueName[64];
            swprintf_s(registryValueName, _countof(registryValueName), REGSTR_VAL_JOYNOEMNAME, ((int)i + 1));

            wchar_t registryValueData[64];
            DWORD registryValueSize = sizeof(registryValueData);
            if (ERROR_SUCCESS != RegGetValue(registryKey, nullptr, registryValueName, RRF_RT_REG_SZ, nullptr, registryValueData, &registryValueSize))
            {
                // If the registry value does not exist, this is past the end of the number of devices WinMM sees.
                joySystemDeviceInfo.push_back({ L"", false });
                Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: (not present - failed to get vendor and product ID strings)", (unsigned int)i);
                continue;
            }

            // Add the vendor ID and product ID string to the list.
            joySystemDeviceInfo.push_back({ registryValueData, false });
            Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: %s", (unsigned int)i, registryValueData);
        }

        Message::Output(Message::ESeverity::Debug, L"Done enumerating system WinMM devices.");
        RegCloseKey(registryKey);

        // Enumerate all devices using DirectInput8 to find any XInput devices with matching vendor and product identifiers.
        // This will provide information on whether each WinMM device supports XInput.
        Message::Output(Message::ESeverity::Debug, L"Using DirectInput to detect XInput devices...");
        IDirectInput8* directInputInterface = nullptr;
        if (S_OK != ImportApiDirectInput::DirectInput8Create(Globals::GetInstanceHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&directInputInterface, nullptr))
        {
            Message::Output(Message::ESeverity::Debug, L"Unable to detect XInput devices because a DirectInput interface object could not be created.");
            return;
        }

        SWinMMEnumCallbackInfo callbackInfo;
        callbackInfo.systemDeviceInfo = &joySystemDeviceInfo;
        callbackInfo.directInputInterface = directInputInterface;
        if (S_OK != directInputInterface->EnumDevices(DI8DEVCLASS_GAMECTRL, CreateSystemDeviceInfoEnumCallback, (LPVOID)&callbackInfo, 0))
        {
            Message::Output(Message::ESeverity::Debug, L"Unable to detect XInput devices because enumeration of DirectInput devices failed.");
            return;
        }

        Message::Output(Message::ESeverity::Debug, L"Done detecting XInput devices.");
    }

    // --------

    BOOL STDMETHODCALLTYPE WrapperJoyWinMM::CreateSystemDeviceInfoEnumCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
    {
        SWinMMEnumCallbackInfo* callbackInfo = (SWinMMEnumCallbackInfo*)pvRef;

        std::wstring devicePath;
        BOOL deviceSupportsXInput = ControllerIdentification::DoesDirectInputControllerSupportXInput<EarliestIDirectInput, EarliestIDirectInputDevice>(callbackInfo->directInputInterface, lpddi->guidInstance, &devicePath);

        if (deviceSupportsXInput)
        {
            const WCHAR* devicePathString = devicePath.c_str();

            // Skip to the part of the path string that identifies vendor and product.
            const wchar_t* devicePathSubstring = wcsstr(devicePathString, L"VID_");
            if (nullptr == devicePathSubstring) devicePathSubstring = wcsstr(devicePathString, L"vid_");
            if (nullptr != devicePathSubstring)
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
                    if (0 == _wcsnicmp(callbackInfo->systemDeviceInfo->at(i).first.c_str(), devicePathSubstring, callbackInfo->systemDeviceInfo->at(i).first.length()))
                    {
                        callbackInfo->systemDeviceInfo->at(i).second = true;
                        Message::OutputFormatted(Message::ESeverity::Debug, L"    [%u]: XInput device", (unsigned int)i);
                    }
                }
            }
        }

        return DIENUM_CONTINUE;
    }

    // --------

    template <> int WrapperJoyWinMM::FillRegistryKeyString<LPSTR>(LPSTR buf, const size_t bufcount)
    {
        return LoadStringA(Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, buf, (int)bufcount);
    }

    template <> int WrapperJoyWinMM::FillRegistryKeyString<LPWSTR>(LPWSTR buf, const size_t bufcount)
    {
        return LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, buf, (int)bufcount);
    }

    // ---------

    void WrapperJoyWinMM::SetControllerNameRegistryInfo(void)
    {
        HKEY registryKey;
        LSTATUS result;
        wchar_t registryKeyName[128];
        wchar_t registryPath[1024];

        FillRegistryKeyString(registryKeyName, _countof(registryKeyName));

        // Place the names into the correct spots for the application to read.
        // These will be in HKCU\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\Xidi# and contain the name of the controller.
        for (DWORD i = 0; i < _countof(controllers); ++i)
        {
            wchar_t valueData[64];
            const int valueDataCount = ControllerIdentification::FillXInputControllerName(valueData, _countof(valueData), i);

            swprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYOEM L"\\%s%u", registryKeyName, i + 1);
            result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, nullptr, REG_OPTION_VOLATILE, KEY_SET_VALUE, nullptr, &registryKey, nullptr);
            if (ERROR_SUCCESS != result) return;

            result = RegSetValueEx(registryKey, REGSTR_VAL_JOYOEMNAME, 0, REG_SZ, (const BYTE*)valueData, (sizeof(wchar_t) * (valueDataCount + 1)));
            RegCloseKey(registryKey);

            if (ERROR_SUCCESS != result) return;
        }

        // Next, add OEM string references to HKCU\System\CurrentControlSet\Control\MediaResources\Joystick\Xidi.
        // These will point a WinMM-based application to another part of the registry, by reference, which actually contain the names.
        // Do this by consuming the joystick index map and system device information data structure.
        swprintf_s(registryPath, _countof(registryPath), REGSTR_PATH_JOYCONFIG L"\\%s\\" REGSTR_KEY_JOYCURR, registryKeyName);

        result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, nullptr, REG_OPTION_VOLATILE, KEY_SET_VALUE, nullptr, &registryKey, nullptr);
        if (ERROR_SUCCESS != result) return;

        for (DWORD i = 0; i < joyIndexMap.size(); ++i)
        {
            wchar_t valueName[64];
            const int valueNameCount = swprintf_s(valueName, _countof(valueName), REGSTR_VAL_JOYNOEMNAME, (i + 1));

            if (joyIndexMap[i] < 0)
            {
                // Map points to a Xidi virtual device.

                // Index is just -1 * the value in the map.
                // Use this value to create the correct string to write to the registry.
                wchar_t valueData[64];
                const int valueDataCount = swprintf_s(valueData, _countof(valueData), L"%s%u", registryKeyName, ((UINT)(-joyIndexMap[i])));

                // Write the value to the registry.
                RegSetValueEx(registryKey, valueName, 0, REG_SZ, (const BYTE*)valueData, (sizeof(wchar_t) * (valueDataCount + 1)));
            }
            else
            {
                // Map points to a non-Xidi device.

                // Just reference the string directly.
                const wchar_t* valueData = joySystemDeviceInfo[joyIndexMap[i]].first.c_str();
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


    // -------- METHODS: WinMM JOYSTICK ------------------------------------ //
    // See WinMM documentation for more information.

    MMRESULT WrapperJoyWinMM::JoyConfigChanged(DWORD dwFlags)
    {
        Message::Output(Message::ESeverity::Info, L"Refreshing joystick state due to a configuration change.");
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

    template <typename JoyCapsType> MMRESULT WrapperJoyWinMM::JoyGetDevCaps(UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc)
    {
        // Special case: index is specified as -1, which the API says just means fill in the registry key.
        if ((UINT_PTR)-1 == uJoyID)
        {
            FillRegistryKeyString(pjc->szRegKey, _countof(pjc->szRegKey));

            const MMRESULT result = JOYERR_NOERROR;
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }

        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex((UINT)uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.
            const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

            if (sizeof(*pjc) != cbjc)
            {
                const MMRESULT result = JOYERR_PARMS;
                LOG_INVALID_PARAMS();
                LOG_INVOCATION((unsigned int)uJoyID, result);
                return result;
            }

            const Controller::SCapabilities& controllerCapabilities = controllers[xJoyID]->GetCapabilities();

            ZeroMemory(pjc, sizeof(*pjc));
            pjc->wMaxAxes = (WORD)controllerCapabilities.numAxes;
            pjc->wMaxButtons = (WORD)controllerCapabilities.numButtons;
            pjc->wNumAxes = (WORD)controllerCapabilities.numAxes;
            pjc->wNumButtons = (WORD)controllerCapabilities.numButtons;
            pjc->wXmin = 0;
            pjc->wXmax = 65535;
            pjc->wYmin = 0;
            pjc->wYmax = 65535;
            pjc->wZmin = 0;
            pjc->wZmax = 65535;
            pjc->wRmin = 0;
            pjc->wRmax = 65535;
            pjc->wUmin = 0;
            pjc->wUmax = 65535;
            pjc->wVmin = 0;
            pjc->wVmax = 65535;

            if (true == controllerCapabilities.hasPov)
                pjc->wCaps = JOYCAPS_HASPOV | JOYCAPS_POVCTS;

            if (true == controllerCapabilities.HasAxis(Controller::EAxis::Z))
                pjc->wCaps |= JOYCAPS_HASZ;

            if (true == controllerCapabilities.HasAxis(Controller::EAxis::RotZ))
                pjc->wCaps |= JOYCAPS_HASR;

            if (true == controllerCapabilities.HasAxis(Controller::EAxis::RotY))
                pjc->wCaps |= JOYCAPS_HASU;

            if (true == controllerCapabilities.HasAxis(Controller::EAxis::RotX))
                pjc->wCaps |= JOYCAPS_HASV;

            FillRegistryKeyString(pjc->szRegKey, _countof(pjc->szRegKey));
            ControllerIdentification::FillXInputControllerName(pjc->szPname, _countof(pjc->szPname), xJoyID);

            const MMRESULT result = JOYERR_NOERROR;
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
        else
        {
            // Querying a non-XInput controller.
            // Replace the registry key but otherwise leave the response unchanged.
            MMRESULT result = ImportedJoyGetDevCaps((UINT_PTR)realJoyID, pjc, cbjc);

            if (JOYERR_NOERROR == result)
                FillRegistryKeyString(pjc->szRegKey, _countof(pjc->szRegKey));

            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }

    template MMRESULT WrapperJoyWinMM::JoyGetDevCaps(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
    template MMRESULT WrapperJoyWinMM::JoyGetDevCaps(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);

    // ---------

    UINT WrapperJoyWinMM::JoyGetNumDevs(void)
    {
        Initialize();

        // Number of controllers = number of XInput controllers + number of driver-reported controllers.
        UINT result = (UINT)joyIndexMap.size();
        Message::OutputFormatted(Message::ESeverity::Debug, L"Invoked %s, result = %u.", __FUNCTIONW__ L"()", result);
        return result;
    }

    // ---------

    MMRESULT WrapperJoyWinMM::JoyGetPos(UINT uJoyID, LPJOYINFO pji)
    {
        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.
            const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

            const Controller::SState kJoyStateData = controllers[xJoyID]->GetState();

            pji->wXpos = (WORD)kJoyStateData.axis[(int)Controller::EAxis::X];
            pji->wYpos = (WORD)kJoyStateData.axis[(int)Controller::EAxis::Y];
            pji->wZpos = (WORD)kJoyStateData.axis[(int)Controller::EAxis::Z];
            pji->wButtons = 0;
            if (true == kJoyStateData.button[0])
                pji->wButtons |= JOY_BUTTON1;
            if (true == kJoyStateData.button[1])
                pji->wButtons |= JOY_BUTTON2;
            if (true == kJoyStateData.button[2])
                pji->wButtons |= JOY_BUTTON3;
            if (true == kJoyStateData.button[3])
                pji->wButtons |= JOY_BUTTON4;

            const MMRESULT result = JOYERR_NOERROR;
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
        else
        {
            // Querying a non-XInput controller.
            const MMRESULT result = ImportApiWinMM::joyGetPos((UINT)realJoyID, pji);
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }

    // ---------

    MMRESULT WrapperJoyWinMM::JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
    {
        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.
            const DWORD xJoyID = (DWORD)((-realJoyID) - 1);

            if (sizeof(*pji) != pji->dwSize)
            {
                MMRESULT result = JOYERR_PARMS;
                LOG_INVALID_PARAMS();
                LOG_INVOCATION((unsigned int)uJoyID, result);
                return result;
            }

            const Controller::SState kJoyStateData = controllers[xJoyID]->GetState();
            const EPovValue kJoyStateDataPovValue = DataFormat::DirectInputPovValue(kJoyStateData.povDirection);

            // Fill in the provided structure.
            // WinMM uses only 16 bits to indicate that the dpad is centered, whereas it is safe to use all 32 in DirectInput, hence the conversion (forgetting this can introduce bugs into games).
            pji->dwPOV = (EPovValue::Center == kJoyStateDataPovValue ? (DWORD)(JOY_POVCENTERED) : (DWORD)kJoyStateDataPovValue);
            pji->dwXpos = kJoyStateData.axis[(int)Controller::EAxis::X];
            pji->dwYpos = kJoyStateData.axis[(int)Controller::EAxis::Y];
            pji->dwZpos = kJoyStateData.axis[(int)Controller::EAxis::Z];
            pji->dwRpos = kJoyStateData.axis[(int)Controller::EAxis::RotZ];
            pji->dwUpos = kJoyStateData.axis[(int)Controller::EAxis::RotY];
            pji->dwVpos = kJoyStateData.axis[(int)Controller::EAxis::RotX];
            pji->dwButtons = 0;
            for (DWORD i = 0; i < kJoyStateData.button.size(); ++i)
            {
                if (true == kJoyStateData.button[i])
                    pji->dwButtons |= (1 << i);
            }

            const MMRESULT result = JOYERR_NOERROR;
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
        else
        {
            // Querying a non-XInput controller.
            const MMRESULT result = ImportApiWinMM::joyGetPosEx((UINT)realJoyID, pji);
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }

    // ---------

    MMRESULT WrapperJoyWinMM::JoyGetThreshold(UINT uJoyID, LPUINT puThreshold)
    {
        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.

            // Operation not supported.
            const MMRESULT result = JOYERR_NOCANDO;
            LOG_UNSUPPORTED_OPERATION();
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return JOYERR_NOCANDO;
        }
        else
        {
            // Querying a non-XInput controller.
            const MMRESULT result = ImportApiWinMM::joyGetThreshold((UINT)realJoyID, puThreshold);
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }

    // ---------

    MMRESULT WrapperJoyWinMM::JoyReleaseCapture(UINT uJoyID)
    {
        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.

            // Operation not supported.
            const MMRESULT result = JOYERR_NOCANDO;
            LOG_UNSUPPORTED_OPERATION();
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
        else
        {
            // Querying a non-XInput controller.
            const MMRESULT result = ImportApiWinMM::joyReleaseCapture((UINT)realJoyID);
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }

    // ---------

    MMRESULT WrapperJoyWinMM::JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
    {
        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.

            // Operation not supported.
            const MMRESULT result = JOYERR_NOCANDO;
            LOG_UNSUPPORTED_OPERATION();
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
        else
        {
            // Querying a non-XInput controller.
            const MMRESULT result = ImportApiWinMM::joySetCapture(hwnd, (UINT)realJoyID, uPeriod, fChanged);
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }

    // ---------

    MMRESULT WrapperJoyWinMM::JoySetThreshold(UINT uJoyID, UINT uThreshold)
    {
        Initialize();
        const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

        if (realJoyID < 0)
        {
            // Querying an XInput controller.

            // Operation not supported.
            const MMRESULT result = JOYERR_NOCANDO;
            LOG_UNSUPPORTED_OPERATION();
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
        else
        {
            // Querying a non-XInput controller.
            const MMRESULT result = ImportApiWinMM::joySetThreshold((UINT)realJoyID, uThreshold);
            LOG_INVOCATION((unsigned int)uJoyID, result);
            return result;
        }
    }
}
