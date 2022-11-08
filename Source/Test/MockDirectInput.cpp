/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MockDirectInput.cpp
 *   Implementation of a mock version of the system-supplied DirectInput
 *   interface object along with additional testing-specific functions.
 *****************************************************************************/

#include "MockDirectInput.h"
#include "TestCase.h"


// -------- MACROS --------------------------------------------------------- //

/// Fails a test because a method was invoked that is beyond the scope of tests and therefore not implemented in the mock version of the DirectInput interface.
#define TEST_FAILED_UNIMPLEMENTED_METHOD            TEST_FAILED_BECAUSE(L"%s: Invoked an unimplemented MockDirectInput method.", __FUNCTIONW__)


namespace XidiTest
{
    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    HRESULT STDMETHODCALLTYPE MockDirectInput::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    ULONG STDMETHODCALLTYPE MockDirectInput::AddRef(void)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    ULONG STDMETHODCALLTYPE MockDirectInput::Release(void)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }


    // -------- METHODS: IDirectInput COMMON ------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT STDMETHODCALLTYPE MockDirectInput::CreateDevice(REFGUID rguid, DirectInputType<kDirectInputTestCharMode>::EarliestIDirectInputDeviceType** lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    HRESULT STDMETHODCALLTYPE MockDirectInput::EnumDevices(DWORD dwDevType, DirectInputType<kDirectInputTestCharMode>::EnumDevicesCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    HRESULT STDMETHODCALLTYPE MockDirectInput::FindDevice(REFGUID rguidClass, DirectInputType<kDirectInputTestCharMode>::ConstStringType ptszName, LPGUID pguidInstance)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    HRESULT STDMETHODCALLTYPE MockDirectInput::GetDeviceStatus(REFGUID rguidInstance)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------
    
    HRESULT STDMETHODCALLTYPE MockDirectInput::Initialize(HINSTANCE hinst, DWORD dwVersion)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    HRESULT STDMETHODCALLTYPE MockDirectInput::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }


#if DIRECTINPUT_VERSION >= 0x0800
    // -------- METHODS: IDirectInput8 ONLY -------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT STDMETHODCALLTYPE MockDirectInput::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, DirectInputType<kDirectInputTestCharMode>::ConfigureDevicesParamsType lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }

    // --------

    HRESULT STDMETHODCALLTYPE MockDirectInput::EnumDevicesBySemantics(DirectInputType<kDirectInputTestCharMode>::ConstStringType ptszUserName, DirectInputType<kDirectInputTestCharMode>::ActionFormatType lpdiActionFormat, DirectInputType<kDirectInputTestCharMode>::EnumDevicesBySemanticsCallbackType lpCallback, LPVOID pvRef, DWORD dwFlags)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }
#else
    // -------- METHODS: IDirectInput LEGACY ------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT STDMETHODCALLTYPE MockDirectInput::CreateDeviceEx(REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
    {
        TEST_FAILED_UNIMPLEMENTED_METHOD;
    }
#endif
}
