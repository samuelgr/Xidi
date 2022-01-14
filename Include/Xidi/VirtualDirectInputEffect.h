/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file VirtualDirectInputEffect.h
 *   Declaration of an IDirectInputEffect interface wrapper around force
 *   feedback effects that are associated with virtual controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ForceFeedbackEffect.h"
#include "ForceFeedbackTypes.h"
#include "VirtualDirectInputDevice.h"

#include <atomic>
#include <memory>
#include <optional>


namespace Xidi
{
    /// Generic base implementation of the DirectInput force feedback effect interface.
    /// Suitable for use with force feedback effects that do not have any type-specific parameters.
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


    protected:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Initialization constructor.
        /// @param [in] associatedDevice DirectInput device object with which to associate this effect.
        /// @param [in] effect Underlying force feedback effect object, which is cloned when this object is constructed.
        /// @param [in] effectGuid GUID that identifies this effect.
        VirtualDirectInputEffect(VirtualDirectInputDevice<charMode>& associatedDevice, const Controller::ForceFeedback::Effect& effect, const GUID& effectGuid);

    public:
        /// Default destructor.
        virtual ~VirtualDirectInputEffect(void);


        // -------- INSTANCE METHODS ----------------------------------------------- //

        /// Retrieves a reference to the underlying effect.
        /// Intended for internal use but additionally exposed for testing.
        /// @return Reference to the underlying effect object.
        inline Controller::ForceFeedback::Effect& UnderlyingEffect(void)
        {
            return *effect;
        }

        /// Internal implementation of starting an effect's playback.
        /// Adds an timestamp parameter and serves as an entry point for tests that start effect playback.
        /// @param [in] dwIterations Number of iterations, similar to the corresponding interface method.
        /// @param [in] dwFlags Flags, similar to the corresponding interface method.
        /// @param [in] timestamp Optional timestamp to associate with the effect's playback starting, which is forwarded to the force feedback device.
        /// @return Result of the operation, similar to the corresponding interface method.
        HRESULT StartPlayback(DWORD dwIterations, DWORD dwFlags, std::optional<Controller::ForceFeedback::TEffectTimeMs> timestamp = std::nullopt);


        // -------- CONCRETE INSTANCE METHODS -------------------------------------- //

        /// Retrieves type-specific effect parameters.
        /// Can be overridden by subclasses. The default implementation indicates no type-specific parameter data and returns success.
        /// @param [in, out] peff Effect structure into which type-specific data should be placed.
        /// @return `DI_OK`, `DIERR_INVALIDPARAM`, or `DIERR_MOREDATA` depending on the contents of the input structure.
        virtual HRESULT GetTypeSpecificParameters(LPDIEFFECT peff)
        {
            peff->cbTypeSpecificParams = 0;
            return DI_OK;
        }

        /// Clones the underlying effect, updates the clone's type-specific effect parameters, and returns the result.
        /// Can be overridden by subclasses. The default implementation does nothing and returns success.
        /// @param [in] peff Structure containing type-specific effect parameter data.
        /// @return Either a copy of the effect with type-specific parameters updated or `nullptr` if the parameters are invalid. Parameters are invalid if the size in the input structure is wrong or if a semantic validity check fails.
        virtual std::unique_ptr<Controller::ForceFeedback::Effect> CloneAndSetTypeSpecificParameters(LPCDIEFFECT peff)
        {
            return effect->Clone();
        }


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

