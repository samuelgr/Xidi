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
        // -------- CONSTANTS ------------------------------------------------------ //
    public:

        /// Scaling factor for converting between DirectInput force feedback effect time units and internal Xidi force feedback time units.
        /// DirectInput expresses all times using microseconds, whereas Xidi uses milliseconds.
        static constexpr DWORD kTimeScalingFactor = 1000;


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


        // -------- CLASS METHODS -------------------------------------------------- //

        /// Converts the specified time interval, represented in DirectInput units, to internal Xidi time units.
        /// @param [in] diTime Amount of time, represented using DirectInput units.
        /// @return Amount of time, represented using internal Xidi units.
        static inline Controller::ForceFeedback::TEffectTimeMs ConvertTimeFromDirectInput(DWORD diTime)
        {
            return (Controller::ForceFeedback::TEffectTimeMs)(diTime / kTimeScalingFactor);
        }

        /// Converts the specified time interval, represented in internal Xidi time units, to DirectInput time units.
        /// @param [in] effectTime Amount of time, represented using internal Xidi units.
        /// @return Amount of time, represented using DirectInput units.
        static inline DWORD ConvertTimeToDirectInput(Controller::ForceFeedback::TEffectTimeMs effectTime)
        {
            return (DWORD)effectTime * kTimeScalingFactor;
        }


        // -------- INSTANCE METHODS ----------------------------------------------- //

        /// Retrieves a reference to the underlying effect.
        /// Intended for internal use but additionally exposed for testing.
        /// @return Reference to the underlying effect object.
        inline Controller::ForceFeedback::Effect& UnderlyingEffect(void)
        {
            return *effect;
        }

        /// Internal implementation of downloading an effect.
        /// See DirectInput documentation for parameter and return type information.
        HRESULT DownloadInternal(void);

        /// Dumps the contents of the provided effect parameter structure to the log.
        /// Intended for internal use.
        /// @param [in] peff Pointer to the effect structure to dump.
        /// @param [in] dwFlags Flags that specify which members are valid.
        void DumpEffectParameters(LPCDIEFFECT peff, DWORD dwFlags) const;

        /// Internal implementation of setting an effect's parameters.
        /// Adds a timestamp parameter and serves as an entry point for tests that set effect parameters.
        /// See DirectInput documentation for parameter and return information.
        HRESULT SetParametersInternal(LPCDIEFFECT peff, DWORD dwFlags, std::optional<Controller::ForceFeedback::TEffectTimeMs> timestamp = std::nullopt);

        /// Internal implementation of starting an effect's playback.
        /// Adds an timestamp parameter and serves as an entry point for tests that start effect playback.
        /// See DirectInput documentation for parameter and return information.
        HRESULT StartInternal(DWORD dwIterations, DWORD dwFlags, std::optional<Controller::ForceFeedback::TEffectTimeMs> timestamp = std::nullopt);


    protected:
        // -------- CONCRETE INSTANCE METHODS -------------------------------------- //

        /// Dumps the type-specific parameters contained in the provided effect parameter structure to the log.
        /// Intended for internal use.
        /// @param [in] peff Pointer to the effect structure to dump.
        virtual void DumpTypeSpecificParameters(LPCDIEFFECT peff) const;

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

        void DumpTypeSpecificParameters(LPCDIEFFECT peff) const override;
    };

    /// Concrete DirectInput force feedback effect object type for periodic effects effects.
    /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types and interfaces.
    template <ECharMode charMode> class PeriodicDirectInputEffect : public VirtualDirectInputEffectWithTypeSpecificParameters<charMode, DIPERIODIC, Controller::ForceFeedback::SPeriodicParameters>
    {
    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Initialization constructor.
        /// Simply delegates to the base class.
        inline PeriodicDirectInputEffect(VirtualDirectInputDevice<charMode>& associatedDevice, const Controller::ForceFeedback::PeriodicEffect& effect, const GUID& effectGuid) : VirtualDirectInputEffectWithTypeSpecificParameters<charMode, DIPERIODIC, Controller::ForceFeedback::SPeriodicParameters>(associatedDevice, effect, effectGuid)
        {
            // Nothing to do here.
        }


    protected:
        // -------- CONCRETE INSTANCE METHODS -------------------------------------- //

        Controller::ForceFeedback::SPeriodicParameters ConvertFromDirectInput(const DIPERIODIC& diTypeSpecificParams) const override
        {
            return {
                .amplitude = (Controller::ForceFeedback::TEffectValue)diTypeSpecificParams.dwMagnitude,
                .offset = (Controller::ForceFeedback::TEffectValue)diTypeSpecificParams.lOffset,
                .phase = (Controller::ForceFeedback::TEffectValue)diTypeSpecificParams.dwPhase,
                .period = (Controller::ForceFeedback::TEffectTimeMs)diTypeSpecificParams.dwPeriod / 1000
            };
        }

        DIPERIODIC ConvertToDirectInput(const Controller::ForceFeedback::SPeriodicParameters& typeSpecificParams) const override
        {
            return {
                .dwMagnitude = (DWORD)typeSpecificParams.amplitude,
                .lOffset = (LONG)typeSpecificParams.offset,
                .dwPhase = (DWORD)typeSpecificParams.phase,
                .dwPeriod = (DWORD)typeSpecificParams.period * 1000
            };
        }

        void DumpTypeSpecificParameters(LPCDIEFFECT peff) const override;
    };
}
