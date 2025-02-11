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

#if DIRECTINPUT_VERSION >= 0x0800
    HRESULT __stdcall DirectInput8Create(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      void* diObject = nullptr;

      if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
      {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return DIERR_INVALIDPARAM;
      }

      if ((IID_IDirectInput8W != riidltf) && (IID_IDirectInput8A != riidltf))
      {
        LogInvalidInterfaceParam();
        return DIERR_INVALIDPARAM;
      }

      HRESULT result =
          ImportApiDirectInput::DirectInput8Create(hinst, dwVersion, riidltf, &diObject, punkOuter);
      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      if (IID_IDirectInput8W == riidltf)
        diObject = new WrapperIDirectInput<ECharMode::W>((IDirectInput8W*)diObject);
      else
        diObject = new WrapperIDirectInput<ECharMode::A>((IDirectInput8A*)diObject);

      *ppvOut = (LPVOID)diObject;

      LogSystemCreateSuccess();
      return result;
    }
#else
    HRESULT __stdcall DirectInputCreateA(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* ppDI, LPUNKNOWN punkOuter)
    {
      IDirectInputA* diObject = nullptr;

      if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
      {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return E_FAIL;
      }

      HRESULT result = ImportApiDirectInput::DirectInputCreateA(
          hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTA*)&diObject, punkOuter);
      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      diObject = new WrapperIDirectInput<ECharMode::A>((LatestIDirectInputA*)diObject);
      *ppDI = (LPDIRECTINPUTA)diObject;

      LogSystemCreateSuccess();
      return result;
    }

    HRESULT __stdcall DirectInputCreateW(
        HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* ppDI, LPUNKNOWN punkOuter)
    {
      IDirectInput* diObject = nullptr;

      if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
      {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
        return E_FAIL;
      }

      HRESULT result = ImportApiDirectInput::DirectInputCreateW(
          hinst, DIRECTINPUT_VERSION, (LPDIRECTINPUTW*)&diObject, punkOuter);
      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      diObject = new WrapperIDirectInput<ECharMode::W>((LatestIDirectInputW*)diObject);
      *ppDI = (LPDIRECTINPUTW)diObject;

      LogSystemCreateSuccess();
      return result;
    }

    HRESULT __stdcall DirectInputCreateEx(
        HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
    {
      void* diObject = nullptr;

      if (dwVersion < DINPUT_VER_MIN || dwVersion > DINPUT_VER_MAX)
      {
        LogVersionOutOfRange(DINPUT_VER_MIN, DINPUT_VER_MAX, dwVersion);
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
        result = ImportApiDirectInput::DirectInputCreateEx(
            hinst, DIRECTINPUT_VERSION, IID_IDirectInput7W, (LPVOID*)&diObject, punkOuter);
      }
      else
      {
        result = ImportApiDirectInput::DirectInputCreateEx(
            hinst, DIRECTINPUT_VERSION, IID_IDirectInput7A, (LPVOID*)&diObject, punkOuter);
      }

      if (DI_OK != result)
      {
        LogSystemCreateError(result);
        return result;
      }

      if (TRUE == useUnicode)
        diObject = new WrapperIDirectInput<ECharMode::W>((LatestIDirectInputW*)diObject);
      else
        diObject = new WrapperIDirectInput<ECharMode::A>((LatestIDirectInputA*)diObject);

      *ppvOut = diObject;

      LogSystemCreateSuccess();
      return result;
    }
#endif

    HRESULT __stdcall DllRegisterServer(void)
    {
      return ImportApiDirectInput::DllRegisterServer();
    }

    HRESULT __stdcall DllUnregisterServer(void)
    {
      return ImportApiDirectInput::DllUnregisterServer();
    }

    HRESULT __stdcall DllCanUnloadNow(void)
    {
      // In practice, there should not be a need to unload Xidi or DirectInput DLLs.
      return S_FALSE;
    }

    HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
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
      return ImportApiDirectInput::DllGetClassObject(rclsid, riid, ppv);
    }

  } // namespace ExportApiDirectInput
} // namespace Xidi
