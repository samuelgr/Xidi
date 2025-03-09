/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file DirectInputClassFactory.cpp
 *   Implementation of COM class factory functionality for DirectInput
 *   objects.
 **************************************************************************************************/

#include "DirectInputClassFactory.h"

#include <Infra/Core/Message.h>

#include "ApiWindows.h"
#include "ImportApiDirectInput.h"
#include "WrapperIDirectInput.h"

namespace Xidi
{
  bool DirectInputClassFactory::CanCreateObjectsOfClass(REFCLSID rclsid)
  {
    return (IsEqualCLSID(CLSID_DirectInput8, rclsid) || IsEqualCLSID(CLSID_DirectInput, rclsid));
  }

  IClassFactory* DirectInputClassFactory::GetInstance(void)
  {
    static DirectInputClassFactory diClassFactory;
    return &diClassFactory;
  }

  HRESULT __stdcall DirectInputClassFactory::QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    if (IsEqualIID(IID_IClassFactory, riid) || IsEqualIID(IID_IUnknown, riid))
    {
      *ppvObj = this;
      return S_OK;
    }

    return E_NOINTERFACE;
  }

  ULONG __stdcall DirectInputClassFactory::AddRef(void)
  {
    // AddRef is a no-op because this is a singleton object.
    return 1;
  }

  ULONG __stdcall DirectInputClassFactory::Release(void)
  {
    // Release is a no-op because this is a singleton object.
    return 1;
  }

  HRESULT __stdcall DirectInputClassFactory::CreateInstance(
      IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
  {
    const bool version8Requested =
        (IsEqualIID(IID_IDirectInput8W, riid) || IsEqualIID(IID_IDirectInput8A, riid));
    const bool versionLegacyRequested =
        (IsEqualIID(IID_IDirectInput7W, riid) || IsEqualIID(IID_IDirectInput7A, riid) ||
         IsEqualIID(IID_IDirectInput2W, riid) || IsEqualIID(IID_IDirectInput2A, riid) ||
         IsEqualIID(IID_IDirectInputW, riid) || IsEqualIID(IID_IDirectInputA, riid));
    const bool validInterfaceRequested = (version8Requested || versionLegacyRequested);
    const bool useUnicode =
        (IsEqualIID(IID_IDirectInput8W, riid) || IsEqualIID(IID_IDirectInput7W, riid) ||
         IsEqualIID(IID_IDirectInput2W, riid) || IsEqualIID(IID_IDirectInputW, riid));

    if (true == validInterfaceRequested)
    {
      IClassFactory* underlyingObjectFactory = nullptr;
      const HRESULT underlyingObjectFactoryCreateResult =
          (version8Requested ? ImportApiDirectInput::Version8::DllGetClassObject(
                                   riid, IID_IClassFactory, (LPVOID*)&underlyingObjectFactory)
                             : ImportApiDirectInput::VersionLegacy::DllGetClassObject(
                                   riid, IID_IClassFactory, (LPVOID*)&underlyingObjectFactory));

      if (S_OK == underlyingObjectFactoryCreateResult)
      {
        LPVOID underlyingDIObject = nullptr;
        HRESULT underlyingDIObjectCreateResult = S_FALSE;

        if (true == useUnicode)
          underlyingDIObjectCreateResult = underlyingObjectFactory->CreateInstance(
              pUnkOuter,
              ((true == version8Requested) ? IID_IDirectInput8W : IID_IDirectInput7W),
              &underlyingDIObject);
        else
          underlyingDIObjectCreateResult = underlyingObjectFactory->CreateInstance(
              pUnkOuter,
              ((true == version8Requested) ? IID_IDirectInput8A : IID_IDirectInput7A),
              &underlyingDIObject);

        underlyingObjectFactory->Release();

        if (S_OK == underlyingDIObjectCreateResult)
        {
          if ((true == useUnicode) && (true == version8Requested))
          {
            *ppvObject = new WrapperIDirectInput<EDirectInputVersion::k8W>(
                reinterpret_cast<IDirectInput8W*>(underlyingDIObject));
          }
          else if ((true == useUnicode) && (false == version8Requested))
          {
            *ppvObject = new WrapperIDirectInput<EDirectInputVersion::kLegacyW>(
                reinterpret_cast<IDirectInput7W*>(underlyingDIObject));
          }
          else if ((false == useUnicode) && (true == version8Requested))
          {
            *ppvObject = new WrapperIDirectInput<EDirectInputVersion::k8A>(
                reinterpret_cast<IDirectInput8A*>(underlyingDIObject));
          }
          else if ((false == useUnicode) && (false == version8Requested))
          {
            *ppvObject = new WrapperIDirectInput<EDirectInputVersion::kLegacyA>(
                reinterpret_cast<IDirectInput7A*>(underlyingDIObject));
          }
          else
          {
            return E_UNEXPECTED;
          }

          return S_OK;
        }
        else
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"DirectInputClassFactory failed with HRESULT code 0x%08x to create an underlying DirectInput object.",
              (unsigned int)underlyingDIObjectCreateResult);
          return underlyingDIObjectCreateResult;
        }
      }
      else
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Warning,
            L"DirectInputClassFactory failed with HRESULT code 0x%08x to create a class factory for an underlying DirectInput object.",
            (unsigned int)underlyingObjectFactoryCreateResult);
        return underlyingObjectFactoryCreateResult;
      }
    }

    Infra::Message::Output(
        Infra::Message::ESeverity::Warning,
        L"DirectInputClassFactory was asked to create an instance of an unsupported interface.");
    return E_NOINTERFACE;
  }

  HRESULT __stdcall DirectInputClassFactory::LockServer(BOOL fLock)
  {
    // LockServer is a no-op because this is a singleton object.
    return S_OK;
  }
} // namespace Xidi
