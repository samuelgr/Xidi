/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ForceFeedbackParameters.cpp
 *   Implementation of computations related to manipulating parameters common
 *   to all force feedback effects.
 *****************************************************************************/

#include "ForceFeedbackMath.h"
#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"

#include <cmath>
#include <numbers>
#include <optional>


namespace Xidi
{
    namespace Controller
    {
        namespace ForceFeedback
        {
            // -------- INTERNAL FUNCTIONS --------------------------------- //

            /// Checks if the supplied angle value is valid, meaning it is within the allowed range.
            /// @param [in] angle Angle to be checked.
            /// @return `true` if the angle is valid, `false` otherwise.
            static inline bool IsAngleValid(TEffectValue angle)
            {
                return ((angle >= kEffectAngleMinimum) && (angle <= kEffectAngleMaximum));
            }

            /// Checks if the supplied axis count is valid, meaning it is within the allowed range.
            /// @param [in] axisCount Axis count to be checked.
            /// @return `true` if the axis count is valid, `false` otherwise.
            static inline bool IsAxisCountValid(int axisCount)
            {
                return ((axisCount >= kEffectAxesMinimumNumber) && (axisCount <= kEffectAxesMaximumNumber));
            }


            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //
            // See "ForceFeedbackParameters.h" for documentation.

            DirectionVector::DirectionVector(void) : numAxes(), isOmnidirectional(), originalCoordinateSystem(), cartesian(), polar(), spherical()
            {
                // Nothing to do here.
            }


            // -------- INSTANCE METHODS ----------------------------------- //
            // See "ForceFeedbackParameters.h" for documentation.

            TMagnitudeComponents DirectionVector::ComputeMagnitudeComponents(TEffectValue magnitude) const
            {
                TMagnitudeComponents magnitudeComponents({});

                if (0 != magnitude)
                {
                    if (true == isOmnidirectional)
                    {
                        // For omni-directional forces, the magnitude is simply copied without transformation to all components.
                        // All of the coordinate systems contain invalid values so they cannot be consulted directly.

                        for (int i = 0; i < numAxes; ++i)
                            magnitudeComponents[i] = magnitude;
                    }
                    else if (1 == numAxes)
                    {
                        // For single-axis forces, only the direction of the single Cartesian coordinate matters.

                        if (cartesian[0] > 0)
                            magnitudeComponents[0] = magnitude;
                        else
                            magnitudeComponents[0] = -magnitude;
                    }
                    else
                    {
                        // For multi-axis forces, the spherical coordinate system makes it easy to convert to individual components.
                        // This is in essence a spherical-to-Cartesian conversion using the force's magnitude as input.

                        for (int i = 0; i < numAxes; ++i)
                            magnitudeComponents[i] = magnitude;

                        // Intuition for this algorithm is as follows.
                        // Component of the highest-numbered dimension (i.e. the highest-indexed element in the Cartesian component array) has a projection along it that is the sine of the highest-index spherical coordinate angle.
                        // All other components use a projection of that same angle along the orthogonal plane, which is to say multiply by the cosine of that same angle.
                        // This acts as a sort of dimensionality reduction which then repeats.
                        // Following this logic for a two-dimensional vector, and assuming dimensions X and Y in that order, Y component = magnitude * sin(spherical[0]), X component = magnitude * cos(spherical[0]).
                        // Extending that logic to three dimensions, and assuming dimensions X, Y, and Z in that order, Z component = magnitude * sin(spherical[1]), X-Y projection vector magnitude = magnitude * cos(spherical[1]), Y component = (X-Y projection) * sin(spherical[0]), X component = (X-Y projection) * cos(spherical[0]).
                        // Same can be extended to four and more dimensions. This pair of loops simply implements the above intuition.
                        for (int coordinateIndex = 0; coordinateIndex < (numAxes - 1); ++coordinateIndex)
                        {
                            for (int axisIndex = 0; axisIndex < numAxes; ++axisIndex)
                            {
                                if (axisIndex <= coordinateIndex)
                                    magnitudeComponents[axisIndex] *= TrigonometryCosine(spherical[coordinateIndex]);
                                else if (axisIndex == (coordinateIndex + 1))
                                    magnitudeComponents[axisIndex] *= TrigonometrySine(spherical[coordinateIndex]);
                            }
                        }
                    }
                }

                return magnitudeComponents;
            }

            // --------

            int DirectionVector::GetCartesianCoordinates(TEffectValue* coordinates, int numCoordinates) const
            {
                int numWritten = 0;

                for (; (numWritten < numCoordinates) && (numWritten < numAxes); ++numWritten)
                    coordinates[numWritten] = cartesian[numWritten];

                return numWritten;
            }

            // --------

            int DirectionVector::GetPolarCoordinates(TEffectValue* coordinates, int numCoordinates) const
            {
                if (0 >= numCoordinates)
                    return 0;

                int numWritten = 0;

                if (2 == numAxes)
                {
                    *coordinates = polar;
                    numWritten = 1;
                }

                return numWritten;
            }

            // --------

            int DirectionVector::GetSphericalCoordinates(TEffectValue* coordinates, int numCoordinates) const
            {
                int numWritten = 0;

                for (; (numWritten < numCoordinates) && (numWritten < (numAxes - 1)); ++numWritten)
                    coordinates[numWritten] = spherical[numWritten];

                return numWritten;
            }

            // --------