    /// Template for DirectInput force feedback effect objects that have type-specific parameters.
    /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types and interfaces.
    /// @tparam DirectInputTypeSpecificParameterType Type used by DirectInput to represent type-specific parameters.
    /// @tparam TypeSpecificParameterType Internal type used to represent type-specific parameters.
    template <ECharMode charMode, typename DirectInputTypeSpecificParameterType, typename TypeSpecificParameterType> class VirtualDirectInputEffectWithTypeSpecificParameters : public VirtualDirectInputEffect<charMode>
    {
    protected:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Initialization constructor.
        /// @param [in] associatedDevice DirectInput device object with which to associate this effect.
        /// @param [in] effect Underlying force feedback effect object.
        /// @param [in] effectGuid GUID that identifies this effect.
        inline VirtualDirectInputEffectWithTypeSpecificParameters(VirtualDirectInputDevice<charMode>& associatedDevice, const Controller::ForceFeedback::EffectWithTypeSpecificParameters<TypeSpecificParameterType>& effect, const GUID& effectGuid) : VirtualDirectInputEffect<charMode>(associatedDevice, effect, effectGuid)
        {
            // Nothing to do here.
        }


        // -------- INSTANCE METHODS ----------------------------------------------- //

        /// Type-casts and returns a reference to the underlying effect.
        /// No run-time checks are performed, but the type-cast operation is safe based on the types allowed for the initialization constructor parameters.
        /// @return Type-casted reference to the underlying effect.
        inline Controller::ForceFeedback::EffectWithTypeSpecificParameters<TypeSpecificParameterType>& TypedUnderlyingEffect(void)
        {
            return static_cast<Controller::ForceFeedback::EffectWithTypeSpecificParameters<TypeSpecificParameterType>&>(VirtualDirectInputEffect<charMode>::UnderlyingEffect());
        }


        // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //

        /// Converts from the DirectInput type-specific parameter type to the internal type-specific parameter type.
        /// Performs no error-checking.
        /// @param [in] diTypeSpecificParams Type-specific parameters in DirectInput format.
        /// @return Results of the conversion.
        virtual TypeSpecificParameterType ConvertFromDirectInput(const DirectInputTypeSpecificParameterType& diTypeSpecificParams) const = 0;

        /// Converts from the internal type-specific parameter type to the DirectInput type-specific parameter type.
        /// Performs no error-checking.
        /// @param [in] typeSpecificParams Type-specific parameters in internal format.
        /// @return Results of the conversion.
        virtual DirectInputTypeSpecificParameterType ConvertToDirectInput(const TypeSpecificParameterType& typeSpecificParams) const = 0;


        // -------- CONCRETE INSTANCE METHODS -------------------------------------- //

        HRESULT GetTypeSpecificParameters(LPDIEFFECT peff) override
        {
            if (peff->cbTypeSpecificParams < sizeof(DirectInputTypeSpecificParameterType))
            {
                peff->cbTypeSpecificParams = sizeof(DirectInputTypeSpecificParameterType);
                return DIERR_MOREDATA;
            }

            if (nullptr == peff->lpvTypeSpecificParams)
                return DIERR_INVALIDPARAM;

            if (false == TypedUnderlyingEffect().HasTypeSpecificParameters())
                return DIERR_INVALIDPARAM;

            peff->cbTypeSpecificParams = sizeof(DirectInputTypeSpecificParameterType);
            *(DirectInputTypeSpecificParameterType*)peff->lpvTypeSpecificParams = ConvertToDirectInput(TypedUnderlyingEffect().GetTypeSpecificParameters().value());
            return DI_OK;
        }

        std::unique_ptr<Controller::ForceFeedback::Effect> CloneAndSetTypeSpecificParameters(LPCDIEFFECT peff)
        {
            if (peff->cbTypeSpecificParams < sizeof(DirectInputTypeSpecificParameterType))
                return nullptr;

            if (nullptr == peff->lpvTypeSpecificParams)
                return nullptr;

            const DirectInputTypeSpecificParameterType& kDirectInputTypeSpecificParams = *((DirectInputTypeSpecificParameterType*)peff->lpvTypeSpecificParams);
            const TypeSpecificParameterType kTypeSpecificParameters = ConvertFromDirectInput(kDirectInputTypeSpecificParams);

            if (false == TypedUnderlyingEffect().AreTypeSpecificParametersValid(kTypeSpecificParameters))
                return nullptr;

            std::unique_ptr<Controller::ForceFeedback::Effect> updatedEffect = TypedUnderlyingEffect().Clone();
            ((Controller::ForceFeedback::EffectWithTypeSpecificParameters<TypeSpecificParameterType>*)updatedEffect.get())->SetTypeSpecificParameters(kTypeSpecificParameters);

            return std::move(updatedEffect);
        }
    };

    /// Concrete DirectInput force feedback effect object type for constant force effects.
    /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types and interfaces.
    template <ECharMode charMode> class ConstantForceDirectInputEffect : public VirtualDirectInputEffectWithTypeSpecificParameters<charMode, DICONSTANTFORCE, Controller::ForceFeedback::SConstantForceParameters>
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Initialization constructor.
        /// Simply delegates to the base class.
        inline ConstantForceDirectInputEffect(VirtualDirectInputDevice<charMode>& associatedDevice, const Controller::ForceFeedback::ConstantForceEffect& effect, const GUID& effectGuid) : VirtualDirectInputEffectWithTypeSpecificParameters<charMode, DICONSTANTFORCE, Controller::ForceFeedback::SConstantForceParameters>(associatedDevice, effect, effectGuid)
        {
            // Nothing to do here.
        }


    protected:
        // -------- CONCRETE INSTANCE METHODS -------------------------------------- //

        Controller::ForceFeedback::SConstantForceParameters ConvertFromDirectInput(const DICONSTANTFORCE& diTypeSpecificParams) const override
        {
            return {.magnitude = (Controller::ForceFeedback::TEffectValue)diTypeSpecificParams.lMagnitude};
        }

        DICONSTANTFORCE ConvertToDirectInput(const Controller::ForceFeedback::SConstantForceParameters& typeSpecificParams) const override
        {
            return {.lMagnitude = (LONG)typeSpecificParams.magnitude};
        }
    };
}
