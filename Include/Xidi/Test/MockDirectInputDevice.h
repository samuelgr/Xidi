/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MockDirectInputDevice.h
 *   Declaration of a mock version of system-supplied DirectInput device interface objects along
 *   with additional testing-specific functions.
 **************************************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "VirtualDirectInputDevice.h"
#include "WrapperIDirectInput.h"

namespace XidiTest
{
  using ::Xidi::DirectInputDeviceType;
  using ::Xidi::DirectInputType;

  /// Character mode used for all DirectInput testing functionality.
  inline constexpr ECharMode kDirectInputTestCharMode = ECharMode::W;

  /// Record type for holding information about a single DirectInpout device property.
  /// Each field holds a representation for a different type of property, with the header being
  /// common to all of them.
  union UDirectInputDeviceProperty
  {
    DIPROPHEADER header;
    DIPROPDWORD dword;
    DIPROPPOINTER pointer;
    DIPROPRANGE range;
    DIPROPCAL cal;
    DIPROPCALPOV calpov;
    DIPROPGUIDANDPATH guidandpath;
    DIPROPSTRING string;
    DIPROPCPOINTS cpoints;
  };

  /// Record type for holding information about a single device that is known to the simulated
  /// system.
  struct SDirectInputDeviceInfo
  {
    bool supportsXInput; ///< Whether or not this device is supposed to be an XInput device.
    DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType
        instance; ///< Device instance record, in the same format as used for device enumeration.
    DIDEVCAPS capabilities; ///< Device capabilities record.
    std::unordered_map<const GUID*, UDirectInputDeviceProperty>
        properties; ///< All device properties that are available to be read, keyed by property GUID
                    ///< address.

    /// Checks if the represented DirectInput device is reported as being an XInput device.
    /// @return `true` if so, `false` otherwise.
    inline bool SupportsXInput(void) const
    {
      return supportsXInput;
    }

    /// Checks if the represented DirectInput device is reported as supporting force feedback.
    /// @return `true` if so, `false` otherwise.
    inline bool SupportsForceFeedback(void) const
    {
      return (0 != (capabilities.dwFlags & DIDC_FORCEFEEDBACK));
    }
  };

  /// Allows less-than comparison between device information records to support many standard
  /// containers that use this type of comparison for sorting. Objects are compared on the basis of
  /// their instance GUIDs.
  /// @param [in] lhs Left-hand side of the binary operator.
  /// @param [in] rhs Right-hand side of the binary operator.
  /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
  inline bool operator<(const SDirectInputDeviceInfo& lhs, const SDirectInputDeviceInfo& rhs)
  {
    return std::less<GUID>()(lhs.instance.guidInstance, rhs.instance.guidInstance);
  }

  /// Allows less-than comparison between device information records and GUIDs to support
  /// transparent lookup of instance GUIDs. Objects are compared on the basis of their instance
  /// GUIDs.
  /// @param [in] lhs Left-hand side of the binary operator.
  /// @param [in] rhs Right-hand side of the binary operator.
  /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
  inline bool operator<(const SDirectInputDeviceInfo& lhs, const GUID& rhs)
  {
    return std::less<GUID>()(lhs.instance.guidInstance, rhs);
  }

  /// Allows less-than comparison between device information records and GUIDs to support
  /// transparent lookup of instance GUIDs. Objects are compared on the basis of their instance
  /// GUIDs.
  /// @param [in] lhs Left-hand side of the binary operator.
  /// @param [in] rhs Right-hand side of the binary operator.
  /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
  inline bool operator<(const GUID& lhs, const SDirectInputDeviceInfo& rhs)
  {
    return std::less<GUID>()(lhs, rhs.instance.guidInstance);
  }

