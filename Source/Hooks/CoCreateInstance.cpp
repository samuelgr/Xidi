/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file CoCreateInstance.cpp
 *   Implementation of hook for CoCreateInstance.
 **************************************************************************************************/

#include <string>
#include <string_view>

#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "SetHooks.h"
#include "Strings.h"

namespace Xidi
{
  /// Creates an instance of the specified COM object by invoking the provided DllGetClassObject
  /// procedure to retrieve the class factory object.
  static HRESULT CreateInstance(
      decltype(&DllGetClassObject) procDllGetClassObject,
      REFCLSID rclsid,
      LPUNKNOWN pUnkOuter,
      REFIID riid,
      LPVOID* ppv)
  {
    if (nullptr == procDllGetClassObject)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Error,
          L"CreateInstance encountered an internal null pointer error while attempting to create an instance of a COM object.");
      return E_FAIL;
    }

    if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Debug))
    {
      LPOLESTR clsidString = nullptr;
      LPOLESTR iidString = nullptr;

      const HRESULT clsidStringResult = StringFromCLSID(rclsid, &clsidString);
      const HRESULT iidStringResult = StringFromIID(riid, &iidString);

      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Debug,
          L"CreateInstance is attempting to create a COM object instance with CLSID=%s and IID=%s",
          (S_OK == clsidStringResult ? clsidString : L"(unknown)"),
          (S_OK == iidStringResult ? iidString : L"(unknown)"));

      if (S_OK == clsidStringResult) CoTaskMemFree(clsidString);

      if (S_OK == iidStringResult) CoTaskMemFree(iidString);
    }

    IClassFactory* comClassObject = nullptr;
    const HRESULT comClassObjectResult =
        procDllGetClassObject(rclsid, IID_IClassFactory, (LPVOID*)&comClassObject);

    if (S_OK != comClassObjectResult)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"CreateInstance failed with HRESULT code 0x%08x to create a class factory object via DllGetClassObject.",
          (unsigned int)comClassObjectResult);
      return comClassObjectResult;
    }

    LPVOID newObject = nullptr;
    const HRESULT newObjectResult = comClassObject->CreateInstance(pUnkOuter, riid, &newObject);

    comClassObject->Release();

    if (S_OK != newObjectResult)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"CreateInstance failed with HRESULT code 0x%08x to create an instance of the requested object.",
          (unsigned int)newObjectResult);
      return newObjectResult;
    }

    Infra::Message::Output(
        Infra::Message::ESeverity::Debug,
        L"CreateInstance successfully created an instance of a COM object.");
    *ppv = newObject;
    return S_OK;
  }
} // namespace Xidi

HRESULT StaticHook_CoCreateInstance::Hook(
    REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
  static const HMODULE xidiLibraryHandle =
      LoadLibraryW(Xidi::Strings::GetXidiMainLibraryFilename().data());
  if (nullptr == xidiLibraryHandle)
  {
    Infra::Message::OutputFormatted(
        Infra::Message::ESeverity::Error,
        L"CoCreateInstance failed to intercept object creation because the library \"%.*s\" could not be loaded.",
        static_cast<int>(Xidi::Strings::GetXidiMainLibraryFilename().length()),
        Xidi::Strings::GetXidiMainLibraryFilename().data());
    return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
  }

  if (IsEqualCLSID(CLSID_DirectInput8, rclsid))
  {
    static const decltype(&DllGetClassObject) procDllGetClassObject =
        reinterpret_cast<decltype(&DllGetClassObject)>(
            GetProcAddress(xidiLibraryHandle, "dinput8_DllGetClassObject"));
    if (nullptr == procDllGetClassObject)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"CoCreateInstance failed to intercept object creation because library \"%.*s\" is missing the required entry point.",
          static_cast<int>(Xidi::Strings::GetXidiMainLibraryFilename().length()),
          Xidi::Strings::GetXidiMainLibraryFilename().data());
      return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }

    Infra::Message::Output(
        Infra::Message::ESeverity::Info,
        L"CoCreateInstance is intercepting creation of object of type CLSID_DirectInput8.");
    return Xidi::CreateInstance(procDllGetClassObject, rclsid, pUnkOuter, riid, ppv);
  }
  else if (IsEqualCLSID(CLSID_DirectInput, rclsid))
  {
    static const decltype(&DllGetClassObject) procDllGetClassObject =
        reinterpret_cast<decltype(&DllGetClassObject)>(
            GetProcAddress(xidiLibraryHandle, "dinput_DllGetClassObject"));
    if (nullptr == procDllGetClassObject)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"CoCreateInstance failed to intercept object creation because library \"%.*s\" is missing the required entry point.",
          static_cast<int>(Xidi::Strings::GetXidiMainLibraryFilename().length()),
          Xidi::Strings::GetXidiMainLibraryFilename().data());
      return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }

    Infra::Message::Output(
        Infra::Message::ESeverity::Info,
        L"CoCreateInstance is intercepting creation of object of type CLSID_DirectInput.");
    return Xidi::CreateInstance(procDllGetClassObject, rclsid, pUnkOuter, riid, ppv);
  }
  else if (
      IsEqualCLSID(CLSID_DirectInputDevice8, rclsid) ||
      IsEqualCLSID(CLSID_DirectInputDevice, rclsid))
  {
    Infra::Message::Output(
        Infra::Message::ESeverity::Warning,
        L"CoCreateInstance is not intercepting creation of object of type CLSID_DirectInputDevice8 or CLSID_DirectInputDevice because support for these classes is not implemented.");
  }

  return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
