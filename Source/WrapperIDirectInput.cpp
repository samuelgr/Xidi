/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *   Fixes issues associated with certain XInput-based controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file WrapperIDirectInput.cpp
 *   Implementation of the wrapper class for IDirectInput.
 **************************************************************************************************/

#include "WrapperIDirectInput.h"

#include <cstddef>
#include <cstdlib>
#include <optional>
#include <unordered_set>

#include <Infra/Core/Message.h>
#include <Infra/Core/TemporaryBuffer.h>

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "Mapper.h"
#include "Strings.h"
#include "VirtualController.h"
#include "VirtualDirectInputDevice.h"

namespace Xidi
{
  /// Contains all information required to intercept callbacks to EnumDevices.
  template <EDirectInputVersion diVersion> struct SEnumDevicesCallbackInfo
  {
    /// #WrapperIDirectInputBase instance that invoked the enumeration.
    WrapperIDirectInputBase<diVersion>* instance;

    /// Application-specified callback that should be invoked.
    typename DirectInputTypes<diVersion>::EnumDevicesCallbackType lpCallback;

    /// Application-specified argument to be provided to the application-specified callback.
    LPVOID pvRef;

    /// Indicates if the application requested that enumeration continue or stop.
    BOOL callbackReturnCode;

    /// Holds device identifiers seen during device enumeration.
    std::unordered_set<GUID> seenInstanceIdentifiers;
  };

  template <EDirectInputVersion diVersion> WrapperIDirectInputBase<diVersion>::
      WrapperIDirectInputBase(DirectInputTypes<diVersion>::IDirectInputType* underlyingDIObject)
      : underlyingDIObject(underlyingDIObject)
  {}

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<
      diVersion>::QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    void* interfacePtr = nullptr;
    const HRESULT result = underlyingDIObject->QueryInterface(riid, &interfacePtr);

    if (S_OK == result)
    {
      if (true == DirectInputTypes<diVersion>::IsCompatibleDirectInputIID(riid))
        *ppvObj = this;
      else
        *ppvObj = interfacePtr;
    }

