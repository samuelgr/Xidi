/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualDirectInputEffect.h
 *   Declaration of an IDirectInputEffect interface wrapper around force
 *   feedback effects that are associated with virtual controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ForceFeedbackEffect.h"

#include <atomic>


namespace Xidi
{
    /// Concrete implementation of the DirectInput force feedback effect interface.
    class VirtualDirectInputEffect : public IDirectInputEffect
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        /// Reference count.
        std::atomic<unsigned long> refCount;


    public:
        // -------- METHODS: IUnknown ---------------------------------------------- //
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) override;
        ULONG STDMETHODCALLTYPE AddRef(void) override;
        ULONG STDMETHODCALLTYPE Release(void) override;

        // -------- METHODS: IDirectInputEffect ------------------------------------ //
        HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) override;
        HRESULT STDMETHODCALLTYPE GetEffectGuid(LPGUID pguid) override;
        HRESULT STDMETHODCALLTYPE GetParameters(LPDIEFFECT peff, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE SetParameters(LPCDIEFFECT peff, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE Start(DWORD dwIterations, DWORD dwFlags) override;
        HRESULT STDMETHODCALLTYPE Stop(void) override;
        HRESULT STDMETHODCALLTYPE GetEffectStatus(LPDWORD pdwFlags) override;
        HRESULT STDMETHODCALLTYPE Download(void) override;
        HRESULT STDMETHODCALLTYPE Unload(void) override;
        HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc) override;
    };
}
