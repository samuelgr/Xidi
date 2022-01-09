/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualDirectInputEffect.cpp
 *   Implementation of an IDirectInputEffect interface wrapper around force
 *   feedback effects that are associated with virtual controllers.
 *****************************************************************************/

#include "ForceFeedbackDevice.h"
#include "ForceFeedbackEffect.h"
#include "VirtualDirectInputDevice.h"
#include "VirtualDirectInputEffect.h"


namespace Xidi
{
    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "VirtualDirectInputEffect.h" for documentation.

    template <ECharMode charMode> VirtualDirectInputEffect<charMode>::VirtualDirectInputEffect(VirtualDirectInputDevice<charMode>& associatedDevice, std::unique_ptr<Controller::ForceFeedback::Effect>&& effect, const GUID& effectGuid) : associatedDevice(associatedDevice), effect(std::move(effect)), effectGuid(effectGuid), refCount(1)
    {
        associatedDevice.AddRef();
        associatedDevice.ForceFeedbackEffectRegister((void*)this);
    }

    // --------

    template <ECharMode charMode> VirtualDirectInputEffect<charMode>::~VirtualDirectInputEffect(void)
    {
        associatedDevice.ForceFeedbackEffectUnregister((void*)this);
        associatedDevice.Release();
    }


    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        if (nullptr == ppvObj)
            return E_POINTER;

        bool validInterfaceRequested = false;

        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputEffect))
            validInterfaceRequested = true;

        if (true == validInterfaceRequested)
        {
            AddRef();
            *ppvObj = this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    // --------

    template <ECharMode charMode> ULONG VirtualDirectInputEffect<charMode>::AddRef(void)
    {
        return ++refCount;
    }

    // --------

    template <ECharMode charMode> ULONG VirtualDirectInputEffect<charMode>::Release(void)
    {
        const unsigned long numRemainingRefs = --refCount;

        if (0 == numRemainingRefs)
            delete this;

        return (ULONG)numRemainingRefs;
    }


    // -------- METHODS: IDirectInputEffect -------------------------------- //
    // See DirectInput documentation for more information.

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::GetEffectGuid(LPGUID pguid)
    {
        if (nullptr == pguid)
            return DIERR_INVALIDPARAM;

        *pguid = effectGuid;
        return DI_OK;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::GetParameters(LPDIEFFECT peff, DWORD dwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::SetParameters(LPCDIEFFECT peff, DWORD dwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Start(DWORD dwIterations, DWORD dwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Stop(void)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::GetEffectStatus(LPDWORD pdwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Download(void)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Unload(void)
    {
        return E_NOTIMPL;
    }

    // --------

    template <ECharMode charMode> HRESULT VirtualDirectInputEffect<charMode>::Escape(LPDIEFFESCAPE pesc)
    {
        return E_NOTIMPL;
    }


    // -------- EXPLICIT TEMPLATE INSTANTIATION ---------------------------- //
    // Instantiates both the ASCII and Unicode versions of this class.

    template class VirtualDirectInputEffect<ECharMode::A>;
    template class VirtualDirectInputEffect<ECharMode::W>;
}
