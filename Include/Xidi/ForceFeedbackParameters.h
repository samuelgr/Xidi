/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file ForceFeedbackParameters.h
 *   Declaration of types and functions used to define the parameters that are
 *   common to all force feedback effects.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"
#include "ForceFeedbackTypes.h"

#include <array>
#include <optional>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            /// Represents the direction vector of a force using Cartesian, polar, and spherical coordinates.
            /// Used only for establishing the direction of a force. The vector's magnitude is unimportant.
            /// See https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416616%28v=vs.85%29 for more information on how direction vectors are represented.
            class DirectionVector
            {
            private:
                // -------- INSTANCE VARIABLES ----------------------------- //

                /// Number of axes represented by this direction vector.
                int numAxes;

                /// Whether or not this direction vector is omnidirectional.
                /// If a direction vector is omnidirectional then, when computing per-component magnitudes given an input magnitude, the input magnitude is simply copied to all the components without transformation.
                bool isOmnidirectional;

                /// Coordinate system that was used to set the direction of this vector, once it is set.
                std::optional<ECoordinateSystem> originalCoordinateSystem;

                /// Direction vector represented using Cartesian coordinates.
                /// Valid when any number of axes are present.
                TMagnitudeComponents cartesian;

                /// Direction vector represented using polar coordinates.
                /// Value is represented as an angle in hundredths of a degree from (0,-1) as a rotation towards (1,0).
                /// Valid only when exactly two axes are present and with certain specific values when only one axis is present.
                TEffectValue polar;

                /// Direction vector represented using spherical coordinates, represented as angles in hundredths of a degree.
                /// First element is an angle from (1,0) to (0,1), next element is the angle from that plane to (0,0,1), and so on.
                /// Valid when more than one axis is present and with certain specific values when only one axis is present.
                std::array<TEffectValue, kEffectAxesMaximumNumber - 1> spherical;


            public:
                // -------- CONSTRUCTION AND DESTRUCTION ------------------- //

                /// Default constructor.
                /// Initializes a direction vector with no coordinates.
                DirectionVector(void);


                // -------- OPERATORS -------------------------------------- //

                /// Simple check for equality. Primarily useful during testing.
                /// Vector equivalence relies on number of axes and spherical coordinates being the same. Polar and Cartesian coordinates can be ignored.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const DirectionVector& other) const
                {
                    if (other.numAxes != numAxes)
                        return false;

                    for (int i = 0; i < (numAxes - 1); ++i)
                    {
                        if (other.spherical[i] != spherical[i])
                            return false;
                    }

                    return true;
                }


                // -------- INSTANCE METHODS ------------------------------- //

                /// Given a force's magnitude, uses the direction represented by this direction vector to split it into per-axis components.
                /// @param [in] magnitude Magnitude of the force.
                /// @return Force vector, one element per axis represented by this direction vector.
                TMagnitudeComponents ComputeMagnitudeComponents(TEffectValue magnitude) const;

                /// Retrieves and returns the number of axes for which this direction vector holds a direction component.
                /// @return Number of axes represented.
                inline int GetNumAxes(void) const
                {
                    return numAxes;
                }

                /// Retrieves and returns the original coordinate system that was used to set this vector's direction.
                /// Does not verify that this vector actually has a direction set. Call #HasDirection to check for that.
                /// @return Coordinate system that was used.
                inline ECoordinateSystem GetOriginalCoordinateSystem(void) const
                {
                    return originalCoordinateSystem.value();
                }

                /// Retrieves the Cartesian coordinate representation of this direction vector.
                /// @param [out] coordinates Pointer to a buffer to hold the Cartesian components, one component per represented axis.
                /// @param [in] numCoordinates Size of the buffer in number of components it can hold.
                /// @return Number of compoments written. If this is less than the number of axes in this vector then the buffer was too small.
                int GetCartesianCoordinates(TEffectValue* coordinates, int numCoordinates) const;

                /// Retrieves the polar coordinate representation of this direction vector, assuming it is valid.
                /// @param [out] coordinates Pointer to a buffer to hold the polar coordinate components. Note that the polar coordinate system uses only one component.
                /// @param [in] numCoordinates Size of the buffer in number of components it can hold.
                /// @return Number of components written, which will be 0 or 1 depending on the buffer size and whether the polar coordinate system is valid for this vector.
                int GetPolarCoordinates(TEffectValue* coordinates, int numCoordinates) const;

                /// Retrieves the spherical coordinate representation of this direction vector.
                /// @param [out] coordinates Pointer to a buffer to hold the spherical coordinate components. Number of components is one less than the number of axes represented.
                /// @param [in] numCoordinates Size of the buffer in number of components it can hold.
                int GetSphericalCoordinates(TEffectValue* coordinates, int numCoordinates) const;

                /// Checks if this direction vector has a direction set.
                /// @return `true` if so, `false` otherwise.
                inline bool HasDirection(void) const
                {
                    return originalCoordinateSystem.has_value();
                }

                /// Checks if this direction vector is in omnidirectional mode.
                /// @return `true` if so, `false` otherwise.
                inline bool IsOmnidirectional(void) const
                {
                    return isOmnidirectional;
                }

                /// Attempts to change the direction represented by this direction vector using Cartesian coordinates.
                /// Number of axes is inferred based on the number of coordinates present.
                /// @param [in] coordinates Pointer to a buffer holding the new Cartesian coordinates, one component per represented axis.
                /// @param [in] numCoordinates Number of coordinates that should be read from the provided buffer.
                /// @return `true` if the direction was updated successfully, `false` otherwise. This method will fail if the provided input parameters are invalid.
                bool SetDirectionUsingCartesian(const TEffectValue* coordinates, int numCoordinates);

                /// Attempts to change the direction represented by this direction vector using polar coordinates.
                /// Only one coordinate can be provided, and number of axes is assumed to be 2.
                /// All angles provided as input must be between 0 and 359.99 degrees, inclusive.
                /// @param [in] coordinates Pointer to a buffer holding the new polar coordinates.
                /// @param [in] numCoordinates Number of coordinates that should be read from the provided buffer. Must be equal to 1.
                /// @return `true` if the direction was updated successfully, `false` otherwise. This method will fail if the provided input parameters are invalid.
                bool SetDirectionUsingPolar(const TEffectValue* coordinates, int numCoordinates);

                /// Attempts to change the direction represented by this direction vector using spherical coordinates.
                /// Number of axes is inferred by adding 1 to the number of coordinates provided.
                /// All angles provided as input must be between 0 and 359.99 degrees, inclusive.
                /// @param [in] coordinates Pointer to a buffer holding the new spherical coordinates, one component per spherical angle.
                /// @param [in] numCoordinates Number of coordinates that should be read from the provided buffer.
                /// @return `true` if the direction was updated successfully, `false` otherwise. This method will fail if the provided input parameters are invalid.
                bool SetDirectionUsingSpherical(const TEffectValue* coordinates, int numCoordinates);

                /// Sets this vector to omnidirectional mode with the specified number of axis and using the specified coordinate system as original.
                /// Performs no error-checking. Intended for internal use but exposed for testing.
                /// @param [in] numAxes Number of axes for which the components should be broadcast when in omnidirectional mode.
                /// @param [in] originalCoordinateSystem Coordinate system to use as the original coordinate system for this direction vector.
                void SetOmnidirectional(int numAxes, ECoordinateSystem originalCoordinateSystem);
            };

            /// Structure for representing an envelope that might be applied to an effect.
            /// See https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416225%28v=vs.85%29 for more information on how envelopes work.
            struct SEnvelope
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
                constexpr inline bool operator==(const SEnvelope& other) const = default;
            };

            /// Structure for holding the identifiers for axes associated with a force feedback effect.
            struct SAssociatedAxes
            {
                int count = 0;                                              ///< Number of associated axes.
                std::array<EAxis, kEffectAxesMaximumNumber> type = {};      ///< Axis type, one element per axis.

                /// Simple check for equality.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SAssociatedAxes& other) const = default;
            };

            /// Structure for holding parameters common to all force feedback effects.
            struct SCommonParameters
            {
                /// Default values for all common parameters.
                /// See specific named field descriptions for documentation.
                static constexpr TEffectTimeMs kDefaultStartDelay = 0;
                static constexpr TEffectTimeMs kDefaultSamplePeriod = 0;
                static constexpr TEffectValue kDefaultGain = kEffectModifierRelativeDenominator;
                static constexpr std::optional<SEnvelope> kDefaultEnvelope = std::nullopt;

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

                /// Alternative representation of the sample period to be used directly by computations.
                /// Avoids a computation-time conditional by providing a value that can be used without checking for equality with 0.
                TEffectTimeMs samplePeriodForComputations = (0 == kDefaultSamplePeriod) ? 1 : kDefaultSamplePeriod;

                /// Overall adjustment to the magnitude of a force feedback effect.
                /// This modifier acts as a per-effect "volume control" knob.
                TEffectValue gain = kDefaultGain;

                /// Alternative representation of the gain as a fraction to be multiplied by the final magnitude.
                /// Stored as a slight performance optimization to avoid a division operation each time magnitude is computed.
                TEffectValue gainFraction = kDefaultGain / kEffectModifierRelativeDenominator;

                /// Optional envelope to be applied as a transformation to this effect.
                /// If not present then no envelope is applied when this effect's force magnitude is computed.
                std::optional<SEnvelope> envelope = kDefaultEnvelope;

                /// Vector that specifies the direction of the force feedback effect. By default this vector does not specify a direction.
                /// Setting a direction is mandatory.
                DirectionVector direction;

                /// Association of direction components with virtual controller axes.
                /// Setting associated axes is mandatory, and there must be at least as many associated axes as there are direction vector components.
                std::optional<SAssociatedAxes> associatedAxes = std::nullopt;

                /// Simple check for equality.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SCommonParameters& other) const = default;

                /// Updates the sample period parameter and ensures both representations are consistent.
                /// A value of 0 means to use the default sample period, which for internal calculation purposes is equivalent to passing in a value of 1.
                /// @param [in] newSamplePeriod New sample period value.
                inline void SetSamplePeriod(TEffectTimeMs newSamplePeriod)
                {
                    samplePeriod = newSamplePeriod;

                    if (0 == newSamplePeriod)
                        samplePeriodForComputations = 1;
                    else
                        samplePeriodForComputations = newSamplePeriod;
                }

                /// Updates the gain parameter and ensures both representations are consistent.
                /// @param [in] newGain New gain value.
                inline void SetGain(TEffectValue newGain)
                {
                    gain = newGain;
                    gainFraction = (newGain / kEffectModifierRelativeDenominator);
                }
            };
        }
    }
}
