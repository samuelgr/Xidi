/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ControllerIdentification.cpp
 *   Implementation of functions for identifying and enumerating Xidi virtual controllers in the
 *   context of DirectInput.
 **************************************************************************************************/

#include "ControllerIdentification.h"

#include <array>
#include <memory>
#include <optional>

#include <SDL3/SDL.h>

#include <Infra/Core/Configuration.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>
#include <Infra/Core/Strings.h>
#include <Infra/Core/TemporaryBuffer.h>

#include "ApiBitSet.h"
#include "ApiDirectInput.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Mapper.h"
#include "Strings.h"

namespace Xidi
{
  /// Enumerates the various HID usage pages that are relevant to Xidi.
  /// See https://www.usb.org/document-library/hid-usage-tables-13 for more information.
  enum class EHidUsagePage : uint16_t
  {
    GeneralDesktop = 0x0001,
    Button = 0x0009
  };

  /// Enumerates the various HID usages within the General Desktop page that are relevant to Xidi.
  /// See https://www.usb.org/document-library/hid-usage-tables-13 for more information.
  enum class EHidUsageGeneralDesktop : uint16_t
  {
    // Identification of entire devices
    Joystick = 0x0004,
    Gamepad = 0x0005,

    // Identification of individual controller elements within devices
    AxisX = 0x0030,
    AxisY = 0x0031,
    AxisZ = 0x0032,
    AxisRotX = 0x0033,
    AxisRotY = 0x0034,
    AxisRotZ = 0x0035,
    HatSwitch = 0x0039
  };

  /// Determines if the specified controller supports force feedback.
  /// @param [in] controllerId Identifier of the controller of interest.
  /// @return `true` if so, `false` otherwise.
  static inline bool DoesControllerSupportForceFeedback(
      Controller::TControllerIdentifier controllerId)
  {
    const Controller::Mapper* const mapper =
        Controller::Mapper::GetConfigured((Controller::TControllerIdentifier)controllerId);
    if (nullptr == mapper) return false;

    return mapper->GetCapabilities().ForceFeedbackIsSupported();
  }

  /// Extracts and returns the instance index from a Xidi virtual controller's GUID.
  /// Does not verify that the supplied GUID actually represents an XInput instance GUID.
  /// @param [in] xguid Xidi virtual controller's instance GUID.
  /// @return Instance index (a.k.a. XInput player number).
  static constexpr Controller::TControllerIdentifier ExtractVirtualControllerInstanceFromGuid(
      REFGUID xguid)
  {
    return (Controller::TControllerIdentifier)((xguid.Data1 >> 16) & 0x0000ffff) -
        (Controller::TControllerIdentifier)VirtualControllerProductId(0);
  }

  /// Computes the HID usage data for a virtual controller axis.
  /// @param [in] axis Axis of interest.
  /// @return Corresponding HID usage data.
  static constexpr SHidUsageData HidUsageDataForAxis(Controller::EAxis axis)
  {
    uint16_t usageForAxis = 0;

    switch (axis)
    {
      case Controller::EAxis::X:
        usageForAxis = (uint16_t)EHidUsageGeneralDesktop::AxisX;
        break;
      case Controller::EAxis::Y:
        usageForAxis = (uint16_t)EHidUsageGeneralDesktop::AxisY;
        break;
      case Controller::EAxis::Z:
        usageForAxis = (uint16_t)EHidUsageGeneralDesktop::AxisZ;
        break;
      case Controller::EAxis::RotX:
        usageForAxis = (uint16_t)EHidUsageGeneralDesktop::AxisRotX;
        break;
      case Controller::EAxis::RotY:
        usageForAxis = (uint16_t)EHidUsageGeneralDesktop::AxisRotY;
        break;
      case Controller::EAxis::RotZ:
        usageForAxis = (uint16_t)EHidUsageGeneralDesktop::AxisRotZ;
        break;
      default:
        break;
    }

    return {.usagePage = (uint16_t)EHidUsagePage::GeneralDesktop, .usage = usageForAxis};
  }

