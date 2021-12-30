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

#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"

#include <cstdint>
#include <optional>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            /// Base class for all force feedback effects.
            /// Holds common parameters and provides some common functionality but otherwise delegates key computations to subclasses.
            class Effect
            {
            private:
                // -------- INSTANCE VARIABLES ----------------------------- //

                /// Holds parameters common to all effects.
                SCommonParameters commonParameters;

                /// Alternative representation of the gain as a fraction to be multiplied by the final magnitude.
                /// Stored as a slight performance optimization to avoid a division operation each time magnitude is computed.
                TEffectValue gainFraction = SCommonParameters::kDefaultGain / kEffectModifierRelativeDenominator;

                /// Alternative representation of the sample period to be used directly by computations.
                /// Avoids a computation-time conditional by providing a value that can be used without checking for equality with 0.
                /// The default sample period is 1, meaning update the magnitude at the finest granularity possible.
                TEffectTimeMs samplePeriodForComputations = (0 == SCommonParameters::kDefaultSamplePeriod) ? 1 : SCommonParameters::kDefaultSamplePeriod;


            public:
                // -------- CONSTRUCTION AND DESTRUCTION ------------------- //

                /// Default destructor.
                virtual ~Effect(void) = default;


            protected:
                // -------- ABSTRACT INSTANCE METHODS ---------------------- //

                /// Internal implementation of calculations for computing the magnitude of a force feedback effect at a given time.
                /// Subclasses must implement this method and in general should not need any access to the common parameters.
                /// For performance reasons this method need not check for any errors and is allowed to return an indistinguishably invalid value, or even throw an exception, if the effect is ill-defined.
                /// @param [in] rawTime Time for which the magnitude is being requested. Raw input to the function that computes magnitude as a function of time.
                /// @return Raw magnitude value that corresponds to the given time, assuming the effect is completely defined (i.e. all parameters are set), and any other value otherwise.
                virtual TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const = 0;


                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                /// Verifies that all required type-specific parameters have been specified for this effect.
                /// The default implementation simply returns `true` because no type-specific parameters exist in the base case.
                /// Subclasses that define their own type-specific parameters should override this method if any type-specific parameters are essential and must be supplied.
                /// @return `true` if all type-specific parameters are valid and have been defined, `false` otherwise.
                virtual bool IsTypeSpecificEffectCompletelyDefined(void) const
                {
                    return true;
                }


            public:
                // -------- INSTANCE METHODS ------------------------------- //

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

                /// Computes the magnitude component vector of the force that this effect should generate at the given time.
                /// @param [in] time Time for which the magnitude is being requested relative to when the application requested the effect be started.
                /// @return Magnitude component vector that corresponds to the given time, assuming the effect is completely defined (i.e. parameters are all set), and any other value otherwise.
                inline TMagnitudeComponents ComputeMagnitudeComponents(TEffectTimeMs time) const
                {
                    return commonParameters.direction.ComputeMagnitudeComponents(ComputeMagnitude(time));
                }

                /// Provides access to the direction vector associated with this force feedback effect.
                /// @return Mutable reference to the direction vector object.
                inline DirectionVector& Direction(void)
                {
                    return commonParameters.direction;
                }

                /// Checks if the direction vector associated with this force feedback effect has a direction set.
                /// @return `true` if so, `false` otherwise.
                inline bool HasDirection(void) const
                {
                    return commonParameters.direction.HasDirection();
                }

                /// Checks if this force feedback effect has a duration set.
                /// @return `true` if so, `false` otherwise.
                inline bool HasDuration(void) const
                {
                    return commonParameters.duration.has_value();
                }

                /// Initializes the direction vector associated with this force feedback effect to a simple default of one axis in the positive direction.
                /// The Cartesian coordinate system is used.
                /// Primarily useful for testing.
                /// @return `true` if the direction initialization operation succeeded, `false` otherwise.
                inline bool InitializeDefaultDirection(void)
                {
                    static constexpr TEffectValue kDefaultCartesianCoordinates[] = { 1 };
                    return commonParameters.direction.SetDirectionUsingCartesian(kDefaultCartesianCoordinates, _countof(kDefaultCartesianCoordinates));
                }

                /// Verifies that all required parameters have been specified for this effect.
                /// If this method returns `true` then the effect is ready to be played.
                /// @return `true` if all parameters have been specified for this effect, `false` otherwise.
                inline bool IsCompletelyDefined(void) const
                {
                    return (HasDirection() && HasDuration() && IsTypeSpecificEffectCompletelyDefined());
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
                inline std::optional<SEnvelope> GetEnvelope(void) const
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
                inline bool SetEnvelope(const SEnvelope& newValue)
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
                // -------- INSTANCE VARIABLES ----------------------------- //

                /// Holds type-specific parameters.
                /// The exact contents vary by subclass.
                std::optional<TypeSpecificParameterType> typeSpecificParameters = std::nullopt;


            protected:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

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
                // -------- INSTANCE METHODS ------------------------------- //

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
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                bool AreTypeSpecificParametersValid(const SConstantForceParameters& newTypeSpecificParameters) const override;
                TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override;
            };
        }
    }
}
