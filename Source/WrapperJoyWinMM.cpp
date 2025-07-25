/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file WrapperJoyWinMM.cpp
 *   Implementation of the wrapper for all WinMM joystick functions.
 **************************************************************************************************/

#include "WrapperJoyWinMM.h"

#include <regstr.h>

#include <climits>
#include <cstdint>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "ImportApiWinMM.h"
#include "Strings.h"
#include "VirtualController.h"

/// Logs a WinMM device-specific function invocation.
#define LOG_INVOCATION(severity, joyID, result)                                                    \
  Infra::Message::OutputFormatted(                                                                 \
      severity, L"Invoked %s on device %d, result = %u.", __FUNCTIONW__ L"()", joyID, result)

/// Logs invocation of an unsupported WinMM operation.
#define LOG_UNSUPPORTED_OPERATION()                                                                \
  Infra::Message::OutputFormatted(                                                                 \
      Infra::Message::ESeverity::Warning,                                                          \
      L"Application invoked %s on a Xidi virtual controller, which is not supported.",             \
      __FUNCTIONW__ L"()")

/// Logs invocation of a WinMM operation with invalid parameters.
#define LOG_INVALID_PARAMS()                                                                           \
  Infra::Message::OutputFormatted(                                                                     \
      Infra::Message::ESeverity::Warning,                                                              \
      L"Application invoked %s on a Xidi virtual controller, which failed due to invalid parameters.", \
      __FUNCTIONW__ L"()")

namespace Xidi
{
  namespace WrapperJoyWinMM
  {
    /// Minimum axis range for a controller presented by WinMM.
    /// WinMM can sometimes use 16-bit unsigned values to represent axis values, so the range needs
    /// to be the representable set of values for that type.
    static constexpr int32_t kAxisRangeMin = 0;

    /// Maximum axis range for a controller presented by WinMM.
    /// WinMM can sometimes use 16-bit unsigned values to represent axis values, so the range needs
    /// to be the representable set of values for that type.
    static constexpr int32_t kAxisRangeMax = USHRT_MAX;

    /// Axis deadzone value to use for a controller presented by WinMM.
    /// WinMM has no way for applications to set deadzone, so use a small part of the axis just to
    /// enable some minor filtering but to avoid interfering with any applications that do their own
    /// filtering.
    static constexpr int32_t kAxisDeadzone = 750;

    /// Axis saturation value to use for a controller presented by WinMM.
    /// WinMM has no way for applications to set saturation, so use a small part of the axis just to
    /// enable some minor filtering but to avoid interfering with any applications that do their own
    /// filtering.
    static constexpr int32_t kAxisSaturation = 9250;

    // Used to provide all information needed to get a list of XInput devices exposed by WinMM.
    struct SWinMMEnumCallbackInfo
    {
      std::vector<std::pair<std::wstring, bool>>* systemDeviceInfo;
      IDirectInput8W* directInputInterface;
    };

    /// Fixed set of virtual controllers.
    static Controller::VirtualController* controllers[Controller::kPhysicalControllerCount];

    /// Maps from application-specified joystick index to the actual indices to present to WinMM or
    /// use internally. Negative values indicate XInput controllers, others indicate values to be
    /// passed to WinMM as is.
    static std::vector<int> joyIndexMap;

    /// Holds information about all devices WinMM makes available.
    /// String specifies the device identifier (vendor ID and product ID string), bool value
    /// specifies whether the device supports XInput.
    static std::vector<std::pair<std::wstring, bool>> joySystemDeviceInfo;

    /// Templated wrapper around the imported `joyGetDevCaps` WinMM function, which ordinarily
    /// exists in a Unicode and non-Unicode version separately.
    /// @tparam JoyCapsType Either JOYCAPSA or JOYCAPSW depending on whether ASCII or Unicode is
    /// desired.
    template <typename JoyCapsType> static inline MMRESULT ImportedJoyGetDevCaps(
        UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc);

