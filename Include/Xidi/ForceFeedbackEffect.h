/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackEffect.h
 *   Interface declaration for objects that represent individual force
 *   feedback effects.
 *****************************************************************************/

#pragma once

#include <cstdint>
#include <optional>


namespace Xidi
{
    namespace ForceFeedback
    {
        /// Type used for keeping track of time as it relates to force feedback effects in millisecond units.
        typedef uint32_t TEffectTimeMs;

        /// Type used for all values used in internal effect-related computations.
        typedef float TEffectValue;

        /// Minimum value for an effect modifier.
        inline constexpr TEffectValue kEffectModifierMinimum = 0;

        /// Maximum value for an effect modifier.
        inline constexpr TEffectValue kEffectModifierMaximum = 10000;

        /// Denominator for relative effect modifiers.
        inline constexpr TEffectValue kEffectModifierRelativeDenominator = (kEffectModifierMaximum - kEffectModifierMinimum);

        /// Minimum value for an effect's output magnitude.
        /// This value is intended to signify full device strength in the negative direction.
        inline constexpr TEffectValue kEffectForceMagnitudeMinimum = -10000;

        /// Maximum value for an effect's output magnitude.
        /// This value is intended to signify full device strength in the positive direction.
        inline constexpr TEffectValue kEffectForceMagnitudeMaximum = 10000;

        /// Zero value for an effect's output magnitude.
        /// This value is intended to signify that there is no force generated at all.
        inline constexpr TEffectValue kEffectForceMagnitudeZero = 0;

        /// Structure for representing an envelope that might be applied to an effect.
        /// See https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416225%28v=vs.85%29 for more information on how envelopes work.
        struct SEffectEnvelope
        {
            /// Duration of the "attack" part of the envelope.
            /// The attack transformation is applied from time 0 to this time.
            TEffectTimeMs attackTime;

            /// Desired amplitude for the "attack" part of the envelope which occurs at the very beginning of the effect.
            TEffectValue attackLevel;

            /// Duration of the "fade" part of the envelope.
            /// The fade transformation is applied from this time before the end of the effect and finishes right at the end of the effect.
            TEffectTimeMs fadeTime;

            /// Desired amplitude for the "fade" part of the envelope which occurs at the very end of the effect.
            TEffectValue fadeLevel;

            /// Simple check for equality.
            /// Primarily useful during testing.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            constexpr inline bool operator==(const SEffectEnvelope& other) const = default;
        };

        /// Structure for holding parameters common to all effects.
        struct SEffectCommonParameters
        {
            /// Default values for all common parameters.
            /// See specific named field descriptions for documentation.
            static constexpr TEffectTimeMs kDefaultStartDelay = 0;
            static constexpr TEffectTimeMs kDefaultSamplePeriod = 0;
            static constexpr TEffectValue kDefaultGain = kEffectModifierRelativeDenominator;
            static constexpr std::optional<SEffectEnvelope> kDefaultEnvelope = std::nullopt;

            /// Total playback time of the effect.
            /// Does not include any start delay, just includes the amount of time potentially generating a force.
            /// It is an error for the application not to specify a value.
            std::optional<TEffectTimeMs> duration = std::nullopt;

            /// Amount of time to wait before starting to play back the effect. Not counted in the duration.
            /// Once the application asks to play the effect, this start delay is a wait time and then immediately thereafter the effect plays for the requested duration.
            TEffectTimeMs startDelay = kDefaultStartDelay;

            /// Granularity with which to generate samples.
            /// The exact magnitude of a force is computed as a function of time.
            /// This value specifies the increments of time that are passed into the computation function.
            /// For example, a value of 10 would indicate that the input to the computation function increases in increments of 10 milliseconds.
            /// A value of 0 means to use the default sample period.
            TEffectTimeMs samplePeriod = kDefaultSamplePeriod;

            /// Overall adjustment to the magnitude of a force feedback effect.
            /// This modifier acts as a per-effect "volume control" knob.
            TEffectValue gain = kDefaultGain;

            /// Optional envelope to be applied as a transformation to this effect.
            /// If not present then no envelope is applied when this effect's force magnitude is computed.
            std::optional<SEffectEnvelope> envelope = kDefaultEnvelope;
        };

        /// Base class for all force feedback effects.
        /// Holds common parameters and provides some common functionality but otherwise delegates key computations to subclasses.
        class Effect
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds parameters common to all effects.
            SEffectCommonParameters commonParameters;

            /// Alternative representation of the gain as a fraction to be multiplied by the final magnitude.
            /// Stored as a slight performance optimization to avoid a division operation each time magnitude is computed.
            TEffectValue gainFraction = SEffectCommonParameters::kDefaultGain / kEffectModifierRelativeDenominator;

            /// Alternative representation of the sample period to be used directly by computations.
            /// Avoids a computation-time conditional by providing a value that can be used without checking for equality with 0.
            /// The default sample period is 1, meaning update the magnitude at the finest granularity possible.
            TEffectTimeMs samplePeriodForComputations = (0 == SEffectCommonParameters::kDefaultSamplePeriod) ? 1 : SEffectCommonParameters::kDefaultSamplePeriod;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default destructor.
            virtual ~Effect(void) = default;


