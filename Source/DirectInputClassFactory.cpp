/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file DirectInputClassFactory.cpp
 *   Implementation of COM class factory functionality for DirectInput
 *   objects.
 *****************************************************************************/

#include "ApiWindows.h"
#include "DirectInputClassFactory.h"
#include "ImportApiDirectInput.h"
#include "Message.h"
#include "WrapperIDirectInput.h"


namespace Xidi
{
    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    // Identifies the COM class that is supported by this factory class.
    // Currently only one class is supported, which makes the implementation simpler.
#if DIRECTINPUT_VERSION >= 0x0800
    static REFCLSID kSupportedClassId = CLSID_DirectInput8;
#else
    static REFCLSID kSupportedClassId = CLSID_DirectInput;
#endif


    // -------- CLASS METHODS ---------------------------------------------- //
    // See "DirectInputClassFactory.h" for documentation.

    bool DirectInputClassFactory::CanCreateObjectsOfClass(REFCLSID rclsid)
    {
        if (IsEqualCLSID(kSupportedClassId, rclsid))
            return true;

        return false;
    }

    // --------

    IClassFactory* DirectInputClassFactory::GetInstance(void)
    {
        static DirectInputClassFactory diClassFactory;
        return &diClassFactory;
    }


    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    HRESULT STDMETHODCALLTYPE DirectInputClassFactory::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        if (IsEqualIID(IID_IClassFactory, riid) || IsEqualIID(IID_IUnknown, riid))
        {
            *ppvObj = this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    // --------

    ULONG STDMETHODCALLTYPE DirectInputClassFactory::AddRef(void)
    {
        // AddRef is a no-op because this is a singleton object.
        return 1;
    }

    // --------

    ULONG STDMETHODCALLTYPE DirectInputClassFactory::Release(void)
    {
        // Release is a no-op because this is a singleton object.
        return 1;
    }


    // -------- METHODS: IClassFactory ------------------------------------- //
    // See IClassFactory documentation for more information.

    HRESULT STDMETHODCALLTYPE DirectInputClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
    {
#if DIRECTINPUT_VERSION >= 0x0800
        const bool validInterfaceRequested = (IsEqualIID(IID_IDirectInput8W, riid) || IsEqualIID(IID_IDirectInput8A, riid));
        const bool useUnicode = (IsEqualIID(IID_IDirectInput8W, riid));
        
#else
        const bool validInterfaceRequested = (IsEqualIID(IID_IDirectInput7W, riid) || IsEqualIID(IID_IDirectInput7A, riid) || IsEqualIID(IID_IDirectInput2W, riid) || IsEqualIID(IID_IDirectInput2A, riid) || IsEqualIID(IID_IDirectInputW, riid) || IsEqualIID(IID_IDirectInputA, riid));
        const bool useUnicode = (IsEqualIID(IID_IDirectInput7W, riid) || IsEqualIID(IID_IDirectInput2W, riid) || IsEqualIID(IID_IDirectInputW, riid));
#endif

        if (true == validInterfaceRequested)
        {
            IClassFactory* underlyingObjectFactory = nullptr;
            const HRESULT underlyingObjectFactoryCreateResult = ImportApiDirectInput::DllGetClassObject(kSupportedClassId, IID_IClassFactory, (LPVOID*)&underlyingObjectFactory);

            if (S_OK == underlyingObjectFactoryCreateResult)
            {
                LPVOID underlyingDIObject = nullptr;
                HRESULT underlyingDIObjectCreateResult = S_FALSE;

                if (true == useUnicode)
                    underlyingDIObjectCreateResult = underlyingObjectFactory->CreateInstance(pUnkOuter, IID_LatestIDirectInputW, &underlyingDIObject);
                else
                    underlyingDIObjectCreateResult = underlyingObjectFactory->CreateInstance(pUnkOuter, IID_LatestIDirectInputA, &underlyingDIObject);

                underlyingObjectFactory->Release();

                if (S_OK == underlyingDIObjectCreateResult)
                {
                    if (true == useUnicode)
                        *ppvObject = new WrapperIDirectInput<ECharMode::W>((LatestIDirectInputW*)underlyingDIObject);
                    else
                        *ppvObject = new WrapperIDirectInput<ECharMode::A>((LatestIDirectInputA*)underlyingDIObject);
                    
                    return S_OK;
                }
                else
                {
                    Message::OutputFormatted(Message::ESeverity::Warning, L"DirectInputClassFactory failed with HRESULT code 0x%08x to create an underlying DirectInput object.", (unsigned int)underlyingDIObjectCreateResult);
                    return underlyingDIObjectCreateResult;
                }
            }
            else
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"DirectInputClassFactory failed with HRESULT code 0x%08x to create a class factory for an underlying DirectInput object.", (unsigned int)underlyingObjectFactoryCreateResult);
                return underlyingObjectFactoryCreateResult;
            }
        }

        Message::Output(Message::ESeverity::Warning, L"DirectInputClassFactory was asked to create an instance of an unsupported interface.");
        return E_NOINTERFACE;
    }

    // --------

    HRESULT STDMETHODCALLTYPE DirectInputClassFactory::LockServer(BOOL fLock)
    {
        // LockServer is a no-op because this is a singleton object.
        return S_OK;
    }
}