    template <> static inline MMRESULT ImportedJoyGetDevCaps<JOYCAPSA>(
        UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
    {
      return ImportApiWinMM::joyGetDevCapsA(uJoyID, pjc, cbjc);
    }

    template <> static inline MMRESULT ImportedJoyGetDevCaps<JOYCAPSW>(
        UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
    {
      return ImportApiWinMM::joyGetDevCapsW(uJoyID, pjc, cbjc);
    }

    /// Templated wrapper around the `LoadString` Windows API function, which ordinarily exists in a
    /// Unicode and non-Unicode version separately.
    /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
    template <typename StringType> static inline int LoadResourceString(
        HINSTANCE hInstance, UINT uID, StringType lpBuffer, int cchBufferMax);

    template <> static inline int LoadResourceString<LPSTR>(
        HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax)
    {
      return LoadStringA(hInstance, uID, lpBuffer, cchBufferMax);
    }

    template <> static inline int LoadResourceString<LPWSTR>(
        HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int cchBufferMax)
    {
      return LoadStringW(hInstance, uID, lpBuffer, cchBufferMax);
    }

    /// Creates the joystick index map.
    /// Requires that the system device information data structure already be filled.
    /// If the user's preferred controller is absent or supports XInput, virtual devices are
    /// presented first, otherwise they are presented last. Any controllers that support XInput are
    /// removed from the mapping.
    static void CreateJoyIndexMap(void)
    {
      const uint64_t activeVirtualControllerMask =
          Globals::GetConfigurationData()
              [Strings::kStrConfigurationSectionWorkarounds]
              [Strings::kStrConfigurationSettingWorkaroundsActiveVirtualControllerMask]
                  .ValueOr(UINT64_MAX);

      const size_t numDevicesFromSystem = joySystemDeviceInfo.size();
      const size_t numXInputVirtualDevices = _countof(controllers);
      const size_t numDevicesTotal = numDevicesFromSystem + numXInputVirtualDevices;

      // Initialize the joystick index map with conservative defaults.
      // In the event of an error, it is safest to avoid enabling any Xidi virtual controllers to
      // prevent binding both to the WinMM version and the Xidi version of the same one.
      joyIndexMap.clear();
      joyIndexMap.reserve(numDevicesTotal);
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Debug,
          L"Presenting the application with these WinMM devices:");

      if ((false == joySystemDeviceInfo[0].second) && !(joySystemDeviceInfo[0].first.empty()))
      {
        // Preferred device is present but does not support XInput.
        // Filter out all XInput devices, but ensure Xidi virtual controllers are mapped to the end.

        for (int i = 0; i < (int)numDevicesFromSystem; ++i)
        {
          if ((false == joySystemDeviceInfo[i].second) && !(joySystemDeviceInfo[i].first.empty()))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]: System-supplied WinMM device %u",
                (unsigned int)joyIndexMap.size(),
                (unsigned int)i);
            joyIndexMap.push_back(i);
          }
        }

