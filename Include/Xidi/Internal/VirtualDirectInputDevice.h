/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file VirtualDirectInputDevice.h
 *   Declaration of an IDirectInputDevice interface wrapper around virtual controllers.
 **************************************************************************************************/

#pragma once

#include <atomic>
#include <memory>
#include <optional>

#include "ApiDirectInput.h"
#include "DataFormat.h"
#include "VirtualController.h"

namespace Xidi
{
  /// Helper types for differentiating between Unicode and ASCII interface versions.
  template <ECharMode charMode> struct DirectInputDeviceType
  {
    using StringType = LPTSTR;
    using ConstStringType = LPCTSTR;
    using DeviceInstanceType = DIDEVICEINSTANCE;
    using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3;
    using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCE;
    using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3;
    using EffectInfoType = DIEFFECTINFO;
    using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACK;
    using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACK;
#if DIRECTINPUT_VERSION >= 0x0800
    using ActionFormatType = DIACTIONFORMAT;
    using DeviceImageInfoHeaderType = DIDEVICEIMAGEINFOHEADER;
#endif
  };

  template <> struct DirectInputDeviceType<ECharMode::A> : public LatestIDirectInputDeviceA
  {
    using StringType = LPSTR;
    using ConstStringType = LPCSTR;
    using DeviceInstanceType = DIDEVICEINSTANCEA;
    using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3A;
    using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCEA;
    using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3A;
    using EffectInfoType = DIEFFECTINFOA;
    using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACKA;
    using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACKA;
#if DIRECTINPUT_VERSION >= 0x0800
    using ActionFormatType = DIACTIONFORMATA;
    using DeviceImageInfoHeaderType = DIDEVICEIMAGEINFOHEADERA;
#endif
  };

  template <> struct DirectInputDeviceType<ECharMode::W> : public LatestIDirectInputDeviceW
  {
    using StringType = LPWSTR;
    using ConstStringType = LPCWSTR;
    using DeviceInstanceType = DIDEVICEINSTANCEW;
    using DeviceInstanceCompatType = DIDEVICEINSTANCE_DX3W;
    using DeviceObjectInstanceType = DIDEVICEOBJECTINSTANCEW;
    using DeviceObjectInstanceCompatType = DIDEVICEOBJECTINSTANCE_DX3W;
    using EffectInfoType = DIEFFECTINFOW;
    using EnumEffectsCallbackType = LPDIENUMEFFECTSCALLBACKW;
    using EnumObjectsCallbackType = LPDIENUMDEVICEOBJECTSCALLBACKW;
#if DIRECTINPUT_VERSION >= 0x0800
    using ActionFormatType = DIACTIONFORMATW;
    using DeviceImageInfoHeaderType = DIDEVICEIMAGEINFOHEADERW;
#endif
  };

  /// Enumerates possible access modes for DirectInput devices.
  enum class ECooperativeLevel
  {
    /// Shared mode, also known as non-exclusive mode. Any number of shared mode acquisitions are
    /// allowed to the same physical device, even if another acquisition already exists in exclusive
    /// mode.
    Shared,

    /// Exclusive mode. Only a single acquisition in exclusive mode is permitted per physical
    /// device.
    Exclusive
  };