            bool DirectionVector::SetDirectionUsingCartesian(const TEffectValue* coordinates, int numCoordinates)
            {
                const int newNumAxes = numCoordinates;
                if (false == IsAxisCountValid(newNumAxes))
                    return false;

                // If all the components are 0 then direction is considered unimportant and the vector is marked as being omnidirectional.
                bool cartesianDirectionIsNonZero = false;
                for (int i = 0; i < newNumAxes; ++i)
                {
                    if (0 != coordinates[i])
                    {
                        cartesianDirectionIsNonZero = true;
                        break;
                    }
                }

                if (false == cartesianDirectionIsNonZero)
                {
                    SetOmnidirectional(newNumAxes, ECoordinateSystem::Cartesian);
                    return true;
                }

                numAxes = newNumAxes;
                isOmnidirectional = false;
                originalCoordinateSystem = ECoordinateSystem::Cartesian;

                // Set the Cartesian coordinate representation.
                for (int i = 0; i < newNumAxes; ++i)
                    cartesian[i] = coordinates[i];

                // Convert to polar if that makes sense.
                // The conversion is a little bit tricky because the polar angle is measured from (0,-1) in the direction of (1,0).
                if (2 == newNumAxes)
                    polar = TrigonometryArcTanOfRatio(cartesian[0], -cartesian[1]);

                // Convert to spherical if that makes sense.
                if (2 <= newNumAxes)
                {
                    // This algorithm basically adds one dimension at a time to the spherical coordinate representation. It works with successive 90-degree triangles, one for each dimension being added.
                    // Each time we add a dimension we get an angle and we need to compute the angle using inverse tangent with base and height dimensions as input.
                    // Base is just the magnitude of the vector in all of the dimensions we considered so far, and height is the value of the next Cartesian component.
                    // In 2D space, assuming X and Y in that order, this just means base is X component and height is Y component. Taking the inverse tangent of (height / base) gives us the value of spherical[0].
                    // Extending this to 3D space, and assuming X, Y, and Z in that order, base is the magnitude of the 2D vector and height is the Z component. Taking the inverse tangent of (height / base) defined this way gives us the value of spherical[1].
                    // Each subsequent iteration we need to recompute the base quantity using the Pythagorean Theorem.
                    // However, we can optimize for the common case of not more than 2 axes by avoiding doing unnecessary power and square root operations when only 2 axes are present.
                    TEffectValue dimensionalBase = cartesian[0];
                    spherical[0] = TrigonometryArcTanOfRatio(cartesian[1], dimensionalBase);

                    for (int i = 1; i < (newNumAxes - 1); ++i)
                    {
                        dimensionalBase = sqrt(pow(dimensionalBase, (TEffectValue)2) + pow(cartesian[i], (TEffectValue)2));
                        spherical[i] = TrigonometryArcTanOfRatio(cartesian[i + 1], dimensionalBase);
                    }
                }

                return true;
            }

            // --------

            bool DirectionVector::SetDirectionUsingPolar(const TEffectValue* coordinates, int numCoordinates)
            {
                const int newNumAxes = 1 + numCoordinates;
                if (2 != newNumAxes)
                    return false;

                if (false == IsAngleValid(coordinates[0]))
                    return false;

                numAxes = newNumAxes;
                isOmnidirectional = false;
                originalCoordinateSystem = ECoordinateSystem::Polar;

                // Set the polar coordinate representation.
                polar = coordinates[0];

                // Convert to Cartesian.
                // Polar angle is measured from (0,-1) in the direction of (1,0). In other words, it is from the second axis whose component is actually negated.
                // Assume a magnitude of 100,000,000 for the purpose of computing Cartesian coordinates, since this guarantees we will have an integer part for each component, and magnitude is unimportant for direction vectors.
                // Since we are operating at hundredths of degrees, the smallest value that can result from trigonometric calculations is sin(0.01) = 0.00017453, and we want to make sure even just the integral part keeps some reasonable amount of precision.
                cartesian[0] = 100000000 * TrigonometrySine(polar);
                cartesian[1] = 100000000 * -TrigonometryCosine(polar);

                // Convert to spherical.
                // Since we have two axes there is only one angle, and the transformation is purely arithmetic because polar and spherical measure differently.
                // A single spherical angle is measured from (1,0) to (0,1). Direction is the same, but there is an offset.
                spherical[0] = 27000 + polar;
                if (spherical[0] >= 36000)
                    spherical[0] -= 36000;

                return true;
            }

            // --------

            bool DirectionVector::SetDirectionUsingSpherical(const TEffectValue* coordinates, int numCoordinates)
            {
                const int newNumAxes = 1 + numCoordinates;
                if (false == IsAxisCountValid(newNumAxes))
                    return false;

                for (int i = 0; i < numCoordinates; ++i)
                {
                    if (false == IsAngleValid(coordinates[i]))
                        return false;
                }

                numAxes = newNumAxes;
                isOmnidirectional = false;
                originalCoordinateSystem = ECoordinateSystem::Spherical;

                if (1 == numAxes)
                {
                    cartesian[0] = 1;
                }
                else
                {
                    // Set the spherical coordinate representation.
                    for (int i = 0; i < numCoordinates; ++i)
                        spherical[i] = coordinates[i];

                    // Convert to polar if that makes sense.
                    if (2 == newNumAxes)
                    {
                        // As with converting from polar to spherical, this is just an arithmetic transformation.
                        polar = spherical[0] - 27000;
                        if (polar < 0)
                            polar += 36000;
                    }

                    // Convert to Cartesian.
                    // Assume a magnitude of 100,000,000 so there will be reasonable precision in the integer part of each Cartesian component.
                    cartesian = ComputeMagnitudeComponents(100000000);
                }

                return true;
            }

            // --------

            void DirectionVector::SetOmnidirectional(int numAxes, ECoordinateSystem originalCoordinateSystem)
            {
                this->numAxes = numAxes;
                this->originalCoordinateSystem = originalCoordinateSystem;

                isOmnidirectional = true;

                cartesian.fill(0);
                polar = 0;
                spherical.fill(0);
            }
        }
    }
}