        for (int i = 0; i < (int)numXInputVirtualDevices; ++i)
        {
          if (0 != (activeVirtualControllerMask & ((uint64_t)1 << i)))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]: Xidi virtual controller %u",
                (unsigned int)joyIndexMap.size(),
                (unsigned int)(i + 1));
            joyIndexMap.push_back(-(i + 1));
          }
        }
      }
      else
      {
        // Preferred device supports XInput or is not present.
        // Filter out all XInput devices and present Xidi virtual controllers at the start.

        for (int i = 0; i < (int)numXInputVirtualDevices; ++i)
        {
          if (0 != (activeVirtualControllerMask & ((uint64_t)1 << i)))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]: Xidi virtual controller %u",
                (unsigned int)joyIndexMap.size(),
                (unsigned int)(i + 1));
            joyIndexMap.push_back(-(i + 1));
          }
        }

        for (int i = 0; i < (int)numDevicesFromSystem; ++i)
        {
          if ((false == joySystemDeviceInfo[i].second) && !(joySystemDeviceInfo[i].first.empty()))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]: System-supplied WinMM device %u",
                (unsigned int)joyIndexMap.size(),
                (unsigned int)i);
            joyIndexMap.push_back(i);
          }
        }
      }
    }

    /// Callback during DirectInput device enumeration.
    /// Used internally to detect which WinMM devices support XInput.
    static BOOL __stdcall CreateSystemDeviceInfoEnumCallback(
        LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
    {
      SWinMMEnumCallbackInfo* callbackInfo = (SWinMMEnumCallbackInfo*)pvRef;

      std::wstring devicePath;
      bool deviceSupportsXInput = DoesDirectInputControllerSupportSdlGamepad<EDirectInputVersion::k8W>(
          callbackInfo->directInputInterface, lpddi->guidInstance, &devicePath);

      if (deviceSupportsXInput)
      {
        // For each element of the WinMM devices list, see if the vendor and product IDs match the
        // one DirectInput presented as being compatible with XInput. If so, mark it in the list as
        // being an XInput controller.
        for (size_t i = 0; i < callbackInfo->systemDeviceInfo->size(); ++i)
        {
          // Already seen this device, skip.
          if (true == callbackInfo->systemDeviceInfo->at(i).second) continue;

          // No device at that position, skip.
          if (callbackInfo->systemDeviceInfo->at(i).first.empty()) continue;

          // Check for a matching vendor and product ID. If so, mark the device as supporting
          // XInput.
          if (true ==
              ApproximatelyEqualVendorAndProductId(
                  devicePath, callbackInfo->systemDeviceInfo->at(i).first)
                  .value_or(false))
          {
            callbackInfo->systemDeviceInfo->at(i).second = true;
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]: %s",
                (unsigned int)i,
                lpddi->tszProductName);
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]:     WinMM ID:       %s",
                (unsigned int)i,
                callbackInfo->systemDeviceInfo->at(i).first.c_str());
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"    [%u]:     DirectInput ID: %s",
                (unsigned int)i,
                devicePath.c_str());
          }
        }
      }

      return DIENUM_CONTINUE;
    }

    /// Fills in the system device info data structure with information from the registry and from
    /// DirectInput.
    static void CreateSystemDeviceInfo(void)
    {
      const size_t numDevicesFromSystem = (size_t)ImportApiWinMM::joyGetNumDevs();
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Debug,
          L"System provides %u WinMM devices.",
          (unsigned int)numDevicesFromSystem);

      // Initialize the system device information data structure.
      joySystemDeviceInfo.clear();
      joySystemDeviceInfo.reserve(numDevicesFromSystem);

      // Figure out the registry key that needs to be opened and open it.
      JOYCAPS joyCaps;
      HKEY registryKey;
      if (JOYERR_NOERROR != ImportApiWinMM::joyGetDevCaps((UINT_PTR)-1, &joyCaps, sizeof(joyCaps)))
      {
        Infra::Message::Output(
            Infra::Message::ESeverity::Warning,
            L"Unable to enumerate system WinMM devices because the correct registry key could not be identified by the system.");
        return;
      }

      wchar_t registryPath[1024];
      swprintf_s(
          registryPath,
          _countof(registryPath),
          REGSTR_PATH_JOYCONFIG L"\\%s\\" REGSTR_KEY_JOYCURR,
          joyCaps.szRegKey);
      if (ERROR_SUCCESS !=
          RegCreateKeyEx(
              HKEY_CURRENT_USER,
              registryPath,
              0,
              nullptr,
              REG_OPTION_VOLATILE,
              KEY_QUERY_VALUE,
              nullptr,
              &registryKey,
              nullptr))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Warning,
            L"Unable to enumerate system WinMM devices because the registry key \"%s\" could not be opened.",
            registryPath);
        return;
      }

      // For each joystick device available in the system, see if it is present and, if so, get its
      // device identifier (vendor ID and product ID string).
      Infra::Message::Output(
          Infra::Message::ESeverity::Debug, L"Enumerating system WinMM devices...");

      for (size_t i = 0; i < numDevicesFromSystem; ++i)
      {
        // Get the device capabilities. If this fails, the device is not present and can be skipped.
        if (JOYERR_NOERROR != ImportApiWinMM::joyGetDevCaps((UINT_PTR)i, &joyCaps, sizeof(joyCaps)))
        {
          joySystemDeviceInfo.push_back({L"", false});
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Debug,
              L"    [%u]: (not present - failed to get capabilities)",
              (unsigned int)i);
          continue;
        }

        // Use the registry to get device vendor ID and product ID string.
        wchar_t registryValueName[64];
        swprintf_s(
            registryValueName, _countof(registryValueName), REGSTR_VAL_JOYNOEMNAME, ((int)i + 1));

        wchar_t registryValueData[64];
        DWORD registryValueSize = sizeof(registryValueData);
        if (ERROR_SUCCESS !=
            RegGetValue(
                registryKey,
                nullptr,
                registryValueName,
                RRF_RT_REG_SZ,
                nullptr,
                registryValueData,
                &registryValueSize))
        {
          // If the registry value does not exist, this is past the end of the number of devices
          // WinMM sees.
          joySystemDeviceInfo.push_back({L"", false});
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Debug,
              L"    [%u]: (not present - failed to get vendor and product ID strings)",
              (unsigned int)i);
          continue;
        }

        // Add the vendor ID and product ID string to the list.
        joySystemDeviceInfo.push_back({registryValueData, false});
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Debug, L"    [%u]: %s", (unsigned int)i, registryValueData);
      }

      Infra::Message::Output(
          Infra::Message::ESeverity::Debug, L"Done enumerating system WinMM devices.");
      RegCloseKey(registryKey);

      // Enumerate all devices using DirectInput8 to find any XInput devices with matching vendor
      // and product identifiers. This will provide information on whether each WinMM device
      // supports XInput.
      Infra::Message::Output(
          Infra::Message::ESeverity::Debug, L"Using DirectInput to detect XInput devices...");
      IDirectInput8W* directInputInterface = nullptr;
      if (S_OK !=
          ImportApiDirectInput::Version8::DirectInput8Create(
              Infra::ProcessInfo::GetThisModuleInstanceHandle(),
              DIRECTINPUT_VERSION,
              IID_IDirectInput8,
              (LPVOID*)&directInputInterface,
              nullptr))
      {
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug,
            L"Unable to detect XInput devices because a DirectInput interface object could not be created.");
        return;
      }

      SWinMMEnumCallbackInfo callbackInfo;
      callbackInfo.systemDeviceInfo = &joySystemDeviceInfo;
      callbackInfo.directInputInterface = directInputInterface;
      if (S_OK !=
          directInputInterface->EnumDevices(
              DI8DEVCLASS_GAMECTRL, CreateSystemDeviceInfoEnumCallback, (LPVOID)&callbackInfo, 0))
      {
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug,
            L"Unable to detect XInput devices because enumeration of DirectInput devices failed.");
        return;
      }

      Infra::Message::Output(Infra::Message::ESeverity::Debug, L"Done detecting XInput devices.");
    }

    /// Fills in the specified buffer with the name of the registry key to use for referencing
    /// controller names.
    /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
    /// @param [out] buf Buffer to be filled.
    /// @param [in] bufcount Number of characters that the buffer can hold.
    /// @return Number of characters written, or negative in the event of an error.
    template <typename StringType> static inline int FillRegistryKeyString(
        StringType buf, const size_t bufcount)
    {
      return LoadResourceString(
          Infra::ProcessInfo::GetThisModuleInstanceHandle(),
          IDS_XIDI_PRODUCT_NAME,
          buf,
          (int)bufcount);
    }

    /// Places the required keys and values into the registry so that WinMM-based applications can
    /// find the correct controller names. Consumes the system device information data structure.
    static void SetControllerNameRegistryInfo(void)
    {
      HKEY registryKey;
      LSTATUS result;
      wchar_t registryKeyName[128];
      wchar_t registryPath[1024];

      FillRegistryKeyString(registryKeyName, _countof(registryKeyName));

      // Place the names into the correct spots for the application to read.
      // These will be in
      // HKCU\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\Xidi#
      // and contain the name of the controller.
      for (int i = 0; i < _countof(controllers); ++i)
      {
        wchar_t valueData[64];
        const int valueDataCount = FillVirtualControllerName(
            valueData, _countof(valueData), (Controller::TControllerIdentifier)i);

        swprintf_s(
            registryPath,
            _countof(registryPath),
            REGSTR_PATH_JOYOEM L"\\%s%u",
            registryKeyName,
            i + 1);
        result = RegCreateKeyEx(
            HKEY_CURRENT_USER,
            registryPath,
            0,
            nullptr,
            REG_OPTION_VOLATILE,
            KEY_SET_VALUE,
            nullptr,
            &registryKey,
            nullptr);
        if (ERROR_SUCCESS != result) return;

        result = RegSetValueEx(
            registryKey,
            REGSTR_VAL_JOYOEMNAME,
            0,
            REG_SZ,
            (const BYTE*)valueData,
            (sizeof(wchar_t) * (valueDataCount + 1)));
        RegCloseKey(registryKey);

        if (ERROR_SUCCESS != result) return;
      }

      // Next, add OEM string references to
      // HKCU\System\CurrentControlSet\Control\MediaResources\Joystick\Xidi. These will point a
      // WinMM-based application to another part of the registry, by reference, which actually
      // contain the names. Do this by consuming the joystick index map and system device
      // information data structure.
      swprintf_s(
          registryPath,
          _countof(registryPath),
          REGSTR_PATH_JOYCONFIG L"\\%s\\" REGSTR_KEY_JOYCURR,
          registryKeyName);

      result = RegCreateKeyEx(
          HKEY_CURRENT_USER,
          registryPath,
          0,
          nullptr,
          REG_OPTION_VOLATILE,
          KEY_SET_VALUE,
          nullptr,
          &registryKey,
          nullptr);
      if (ERROR_SUCCESS != result) return;

      for (size_t i = 0; i < joyIndexMap.size(); ++i)
      {
        wchar_t valueName[64];
        const int valueNameCount =
            swprintf_s(valueName, _countof(valueName), REGSTR_VAL_JOYNOEMNAME, ((int)i + 1));

        if (joyIndexMap[i] < 0)
        {
          // Map points to a Xidi virtual controller.

          // Index is just -1 * the value in the map.
          // Use this value to create the correct string to write to the registry.
          wchar_t valueData[64];
          const int valueDataCount = swprintf_s(
              valueData, _countof(valueData), L"%s%u", registryKeyName, ((UINT)(-joyIndexMap[i])));

          // Write the value to the registry.
          RegSetValueEx(
              registryKey,
              valueName,
              0,
              REG_SZ,
              (const BYTE*)valueData,
              (sizeof(wchar_t) * (valueDataCount + 1)));
        }
        else
        {
          // Map points to a non-Xidi device.

          // Just reference the string directly.
          const wchar_t* valueData = joySystemDeviceInfo[joyIndexMap[i]].first.c_str();
          const int valueDataCount = (int)joySystemDeviceInfo[joyIndexMap[i]].first.length();

          // Write the value to the registry.
          RegSetValueEx(
              registryKey,
              valueName,
              0,
              REG_SZ,
              (const BYTE*)valueData,
              (sizeof(joySystemDeviceInfo[joyIndexMap[i]].first[0]) * (valueDataCount + 1)));
        }
      }
    }

    /// Translates an application-supplied joystick index to an internal joystick index using the
    /// map.
    /// @param [in] uJoyID WinMM joystick ID supplied by the application.
    /// @return Internal joystick index to either handle or pass to WinMM.
    static int TranslateApplicationJoyIndex(UINT uJoyID)
    {
      if (joyIndexMap.size() <= (size_t)uJoyID)
        return INT_MAX;
      else
        return joyIndexMap[uJoyID];
    }

    /// Initializes all WinMM functionality.
    static void Initialize(void)
    {
      // There is overhead to using call_once, even after the operation is completed, and WinMM
      // wrapper functions are called frequently. Using this additional flag avoids that overhead in
      // the common case.
      static bool isInitialized = false;
      if (true == isInitialized) return;

      static std::once_flag initializationFlag;
      std::call_once(
          initializationFlag,
          []() -> void
          {
            const bool enableAxisProperites =
                Globals::GetConfigurationData()
                    [Strings::kStrConfigurationSectionProperties]
                    [Strings::kStrConfigurationSettingsPropertiesUseBuiltinProperties]
                        .ValueOr(true);
            const uint64_t activeVirtualControllerMask =
                Globals::GetConfigurationData()
                    [Strings::kStrConfigurationSectionWorkarounds]
                    [Strings::kStrConfigurationSettingWorkaroundsActiveVirtualControllerMask]
                        .ValueOr(UINT64_MAX);

            for (Controller::TControllerIdentifier i = 0; i < _countof(controllers); ++i)
            {
              controllers[i] = nullptr;

              if (0 != (activeVirtualControllerMask & ((uint64_t)1 << i)))
              {
                controllers[i] = new Controller::VirtualController(i);
                controllers[i]->SetAllAxisRange(kAxisRangeMin, kAxisRangeMax);

                if (enableAxisProperites)
                {
                  controllers[i]->SetAllAxisDeadzone(kAxisDeadzone);
                  controllers[i]->SetAllAxisSaturation(kAxisSaturation);
                }
              }
            }

            // Enumerate all devices exposed by WinMM.
            CreateSystemDeviceInfo();

            // Initialize the joystick index map.
            CreateJoyIndexMap();

            // Ensure all controllers have their names published in the system registry.
            SetControllerNameRegistryInfo();

            // Initialization complete.
            Infra::Message::Output(
                Infra::Message::ESeverity::Info,
                L"Completed initialization of WinMM joystick wrapper.");

            isInitialized = true;
          });
    }

    /// Templated implementation of the `joyGetDevCaps` function, allowing an "A" version and a "W"
    /// version to be exported separately.
    template <typename JoyCapsType> static inline MMRESULT JoyGetDevCapsInternal(
        UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc)
    {
      // Special case: index is specified as -1, which the API says just means fill in the registry
      // key.
      if ((UINT_PTR)-1 == uJoyID)
      {
        FillRegistryKeyString(pjc->szRegKey, _countof(pjc->szRegKey));

        const MMRESULT result = JOYERR_NOERROR;
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }

      const int realJoyID = TranslateApplicationJoyIndex((UINT)uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.
        const Controller::TControllerIdentifier xJoyID =
            (Controller::TControllerIdentifier)((-realJoyID) - 1);

        if (sizeof(*pjc) != cbjc)
        {
          const MMRESULT result = JOYERR_PARMS;
          LOG_INVALID_PARAMS();
          LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
          return result;
        }

        const Controller::SCapabilities controllerCapabilities =
            controllers[xJoyID]->GetCapabilities();

        ZeroMemory(pjc, sizeof(*pjc));
        pjc->wMaxAxes = (WORD)Controller::EAxis::Count;
        pjc->wMaxButtons = (WORD)Controller::EButton::Count;
        pjc->wNumAxes = (WORD)controllerCapabilities.numAxes;
        pjc->wNumButtons = (WORD)controllerCapabilities.numButtons;
        pjc->wXmin = kAxisRangeMin;
        pjc->wXmax = kAxisRangeMax;
        pjc->wYmin = kAxisRangeMin;
        pjc->wYmax = kAxisRangeMax;
        pjc->wZmin = kAxisRangeMin;
        pjc->wZmax = kAxisRangeMax;
        pjc->wRmin = kAxisRangeMin;
        pjc->wRmax = kAxisRangeMax;
        pjc->wUmin = kAxisRangeMin;
        pjc->wUmax = kAxisRangeMax;
        pjc->wVmin = kAxisRangeMin;
        pjc->wVmax = kAxisRangeMax;

        if (true == controllerCapabilities.HasPov()) pjc->wCaps = JOYCAPS_HASPOV | JOYCAPS_POVCTS;

        if (true == controllerCapabilities.HasAxis(Controller::EAxis::Z))
          pjc->wCaps |= JOYCAPS_HASZ;

        if (true == controllerCapabilities.HasAxis(Controller::EAxis::RotZ))
          pjc->wCaps |= JOYCAPS_HASR;

        if (true == controllerCapabilities.HasAxis(Controller::EAxis::RotY))
          pjc->wCaps |= JOYCAPS_HASU;

        if (true == controllerCapabilities.HasAxis(Controller::EAxis::RotX))
          pjc->wCaps |= JOYCAPS_HASV;

        FillRegistryKeyString(pjc->szRegKey, _countof(pjc->szRegKey));
        FillVirtualControllerName(pjc->szPname, _countof(pjc->szPname), xJoyID);

        const MMRESULT result = JOYERR_NOERROR;
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
      else
      {
        // Querying a non-XInput controller.
        // Replace the registry key but otherwise leave the response unchanged.
        MMRESULT result = ImportedJoyGetDevCaps((UINT_PTR)realJoyID, pjc, cbjc);

        if (JOYERR_NOERROR == result) FillRegistryKeyString(pjc->szRegKey, _countof(pjc->szRegKey));

        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
    }

    MMRESULT __stdcall joyConfigChanged(DWORD dwFlags)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Info,
          L"Refreshing joystick state due to a configuration change.");
      Initialize();

      // Redirect to the imported API so that its view of the registry can be updated.
      HRESULT result = ImportApiWinMM::joyConfigChanged(dwFlags);

      // Update Xidi's view of devices.
      CreateSystemDeviceInfo();
      CreateJoyIndexMap();
      SetControllerNameRegistryInfo();

      return result;
    }

    MMRESULT __stdcall joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
    {
      Initialize();
      return JoyGetDevCapsInternal(uJoyID, pjc, cbjc);
    }

    MMRESULT __stdcall joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
    {
      Initialize();
      return JoyGetDevCapsInternal(uJoyID, pjc, cbjc);
    }

    UINT __stdcall joyGetNumDevs(void)
    {
      Initialize();

      // Number of controllers = number of XInput controllers + number of driver-reported
      // controllers.
      UINT result = (UINT)joyIndexMap.size();
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Debug,
          L"Invoked %s, result = %u.",
          __FUNCTIONW__ L"()",
          result);
      return result;
    }

    MMRESULT __stdcall joyGetPos(UINT uJoyID, LPJOYINFO pji)
    {
      Initialize();
      const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.
        const Controller::TControllerIdentifier xJoyID =
            (Controller::TControllerIdentifier)((-realJoyID) - 1);

        const Controller::SState joyStateData = controllers[xJoyID]->GetState();

        pji->wXpos = (WORD)joyStateData[Controller::EAxis::X];
        pji->wYpos = (WORD)joyStateData[Controller::EAxis::Y];
        pji->wZpos = (WORD)joyStateData[Controller::EAxis::Z];
        pji->wButtons = 0;

        constexpr size_t maxButtonIndex = 8 * sizeof(pji->wButtons);
        for (size_t i = 0; ((i < joyStateData.button.size()) && (i < maxButtonIndex)); ++i)
        {
          if (true == joyStateData.button[i]) pji->wButtons |= (1 << i);
        }

        const MMRESULT result = JOYERR_NOERROR;
        LOG_INVOCATION(Infra::Message::ESeverity::SuperDebug, (unsigned int)uJoyID, result);
        return result;
      }
      else
      {
        // Querying a non-XInput controller.
        const MMRESULT result = ImportApiWinMM::joyGetPos((UINT)realJoyID, pji);
        LOG_INVOCATION(Infra::Message::ESeverity::SuperDebug, (unsigned int)uJoyID, result);
        return result;
      }
    }

    MMRESULT __stdcall joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
    {
      Initialize();
      const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.
        const Controller::TControllerIdentifier xJoyID =
            (Controller::TControllerIdentifier)((-realJoyID) - 1);

        if (sizeof(*pji) != pji->dwSize)
        {
          MMRESULT result = JOYERR_PARMS;
          LOG_INVALID_PARAMS();
          LOG_INVOCATION(Infra::Message::ESeverity::SuperDebug, (unsigned int)uJoyID, result);
          return result;
        }

        const Controller::SState joyStateData = controllers[xJoyID]->GetState();
        const EPovValue joyStateDataPovValue =
            DataFormat::DirectInputPovValue(joyStateData.povDirection);

        // Fill in the provided structure.
        // WinMM uses only 16 bits to indicate that the dpad is centered, whereas it is safe to
        // use all 32 in DirectInput, hence the conversion (forgetting this can introduce bugs
        // into games).
        pji->dwPOV =
            (EPovValue::Center == joyStateDataPovValue ? (DWORD)(JOY_POVCENTERED)
                                                       : (DWORD)joyStateDataPovValue);
        pji->dwXpos = joyStateData[Controller::EAxis::X];
        pji->dwYpos = joyStateData[Controller::EAxis::Y];
        pji->dwZpos = joyStateData[Controller::EAxis::Z];
        pji->dwRpos = joyStateData[Controller::EAxis::RotZ];
        pji->dwUpos = joyStateData[Controller::EAxis::RotY];
        pji->dwVpos = joyStateData[Controller::EAxis::RotX];
        pji->dwButtons = 0;

        constexpr size_t maxButtonIndex = 8 * sizeof(pji->dwButtons);
        for (size_t i = 0; ((i < joyStateData.button.size()) && (i < maxButtonIndex)); ++i)
        {
          if (true == joyStateData.button[i]) pji->dwButtons |= (1 << i);
        }

        const MMRESULT result = JOYERR_NOERROR;
        LOG_INVOCATION(Infra::Message::ESeverity::SuperDebug, (unsigned int)uJoyID, result);
        return result;
      }
      else
      {
        // Querying a non-XInput controller.
        const MMRESULT result = ImportApiWinMM::joyGetPosEx((UINT)realJoyID, pji);
        LOG_INVOCATION(Infra::Message::ESeverity::SuperDebug, (unsigned int)uJoyID, result);
        return result;
      }
    }

    MMRESULT __stdcall joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
    {
      Initialize();
      const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.

        // Operation not supported.
        const MMRESULT result = JOYERR_NOCANDO;
        LOG_UNSUPPORTED_OPERATION();
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return JOYERR_NOCANDO;
      }
      else
      {
        // Querying a non-XInput controller.
        const MMRESULT result = ImportApiWinMM::joyGetThreshold((UINT)realJoyID, puThreshold);
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
    }

    MMRESULT __stdcall joyReleaseCapture(UINT uJoyID)
    {
      Initialize();
      const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.

        // Operation not supported.
        const MMRESULT result = JOYERR_NOCANDO;
        LOG_UNSUPPORTED_OPERATION();
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
      else
      {
        // Querying a non-XInput controller.
        const MMRESULT result = ImportApiWinMM::joyReleaseCapture((UINT)realJoyID);
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
    }

    MMRESULT __stdcall joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
    {
      Initialize();
      const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.

        // Operation not supported.
        const MMRESULT result = JOYERR_NOCANDO;
        LOG_UNSUPPORTED_OPERATION();
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
      else
      {
        // Querying a non-XInput controller.
        const MMRESULT result =
            ImportApiWinMM::joySetCapture(hwnd, (UINT)realJoyID, uPeriod, fChanged);
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
    }

    MMRESULT __stdcall joySetThreshold(UINT uJoyID, UINT uThreshold)
    {
      Initialize();
      const int realJoyID = TranslateApplicationJoyIndex(uJoyID);

      if (realJoyID < 0)
      {
        // Querying an XInput controller.

        // Operation not supported.
        const MMRESULT result = JOYERR_NOCANDO;
        LOG_UNSUPPORTED_OPERATION();
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
      else
      {
        // Querying a non-XInput controller.
        const MMRESULT result = ImportApiWinMM::joySetThreshold((UINT)realJoyID, uThreshold);
        LOG_INVOCATION(Infra::Message::ESeverity::Info, (unsigned int)uJoyID, result);
        return result;
      }
    }
  } // namespace WrapperJoyWinMM
} // namespace Xidi