  /// Mock version of the IDirectInput interface, used to test interaction with system-supplied
  /// DirectInput objects. Objects of this type should only be created via appropriate device
  /// creation calls to #MockDirectInput. Not all methods are fully implemented based on the
  /// requirements of the test cases that exist.
  class MockDirectInputDevice
      : public DirectInputType<kDirectInputTestCharMode>::EarliestIDirectInputDeviceType
  {
  public:

    inline MockDirectInputDevice(const SDirectInputDeviceInfo& kDeviceInfo)
        : kDeviceInfo(kDeviceInfo)
    {}

    // IDirectInputDevice
    HRESULT __stdcall Acquire(void) override;
    HRESULT __stdcall CreateEffect(
        REFGUID rguid,
        LPCDIEFFECT lpeff,
        LPDIRECTINPUTEFFECT* ppdeff,
        LPUNKNOWN punkOuter) override;
    HRESULT __stdcall EnumCreatedEffectObjects(
        LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl) override;
    HRESULT __stdcall EnumEffects(
        DirectInputDeviceType<kDirectInputTestCharMode>::EnumEffectsCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwEffType) override;
    HRESULT __stdcall EnumEffectsInFile(
        DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lptszFileName,
        LPDIENUMEFFECTSINFILECALLBACK pec,
        LPVOID pvRef,
        DWORD dwFlags) override;
    HRESULT __stdcall EnumObjects(
        DirectInputDeviceType<kDirectInputTestCharMode>::EnumObjectsCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwFlags) override;
    HRESULT __stdcall Escape(LPDIEFFESCAPE pesc) override;
    HRESULT __stdcall GetCapabilities(LPDIDEVCAPS lpDIDevCaps) override;
    HRESULT __stdcall GetDeviceData(
        DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override;
    HRESULT __stdcall GetDeviceInfo(
        DirectInputDeviceType<kDirectInputTestCharMode>::DeviceInstanceType* pdidi) override;
    HRESULT __stdcall GetDeviceState(DWORD cbData, LPVOID lpvData) override;
    HRESULT __stdcall GetEffectInfo(
        DirectInputDeviceType<kDirectInputTestCharMode>::EffectInfoType* pdei,
        REFGUID rguid) override;
    HRESULT __stdcall GetForceFeedbackState(LPDWORD pdwOut) override;
    HRESULT __stdcall GetObjectInfo(
        DirectInputDeviceType<kDirectInputTestCharMode>::DeviceObjectInstanceType* pdidoi,
        DWORD dwObj,
        DWORD dwHow) override;
    HRESULT __stdcall GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph) override;
    HRESULT __stdcall Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) override;
    HRESULT __stdcall Poll(void) override;
    HRESULT __stdcall RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;
    HRESULT __stdcall SendDeviceData(
        DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) override;
    HRESULT __stdcall SendForceFeedbackCommand(DWORD dwFlags) override;
    HRESULT __stdcall SetCooperativeLevel(HWND hwnd, DWORD dwFlags) override;
    HRESULT __stdcall SetDataFormat(LPCDIDATAFORMAT lpdf) override;
    HRESULT __stdcall SetEventNotification(HANDLE hEvent) override;
    HRESULT __stdcall SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) override;
    HRESULT __stdcall Unacquire(void) override;
    HRESULT __stdcall WriteEffectToFile(
        DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lptszFileName,
        DWORD dwEntries,
        LPDIFILEEFFECT rgDiFileEft,
        DWORD dwFlags) override;
#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT __stdcall BuildActionMap(
        DirectInputDeviceType<kDirectInputTestCharMode>::ActionFormatType* lpdiaf,
        DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lpszUserName,
        DWORD dwFlags) override;
    HRESULT __stdcall GetImageInfo(
        DirectInputDeviceType<kDirectInputTestCharMode>::DeviceImageInfoHeaderType*
            lpdiDevImageInfoHeader) override;
    HRESULT __stdcall SetActionMap(
        DirectInputDeviceType<kDirectInputTestCharMode>::ActionFormatType* lpdiActionFormat,
        DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lptszUserName,
        DWORD dwFlags) override;
#endif

    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

  private:

    /// Read-only device information, which defines both instance information and device
    /// properties. Owned by the #MockDirectInput device that creates this object.
    const SDirectInputDeviceInfo& kDeviceInfo;
  };
} // namespace XidiTest