    return result;
  }

  template <EDirectInputVersion diVersion> ULONG __stdcall WrapperIDirectInputBase<
      diVersion>::AddRef(void)
  {
    return underlyingDIObject->AddRef();
  }

  template <EDirectInputVersion diVersion> ULONG __stdcall WrapperIDirectInputBase<
      diVersion>::Release(void)
  {
    const ULONG numRemainingRefs = underlyingDIObject->Release();

    if (0 == numRemainingRefs) delete this;

    return numRemainingRefs;
  }

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<diVersion>::
      CreateDevice(
          REFGUID rguid,
          DirectInputTypes<diVersion>::IDirectInputDeviceCompatType** lplpDirectInputDevice,
          LPUNKNOWN pUnkOuter)
  {
    // Check if the specified instance GUID is an Xidi virtual controller GUID.
    const std::optional<Controller::TControllerIdentifier> maybeVirtualControllerId =
        VirtualControllerIdFromInstanceGuid(rguid);

    if (false == maybeVirtualControllerId.has_value())
    {
      // Not a virtual controller GUID, so just create the device as requested by the application.
      // However, first dump some information about the device.
      typename DirectInputTypes<diVersion>::IDirectInputDeviceCompatType* createdDevice = nullptr;
      const HRESULT createDeviceResult =
          underlyingDIObject->CreateDevice(rguid, &createdDevice, pUnkOuter);
      if (DI_OK == createDeviceResult)
      {
        if (GUID_SysKeyboard == rguid)
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Info,
              L"Binding to the system keyboard device. Xidi will not handle communication with it.");
        }
        else if (GUID_SysMouse == rguid)
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Info,
              L"Binding to the system mouse device. Xidi will not handle communication with it.");
        }
        else
        {
          typename DirectInputTypes<diVersion>::DeviceInstanceType deviceInfo = {
              .dwSize = sizeof(typename DirectInputTypes<diVersion>::DeviceInstanceType)};
          const HRESULT deviceInfoResult = createdDevice->GetDeviceInfo(&deviceInfo);

          const bool deviceSupportsXInput =
              DoesDirectInputControllerSupportSdlGamepad<diVersion>(underlyingDIObject, rguid);
          if (true == deviceSupportsXInput)
          {
            if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Info))
            {
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Attempting to bind directly to XInput device \"%s\" with instance GUID %s. Xidi hides XInput devices and therefore is rejecting this request.",
                  Infra::TemporaryString(deviceInfo.tszProductName).AsCString(),
                  Strings::GuidToString(deviceInfo.guidInstance).AsCString());
            }
            createdDevice->Release();
            return DIERR_DEVICENOTREG;
          }

          if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Info))
          {
            if (DI_OK == deviceInfoResult)
            {
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Binding to non-XInput device \"%s\" with instance GUID %s. Xidi will not handle communication with it.",
                  Infra::TemporaryString(deviceInfo.tszProductName).AsCString(),
                  Strings::GuidToString(deviceInfo.guidInstance).AsCString());
            }
            else
            {
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Binding to an unknown non-XInput device with instance GUID %s. Xidi will not handle communication with it.",
                  Strings::GuidToString(deviceInfo.guidInstance).AsCString());
            }
          }
        }
      }
      else
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Info,
            L"Failed (result = 0x%08x) to bind to a non-XInput device.",
            static_cast<unsigned int>(createDeviceResult));
      }

      if (nullptr != createdDevice) *lplpDirectInputDevice = createdDevice;
      return createDeviceResult;
    }
    else
    {
      const Controller::TControllerIdentifier virtualControllerId =
          maybeVirtualControllerId.value();

      // Is a virtual controller GUID, so create a virtual controller wrapped with a DirectInput
      // interface.
      VirtualDirectInputDevice<diVersion>* const newDirectInputDeviceInterfaceObject =
          new VirtualDirectInputDevice<diVersion>(
              std::make_unique<Controller::VirtualController>(virtualControllerId));
      *lplpDirectInputDevice = newDirectInputDeviceInterfaceObject;

      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Info,
          L"Binding to Xidi virtual controller %u with interface object %u.",
          (virtualControllerId + 1),
          newDirectInputDeviceInterfaceObject->ObjectIdentifier());
      if (nullptr != pUnkOuter)
        Infra::Message::Output(
            Infra::Message::ESeverity::Warning,
            L"Application requested COM aggregation, which is not implemented, while binding to a Xidi virtual controller.");

      return DI_OK;
    }
  }

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<diVersion>::
      EnumDevices(
          DWORD dwDevType,
          DirectInputTypes<diVersion>::EnumDevicesCallbackType lpCallback,
          LPVOID pvRef,
          DWORD dwFlags)
  {
    const DWORD gameControllerDevClass = 4; // DI8DEVCLASS_GAMECTRL, DIDEVTYPE_JOYSTICK
    const DWORD allDevicesDevClass = 0;     // DI8DEVCLASS_ALL, undefined for legacy
    const BOOL gameControllersRequested =
        ((allDevicesDevClass == dwDevType) || (gameControllerDevClass == dwDevType));

    const bool forceFeedbackControllersOnly = (0 != (dwFlags & DIEDFL_FORCEFEEDBACK));

    SEnumDevicesCallbackInfo<diVersion> callbackInfo;
    callbackInfo.instance = this;
    callbackInfo.lpCallback = lpCallback;
    callbackInfo.pvRef = pvRef;
    callbackInfo.callbackReturnCode = DIENUM_CONTINUE;
    callbackInfo.seenInstanceIdentifiers.clear();

    HRESULT enumResult = DI_OK;
    Infra::Message::Output(
        Infra::Message::ESeverity::Debug, L"Starting to enumerate DirectInput devices.");
    Infra::Message::OutputFormatted(
        Infra::Message::ESeverity::Debug,
        L"Enumerate: dwDevType = 0x%08x, dwFlags = 0x%08x.",
        dwDevType,
        dwFlags);

    // Enumerating game controllers requires some manipulation.
    if (gameControllersRequested)
    {
      // First scan the system for any XInput-compatible game controllers that match the enumeration
      // request. In general, XInput devices enumerated via DirectInput do not report supporting
      // force feedback, even though Xidi does implement such support. For this reason any filtering
      // by force feedback support must be removed while using the system-supplied interfaces to
      // scan for XInput devices.
      enumResult = underlyingDIObject->EnumDevices(
          dwDevType,
          &WrapperIDirectInputBase<diVersion>::CallbackEnumGameControllersXInputScan,
          (LPVOID)&callbackInfo,
          (dwFlags & ~(DIEDFL_FORCEFEEDBACK)));
      if (DI_OK != enumResult) return enumResult;

      const BOOL systemHasXInputDevices = (0 != callbackInfo.seenInstanceIdentifiers.size());

      if (systemHasXInputDevices)
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug,
            L"Enumerate: System has XInput devices, so Xidi virtual controllers are being presented to the application before other controllers.");
      else
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug,
            L"Enumerate: System has no XInput devices, so Xidi virtual controllers are being presented to the application after other controllers.");

      // Second, if the system has XInput controllers, enumerate them.
      // These will be the first controllers seen by the application.
      if (systemHasXInputDevices)
      {
        callbackInfo.callbackReturnCode =
            EnumerateVirtualControllers<diVersion>(lpCallback, pvRef, forceFeedbackControllersOnly);

        if (DIENUM_STOP == callbackInfo.callbackReturnCode)
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Debug, L"Application has terminated enumeration.");
          return enumResult;
        }
      }

      // Third, enumerate all other game controllers, filtering out those that support XInput.
      enumResult = underlyingDIObject->EnumDevices(
          gameControllerDevClass, &CallbackEnumDevicesFiltered, (LPVOID)&callbackInfo, dwFlags);

      if (DI_OK != enumResult) return enumResult;

      if (DIENUM_STOP == callbackInfo.callbackReturnCode)
      {
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug, L"Application has terminated enumeration.");
        return enumResult;
      }

      // Finally, if the system did not have any XInput controllers, enumerate them anyway.
      // These will be the last controllers seen by the application.
      if (!systemHasXInputDevices)
      {
        callbackInfo.callbackReturnCode =
            EnumerateVirtualControllers<diVersion>(lpCallback, pvRef, forceFeedbackControllersOnly);

        if (DIENUM_STOP == callbackInfo.callbackReturnCode)
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Debug, L"Application has terminated enumeration.");
          return enumResult;
        }
      }
    }

    // Enumerate anything else the application requested, filtering out game controllers.
    enumResult = underlyingDIObject->EnumDevices(
        dwDevType, &CallbackEnumDevicesFiltered, (LPVOID)&callbackInfo, dwFlags);

    if (DI_OK != enumResult) return enumResult;

    if (DIENUM_STOP == callbackInfo.callbackReturnCode)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Debug, L"Application has terminated enumeration.");
      return enumResult;
    }

    Infra::Message::Output(
        Infra::Message::ESeverity::Debug, L"Finished enumerating DirectInput devices.");
    return enumResult;
  }

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<diVersion>::
      FindDevice(
          REFGUID rguidClass,
          DirectInputTypes<diVersion>::ConstStringType ptszName,
          LPGUID pguidInstance)
  {
    return underlyingDIObject->FindDevice(rguidClass, ptszName, pguidInstance);
  }

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<
      diVersion>::GetDeviceStatus(REFGUID rguidInstance)
  {
    // Check if the specified instance GUID is an XInput GUID.
    const std::optional<Controller::TControllerIdentifier> maybeVirtualControllerId =
        VirtualControllerIdFromInstanceGuid(rguidInstance);

    if (false == maybeVirtualControllerId.has_value())
    {
      // Not an XInput GUID, so ask the underlying implementation for status.
      return underlyingDIObject->GetDeviceStatus(rguidInstance);
    }
    else
    {
      // Is an XInput GUID, so specify that it is connected.
      return DI_OK;
    }
  }

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<
      diVersion>::Initialize(HINSTANCE hinst, DWORD dwVersion)
  {
    return underlyingDIObject->Initialize(hinst, dwVersion);
  }

  template <EDirectInputVersion diVersion> HRESULT __stdcall WrapperIDirectInputBase<
      diVersion>::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
  {
    return underlyingDIObject->RunControlPanel(hwndOwner, dwFlags);
  }

  template <EDirectInputVersion diVersion> BOOL __stdcall WrapperIDirectInputBase<diVersion>::
      CallbackEnumGameControllersXInputScan(
          const DirectInputTypes<diVersion>::DeviceInstanceType* lpddi, LPVOID pvRef)
  {
    SEnumDevicesCallbackInfo<diVersion>* callbackInfo = (SEnumDevicesCallbackInfo<diVersion>*)pvRef;

    // If the present controller supports XInput, indicate such by adding it to the set of instance
    // identifiers of interest.
    if (DoesDirectInputControllerSupportSdlGamepad<diVersion>(
            callbackInfo->instance->underlyingDIObject, lpddi->guidInstance))
    {
      callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
      if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Debug))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Debug,
            L"Enumerate: DirectInput device \"%s\" with instance GUID %s supports XInput and will not be presented to the application.",
            Infra::TemporaryString(lpddi->tszProductName).AsCString(),
            Strings::GuidToString(lpddi->guidInstance).AsCString());
      }
    }
    else
    {
      if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Debug))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Debug,
            L"Enumerate: DirectInput device \"%s\" with instance GUID %s does not support XInput and may be presented to the application.",
            Infra::TemporaryString(lpddi->tszProductName).AsCString(),
            Strings::GuidToString(lpddi->guidInstance).AsCString());
      }
    }

    return DIENUM_CONTINUE;
  }

  template <EDirectInputVersion diVersion> BOOL __stdcall WrapperIDirectInputBase<diVersion>::
      CallbackEnumDevicesFiltered(
          const DirectInputTypes<diVersion>::DeviceInstanceType* lpddi, LPVOID pvRef)
  {
    SEnumDevicesCallbackInfo<diVersion>* callbackInfo = (SEnumDevicesCallbackInfo<diVersion>*)pvRef;

    // If the device has not been seen already, add it to the set and present it to the application.
    if (0 == callbackInfo->seenInstanceIdentifiers.count(lpddi->guidInstance))
    {
      if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Info))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Info,
            L"Enumerate: Presenting DirectInput device \"%s\" (instance GUID %s) to the application.",
            Infra::TemporaryString(lpddi->tszProductName).AsCString(),
            Strings::GuidToString(lpddi->guidInstance).AsCString());
      }

      callbackInfo->seenInstanceIdentifiers.insert(lpddi->guidInstance);
      callbackInfo->callbackReturnCode =
          ((BOOL(FAR PASCAL*)(const DirectInputTypes<diVersion>::DeviceInstanceType*, LPVOID))(
              callbackInfo->lpCallback))(lpddi, callbackInfo->pvRef);
      return callbackInfo->callbackReturnCode;
    }
    else
    {
      // Otherwise, just skip the device and move onto the next one.
      return DIENUM_CONTINUE;
    }
  }

  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIs8<diVersion>)
  HRESULT __stdcall WrapperIDirectInputVersion8Only<diVersion>::ConfigureDevices(
      LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
      DirectInputTypes<diVersion>::ConfigureDevicesParamsType* lpdiCDParams,
      DWORD dwFlags,
      LPVOID pvRefData)
  {
    return this->underlyingDIObject->ConfigureDevices(
        lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
  }

  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIs8<diVersion>)
  HRESULT __stdcall WrapperIDirectInputVersion8Only<diVersion>::EnumDevicesBySemantics(
      DirectInputTypes<diVersion>::ConstStringType ptszUserName,
      DirectInputTypes<diVersion>::ActionFormatType* lpdiActionFormat,
      DirectInputTypes<diVersion>::EnumDevicesBySemanticsCallbackType lpCallback,
      LPVOID pvRef,
      DWORD dwFlags)
  {
    // Operation not supported.
    return DIERR_UNSUPPORTED;
  }

  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIsLegacy<diVersion>)
  HRESULT __stdcall WrapperIDirectInputVersionLegacyOnly<diVersion>::CreateDeviceEx(
      REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
  {
    if (false == DirectInputTypes<diVersion>::IsCompatibleDirectInputDeviceIID(riid))
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Warning, L"CreateDeviceEx failed due to an invalid IID.");
      return DIERR_NOINTERFACE;
    }

    return this->CreateDevice(
        rguid,
        reinterpret_cast<typename DirectInputTypes<diVersion>::IDirectInputDeviceCompatType**>(
            lplpDirectInputDevice),
        pUnkOuter);
  }

  template class WrapperIDirectInputBase<EDirectInputVersion::k8A>;
  template class WrapperIDirectInputBase<EDirectInputVersion::k8W>;
  template class WrapperIDirectInputBase<EDirectInputVersion::kLegacyA>;
  template class WrapperIDirectInputBase<EDirectInputVersion::kLegacyW>;
  template class WrapperIDirectInputVersion8Only<EDirectInputVersion::k8A>;
  template class WrapperIDirectInputVersion8Only<EDirectInputVersion::k8W>;
  template class WrapperIDirectInputVersionLegacyOnly<EDirectInputVersion::kLegacyA>;
  template class WrapperIDirectInputVersionLegacyOnly<EDirectInputVersion::kLegacyW>;
  template class WrapperIDirectInput<EDirectInputVersion::k8A>;
  template class WrapperIDirectInput<EDirectInputVersion::k8W>;
  template class WrapperIDirectInput<EDirectInputVersion::kLegacyA>;
  template class WrapperIDirectInput<EDirectInputVersion::kLegacyW>;
} // namespace Xidi
