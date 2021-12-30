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

#include "ForceFeedbackEffect.h"
#include "VirtualDirectInputEffect.h"


namespace Xidi
{
    // -------- METHODS: IUnknown ------------------------------------------ //
    // See IUnknown documentation for more information.

    HRESULT VirtualDirectInputEffect::QueryInterface(REFIID riid, LPVOID* ppvObj)
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

    ULONG VirtualDirectInputEffect::AddRef(void)
    {
        return ++refCount;
    }

    // --------

    ULONG VirtualDirectInputEffect::Release(void)
    {
        const unsigned long numRemainingRefs = --refCount;

        if (0 == numRemainingRefs)
            delete this;

        return (ULONG)numRemainingRefs;
    }


    // -------- METHODS: IDirectInputEffect -------------------------------- //
    // See DirectInput documentation for more information.

    HRESULT VirtualDirectInputEffect::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::GetEffectGuid(LPGUID pguid)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::GetParameters(LPDIEFFECT peff, DWORD dwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::SetParameters(LPCDIEFFECT peff, DWORD dwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::Start(DWORD dwIterations, DWORD dwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::Stop(void)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::GetEffectStatus(LPDWORD pdwFlags)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::Download(void)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::Unload(void)
    {
        return E_NOTIMPL;
    }

    // --------

    HRESULT VirtualDirectInputEffect::Escape(LPDIEFFESCAPE pesc)
    {
        return E_NOTIMPL;
    }
}