  /// Computes the HID usage data for a virtual controller button.
  /// @param [in] button Button of interest.
  /// @return Corresponding HID usage data.
  static constexpr SHidUsageData HidUsageDataForButton(Controller::EButton button)
  {
    uint16_t usageForButton = (uint16_t)1 + (uint16_t)button;
    return {.usagePage = (uint16_t)EHidUsagePage::Button, .usage = usageForButton};
  }

  /// Computes the HID usage data for a virtual controller POV.
  /// @return Corresponding HID usage data.
  static constexpr SHidUsageData HidUsageDataForPov(void)
  {
    return {
        .usagePage = (uint16_t)EHidUsagePage::GeneralDesktop,
        .usage = (uint16_t)EHidUsageGeneralDesktop::HatSwitch};
  }

  /// Computes the HID usage data for an entire virtual controller.
  /// @return Corresponding HID usage data.
  static constexpr SHidUsageData HidUsageDataForVirtualController(void)
  {
    return {
        .usagePage = (uint16_t)EHidUsagePage::GeneralDesktop,
        .usage = (uint16_t)EHidUsageGeneralDesktop::Gamepad};
  }

  /// Determines whether or not the "friendly" names for virtual controllers should use the short
  /// format. Default behavior is to use a long format for these names, but this default can be
  /// overridden using a workaround in the configuration file. This function reads and caches that
  /// configuration setting.
  /// @return `true` if the short name format should be used, `false` otherwise.
  static bool ShouldUseShortNameFormatForVirtualControllers(void)
  {
    static const bool useShortVirtualControllerNames =
        Globals::GetConfigurationData()
            [Strings::kStrConfigurationSectionWorkarounds]
            [Strings::kStrConfigurationSettingsWorkaroundsUseShortVirtualControllerNames]
                .ValueOr(false);
    return useShortVirtualControllerNames;
  }

  std::optional<bool> ApproximatelyEqualVendorAndProductId(
      std::wstring_view controllerStringA, std::wstring_view controllerStringB)
  {
    static constexpr std::wstring_view kVendorIdPrefix = L"VID";
    static constexpr std::wstring_view kProductIdPrefix = L"PID";

    Infra::TemporaryVector<std::wstring_view> piecesA =
        Infra::Strings::Split<wchar_t>(controllerStringA, {L"_", L"&", L"#"});
    Infra::TemporaryVector<std::wstring_view> piecesB =
        Infra::Strings::Split<wchar_t>(controllerStringB, {L"_", L"&", L"#"});

    std::wstring_view vendorIdA, vendorIdB, productIdA, productIdB;

    for (unsigned int i = 0; i < (piecesA.Size() - 1); ++i)
    {
      if (true == Infra::Strings::EqualsCaseInsensitive(piecesA[i], kVendorIdPrefix))
        vendorIdA = piecesA[++i];
      else if (true == Infra::Strings::EqualsCaseInsensitive(piecesA[i], kProductIdPrefix))
        productIdA = piecesA[++i];
    }

    for (unsigned int i = 0; i < (piecesB.Size() - 1); ++i)
    {
      if (true == Infra::Strings::EqualsCaseInsensitive(piecesB[i], kVendorIdPrefix))
        vendorIdB = piecesB[++i];
      if (true == Infra::Strings::EqualsCaseInsensitive(piecesB[i], kProductIdPrefix))
        productIdB = piecesB[++i];
    }

    if (vendorIdA.empty() || vendorIdB.empty() || productIdA.empty() || productIdB.empty())
      return std::nullopt;

    if (false == Infra::Strings::EqualsCaseInsensitive(productIdA, productIdB)) return false;

    if ((vendorIdA.length() == vendorIdB.length()) && (vendorIdA == vendorIdB))
      return Infra::Strings::EqualsCaseInsensitive(vendorIdA, vendorIdB);
    else if (vendorIdA.length() < vendorIdB.length())
      return Infra::Strings::EqualsCaseInsensitive(
          vendorIdA, vendorIdB.substr(vendorIdB.length() - vendorIdA.length()));
    else
      return Infra::Strings::EqualsCaseInsensitive(
          vendorIdA.substr(vendorIdA.length() - vendorIdB.length()), vendorIdB);
  }

