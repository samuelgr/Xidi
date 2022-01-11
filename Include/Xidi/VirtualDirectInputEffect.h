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
#include "VirtualDirectInputDevice.h"

#include <atomic>
#include <memory>


namespace Xidi
{
    /// Concrete implementation of the DirectInput force feedback effect interface.
    /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types and interfaces.
    template <ECharMode charMode> class VirtualDirectInputEffect : public IDirectInputEffect
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        /// Associated DirectInput device object.
        VirtualDirectInputDevice<charMode>& associatedDevice;

        /// Underlying force feedback effect object.
        std::unique_ptr<Controller::ForceFeedback::Effect> effect;

        /// GUID that identifies this effect.
        const GUID& effectGuid;

        /// Reference count.
        std::atomic<unsigned long> refCount;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Initialization constructor.
        /// @param [in] associatedDevice DirectInput device object with which to associate this effect.
        /// @param [in] effect Underlying force feedback effect object.
        /// @param [in] effectGuid GUID that identifies this effect.
        VirtualDirectInputEffect(VirtualDirectInputDevice<charMode>& associatedDevice, std::unique_ptr<Controller::ForceFeedback::Effect>&& effect, const GUID& effectGuid);

        /// Default destructor.
        virtual ~VirtualDirectInputEffect(void);


        // -------- CONCRETE INSTANCE METHODS -------------------------------------- //

        /// Retrieves type-specific effect parameters.
        /// Can be overridden by subclasses. The default implementation indicates no type-specific parameter data and returns success.
        /// @param [in, out] peff Effect structure into which type-specific data should be placed.
        /// @return Either `DI_OK` or `DIERR_MOREDATA` depending if the type-specific parameter buffer is large enough.
        virtual HRESULT GetTypeSpecificParameters(LPDIEFFECT peff)
        {
            peff->cbTypeSpecificParams = 0;
            return DI_OK;
        }

        /// Sets type-specific effect parameters.
        /// Can be overridden by subclasses. The default implementation does nothing and returns success.
        /// @param [in] peff Structure containing type-specific effect parameter data.
        /// @param [out] effect Effect to be updated.
        /// @return Either `DI_OK` or `DIERR_INVALIDPARAM` based on the result of the operation. Parameters are invalid if the size in the input structure is wrong or if a semantic validity check fails.
        virtual HRESULT SetTypeSpecificParameters(LPCDIEFFECT peff, Controller::ForceFeedback::Effect& effect)
        {
            return DI_OK;
        }


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
