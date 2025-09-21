/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ExportApiDirectInput.cpp
 *   Implementation of primary exported functions for the DirectInput library.
 **************************************************************************************************/

#include "ExportApiDirectInput.h"

#include <Infra/Core/Message.h>

#include "ApiDirectInput.h"
#include "DirectInputClassFactory.h"
#include "ImportApiDirectInput.h"
#include "WrapperIDirectInput.h"

namespace Xidi
{
  namespace ExportApiDirectInput
  {
    static constexpr DWORD dinputVersion8Min = 0x0800;
    static constexpr DWORD dinputVersion8Max = 0x08ff;

    static constexpr DWORD dinputVersionLegacyMin = 0x0200;
    static constexpr DWORD dinputVersionLegacyMax = 0x07ff;

    static constexpr DWORD dinputVersionLegacy = 0x0700;

    /// Logs an error event indicating that an instance of IDirectInput(8) could not be created due
    /// to a version out-of-range error.
    /// @param [in] minVersion Minimum allowed version.
    /// @param [in] maxVersion Maximum allowed version.
    /// @param [in] receivedVersion Actual version received.
    static inline void LogVersionOutOfRange(
        DWORD minVersion, DWORD maxVersion, DWORD receivedVersion)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to create a DirectInput interface object because the version is out of range (expected 0x%04x to 0x%04x, got 0x%04x).",
          minVersion,
          maxVersion,
          receivedVersion);
    }

    /// Logs an error event indicating that an instance of IDirectInput(8) could not be created due
    /// to an invalid interface parameter.
    static inline void LogInvalidInterfaceParam(void)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Error,
          L"Failed to create a DirectInput interface object because the requested IID is invalid.");
    }

    /// Logs an error event indicating that an instance of IDirectInput(8) could not be created due
    /// to an error having been returned by the system.
    /// @param [in] errorCode Error code returned by the system.
    static inline void LogSystemCreateError(HRESULT errorCode)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to create a DirectInput interface object because the imported function returned code 0x%08x.",
          errorCode);
    }

    /// Logs an informational event indicating that an instance of IDirectInput(8) was created
    /// successfully.
    static inline void LogSystemCreateSuccess(void)
    {
      Infra::Message::Output(
          Infra::Message::ESeverity::Info, L"Successfully created a DirectInput interface object.");
    }

    HRESULT __stdcall Version8DirectInput8Create(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      void* diObject = nullptr;

      if (dwVersion < dinputVersion8Min || dwVersion > dinputVersion8Max)
      {
        LogVersionOutOfRange(dinputVersion8Min, dinputVersion8Max, dwVersion);
        return DIERR_INVALIDPARAM;
      }

      if ((IID_IDirectInput8W != riidltf) && (IID_IDirectInput8A != riidltf))
      {
        LogInvalidInterfaceParam();
        return DIERR_INVALIDPARAM;
      }

      HRESULT result = ImportApiDirectInput::Version8::DirectInput8Create(
          hinst, dwVersion, riidltf, &diObject, punkOuter);
      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      if (IID_IDirectInput8W == riidltf)
        diObject = new WrapperIDirectInput<EDirectInputVersion::k8W>(
            reinterpret_cast<IDirectInput8W*>(diObject));
      else
        diObject = new WrapperIDirectInput<EDirectInputVersion::k8A>(
            reinterpret_cast<IDirectInput8A*>(diObject));

      *ppvOut = (LPVOID)diObject;

      LogSystemCreateSuccess();
      return result;
    }

    HRESULT __stdcall VersionLegacyDirectInputCreateA(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
    {
      IDirectInputA* diObject = nullptr;

      if (dwVersion < dinputVersionLegacyMin || dwVersion > dinputVersionLegacyMax)
      {
        LogVersionOutOfRange(dinputVersionLegacyMin, dinputVersionLegacyMax, dwVersion);
        return E_FAIL;
      }

      HRESULT result = ImportApiDirectInput::VersionLegacy::DirectInputCreateA(
          hinst, dinputVersionLegacy, reinterpret_cast<LPDIRECTINPUTA*>(&diObject), punkOuter);
      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      diObject = new WrapperIDirectInput<EDirectInputVersion::kLegacyA>(
          reinterpret_cast<IDirectInput7A*>(diObject));
      *ppDI = (LPDIRECTINPUTA)diObject;

      LogSystemCreateSuccess();
      return result;
    }

    HRESULT __stdcall VersionLegacyDirectInputCreateW(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
    {
      IDirectInputW* diObject = nullptr;

      if (dwVersion < dinputVersionLegacyMin || dwVersion > dinputVersionLegacyMax)
      {
        LogVersionOutOfRange(dinputVersionLegacyMin, dinputVersionLegacyMax, dwVersion);
        return E_FAIL;
      }

      HRESULT result = ImportApiDirectInput::VersionLegacy::DirectInputCreateW(
          hinst, dinputVersionLegacy, reinterpret_cast<LPDIRECTINPUTW*>(&diObject), punkOuter);
      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      diObject = new WrapperIDirectInput<EDirectInputVersion::kLegacyW>(
          reinterpret_cast<IDirectInput7W*>(diObject));
      *ppDI = (LPDIRECTINPUTW)diObject;

      LogSystemCreateSuccess();
      return result;
    }

    HRESULT __stdcall VersionLegacyDirectInputCreateEx(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      void* diObject = nullptr;

      if (dwVersion < dinputVersionLegacyMin || dwVersion > dinputVersionLegacyMax)
      {
        LogVersionOutOfRange(dinputVersionLegacyMin, dinputVersionLegacyMax, dwVersion);
        return E_FAIL;
      }

      if ((IID_IDirectInputW != riidltf) && (IID_IDirectInput2W != riidltf) &&
          (IID_IDirectInput7W != riidltf) && (IID_IDirectInputA != riidltf) &&
          (IID_IDirectInput2A != riidltf) && (IID_IDirectInput7A != riidltf))
      {
        LogInvalidInterfaceParam();
        return DIERR_INVALIDPARAM;
      }

      HRESULT result;
      BOOL useUnicode = FALSE;

      if (IID_IDirectInputW == riidltf || IID_IDirectInput2W == riidltf ||
          IID_IDirectInput7W == riidltf)
      {
        useUnicode = TRUE;
        result = ImportApiDirectInput::VersionLegacy::DirectInputCreateEx(
            hinst, dinputVersionLegacy, riidltf, &diObject, punkOuter);
      }
      else
      {
        result = ImportApiDirectInput::VersionLegacy::DirectInputCreateEx(
            hinst, dinputVersionLegacy, riidltf, &diObject, punkOuter);
      }

      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      if (TRUE == useUnicode)
        diObject = new WrapperIDirectInput<EDirectInputVersion::kLegacyW>(
            reinterpret_cast<IDirectInput7W*>(diObject));
      else
        diObject = new WrapperIDirectInput<EDirectInputVersion::kLegacyA>(
            reinterpret_cast<IDirectInput7A*>(diObject));

      *ppvOut = diObject;

      LogSystemCreateSuccess();
      return result;
    }

    HRESULT __stdcall Version8DllRegisterServer(void)
    {
      return ImportApiDirectInput::Version8::DllRegisterServer();
    }

    HRESULT __stdcall VersionLegacyDllRegisterServer(void)
    {
      return ImportApiDirectInput::VersionLegacy::DllRegisterServer();
    }

    HRESULT __stdcall Version8DllUnregisterServer(void)
    {
      return ImportApiDirectInput::Version8::DllUnregisterServer();
    }

    HRESULT __stdcall VersionLegacyDllUnregisterServer(void)
    {
      return ImportApiDirectInput::VersionLegacy::DllUnregisterServer();
    }

    HRESULT __stdcall Version8DllCanUnloadNow(void)
    {
      return ImportApiDirectInput::Version8::DllCanUnloadNow();
    }

    HRESULT __stdcall VersionLegacyDllCanUnloadNow(void)
    {
      return ImportApiDirectInput::VersionLegacy::DllCanUnloadNow();
    }

    HRESULT __stdcall Version8DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
      if (DirectInputClassFactory::CanCreateObjectsOfClass(rclsid))
      {
        if (IsEqualIID(IID_IClassFactory, riid))
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Info,
              L"DllGetClassObject is intercepting the class object request because the class ID is supported and the interface ID is correct.");
          *ppv = DirectInputClassFactory::GetInstance();
          return S_OK;
        }
        else
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Warning,
              L"DllGetClassObject failed to intercept the class object request because the class ID is supported but the interface ID is incorrect.");
          return E_NOINTERFACE;
        }
      }

      Infra::Message::Output(
          Infra::Message::ESeverity::Info,
          L"DllGetClassObject is not intercepting the class object request because the class ID is unsupported.");
      return ImportApiDirectInput::Version8::DllGetClassObject(rclsid, riid, ppv);
    }

    HRESULT __stdcall VersionLegacyDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
      if (DirectInputClassFactory::CanCreateObjectsOfClass(rclsid))
      {
        if (IsEqualIID(IID_IClassFactory, riid))
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Info,
              L"DllGetClassObject is intercepting the class object request because the class ID is supported and the interface ID is correct.");
          *ppv = DirectInputClassFactory::GetInstance();
          return S_OK;
        }
        else
        {
          Infra::Message::Output(
              Infra::Message::ESeverity::Warning,
              L"DllGetClassObject failed to intercept the class object request because the class ID is supported but the interface ID is incorrect.");
          return E_NOINTERFACE;
        }
      }

      Infra::Message::Output(
          Infra::Message::ESeverity::Info,
          L"DllGetClassObject is not intercepting the class object request because the class ID is unsupported.");
      return ImportApiDirectInput::VersionLegacy::DllGetClassObject(rclsid, riid, ppv);
    }

  } // namespace ExportApiDirectInput
} // namespace Xidi