        protected:
            // -------- ABSTRACT INSTANCE METHODS -------------------------- //

            /// Internal implementation of calculations for computing the magnitude of a force feedback effect at a given time.
            /// Subclasses must implement this method and in general should not need any access to the common parameters.
            /// For performance reasons this method need not check for any errors and is allowed to return an indistinguishably invalid value, or even throw an exception, if the effect is ill-defined.
            /// @param [in] rawTime Time for which the magnitude is being requested. Raw input to the function that computes magnitude as a function of time.
            /// @return Raw magnitude value that corresponds to the given time, assuming the effect is completely defined (i.e. all parameters are set), and any other value otherwise.
            virtual TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const = 0;


            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            /// Verifies that all required type-specific parameters have been specified for this effect.
            /// The default implementation simply returns `true` because no type-specific parameters exist in the base case.
            /// Subclasses that define their own type-specific parameters should override this method if any type-specific parameters are essential and must be supplied.
            /// @return `true` if all type-specific parameters are valid and have been defined, `false` otherwise.
            virtual bool IsTypeSpecificEffectCompletelyDefined(void) const
            {
                return true;
            }


        public:
            // -------- INSTANCE METHODS ----------------------------------- //

            /// Applies the envelope parameter to transform the specified sustain level value at a given time.
            /// Intended to be invoked by subclasses to assist with envelope transformations but exposed for testing.
            /// For performance reasons this method does not check if the effect is ill-formed and may throw an exception if it is.
            /// @param [in] rawTime Time for which the transformation is being requested. Raw input to the function that computes magnitude as a function of time.
            /// @param [in] sustainLevel Sustain level of the effect, often just the effect's nominal amplitude. Subclasses are able to compute a sustain level based on whatever type-specific parameters they use. This parameter must be non-negative.
            /// @return Amplitude of the effect for the specified time that results from applying this effect's envelope transformation, if it exists.
            TEffectValue ApplyEnvelope(TEffectTimeMs rawTime, TEffectValue sustainLevel) const;

            /// Clears this effect's envelope parameter structure, which results in disabling envelope transformations for this effect.
            inline void ClearEnvelope(void)
            {
                commonParameters.envelope = std::nullopt;
            }

            /// Computes the magnitude of the force that this effect should generate at the given time.
            /// Returns zero-magnitude during the start delay interval or once the duration has fully elapsed.
            /// Internally, this method performs computations common to all force feedback effects before delegating the raw calculations to subclasses.
            /// Intended to be invoked externally for determining the magnitude contribution of a force.
            /// For performance reasons this method does not check for any errors and will simply return an indistinguishably invalid value, or even throw an exception, if the effect is ill-defined.
            /// @param [in] time Time for which the magnitude is being requested relative to when the application requested the effect be started.
            /// @return Magnitude value that corresponds to the given time, assuming the effect is completely defined (i.e. parameters are all set), and any other value otherwise.
            TEffectValue ComputeMagnitude(TEffectTimeMs time) const;

            /// Verifies that all required parameters have been specified for this effect.
            /// If this method returns `true` then the effect is ready to be played.
            /// @return `true` if all parameters have been specified for this effect, `false` otherwise.
            inline bool IsCompletelyDefined(void) const
            {
                return (true == commonParameters.duration.has_value()) && IsTypeSpecificEffectCompletelyDefined();
            }

            /// Retrieves and returns this effect's duration parameter.
            /// @return Requested parameter value, if it exists.
            inline std::optional<TEffectTimeMs> GetDuration(void) const
            {
                return commonParameters.duration;
            }

            /// Retrieves and returns this effect's start delay parameter.
            /// @return Requested parameter value.
            inline TEffectTimeMs GetStartDelay(void) const
            {
                return commonParameters.startDelay;
            }

            /// Retrieves and returns this effect's sample period parameter.
            /// @return Requested parameter value.
            inline TEffectTimeMs GetSamplePeriod(void) const
            {
                return commonParameters.samplePeriod;
            }

            /// Retrieves and returns this effect's gain parameter.
            /// @return Requested parameter value.
            inline TEffectValue GetGain(void) const
            {
                return commonParameters.gain;
            }

            /// Retrieves and returns this effect's envelope parameter.
            /// @return Envelope parameter structure, if it exists.
            inline std::optional<SEffectEnvelope> GetEnvelope(void) const
            {
                return commonParameters.envelope;
            }

            /// Computes and returns this effect's total time.
            /// Includes both duration and any start delay.
            /// @return Effect's total time.
            inline TEffectTimeMs GetTotalTime(void) const
            {
                return commonParameters.duration.value_or(0) + commonParameters.startDelay;
            }

            /// Updates this effect's duration parameter.
            /// @param [in] newValue New parameter value.
            /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
            inline bool SetDuration(TEffectTimeMs newValue)
            {
                if (newValue > 0)
                {
                    commonParameters.duration = newValue;
                    return true;
                }

                return false;
            }