  /// Inherits from whatever IDirectInputDevice version is appropriate.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  template <ECharMode charMode> class VirtualDirectInputDevice
      : public DirectInputDeviceType<charMode>
  {
  public:

    VirtualDirectInputDevice(std::unique_ptr<Controller::VirtualController>&& controller);

    ~VirtualDirectInputDevice(void);

    /// Fills the specified buffer with a friendly string representation of the specified controller
    /// element. Intended for internal use, primarily for log message generation.
    /// @param [in] element Controller element for which a string is desired.
    /// @param [out] buf Buffer to be filled with the string.
    /// @param [in] bufcount Buffer size in number of characters.
    static void ElementToString(
        Controller::SElementIdentifier element,
        DirectInputDeviceType<charMode>::StringType buf,
        int bufcount);

    /// Determines if the specified GUID is supported for creating a force feedback effect object.
    /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of
    /// types and interfaces.
    /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
    /// @return `true` if the GUID is recognized and can be used to make a force feedback effect
    /// object, `false` otherwise.
    static bool ForceFeedbackEffectCanCreateObject(REFGUID rguidEffect);

    /// Obtains the force feedback device associated with this controller.
    /// If this controller is not yet acquired then an attempt is made to acquire it automatically.
    /// @return Pointer to the force feedback device object if successful, `nullptr` otherwise.
    Controller::ForceFeedback::Device* AutoAcquireAndGetForceFeedbackDevice(void);

    /// Registers a force feedback effect by adding it to the effect registry.
    /// Intended to be invoked automaticaly as effects are constructed.
    /// @param [in] effect Address of the effect object to register.
    inline void ForceFeedbackEffectRegister(void* effect)
    {
      effectRegistry.insert(effect);
    }

    /// Unregisters a force feedback effect by removing it from the effect registry.
    /// Intended to be invoked automaticaly as effects are destroyed.
    /// @param [in] effect Address of the effect object to unregister.
    inline void ForceFeedbackEffectUnregister(void* effect)
    {
      effectRegistry.erase(effect);
    }

    /// Retrieves and returns the configured cooperative level that defines how access to the
    /// underlying physical device is shared with other objects. The cooperative level defaults to
    /// shared but can be updated by the application via an interface method.
    /// @return Configured cooperative level of this object.
    inline ECooperativeLevel GetCooperativeLevel(void) const
    {
      return cooperativeLevel;
    }

    /// Retrieves a reference to the underlying virtual controller object.
    /// Returned reference remains valid only as long as this object exists.
    /// Primarily intended for testing.
    /// @return Reference to the underlying virtual controller object.
    inline Controller::VirtualController& GetVirtualController(void) const
    {
      return *controller;
    }

    /// Identifies a controller element, given a DirectInput-style element identifier.
    /// Parameters are named after common DirectInput field and method parameters that are used for
    /// this purpose.
    /// @param [in] dwObj Object identifier, whose semantics depends on identification method. See
    /// DirectInput documentation for more information.
    /// @param [in] dwHow Identification method. See DirectInput documentation for more information.
    /// @return Virtual controller element identifier that matches the DirectInput-style element
    /// identifier, if such a match exists.
    std::optional<Controller::SElementIdentifier> IdentifyElement(DWORD dwObj, DWORD dwHow) const;

    /// Identifies a controller element using a DirectInput-style object ID.
    /// @param [in] element Controller element to be identified.
    /// @return Resulting DirectInput-style identifier, if it exists.
    std::optional<DWORD> IdentifyObjectById(Controller::SElementIdentifier element) const;

    /// Identifies a controller element using a DirectInput-style offset into the application's data
    /// format.
    /// @param [in] element Controller element to be identified.
    /// @return Resulting DirectInput-style identifier, if it exists.
    std::optional<TOffset> IdentifyObjectByOffset(Controller::SElementIdentifier element) const;

    /// Specifies if the application's data format is set.
    /// @return `true` if the application's data format has been successfully set, `false`
    /// otherwise.
    inline bool IsApplicationDataFormatSet(void) const
    {
      return (nullptr != dataFormat);
    }

    /// Retrieves and returns the unique internal identifier associated with this interface object.
    /// Intended for logging and debugging use.
    /// @return Unique internal identifier of this object.
    inline unsigned int ObjectIdentifier(void) const
    {
      return kObjectId;
    }

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
        DirectInputDeviceType<charMode>::EnumEffectsCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwEffType) override;
    HRESULT __stdcall EnumEffectsInFile(
        DirectInputDeviceType<charMode>::ConstStringType lptszFileName,
        LPDIENUMEFFECTSINFILECALLBACK pec,
        LPVOID pvRef,
        DWORD dwFlags) override;
    HRESULT __stdcall EnumObjects(
        DirectInputDeviceType<charMode>::EnumObjectsCallbackType lpCallback,
        LPVOID pvRef,
        DWORD dwFlags) override;
    HRESULT __stdcall Escape(LPDIEFFESCAPE pesc) override;
    HRESULT __stdcall GetCapabilities(LPDIDEVCAPS lpDIDevCaps) override;
    HRESULT __stdcall GetDeviceData(
        DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override;
    HRESULT __stdcall GetDeviceInfo(
        DirectInputDeviceType<charMode>::DeviceInstanceType* pdidi) override;
    HRESULT __stdcall GetDeviceState(DWORD cbData, LPVOID lpvData) override;
    HRESULT __stdcall GetEffectInfo(
        DirectInputDeviceType<charMode>::EffectInfoType* pdei, REFGUID rguid) override;
    HRESULT __stdcall GetForceFeedbackState(LPDWORD pdwOut) override;
    HRESULT __stdcall GetObjectInfo(
        DirectInputDeviceType<charMode>::DeviceObjectInstanceType* pdidoi,
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
        DirectInputDeviceType<charMode>::ConstStringType lptszFileName,
        DWORD dwEntries,
        LPDIFILEEFFECT rgDiFileEft,
        DWORD dwFlags) override;
#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT __stdcall BuildActionMap(
        DirectInputDeviceType<charMode>::ActionFormatType* lpdiaf,
        DirectInputDeviceType<charMode>::ConstStringType lpszUserName,
        DWORD dwFlags) override;
    HRESULT __stdcall GetImageInfo(
        DirectInputDeviceType<charMode>::DeviceImageInfoHeaderType* lpdiDevImageInfoHeader)
        override;
    HRESULT __stdcall SetActionMap(
        DirectInputDeviceType<charMode>::ActionFormatType* lpdiActionFormat,
        DirectInputDeviceType<charMode>::ConstStringType lptszUserName,
        DWORD dwFlags) override;
#endif

    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

  private:

    /// Unique internal object identifier. Used for logging purposes to distinguish between multiple
    /// objects associated with the same virtual controller.
    const unsigned int kObjectId;

    /// Virtual controller with which to interface.
    std::unique_ptr<Controller::VirtualController> controller;

    /// Cooperative level that defines the desired level of access to the underlying physical
    /// device. Shared by default, but applications can request exclusive mode. Force feedback
    /// requires that an applicaton acquire the device in exclusive mode.
    ECooperativeLevel cooperativeLevel;

    /// Data format specification for communicating with the DirectInput application.
    std::unique_ptr<DataFormat> dataFormat;

    /// Registry of all force feedback effect objects created by this object. Deliberately not
    /// type-safe to avoid a circular dependency between header files. Used exclusively to allow
    /// DirectInput device objects to enumerate the effect objects associated with them.
    std::set<void*> effectRegistry;

    /// Reference count.
    std::atomic<unsigned long> refCount;

    /// Storage for all properties that are silently supported but not used by Xidi. Others can be
    /// added here as needed.
    struct
    {
      DWORD autocenter = DIPROPAUTOCENTER_OFF;
    } unusedProperties;
  };
} // namespace Xidi
