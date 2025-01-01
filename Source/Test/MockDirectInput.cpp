/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MockDirectInput.cpp
 *   Implementation of a mock version of the system-supplied DirectInput interface object along
 *   with additional testing-specific functions.
 **************************************************************************************************/

#include "MockDirectInput.h"

#include <Infra/Test/TestCase.h>

/// Fails a test because a method was invoked that is beyond the scope of tests and therefore not
/// implemented in the mock version of the DirectInput interface.
#define TEST_FAILED_UNIMPLEMENTED_METHOD                                                           \
  TEST_FAILED_BECAUSE(L"%s: Invoked an unimplemented MockDirectInput method.", __FUNCTIONW__)

namespace XidiTest
{
  HRESULT __stdcall MockDirectInput::QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  ULONG __stdcall MockDirectInput::AddRef(void)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  ULONG __stdcall MockDirectInput::Release(void)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInput::CreateDevice(
      REFGUID rguid,
      DirectInputType<kDirectInputTestCharMode>::EarliestIDirectInputDeviceType**
          lplpDirectInputDevice,
      LPUNKNOWN pUnkOuter)
  {
    const auto systemDeviceIter = kMockSystemDevices.find(rguid);
    if (kMockSystemDevices.cend() == systemDeviceIter) return DIERR_DEVICENOTREG;

    const auto newDeviceResult =
        createdDevices.emplace(std::make_unique<MockDirectInputDevice>(*systemDeviceIter));
    if (false == newDeviceResult.second)
      TEST_FAILED_BECAUSE(L"Failed to register a new MockDirectInputDevice object.");

    *lplpDirectInputDevice = newDeviceResult.first->get();
    return DI_OK;
  }

  HRESULT __stdcall MockDirectInput::EnumDevices(
      DWORD dwDevType,
      DirectInputType<kDirectInputTestCharMode>::EnumDevicesCallbackType lpCallback,
      LPVOID pvRef,
      DWORD dwFlags)
  {
    for (const auto& mockSystemDevice : kMockSystemDevices)
    {
      if ((0 == dwDevType) || ((dwDevType & mockSystemDevice.instance.dwDevType) == dwDevType))
      {
        // Flag constants allowed for enumeration filters (DIEDFL_*) are equal to flag constants for
        // capabilities (DIDC_*).
        if ((0 == dwFlags) || ((dwFlags & mockSystemDevice.capabilities.dwFlags) == dwFlags))
        {
          if (DIENUM_CONTINUE != lpCallback(&mockSystemDevice.instance, pvRef)) return DI_OK;
        }
      }
    }

    return DI_OK;
  }

  HRESULT __stdcall MockDirectInput::FindDevice(
      REFGUID rguidClass,
      DirectInputType<kDirectInputTestCharMode>::ConstStringType ptszName,
      LPGUID pguidInstance)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInput::GetDeviceStatus(REFGUID rguidInstance)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInput::Initialize(HINSTANCE hinst, DWORD dwVersion)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInput::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

#if DIRECTINPUT_VERSION >= 0x0800

  HRESULT __stdcall MockDirectInput::ConfigureDevices(
      LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
      DirectInputType<kDirectInputTestCharMode>::ConfigureDevicesParamsType lpdiCDParams,
      DWORD dwFlags,
      LPVOID pvRefData)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInput::EnumDevicesBySemantics(
      DirectInputType<kDirectInputTestCharMode>::ConstStringType ptszUserName,
      DirectInputType<kDirectInputTestCharMode>::ActionFormatType lpdiActionFormat,
      DirectInputType<kDirectInputTestCharMode>::EnumDevicesBySemanticsCallbackType lpCallback,
      LPVOID pvRef,
      DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }
#else

  HRESULT __stdcall MockDirectInput::CreateDeviceEx(
      REFGUID rguid, REFIID riid, LPVOID* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }
#endif
} // namespace XidiTest