  std::optional<Controller::SElementIdentifier> ControllerElementFromHidUsageData(
      SHidUsageData hidUsageData)
  {
    switch (hidUsageData.usagePage)
    {
      case (uint16_t)EHidUsagePage::GeneralDesktop:
        switch (hidUsageData.usage)
        {
          case HidUsageDataForAxis(Controller::EAxis::X).usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::Axis, .axis = Controller::EAxis::X});
          case HidUsageDataForAxis(Controller::EAxis::Y).usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::Axis, .axis = Controller::EAxis::Y});
          case HidUsageDataForAxis(Controller::EAxis::Z).usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::Axis, .axis = Controller::EAxis::Z});
          case HidUsageDataForAxis(Controller::EAxis::RotX).usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::Axis, .axis = Controller::EAxis::RotX});
          case HidUsageDataForAxis(Controller::EAxis::RotY).usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::Axis, .axis = Controller::EAxis::RotY});
          case HidUsageDataForAxis(Controller::EAxis::RotZ).usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::Axis, .axis = Controller::EAxis::RotZ});
          case HidUsageDataForPov().usage:
            return Controller::SElementIdentifier({.type = Controller::EElementType::Pov});
          case HidUsageDataForVirtualController().usage:
            return Controller::SElementIdentifier(
                {.type = Controller::EElementType::WholeController});
          default:
            break;
        }
        break;

      case (uint16_t)EHidUsagePage::Button:
        if (hidUsageData.usage <= (uint16_t)Controller::EButton::Count)
          return Controller::SElementIdentifier(
              {.type = Controller::EElementType::Button,
               .button = (Controller::EButton)(hidUsageData.usage - 1)});
        break;

      default:
        break;
    }

    return std::nullopt;
  }

  template <EDirectInputVersion diVersion> bool DoesDirectInputControllerSupportSdlGamepad(
      typename DirectInputTypes<diVersion>::IDirectInputCompatType* dicontext,
      REFGUID instanceGUID,
      std::wstring* devicePath)
  {
    bool deviceSupportsSdlGamepad = false;

    typename DirectInputTypes<diVersion>::IDirectInputDeviceCompatType* didevice = nullptr;
    HRESULT result = dicontext->CreateDevice(instanceGUID, &didevice, nullptr);

    if (DI_OK != result)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Unable to check if device with instance GUID %s supports SDL3 Gamepad: Failed to create the device (result = 0x%08x).",
          Strings::GuidToString(instanceGUID).AsCString(),
          static_cast<unsigned int>(result));
      return false;
    }

    DIPROPDWORD devPidVid;
    devPidVid.diph.dwSize = sizeof(DIPROPDWORD);
    devPidVid.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    devPidVid.diph.dwObj = 0;
    devPidVid.diph.dwHow = DIPH_DEVICE;

    result = didevice->GetProperty(DIPROP_VIDPID, &devPidVid.diph);
    didevice->Release();

    if (DI_OK != result)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Unable to check if device with instance GUID %s supports SDL3 Gamepad: Failed to query for property DIPROP_VIDPID (result = 0x%08x).",
          Strings::GuidToString(instanceGUID).AsCString(),
          static_cast<unsigned int>(result));
      return false;
    }   

    uint16_t wVendorID = LOWORD(devPidVid.dwData);
    uint16_t wProductID = HIWORD(devPidVid.dwData);

    int sdlJoystickCount;
    SDL_JoystickID* sdlJoysticks = SDL_GetJoysticks(&sdlJoystickCount);
    for (int i = 0; i < sdlJoystickCount; i++)
    {
      if (wVendorID == SDL_GetJoystickVendorForID(sdlJoysticks[i]) &&
          wProductID == SDL_GetJoystickProductForID(sdlJoysticks[i]) &&
          SDL_IsGamepad(sdlJoysticks[i]))
      {
        deviceSupportsSdlGamepad = true;
      }
    }

    Infra::Message::OutputFormatted(
        Infra::Message::ESeverity::Debug,
        L"Device with instance GUID %s %s SDL3 Gamepad.",
        Strings::GuidToString(instanceGUID).AsCString(),
        (deviceSupportsSdlGamepad ? L"supports" : L"does not support"));

    return deviceSupportsSdlGamepad;
  }

  template bool DoesDirectInputControllerSupportSdlGamepad<EDirectInputVersion::k8A>(
      IDirectInput8A*, REFGUID, std::wstring*);
  template bool DoesDirectInputControllerSupportSdlGamepad<EDirectInputVersion::k8W>(
      IDirectInput8W*, REFGUID, std::wstring*);
  template bool DoesDirectInputControllerSupportSdlGamepad<EDirectInputVersion::kLegacyA>(
      IDirectInputA*, REFGUID, std::wstring*);
  template bool DoesDirectInputControllerSupportSdlGamepad<EDirectInputVersion::kLegacyW>(
      IDirectInputW*, REFGUID, std::wstring*);

  template <EDirectInputVersion diVersion> BOOL EnumerateVirtualControllers(
      BOOL(FAR PASCAL* lpCallback)(
          const typename DirectInputTypes<diVersion>::DeviceInstanceType*, LPVOID),
      LPVOID pvRef,
      bool forceFeedbackRequired)
  {
    std::unique_ptr<typename DirectInputTypes<diVersion>::DeviceInstanceType> instanceInfo =
        std::make_unique<typename DirectInputTypes<diVersion>::DeviceInstanceType>();
    uint32_t numControllersToEnumerate = Controller::kPhysicalControllerCount;

    const uint64_t activeVirtualControllerMask =
        Globals::GetConfigurationData()
            [Strings::kStrConfigurationSectionWorkarounds]
            [Strings::kStrConfigurationSettingWorkaroundsActiveVirtualControllerMask]
                .ValueOr(UINT64_MAX);

    for (uint32_t idx = 0; idx < numControllersToEnumerate; ++idx)
    {
      if ((true == forceFeedbackRequired) && (false == DoesControllerSupportForceFeedback(idx)))
        continue;

      *instanceInfo = {.dwSize = sizeof(*instanceInfo)};
      FillVirtualControllerInfo<diVersion>(*instanceInfo, idx);

      if (0 != (activeVirtualControllerMask & ((uint64_t)1 << idx)))
      {
        if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Info))
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Info,
              L"Enumerate: Presenting Xidi virtual controller %u (instance GUID %s) to the application.",
              (1 + idx),
              Strings::GuidToString(instanceInfo->guidInstance).AsCString());
        }

        if (DIENUM_CONTINUE != lpCallback(instanceInfo.get(), pvRef)) return DIENUM_STOP;
      }
    }

    return DIENUM_CONTINUE;
  }

  template BOOL EnumerateVirtualControllers<EDirectInputVersion::k8A>(
      LPDIENUMDEVICESCALLBACKA, LPVOID, bool);
  template BOOL EnumerateVirtualControllers<EDirectInputVersion::k8W>(
      LPDIENUMDEVICESCALLBACKW, LPVOID, bool);
  template BOOL EnumerateVirtualControllers<EDirectInputVersion::kLegacyA>(
      LPDIENUMDEVICESCALLBACKA, LPVOID, bool);
  template BOOL EnumerateVirtualControllers<EDirectInputVersion::kLegacyW>(
      LPDIENUMDEVICESCALLBACKW, LPVOID, bool);

  template <> int FillHidCollectionName<LPSTR>(
      LPSTR buf, size_t bufcount, uint16_t hidCollectionNumber)
  {
    Infra::TemporaryBuffer<CHAR> hidCollectionNameNameFormatString;

    if (kVirtualControllerHidCollectionForEntireDevice == hidCollectionNumber)
      LoadStringA(
          Infra::ProcessInfo::GetThisModuleInstanceHandle(),
          IDS_XIDI_CONTROLLERIDENTIFICATION_HID_COLLECTION_NAME_PLUS_CONTROLLER_TYPE_FORMAT,
          hidCollectionNameNameFormatString.Data(),
          hidCollectionNameNameFormatString.Capacity());
    else
      LoadStringA(
          Infra::ProcessInfo::GetThisModuleInstanceHandle(),
          IDS_XIDI_CONTROLLERIDENTIFICATION_HID_COLLECTION_NAME_FORMAT,
          hidCollectionNameNameFormatString.Data(),
          hidCollectionNameNameFormatString.Capacity());

    return sprintf_s(
        buf,
        bufcount,
        (LPCSTR)hidCollectionNameNameFormatString.Data(),
        (unsigned int)hidCollectionNumber);
  }

  template <> int FillHidCollectionName<LPWSTR>(
      LPWSTR buf, size_t bufcount, uint16_t hidCollectionNumber)
  {
    Infra::TemporaryBuffer<WCHAR> hidCollectionNameNameFormatString;

    if (kVirtualControllerHidCollectionForEntireDevice == hidCollectionNumber)
      LoadStringW(
          Infra::ProcessInfo::GetThisModuleInstanceHandle(),
          IDS_XIDI_CONTROLLERIDENTIFICATION_HID_COLLECTION_NAME_PLUS_CONTROLLER_TYPE_FORMAT,
          hidCollectionNameNameFormatString.Data(),
          hidCollectionNameNameFormatString.Capacity());
    else
      LoadStringW(
          Infra::ProcessInfo::GetThisModuleInstanceHandle(),
          IDS_XIDI_CONTROLLERIDENTIFICATION_HID_COLLECTION_NAME_FORMAT,
          hidCollectionNameNameFormatString.Data(),
          hidCollectionNameNameFormatString.Capacity());

    return swprintf_s(
        buf,
        bufcount,
        (LPCWSTR)hidCollectionNameNameFormatString.Data(),
        (unsigned int)hidCollectionNumber);
  }

  template <EDirectInputVersion diVersion> void FillVirtualControllerInfo(
      typename DirectInputTypes<diVersion>::DeviceInstanceType& instanceInfo,
      Controller::TControllerIdentifier controllerId)
  {
    instanceInfo.guidInstance = VirtualControllerGuid(controllerId);
    instanceInfo.guidProduct = VirtualControllerGuid(controllerId);
    instanceInfo.dwDevType = DirectInputTypes<diVersion>::XinputGamepadDeviceType();
    FillVirtualControllerName(
        instanceInfo.tszInstanceName, _countof(instanceInfo.tszInstanceName), controllerId);
    FillVirtualControllerName(
        instanceInfo.tszProductName, _countof(instanceInfo.tszProductName), controllerId);

    // DirectInput versions 5 and higher include extra members in this structure, and this is
    // indicated on input using the size member of the structure.
    if (instanceInfo.dwSize >
        offsetof(typename DirectInputTypes<diVersion>::DeviceInstanceType, tszProductName) +
            sizeof(DirectInputTypes<diVersion>::DeviceInstanceType::tszProductName))
    {
      if (true == DoesControllerSupportForceFeedback(controllerId))
        instanceInfo.guidFFDriver = kVirtualControllerForceFeedbackDriverGuid;
      else
        instanceInfo.guidFFDriver = {};

      const SHidUsageData virtualControllerHidData = HidUsageDataForVirtualController();
      instanceInfo.wUsagePage = virtualControllerHidData.usagePage;
      instanceInfo.wUsage = virtualControllerHidData.usage;
    }
  }

  template void FillVirtualControllerInfo<EDirectInputVersion::k8A>(
      DIDEVICEINSTANCEA&, Controller::TControllerIdentifier);
  template void FillVirtualControllerInfo<EDirectInputVersion::k8W>(
      DIDEVICEINSTANCEW&, Controller::TControllerIdentifier);
  template void FillVirtualControllerInfo<EDirectInputVersion::kLegacyA>(
      DIDEVICEINSTANCEA&, Controller::TControllerIdentifier);
  template void FillVirtualControllerInfo<EDirectInputVersion::kLegacyW>(
      DIDEVICEINSTANCEW&, Controller::TControllerIdentifier);

  template <> int FillVirtualControllerName<LPSTR>(
      LPSTR buf, size_t bufcount, Controller::TControllerIdentifier controllerIndex)
  {
    Infra::TemporaryBuffer<CHAR> xidiControllerNameFormatString;
    LoadStringA(
        Infra::ProcessInfo::GetThisModuleInstanceHandle(),
        (ShouldUseShortNameFormatForVirtualControllers()
             ? IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_SHORT_NAME_FORMAT
             : IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT),
        xidiControllerNameFormatString.Data(),
        xidiControllerNameFormatString.Capacity());

    return sprintf_s(
        buf, bufcount, (LPCSTR)xidiControllerNameFormatString.Data(), (controllerIndex + 1));
  }

  template <> int FillVirtualControllerName<LPWSTR>(
      LPWSTR buf, size_t bufcount, Controller::TControllerIdentifier controllerIndex)
  {
    Infra::TemporaryBuffer<WCHAR> xidiControllerNameFormatString;
    LoadStringW(
        Infra::ProcessInfo::GetThisModuleInstanceHandle(),
        (ShouldUseShortNameFormatForVirtualControllers()
             ? IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_SHORT_NAME_FORMAT
             : IDS_XIDI_CONTROLLERIDENTIFICATION_CONTROLLER_NAME_FORMAT),
        xidiControllerNameFormatString.Data(),
        xidiControllerNameFormatString.Capacity());

    return swprintf_s(
        buf, bufcount, (LPCWSTR)xidiControllerNameFormatString.Data(), (controllerIndex + 1));
  }

  template <typename StringType> int FillVirtualControllerPath(
      StringType buf, size_t bufcount, Controller::TControllerIdentifier controllerId)
  {
    // Paths are not currently meaningful, so just a single null character is used to indicate an
    // empty string path.
    if (bufcount > 0)
    {
      buf[0] = 0;
      return 1;
    }

    return 0;
  }

  template int FillVirtualControllerPath(
      LPSTR buf, size_t bufcount, Controller::TControllerIdentifier controllerId);
  template int FillVirtualControllerPath(
      LPWSTR buf, size_t bufcount, Controller::TControllerIdentifier controllerId);

  SHidUsageData HidUsageDataForControllerElement(Controller::SElementIdentifier element)
  {
    switch (element.type)
    {
      case Controller::EElementType::Axis:
        return HidUsageDataForAxis(element.axis);

      case Controller::EElementType::Button:
        return HidUsageDataForButton(element.button);

      case Controller::EElementType::Pov:
        return HidUsageDataForPov();

      default:
        return HidUsageDataForVirtualController();
    }
  }

  std::optional<Controller::TControllerIdentifier> VirtualControllerIdFromInstanceGuid(
      REFGUID instanceGUID)
  {
    Controller::TControllerIdentifier xindex =
        ExtractVirtualControllerInstanceFromGuid(instanceGUID);

    if (xindex < Controller::kPhysicalControllerCount)
    {
      GUID realXInputGUID = VirtualControllerGuid(xindex);
      if (realXInputGUID == instanceGUID) return (Controller::TControllerIdentifier)xindex;
    }

    return std::nullopt;
  }
} // namespace Xidi
