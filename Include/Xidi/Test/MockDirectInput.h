/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MockDirectInput.h
 *   Declaration of a mock version of the system-supplied DirectInput
 *   interface object along with additional testing-specific functions.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "MockDirectInputDevice.h"
#include "WrapperIDirectInput.h"

#include <memory>
#include <unordered_map>
#include <set>


namespace XidiTest
{
    using ::Xidi::DirectInputType;


    /// Mock version of the IDirectInput interface, used to test interaction with system-supplied DirectInput objects.
    /// Not all methods are fully implemented based on the requirements of the test cases that exist.
    class MockDirectInput : public DirectInputType<kDirectInputTestCharMode>::LatestIDirectInputType
    {
    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// All devices known to the simulated system.
        /// These are the devices that are available to be created and enumerated.
        /// Set once at compile-time and never updated.
        const std::set<SDirectInputDeviceInfo, std::less<>> kMockSystemDevices;

        /// Registry of all device objects created via method calls to this object.
        /// All such objects are automatically destroyed when this object is destroyed.
        std::set<std::unique_ptr<MockDirectInputDevice>> createdDevices;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor.
        /// Establishes an empty set of system devices.
        inline MockDirectInput(void) = default;

        /// Initialization constructor.
        /// Moves a set of system devices into the mock system held by this object.
        inline MockDirectInput(std::set<SDirectInputDeviceInfo, std::less<>>&& mockSystemDevices) : kMockSystemDevices(std::move(mockSystemDevices))
        {
            // Nothing to do here.
        }


        // -------- INSTANCE METHODS --------------------------------------- //

        /// Retrieves and returns the number of system devices held by this object.
        /// @return Number of system devices present.
        inline size_t GetSystemDeviceCount(void) const
        {
            return kMockSystemDevices.size();
        }


        // -------- METHODS: IUnknown -------------------------------------- //
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;


        // -------- METHODS: IDirectInput COMMON --------------------------- //
        HRESULT STDMETHODCALLTYPE CreateDevice(REFGUID rguid, DirectInputType<kDirectInputTestCharMode>::EarliestIDirectInputDeviceType** lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
        HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, DirectInputType<kDirectInputTestCharMode>::EnumDevicesCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE FindDevice(REFGUID rguidClass, DirectInputType<kDirectInputTestCharMode>::ConstStringType ptszName, LPGUID pguidInstance) override;
        HRESULT STDMETHODCALLTYPE GetDeviceStatus(REFGUID rguidInstance) override;
        HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) override;
        HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) override;

#if DIRECTINPUT_VERSION >= 0x0800
        // -------- METHODS: IDirectInput8 ONLY ---------------------------- //
        HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, DirectInputType<kDirectInputTestCharMode>::ConfigureDevicesParamsType lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) override;
        HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(DirectInputType<kDirectInputTestCharMode>::ConstStringType ptszUserName, DirectInputType<kDirectInputTestCharMode>::ActionFormatType lpdiActionFormat, DirectInputType<kDirectInputTestCharMode>::EnumDevicesBySemanticsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags) override;
#else
        // -------- METHODS: IDirectInput LEGACY --------------------------- //
        HRESULT STDMETHODCALLTYPE CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) override;
#endif
    };
}
