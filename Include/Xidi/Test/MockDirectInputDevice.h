/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file MockDirectInputDevice.h
 *   Declaration of a mock version of system-supplied DirectInput device
 *   interface objects along with additional testing-specific functions.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "VirtualDirectInputDevice.h"
#include "WrapperIDirectInput.h"


namespace XidiTest
{
    using ::Xidi::DirectInputType;
    using ::Xidi::DirectInputDeviceType;


    /// Character mode used for all DirectInput testing functionality.
    inline constexpr ECharMode kDirectInputTestCharMode = ECharMode::W;


    /// Record type for holding information about a single DirectInpout device property.
    /// Each field holds a representation for a different type of property, with the header being common to all of them.
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

    /// Record type for holding information about a single device that is known to the simulated system.
    struct SDirectInputDeviceInfo
    {
        bool supportsXInput;                                                        ///< Whether or not this device is supposed to be an XInput device.
        DirectInputType<kDirectInputTestCharMode>::DeviceInstanceType instance;     ///< Device instance record, in the same format as used for device enumeration.
        DIDEVCAPS capabilities;                                                     ///< Device capabilities record.
        std::unordered_map<const GUID*, UDirectInputDeviceProperty> properties;     ///< All device properties that are available to be read, keyed by property GUID address.

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

    /// Allows less-than comparison between device information records to support many standard containers that use this type of comparison for sorting.
    /// Objects are compared on the basis of their instance GUIDs.
    /// @param [in] lhs Left-hand side of the binary operator.
    /// @param [in] rhs Right-hand side of the binary operator.
    /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
    inline bool operator<(const SDirectInputDeviceInfo& lhs, const SDirectInputDeviceInfo& rhs)
    {
        return std::less<GUID>()(lhs.instance.guidInstance, rhs.instance.guidInstance);
    }

    /// Allows less-than comparison between device information records and GUIDs to support transparent lookup of instance GUIDs.
    /// Objects are compared on the basis of their instance GUIDs.
    /// @param [in] lhs Left-hand side of the binary operator.
    /// @param [in] rhs Right-hand side of the binary operator.
    /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
    inline bool operator<(const SDirectInputDeviceInfo& lhs, const GUID& rhs)
    {
        return std::less<GUID>()(lhs.instance.guidInstance, rhs);
    }

    /// Allows less-than comparison between device information records and GUIDs to support transparent lookup of instance GUIDs.
    /// Objects are compared on the basis of their instance GUIDs.
    /// @param [in] lhs Left-hand side of the binary operator.
    /// @param [in] rhs Right-hand side of the binary operator.
    /// @return `true` if this object (lhs) is less than the other object (rhs), `false` otherwise.
    inline bool operator<(const GUID& lhs, const SDirectInputDeviceInfo& rhs)
    {
        return std::less<GUID>()(lhs, rhs.instance.guidInstance);
    }

    /// Mock version of the IDirectInput interface, used to test interaction with system-supplied DirectInput objects.
    /// Objects of this type should only be created via appropriate device creation calls to #MockDirectInput.
    /// Not all methods are fully implemented based on the requirements of the test cases that exist.
    class MockDirectInputDevice : public DirectInputType<kDirectInputTestCharMode>::EarliestIDirectInputDeviceType
    {
    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Read-only device information, which defines both instance information and device properties.
        /// Owned by the #MockDirectInput device that creates this object.
        const SDirectInputDeviceInfo& kDeviceInfo;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Initialization constructor.
        /// Requires a device information object.
        inline MockDirectInputDevice(const SDirectInputDeviceInfo& kDeviceInfo) : kDeviceInfo(kDeviceInfo)
        {
            // Nothing to do here.
        }


        // -------- METHODS: IUnknown -------------------------------------- //
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;


        // -------- METHODS: IDirectInputDevice COMMON --------------------- //
        HRESULT STDMETHODCALLTYPE Acquire(void) override;
        HRESULT STDMETHODCALLTYPE CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter) override;
        HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl) override;
        HRESULT STDMETHODCALLTYPE EnumEffects(DirectInputDeviceType<kDirectInputTestCharMode>::EnumEffectsCallbackType lpCallback, LPVOID pvRef, DWORD dwEffType) override;
        HRESULT STDMETHODCALLTYPE EnumEffectsInFile(DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE EnumObjects(DirectInputDeviceType<kDirectInputTestCharMode>::EnumObjectsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc) override;
        HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) override;
        HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE GetDeviceInfo(DirectInputDeviceType<kDirectInputTestCharMode>::DeviceInstanceType* pdidi) override;
        HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) override;
        HRESULT STDMETHODCALLTYPE GetEffectInfo(DirectInputDeviceType<kDirectInputTestCharMode>::EffectInfoType* pdei, REFGUID rguid) override;
        HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut) override;
        HRESULT STDMETHODCALLTYPE GetObjectInfo(DirectInputDeviceType<kDirectInputTestCharMode>::DeviceObjectInstanceType* pdidoi, DWORD dwObj, DWORD dwHow) override;
        HRESULT STDMETHODCALLTYPE GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph) override;
        HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) override;
        HRESULT STDMETHODCALLTYPE Poll(void) override;
        HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) override;
        HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) override;
        HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) override;
        HRESULT STDMETHODCALLTYPE SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) override;
        HRESULT STDMETHODCALLTYPE Unacquire(void) override;
        HRESULT STDMETHODCALLTYPE WriteEffectToFile(DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) override;

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInputDevice8 ONLY ---------------------- //
        HRESULT STDMETHODCALLTYPE BuildActionMap(DirectInputDeviceType<kDirectInputTestCharMode>::ActionFormatType* lpdiaf, DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lpszUserName, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE GetImageInfo(DirectInputDeviceType<kDirectInputTestCharMode>::DeviceImageInfoHeaderType* lpdiDevImageInfoHeader) override;
        HRESULT STDMETHODCALLTYPE SetActionMap(DirectInputDeviceType<kDirectInputTestCharMode>::ActionFormatType* lpdiActionFormat, DirectInputDeviceType<kDirectInputTestCharMode>::ConstStringType lptszUserName, DWORD dwFlags) override;
#endif
    };
}