            /// Updates this effect's start delay parameter.
            /// @param [in] newValue New parameter value.
            /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
            inline bool SetStartDelay(TEffectTimeMs newValue)
            {
                commonParameters.startDelay = newValue;
                return true;
            }

            /// Updates this effect's sample period parameter.
            /// A value of 0 means to use the default sample period, which for internal calculation purposes is equivalent to passing in a value of 1.
            /// Internally, a value of 0 is invalid, though externally it is allowed to be passed.
            /// @param [in] newValue New parameter value.
            /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
            inline bool SetSamplePeriod(TEffectTimeMs newValue)
            {
                commonParameters.samplePeriod = newValue;

                if (0 == newValue)
                    samplePeriodForComputations = 1;
                else
                    samplePeriodForComputations = newValue;

                return true;
            }

            /// Updates this effect's gain parameter.
            /// @param [in] newValue New parameter value.
            /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
            inline bool SetGain(TEffectValue newValue)
            {
                if ((newValue >= kEffectModifierMinimum) && (newValue <= kEffectModifierMaximum))
                {
                    commonParameters.gain = newValue;
                    gainFraction = (newValue / kEffectModifierRelativeDenominator);
                    return true;
                }

                return false;
            }

            /// Updates this effect's envelope parameter structure.
            /// @param [in] newValue New parameter value.
            /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
            inline bool SetEnvelope(const SEffectEnvelope& newValue)
            {
                if ((newValue.attackLevel < kEffectModifierMinimum) || (newValue.attackLevel > kEffectModifierMaximum))
                    return false;

                if ((newValue.fadeLevel < kEffectModifierMinimum) || (newValue.fadeLevel > kEffectModifierMaximum))
                    return false;

                commonParameters.envelope = newValue;
                return true;
            }
        };

        /// Intermediate abstract class for all effects that define their own type-specific parameters.
        /// Provides a default implementation of certain common functionality for such effects.
        /// @tparam TypeSpecificParameterType Type that holds all type-specific effect parameters, usually a structure.
        template <typename TypeSpecificParameterType> class EffectWithTypeSpecificParameters : public Effect
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Holds type-specific parameters.
            /// The exact contents vary by subclass.
            std::optional<TypeSpecificParameterType> typeSpecificParameters = std::nullopt;


        protected:
            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            /// Validates that the contents of the supplied type-specific parameters are valid.
            /// Invoked by this class whenever it is requested that type-specific parameters be set.
            /// Default implementation does not perform any actual checks and simply returns success. Subclasses should override this method.
            /// @param newTypeSpecificParameters Read-only reference to the type-specific parameters to check.
            /// @return `true` if the contents of the supplied parameter pass semantic validation checks, `false` otherwise.
            virtual bool AreTypeSpecificParametersValid(const TypeSpecificParameterType& newTypeSpecificParameters) const
            {
                return true;
            }

            /// Default implementation of checking that this type-specific event is completely defined, which simply verifies that type-specific parameters exist.
            /// Subclasses that define more complex type-specific parameters or need to do other checks can override this method.
            /// @return `true` if all type-specific parameters are valid and have been defined, `false` otherwise.
            bool IsTypeSpecificEffectCompletelyDefined(void) const override
            {
                return typeSpecificParameters.has_value();
            }


        public:
            // -------- INSTANCE METHODS ----------------------------------- //

            /// Clears this effect's type-specific parameters.
            inline void ClearTypeSpecificParameters(void)
            {
                typeSpecificParameters = std::nullopt;
            }

            /// Retrieves and returns this effect's type-specific parameters as a read-only reference.
            /// @return Type-specific parameters, if they exist.
            inline const std::optional<TypeSpecificParameterType>& GetTypeSpecificParameters(void) const
            {
                return typeSpecificParameters;
            }

            /// Updates this effect's type-specific parameters.
            /// @param [in] newTypeSpecificParameters New type-specific parameters.
            /// @return `true` if successful, `false` otherwise. This method will fail if the new type-specific parameters are invalid.
            inline bool SetTypeSpecificParameters(const TypeSpecificParameterType& newTypeSpecificParameters)
            {
                if (true == AreTypeSpecificParametersValid(newTypeSpecificParameters))
                {
                    typeSpecificParameters = newTypeSpecificParameters;
                    return true;
                }

                return false;
            }
        };

        /// Holds all type-specific parameters for constant force effects.
        struct SConstantForceParameters
        {
            TEffectValue magnitude;                                         ///< Magnitude of the constant force, which must fall within the allowed magnitude range.
        };

        /// Implements a force feedback effect based on a force of constant magnitude.
        class ConstantForceEffect : public EffectWithTypeSpecificParameters<SConstantForceParameters>
        {
        protected:
            // -------- CONCRETE INSTANCE METHODS -------------------------- //

            bool AreTypeSpecificParametersValid(const SConstantForceParameters& newTypeSpecificParameters) const override;
            TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override;
        };
    }
}
