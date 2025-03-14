/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MockDirectInputDevice.cpp
 *   Implementation of a mock version of system-supplied DirectInput device interface objects
 *   along with additional testing-specific functions.
 **************************************************************************************************/

#include "MockDirectInputDevice.h"

#include <Infra/Test/TestCase.h>

/// Fails a test because a method was invoked that is beyond the scope of tests and therefore not
/// implemented in the mock version of the DirectInput interface.
#define TEST_FAILED_UNIMPLEMENTED_METHOD                                                           \
  TEST_FAILED_BECAUSE(L"%s: Invoked an unimplemented MockDirectInputDevice method.", __FUNCTIONW__)

namespace XidiTest
{
  HRESULT __stdcall MockDirectInputDevice::QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  ULONG __stdcall MockDirectInputDevice::AddRef(void)
  {
    // This is a no-op for mock objects.
    return 1;
  }

  ULONG __stdcall MockDirectInputDevice::Release(void)
  {
    // This is a no-op for mock objects.
    return 1;
  }

  HRESULT __stdcall MockDirectInputDevice::Acquire(void)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::CreateEffect(
      REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::EnumCreatedEffectObjects(
      LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::EnumEffects(
      DirectInputTypes<EDirectInputVersion::k8W>::EnumEffectsCallbackType lpCallback,
      LPVOID pvRef,
      DWORD dwEffType)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::EnumEffectsInFile(
      DirectInputTypes<EDirectInputVersion::k8W>::ConstStringType lptszFileName,
      LPDIENUMEFFECTSINFILECALLBACK pec,
      LPVOID pvRef,
      DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::EnumObjects(
      DirectInputTypes<EDirectInputVersion::k8W>::EnumObjectsCallbackType lpCallback,
      LPVOID pvRef,
      DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::Escape(LPDIEFFESCAPE pesc)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
  {
    if (nullptr == lpDIDevCaps) return E_POINTER;

    memcpy(
        lpDIDevCaps,
        &kDeviceInfo.capabilities,
        std::min(lpDIDevCaps->dwSize, kDeviceInfo.capabilities.dwSize));
    return DI_OK;
  }

  HRESULT __stdcall MockDirectInputDevice::GetDeviceData(
      DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::GetDeviceInfo(
      DirectInputTypes<EDirectInputVersion::k8W>::DeviceInstanceType* pdidi)
  {
    if (nullptr == pdidi) return E_POINTER;

    memcpy(pdidi, &kDeviceInfo.instance, std::min(pdidi->dwSize, kDeviceInfo.instance.dwSize));
    return DI_OK;
  }

  HRESULT __stdcall MockDirectInputDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::GetEffectInfo(
      DirectInputTypes<EDirectInputVersion::k8W>::EffectInfoType* pdei, REFGUID rguid)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::GetForceFeedbackState(LPDWORD pdwOut)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::GetObjectInfo(
      DirectInputTypes<EDirectInputVersion::k8W>::DeviceObjectInstanceType* pdidoi,
      DWORD dwObj,
      DWORD dwHow)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
  {
    const auto propertyIterator = kDeviceInfo.properties.find(&rguidProp);
    if (kDeviceInfo.properties.cend() == propertyIterator) return DIERR_UNSUPPORTED;

    const UDirectInputDeviceProperty& kDeviceProperty = propertyIterator->second;
    memcpy(pdiph, &kDeviceProperty, std::min(pdiph->dwSize, kDeviceProperty.header.dwSize));
    return DI_OK;
  }

  HRESULT __stdcall MockDirectInputDevice::Initialize(
      HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::Poll(void)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::SendDeviceData(
      DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::SendForceFeedbackCommand(DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::SetEventNotification(HANDLE hEvent)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::Unacquire(void)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT __stdcall MockDirectInputDevice::WriteEffectToFile(
      DirectInputTypes<EDirectInputVersion::k8W>::ConstStringType lptszFileName,
      DWORD dwEntries,
      LPDIFILEEFFECT rgDiFileEft,
      DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT MockDirectInputDevice::BuildActionMap(
      DirectInputTypes<EDirectInputVersion::k8W>::ActionFormatType* lpdiaf,
      DirectInputTypes<EDirectInputVersion::k8W>::ConstStringType lpszUserName,
      DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT MockDirectInputDevice::GetImageInfo(
      DirectInputTypes<EDirectInputVersion::k8W>::DeviceImageInfoHeaderType* lpdiDevImageInfoHeader)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }

  HRESULT MockDirectInputDevice::SetActionMap(
      DirectInputTypes<EDirectInputVersion::k8W>::ActionFormatType* lpdiActionFormat,
      DirectInputTypes<EDirectInputVersion::k8W>::ConstStringType lptszUserName,
      DWORD dwFlags)
  {
    TEST_FAILED_UNIMPLEMENTED_METHOD;
  }
} // namespace XidiTest
