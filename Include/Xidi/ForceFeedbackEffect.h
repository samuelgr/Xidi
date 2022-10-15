/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file ForceFeedbackEffect.h
 *   Interface declaration for objects that represent individual force
 *   feedback effects.
 *****************************************************************************/

#pragma once

#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"

#include <memory>
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

                /// Effect identifier.
                /// Effect objects of the same type can exist in multiple instances based on the idea of an effect object existing both in software and in a physical device buffer.
                /// In software, effect parameters can change and then they will need to be synchronized with the physical device buffer's version of the effect.
                /// An effect identifier being the same between two different instances means they are eligible for such synchronization because they are semantically supposed to refer to the same effect.
                const TEffectIdentifier id;

                /// Holds parameters common to all effects.
                SCommonParameters commonParameters;


            public:
                // -------- CONSTRUCTION AND DESTRUCTION ------------------- //

                /// Default constructor.
                Effect(void);

                /// Default destructor.
                virtual ~Effect(void) = default;


                // -------- ABSTRACT INSTANCE METHODS ---------------------- //

                /// Allocates, constructs, and returns a pointer to a copy of this force feedback effect.
                /// @return Smart pointer to a copy of this force feedback effect.
                virtual std::unique_ptr<Effect> Clone(void) const = 0;

            protected:
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

                /// Synchronizes the type-specific parameters in this effect with those in the supplied source effect.
                /// This is accomplished by copying the parameter values from the source effect.
                /// The default implementation does nothing, but subclasses that use type-specific parameters should override this method.
                /// No error-checking is required of subclasses.
                /// @param [in] source Source effect object from which type-specific parameters should be synchronized.
                virtual void SyncTypeSpecificParametersFrom(const Effect& source)
                {
                    // Nothing to do here.
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

                /// Retrieves and returns a read-only reference to the entire common parameters record associated with this effect.
                /// Intended to be used by tests.
                /// @return Read-only reference to common parameters data structure.
                inline const SCommonParameters& CommonParameters(void)
                {
                    return commonParameters;
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

                /// Computes the magnitude component vector of the force that this effect should generate at the given time using a globally-understood ordering scheme for the components.
                /// @param [in] time Time for which the magnitude is being requested relative to when the application requested the effect be started.
                /// @return Ordered magnitude component vector that corresponds to the given time, assuming the effect is completely defined (i.e. parameters are all set), and any other value otherwise.
                inline TOrderedMagnitudeComponents ComputeOrderedMagnitudeComponents(TEffectTimeMs time) const
                {
                    return OrderMagnitudeComponents(ComputeMagnitudeComponents(time));
                }

                /// Provides access to the direction vector associated with this force feedback effect.
                /// @return Mutable reference to the direction vector object.
                inline DirectionVector& Direction(void)
                {
                    return commonParameters.direction;
                }

                /// Retrieves and returns this effect's associated axes as a read-only reference.
                /// @return Associated axis data structure, if it exists.
                inline const std::optional<SAssociatedAxes>& GetAssociatedAxes(void) const
                {
                    return commonParameters.associatedAxes;
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

                /// Checks if there are valid axes associated with this force feedback effect.
                /// @return `true` if so, `false` otherwise.
                inline bool HasAssociatedAxes(void) const
                {
                    return commonParameters.associatedAxes.has_value();
                }

                /// Checks if the direction and associated axes are complete and consistent.
                /// @return `true` if so, `false` otherwise.
                inline bool HasCompleteDirection(void) const
                {
                    return (HasAssociatedAxes() && HasDirection() && (commonParameters.associatedAxes.value().count >= commonParameters.direction.GetNumAxes()));
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

                /// Checks if this force feedback effect has an envelope.
                /// @return `true` if so, `false` otherwise.
                inline bool HasEnvelope(void) const
                {
                    return commonParameters.envelope.has_value();
                }

                /// Retrieves and returns this effect's identifier.
                /// @return This effect's identifier.
                inline TEffectIdentifier Identifier(void) const
                {
                    return id;
                }
                
                /// Initializes the axes associated with this force feedback effect to a simple default of the X axis.
                /// @return `true` if the associated axis initialization operation succeeded, `false` otherwise.
                inline bool InitializeDefaultAssociatedAxes(void)
                {
                    static constexpr SAssociatedAxes kDefaultAssociatedAxes = {.count = 1, .type = {EAxis::X}};
                    return SetAssociatedAxes(kDefaultAssociatedAxes);
                }

                /// Initializes the direction vector associated with this force feedback effect to a simple default of one axis in the positive direction.
                /// The Cartesian coordinate system is used.
                /// Primarily useful for testing.
                /// @return `true` if the direction initialization operation succeeded, `false` otherwise.
                inline bool InitializeDefaultDirection(void)
                {
                    static const TEffectValue kDefaultCartesianCoordinates[] = {1};
                    return commonParameters.direction.SetDirectionUsingCartesian(kDefaultCartesianCoordinates, _countof(kDefaultCartesianCoordinates));
                }

                /// Verifies that all required parameters have been specified for this effect.
                /// If this method returns `true` then the effect is ready to be played.
                /// @return `true` if all parameters have been specified for this effect, `false` otherwise.
                inline bool IsCompletelyDefined(void) const
                {
                    return (HasCompleteDirection() && HasDuration() && IsTypeSpecificEffectCompletelyDefined());
                }

                /// Orders the elements in a magnitude component vector using a globally-understood ordering scheme for the components.
                /// Exposed primarily for testing.
                /// @param [in] unorderedMagnitudeComponents Raw magnitude component vector, such as that produced by #ComputeMagnitudeComponents.
                /// @return Ordered magnitude component vector that corresponds to the unordered magnitude component vector provided as input assuming this effect is completely defined, and any other value otherwise.
                inline TOrderedMagnitudeComponents OrderMagnitudeComponents(TMagnitudeComponents unorderedMagnitudeComponents) const
                {
                    const SAssociatedAxes& kAssociatedAxes = commonParameters.associatedAxes.value();

                    TOrderedMagnitudeComponents orderedMagnitudeComponents = {};

                    // Compare with the number of axes in the direction vector because it is allowed to be less, but not greater, than the number of axes in the associated axis array.
                    for (int i = 0; i < commonParameters.direction.GetNumAxes(); ++i)
                        orderedMagnitudeComponents[(int)kAssociatedAxes.type[i]] = unorderedMagnitudeComponents[i];

                    return orderedMagnitudeComponents;
                }

                /// Updates this effect's associated axes.
                /// @param [in] newValue New parameter value.
                /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
                inline bool SetAssociatedAxes(SAssociatedAxes newValue)
                {
                    if ((newValue.count >= kEffectAxesMinimumNumber) && (newValue.count <= kEffectAxesMaximumNumber))
                    {
                        commonParameters.associatedAxes = newValue;
                        return true;
                    }

                    return false;
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
                    commonParameters.SetSamplePeriod(newValue);
                    return true;
                }

                /// Updates this effect's gain parameter.
                /// @param [in] newValue New parameter value.
                /// @return `true` if successful, `false` otherwise. This method will fail if the new parameter value is invalid.
                inline bool SetGain(TEffectValue newValue)
                {
                    if ((newValue >= kEffectModifierMinimum) && (newValue <= kEffectModifierMaximum))
                    {
                        commonParameters.SetGain(newValue);
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

                /// Synchronizes the parameters in this effect with those in the supplied source effect by copying the parameter values from the source effect.
                /// This is only possible if this effect and the other effect share the same identifier.
                /// @param [in] source Source effect object from which parameters should be synchronized.
                /// @return `true` if successful, `false` otherwise. This method will fail if the source effect's identifier does not match that of this effect.
                inline bool SyncParametersFrom(const Effect& other)
                {
                    if (other.id == id)
                    {
                        commonParameters = other.commonParameters;
                        SyncTypeSpecificParametersFrom(other);
                        return true;
                    }

                    return false;
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


            public:
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

                /// Checks the contents of the supplied type-specific parameters and applies modifications in case they have invalidities that can easily be corrected.
                /// Invoked by this class whenever it is requested that type-specific parameters be set but before it checks them for validity, as above.
                /// Default implementation does not perform any action whatsoever. Subclasses may override this method.
                /// @param newTypeSpecificParameters Mutable reference to the type-specific parameters to check and possibly fix.
                virtual void CheckAndFixTypeSpecificParameters(TypeSpecificParameterType& newTypeSpecificParameters) const
                {
                    // Nothing to do here.
                }

            protected:
                /// Default implementation of checking that this type-specific event is completely defined, which simply verifies that type-specific parameters exist.
                /// Subclasses that define more complex type-specific parameters or need to do other checks can override this method.
                /// @return `true` if all type-specific parameters are valid and have been defined, `false` otherwise.
                bool IsTypeSpecificEffectCompletelyDefined(void) const override
                {
                    return typeSpecificParameters.has_value();
                }

                /// Default implementation of synchronizing type-specific parameters from the supplied source.
                /// No error checking is required here because the superclass takes care of that.
                /// @param [in] source Source effect object from which type-specific parameters should be synchronized.
                void SyncTypeSpecificParametersFrom(const Effect& source) override
                {
                    typeSpecificParameters = ((const EffectWithTypeSpecificParameters&)source).typeSpecificParameters;
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

                /// Checks if this object has type-specific parameters set.
                /// @return `true` if so, `false` otherwise.
                inline bool HasTypeSpecificParameters(void) const
                {
                    return typeSpecificParameters.has_value();
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
                    else
                    {
                        TypeSpecificParameterType fixedTypeSpecificParameters = newTypeSpecificParameters;
                        CheckAndFixTypeSpecificParameters(fixedTypeSpecificParameters);

                        if (true == AreTypeSpecificParametersValid(fixedTypeSpecificParameters))
                        {
                            typeSpecificParameters = fixedTypeSpecificParameters;
                            return true;
                        }
                    }

                    return false;
                }
            };

            /// Holds all type-specific parameters for constant force effects.
            struct SConstantForceParameters
            {
                /// Magnitude of the constant force, which must fall within the allowed magnitude range.
                TEffectValue magnitude;

                /// Simple check for equality.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SConstantForceParameters& other) const = default;
            };

            /// Implements a force feedback effect based on a force of constant magnitude.
            class ConstantForceEffect : public EffectWithTypeSpecificParameters<SConstantForceParameters>
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                bool AreTypeSpecificParametersValid(const SConstantForceParameters& newTypeSpecificParameters) const override;
                void CheckAndFixTypeSpecificParameters(SConstantForceParameters& newTypeSpecificParameters) const override;
                std::unique_ptr<Effect> Clone(void) const override;

            protected:
                TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override;
            };

            /// Holds all type-specific parameters for periodic effects.
            struct SPeriodicParameters
            {
                /// Amplitude of the periodic effect, which must be non-negative and within the allowed magnitude range.
                TEffectValue amplitude;

                /// Relative baseline for the amplitude. Typically this is zero, but a non-zero value here can shift the periodic effect up or down. Must be within the allowed magnitude range.
                TEffectValue offset;

                /// Position in the cycle at which the effect starts, measured in hundredths of degrees. Must be within the allowed angle range.
                TEffectValue phase;

                /// Time length of the cycle of the effect.
                TEffectTimeMs period;

                /// Simple check for equality.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SPeriodicParameters& other) const = default;
            };

            /// Abstract base class for periodic force feedback effects.
            class PeriodicEffect : public EffectWithTypeSpecificParameters<SPeriodicParameters>
            {
            public:
                // -------- INSTANCE METHODS ------------------------------- //

                /// Computes the current phase point within the waveform at the specified time.
                /// Intended for internal use but exposed for testing.
                /// @param [in] rawTime Time for which the phase point is being requested.
                /// @return Current phase point, expressed as an angle measured in hundredths of degrees.
                TEffectValue ComputePhase(TEffectTimeMs rawTime) const;


                // -------- ABSTRACT INSTANCE METHODS ---------------------- //

                /// Computes the amplutide proportion for the given phase.
                /// This method is intended to return a value between -1.0 and 1.0 inclusive that defines the waveform of the periodic effect.
                /// @param [in] phase Current point in the phase of the waveform for which an amplitude proportion is desired.
                /// @return Value between -1.0 and 1.0 indicating the behavior of the waveform at the specified point in the phase.
                virtual TEffectValue WaveformAmplitude(TEffectValue phase) const = 0;


                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                bool AreTypeSpecificParametersValid(const SPeriodicParameters& newTypeSpecificParameters) const override;

            protected:
                TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override;
            };

            /// Concrete implementation of a periodic effect for waves that follow a sawtooth pattern in the downwards direction.
            class SawtoothDownEffect : public PeriodicEffect
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                TEffectValue WaveformAmplitude(TEffectValue phase) const override;
                std::unique_ptr<Effect> Clone(void) const override;
            };

            /// Concrete implementation of a periodic effect for waves that follow a sawtooth pattern in the upwards direction.
            class SawtoothUpEffect : public PeriodicEffect
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                TEffectValue WaveformAmplitude(TEffectValue phase) const override;
                std::unique_ptr<Effect> Clone(void) const override;
            };

            /// Concrete implementation of a periodic effect for sine waves.
            class SineWaveEffect : public PeriodicEffect
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                TEffectValue WaveformAmplitude(TEffectValue phase) const override;
                std::unique_ptr<Effect> Clone(void) const override;
            };

            /// Concrete implementation of a periodic effect for square waves.
            class SquareWaveEffect : public PeriodicEffect
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                TEffectValue WaveformAmplitude(TEffectValue phase) const override;
                std::unique_ptr<Effect> Clone(void) const override;
            };

            /// Concrete implementation of a periodic effect for triangle waves.
            class TriangleWaveEffect : public PeriodicEffect
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                TEffectValue WaveformAmplitude(TEffectValue phase) const override;
                std::unique_ptr<Effect> Clone(void) const override;
            };

            /// Holds all type-specific parameters for ramp force effects.
            struct SRampForceParameters
            {
                /// Starting magnitude, which must fall within the allowed magnitude range.
                TEffectValue magnitudeStart;

                /// Ending magnitude, which must fall within the allowed magnitude range.
                TEffectValue magnitudeEnd;

                /// Simple check for equality.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SRampForceParameters& other) const = default;
            };

            /// Implements a force feedback effect based on a force with a magnitude that changes linearly with time.
            class RampForceEffect : public EffectWithTypeSpecificParameters<SRampForceParameters>
            {
            public:
                // -------- CONCRETE INSTANCE METHODS ---------------------- //

                bool AreTypeSpecificParametersValid(const SRampForceParameters& newTypeSpecificParameters) const override;
                std::unique_ptr<Effect> Clone(void) const override;

            protected:
                TEffectValue ComputeRawMagnitude(TEffectTimeMs rawTime) const override;
            };
        }
    }
}
