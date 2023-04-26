/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ForceFeedbackParametersTest.cpp
 *   Unit tests for functionality related to force feedback parameters,
 *   especially direction vectors.
 *****************************************************************************/

#include "ForceFeedbackParameters.h"
#include "ForceFeedbackTypes.h"
#include "TestCase.h"

#include <array>
#include <cmath>


namespace XidiTest
{
    using namespace ::Xidi::Controller::ForceFeedback;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Square root of 2.
    static const TEffectValue kSqrt2 = (TEffectValue)sqrt(2);

    /// Square root of 3.
    static const TEffectValue kSqrt3 = (TEffectValue)sqrt(3);

    /// Result of cos(30 deg).
    static const TEffectValue kCos30 = kSqrt3 / 2;

    /// Result of cos(45 deg).
    static const TEffectValue kCos45 = kSqrt2 / 2;

    /// Result of cos(60 deg).
    static const TEffectValue kCos60 = 0.5;

    /// Result of sin(30 deg).
    static const TEffectValue kSin30 = 0.5;

    /// Result of sin(45 deg).
    static const TEffectValue kSin45 = kSqrt2 / 2;

    /// Result opf sin(60 deg).
    static const TEffectValue kSin60 = kSqrt3 / 2;


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Record type for holding expected coordinate system conversion test data.
    /// @tparam kNumAxes Number of axes in the direction vector represented by the test data.
    template <int kNumAxes> struct SCoordinateConversionTestData
    {
        static_assert(kNumAxes >= 2, "Coordinate conversion tests are only valid with at least 2 axes.");

        std::array<TEffectValue, kNumAxes> cartesian = {};                  ///< Cartesian coordinates, one coordinate per element and one coordinate per axis.
        std::optional<TEffectValue> polar = std::nullopt;                   ///< Optional polar coordinates, either one angle value is present or it is not.
        std::array<TEffectValue, kNumAxes - 1> spherical = {};              ///< Spherical coordinates, one coordinate per element and one less total number of coordinates than the number of axes.
    };

    /// Record type for holding a direction and an expected set of magnitude components.
    /// Used for tests that involve computing magnitude components for a given force vector.
    /// @tparam kNumAxes Number of axes in the direction vector represented by the test data.
    template <int kNumAxes> struct SMagnitudeComponentsTestData
    {
        static_assert(kNumAxes >= 1, "Magnitude component tests are only valid with at least 1 axis.");

        std::array<TEffectValue, kNumAxes> directionCartesian = {};         ///< Direction expressed as Cartesian coordinates.
        TMagnitudeComponents magnitudeComponents;                           ///< Associated magnitude components.
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Checks if two values are approximately equal.
    /// If one value is zero, the other is checked for exact equality with 0.
    /// Otherwise the ratio is computed and it must show a difference of at most 3% for the two values to be considered approximately equal.
    /// @tparam ValueType Type of values to compare.
    /// @param [in] valueA First of the two values to compare.
    /// @param [in] valueB Second of the two values to compare.
    /// @return `true` if the two values are approximately equal, `false` otherwise.
    template <typename ValueType> static bool ApproximatelyEqual(ValueType valueA, ValueType valueB)
    {
        static constexpr double kMaxRelativeError = 0.03;

        // Zero values need to be exactly equal, otherwise we might end up dividing by zero.
        if (((ValueType)0 == valueA) || ((ValueType)0 == valueB))
            return (valueA == valueB);

        // Since we might be dealing with floating point quantities, there could be some imprecision, so allow a maximum error as specified at the top of this function.
        // A difference in sign would produce a negative ratio, which would return false.
        const double ratioSimilarity = (double)valueA / (double)valueB;
        return (ratioSimilarity >= (1.0 - kMaxRelativeError) && ratioSimilarity <= (1.0 + kMaxRelativeError));
    }

    /// Specialization for checking if two magnitude component vectors are approximately equal.
    /// @param [in] valueA First of the two magnitude component vectors to compare.
    /// @param [in] valueB Second of the two magnitude component to compare.
    /// @return `true` if the two magnitude component vectors all have components that are approximately equal, `false` otherwise.
    template <> static bool ApproximatelyEqual<TMagnitudeComponents>(TMagnitudeComponents valueA, TMagnitudeComponents valueB)
    {
        for (size_t i = 0; i < valueA.size(); ++i)
        {
            if (false == ApproximatelyEqual(valueA[i], valueB[i]))
                return false;
        }

        return true;
    }

    /// Compares two sets of Cartesian coordinates for direction equivalence.
    /// @param [in] coordinatesA First array of the comparison.
    /// @param [in] coordinatesB Second array of the comparison.
    /// @param [in] numCoordinates Number of coordinates contained in both arrays.
    static void CheckCartesianDirectionEquivalence(const TEffectValue* coordinatesA, const TEffectValue* coordinatesB, int numCoordinates)
    {
        std::optional<double> maybeExpectedRatio;

        // All non-zero coordinates need to follow the same ratio and all zero coordinates need to be exactly equal.
        // We don't know what that ratio is, however, until we start comparing individual components one at a time.

        for (int i = 0; i < numCoordinates; ++i)
        {
            if ((0 == coordinatesA[i]) || (0 == coordinatesB[i]))
            {
                TEST_ASSERT(coordinatesA[i] == coordinatesB[i]);
            }
            else
            {
                if (false == maybeExpectedRatio.has_value())
                {
                    maybeExpectedRatio = coordinatesB[i] / coordinatesA[i];
                }
                else
                {
                    const double expectedRatio = maybeExpectedRatio.value();
                    const double actualRatio = coordinatesB[i] / coordinatesA[i];
                    TEST_ASSERT(true == ApproximatelyEqual(actualRatio, expectedRatio));
                }
            }
        }
    }

    /// Creates a direction vector and verifies that it performs correct coordinate system conversion according to the supplied test data record.
    /// @param [in] testData Test data record to use as the basis for the test.
    /// @tparam kNumAxes Number of axes in the direction vector represented by the test data.
    template <int kNumAxes> static void DirectionVectorCoordinateConversionTest(const SCoordinateConversionTestData<kNumAxes>& testData)
    {
        // Conversion from Cartesian
        DirectionVector vectorCartesian;
        TEST_ASSERT(true == vectorCartesian.SetDirectionUsingCartesian(&testData.cartesian[0], (int)testData.cartesian.size()));

        if (true == testData.polar.has_value())
        {
            const auto expectedCartesianToPolar = testData.polar.value();
            TEffectValue actualCartesianToPolar = 0;
            TEST_ASSERT(1 == vectorCartesian.GetPolarCoordinates(&actualCartesianToPolar, 1));
            TEST_ASSERT(actualCartesianToPolar == expectedCartesianToPolar);
        }
        else
        {
            TEffectValue unusedCartesianToPolar = 0;
            TEST_ASSERT(0 == vectorCartesian.GetPolarCoordinates(&unusedCartesianToPolar, 1));
        }

        const auto expectedCartesianToSpherical = testData.spherical;
        decltype(testData.spherical) actualCartesianToSpherical;
        TEST_ASSERT((int)actualCartesianToSpherical.size() == vectorCartesian.GetSphericalCoordinates(&actualCartesianToSpherical[0], (int)actualCartesianToSpherical.size()));
        TEST_ASSERT(actualCartesianToSpherical == expectedCartesianToSpherical);

        // Conversion from polar (optional, not always valid)
        if (testData.polar.has_value())
        {
            DirectionVector vectorPolar;
            TEST_ASSERT(true == vectorPolar.SetDirectionUsingPolar(&testData.polar.value(), 1));

            const auto expectedPolarToCartesian = testData.cartesian;
            decltype(testData.cartesian) actualPolarToCartesian;
            TEST_ASSERT((int)actualPolarToCartesian.size() == vectorPolar.GetCartesianCoordinates(&actualPolarToCartesian[0], (int)actualPolarToCartesian.size()));
            CheckCartesianDirectionEquivalence(&expectedPolarToCartesian[0], &actualPolarToCartesian[0], (int)kNumAxes);

            const auto expectedPolarToSpherical = testData.spherical;
            decltype(testData.spherical) actualPolarToSpherical;
            TEST_ASSERT((int)actualPolarToSpherical.size() == vectorPolar.GetSphericalCoordinates(&actualPolarToSpherical[0], (int)actualPolarToSpherical.size()));
            TEST_ASSERT(actualPolarToSpherical == expectedPolarToSpherical);
        }

        // Conversion from spherical
        DirectionVector vectorSpherical;
        TEST_ASSERT(true == vectorSpherical.SetDirectionUsingSpherical(&testData.spherical[0], (int)testData.spherical.size()));

        const auto expectedSphericalToCartesian = testData.cartesian;
        decltype(testData.cartesian) actualSphericalToCartesian;
        TEST_ASSERT((int)actualSphericalToCartesian.size() == vectorSpherical.GetCartesianCoordinates(&actualSphericalToCartesian[0], (int)actualSphericalToCartesian.size()));
        CheckCartesianDirectionEquivalence(&expectedSphericalToCartesian[0], &actualSphericalToCartesian[0], (int)kNumAxes);

        if (true == testData.polar.has_value())
        {
            const auto expectedSphericalToPolar = testData.polar.value();
            TEffectValue actualSphericalToPolar = 0;
            TEST_ASSERT(1 == vectorSpherical.GetPolarCoordinates(&actualSphericalToPolar, 1));
            TEST_ASSERT(actualSphericalToPolar == expectedSphericalToPolar);
        }
        else
        {
            TEffectValue unusedCartesianToPolar = 0;
            TEST_ASSERT(0 == vectorSpherical.GetPolarCoordinates(&unusedCartesianToPolar, 1));
        }
    }

    /// Creates a direction vector and verifies that it performs correct magnitude component computations according to the supplied test data record.
    /// @param [in] magnitude Magnitude of the force vector for which components are desired.
    /// @param [in] testData Test data record to use as the basis for the test.
    /// @tparam kNumAxes Number of axes in the direction vector represented by the test data.
    template <int kNumAxes> static void DirectionVectorMagnitudeComponentsTest(TEffectValue magnitude, const SMagnitudeComponentsTestData<kNumAxes>& testData)
    {
        DirectionVector vector;
        TEST_ASSERT(true == vector.SetDirectionUsingCartesian(&testData.directionCartesian[0], (int)testData.directionCartesian.size()));

        const TMagnitudeComponents& expectedMagnitudeComponents = testData.magnitudeComponents;
        const TMagnitudeComponents actualMagnitudeComponents = vector.ComputeMagnitudeComponents(magnitude);
        TEST_ASSERT(true == ApproximatelyEqual(actualMagnitudeComponents, expectedMagnitudeComponents));
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Exercises coordinate system setting, getting, and converting with single-axis direction vectors.
    // The only possible input coordinate system is Cartesian, and all of these attempted conversions should fail.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_1D_Conversions)
    {
        constexpr TEffectValue kTestCoordinates[] = {-100000000, -10000, -100, -1, 1, 100, 10000, 100000000};
        
        for (const auto testCoordinate : kTestCoordinates)
        {
            DirectionVector vector;
            TEST_ASSERT(true == vector.SetDirectionUsingCartesian(&testCoordinate, 1));

            // Simple retrieval should succeed without any transformation.
            std::array<TEffectValue, kEffectAxesMaximumNumber> actualOutputCoordinates = {0};
            TEST_ASSERT(1 == vector.GetCartesianCoordinates(&actualOutputCoordinates[0], (int)actualOutputCoordinates.size()));
            TEST_ASSERT(actualOutputCoordinates[0] == testCoordinate);

            // All conversions should fail, so there should be no output written to the actual output coordinate variable.
            constexpr std::array<TEffectValue, kEffectAxesMaximumNumber> kExpectedOutputCoordinates = {55, 66};

            actualOutputCoordinates = kExpectedOutputCoordinates;
            TEST_ASSERT(0 == vector.GetPolarCoordinates(&actualOutputCoordinates[0], (int)actualOutputCoordinates.size()));
            TEST_ASSERT(actualOutputCoordinates == kExpectedOutputCoordinates);

            actualOutputCoordinates = kExpectedOutputCoordinates;
            TEST_ASSERT(0 == vector.GetSphericalCoordinates(&actualOutputCoordinates[0], (int)actualOutputCoordinates.size()));
            TEST_ASSERT(actualOutputCoordinates == kExpectedOutputCoordinates);
        }
    }

    // Exercises computation of a force's magnitude components using a single-axis direction vector.
    // Magnitude of the direction vector itself does not matter, only the sign does, so the expected output magnitude is single-component with the same absolute value and either the same sign of, or opposite sign of, the input.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_1D_MagnitudeComponents)
    {
        constexpr TEffectValue kTestMagnitudes[] = {-1000, -10, 0, 100, 10000};
        constexpr TEffectValue kTestCoordinates[] = {-100000000, -10000, -100, -1, 1, 100, 10000, 100000000};

        for (const auto testMagnitude : kTestMagnitudes)
        {
            // Verify that a zero-coordinate spherical specification works and also produces the same single axis vector as above.
            do {
                DirectionVector vector;
                TEST_ASSERT(true == vector.SetDirectionUsingSpherical(nullptr, 0));

                const TMagnitudeComponents expectedOutput = {testMagnitude};
                const TMagnitudeComponents actualOutput = vector.ComputeMagnitudeComponents(testMagnitude);
                TEST_ASSERT(actualOutput == expectedOutput);
            } while (false);

            // Verify all the specified Cartesian test coordinates.
            for (const auto testCoordinate : kTestCoordinates)
            {
                DirectionVector vector;
                TEST_ASSERT(true == vector.SetDirectionUsingCartesian(&testCoordinate, 1));

                const TMagnitudeComponents expectedOutput = {((testCoordinate > 0) ? testMagnitude : -testMagnitude)};
                const TMagnitudeComponents actualOutput = vector.ComputeMagnitudeComponents(testMagnitude);
                TEST_ASSERT(actualOutput == expectedOutput);
            }
        }
    }

    // Exercises coordinate system setting, getting, and converting with two-axis direction vectors.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_2D_Conversions)
    {
        const SCoordinateConversionTestData<2> kTestData[] = {

            // Single direction component
            {.cartesian = {1, 0},           .polar = (TEffectValue)9000,    .spherical = {0}},
            {.cartesian = {1000, 0},        .polar = (TEffectValue)9000,    .spherical = {0}},
            {.cartesian = {0, 1},           .polar = (TEffectValue)18000,   .spherical = {9000}},
            {.cartesian = {0, 1000},        .polar = (TEffectValue)18000,   .spherical = {9000}},
            {.cartesian = {-1, 0},          .polar = (TEffectValue)27000,   .spherical = {18000}},
            {.cartesian = {-1000, 0},       .polar = (TEffectValue)27000,   .spherical = {18000}},
            {.cartesian = {0, -1},          .polar = (TEffectValue)0,       .spherical = {27000}},
            {.cartesian = {0, -1000},       .polar = (TEffectValue)0,       .spherical = {27000}},

            // Two direction components, simple
            {.cartesian = {1, 1},           .polar = (TEffectValue)13500,   .spherical = {4500}},
            {.cartesian = {1, -1},          .polar = (TEffectValue)4500,    .spherical = {31500}},
            {.cartesian = {-1, 1},          .polar = (TEffectValue)22500,   .spherical = {13500}},
            {.cartesian = {-1, -1},         .polar = (TEffectValue)31500,   .spherical = {22500}},

            // Two direction components, complex
            {.cartesian = {1, kSqrt3},      .polar = (TEffectValue)15000,   .spherical = {6000}},
            {.cartesian = {kSqrt3, 1},      .polar = (TEffectValue)12000,   .spherical = {3000}},
            {.cartesian = {-1, kSqrt3},     .polar = (TEffectValue)21000,   .spherical = {12000}},
            {.cartesian = {-kSqrt3, 1},     .polar = (TEffectValue)24000,   .spherical = {15000}},
            {.cartesian = {-kSqrt3, -1},    .polar = (TEffectValue)30000,   .spherical = {21000}},
            {.cartesian = {-1, -kSqrt3},    .polar = (TEffectValue)33000,   .spherical = {24000}},
            {.cartesian = {1, -kSqrt3},     .polar = (TEffectValue)3000,    .spherical = {30000}},
            {.cartesian = {kSqrt3, -1},     .polar = (TEffectValue)6000,    .spherical = {33000}}
        };

        for (const auto& testData : kTestData)
            DirectionVectorCoordinateConversionTest(testData);
    }

    // Exercises computation of a force's magnitude components using two-axis direction vectors.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_2D_MagnitudeComponents)
    {
        constexpr TEffectValue kTestMagnitude = 1000;
        const SMagnitudeComponentsTestData<2> kTestData[] = {

            // Single direction component.
            {.directionCartesian = {1, 0},                                  .magnitudeComponents = {kTestMagnitude, 0}},
            {.directionCartesian = {1000, 0},                               .magnitudeComponents = {kTestMagnitude, 0}},
            {.directionCartesian = {0, 1},                                  .magnitudeComponents = {0, kTestMagnitude}},
            {.directionCartesian = {0, 1000},                               .magnitudeComponents = {0, kTestMagnitude}},
            {.directionCartesian = {-1, 0},                                 .magnitudeComponents = {-kTestMagnitude, 0}},
            {.directionCartesian = {-1000, 0},                              .magnitudeComponents = {-kTestMagnitude, 0}},
            {.directionCartesian = {0, -1},                                 .magnitudeComponents = {0, -kTestMagnitude}},
            {.directionCartesian = {0, -1000},                              .magnitudeComponents = {0, -kTestMagnitude}},

            // Two direction components, simple
            {.directionCartesian = {1, 1},                                  .magnitudeComponents = {kTestMagnitude * kCos45, kTestMagnitude * kSin45}},
            {.directionCartesian = {1, -1},                                 .magnitudeComponents = {kTestMagnitude * kCos45, -kTestMagnitude * kSin45}},
            {.directionCartesian = {-1, 1},                                 .magnitudeComponents = {-kTestMagnitude * kCos45, kTestMagnitude * kSin45}},
            {.directionCartesian = {-1, -1},                                .magnitudeComponents = {-kTestMagnitude * kCos45, -kTestMagnitude * kSin45}},

            // Two direction components, complex
            {.directionCartesian = {1, kSqrt3},                             .magnitudeComponents = {kTestMagnitude * kCos60, kTestMagnitude * kSin60}},
            {.directionCartesian = {kSqrt3, 1},                             .magnitudeComponents = {kTestMagnitude * kCos30, kTestMagnitude * kSin30}},
            {.directionCartesian = {-1, kSqrt3},                            .magnitudeComponents = {-kTestMagnitude * kCos60, kTestMagnitude * kSin60}},
            {.directionCartesian = {-kSqrt3, 1},                            .magnitudeComponents = {-kTestMagnitude * kCos30, kTestMagnitude * kSin30}},
            {.directionCartesian = {-kSqrt3, -1},                           .magnitudeComponents = {-kTestMagnitude * kCos30, -kTestMagnitude * kSin30}},
            {.directionCartesian = {-1, -kSqrt3},                           .magnitudeComponents = {-kTestMagnitude * kCos60, -kTestMagnitude * kSin60}},
            {.directionCartesian = {1, -kSqrt3},                            .magnitudeComponents = {kTestMagnitude * kCos60, -kTestMagnitude * kSin60}},
            {.directionCartesian = {kSqrt3, -1},                            .magnitudeComponents = {kTestMagnitude * kCos30, -kTestMagnitude * kSin30}}
        };

        for (const auto& testData : kTestData)
            DirectionVectorMagnitudeComponentsTest(kTestMagnitude, testData);
    }

    // Exercises coordinate system setting, getting, and converting with three-axis direction vectors.
    // Polar coordinates are invalid here.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_3D_Conversions)
    {
        const SCoordinateConversionTestData<3> kTestData[] = {

            // Single direction component
            {.cartesian = {1, 0, 0},                                        .spherical = {0, 0}},
            {.cartesian = {0, 1, 0},                                        .spherical = {9000, 0}},
            {.cartesian = {0, 0, 1},                                        .spherical = {0, 9000}},
            {.cartesian = {-10, 0, 0},                                      .spherical = {18000, 0}},
            {.cartesian = {0, -20, 0},                                      .spherical = {27000, 0}},
            {.cartesian = {0, 0, -30},                                      .spherical = {0, 27000}},

            // Two direction components
            {.cartesian = {0, 1, 1},                                        .spherical = {9000, 4500}},
            {.cartesian = {1, 0, 1},                                        .spherical = {0, 4500}},
            {.cartesian = {1, 1, 0},                                        .spherical = {4500, 0}},
            {.cartesian = {0, -1, -1},                                      .spherical = {27000, 31500}},
            {.cartesian = {-1, 0, -1},                                      .spherical = {18000, 31500}},
            {.cartesian = {-1, -1, 0},                                      .spherical = {22500, 0}},

            // Three direction components, simple
            {.cartesian = {1, 1, kSqrt2},                                   .spherical = {4500, 4500}},
            {.cartesian = {1, 1, -kSqrt2},                                  .spherical = {4500, 31500}},
            {.cartesian = {1, -1, kSqrt2},                                  .spherical = {31500, 4500}},
            {.cartesian = {-1, -1, -kSqrt2},                                .spherical = {22500, 31500}},

            // Three direction components, complex
            {.cartesian = {1, kSqrt3, kSqrt3 * 2},                          .spherical = {6000, 6000}},
            {.cartesian = {kSqrt3, 1, kSqrt3 * 2},                          .spherical = {3000, 6000}},
            {.cartesian = {1, kSqrt3, 2 / kSqrt3},                          .spherical = {6000, 3000}},
            {.cartesian = {kSqrt3, 1, 2 / kSqrt3},                          .spherical = {3000, 3000}}
        };

        for (const auto& testData : kTestData)
            DirectionVectorCoordinateConversionTest(testData);
    }

    // Exercises computation of a force's magnitude components using three-axis direction vectors.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_3D_MagnitudeComponents)
    {
        constexpr TEffectValue kTestMagnitude = -1000;
        const SMagnitudeComponentsTestData<3> kTestData[] = {

            // Single direction component
            {.directionCartesian = {1, 0, 0},                               .magnitudeComponents = {kTestMagnitude, 0, 0}},
            {.directionCartesian = {0, 1, 0},                               .magnitudeComponents = {0, kTestMagnitude, 0}},
            {.directionCartesian = {0, 0, 1},                               .magnitudeComponents = {0, 0, kTestMagnitude}},
            {.directionCartesian = {-10, 0, 0},                             .magnitudeComponents = {-kTestMagnitude, 0, 0}},
            {.directionCartesian = {0, -20, 0},                             .magnitudeComponents = {0, -kTestMagnitude, 0}},
            {.directionCartesian = {0, 0, -30},                             .magnitudeComponents = {0, 0, -kTestMagnitude}},

            // Two direction components
            {.directionCartesian = {0, 1, 1},                               .magnitudeComponents = {0, kTestMagnitude * kCos45, kTestMagnitude * kSin45}},
            {.directionCartesian = {1, 0, 1},                               .magnitudeComponents = {kTestMagnitude * kCos45, 0, kTestMagnitude * kSin45}},
            {.directionCartesian = {1, 1, 0},                               .magnitudeComponents = {kTestMagnitude * kCos45, kTestMagnitude * kSin45, 0}},
            {.directionCartesian = {0, -1, -1},                             .magnitudeComponents = {0, -kTestMagnitude * kCos45, -kTestMagnitude * kSin45}},
            {.directionCartesian = {-1, 0, -1},                             .magnitudeComponents = {-kTestMagnitude * kCos45, 0, -kTestMagnitude * kSin45}},
            {.directionCartesian = {-1, -1, 0},                             .magnitudeComponents = {-kTestMagnitude * kCos45, -kTestMagnitude * kSin45, 0}},

            // Three direction components, simple
            {.directionCartesian = {1, 1, kSqrt2},                          .magnitudeComponents = {kTestMagnitude * kCos45 * kCos45, kTestMagnitude * kCos45 * kSin45, kTestMagnitude * kSin45}},
            {.directionCartesian = {1, 1, -kSqrt2},                         .magnitudeComponents = {kTestMagnitude * kCos45 * kCos45, kTestMagnitude * kCos45 * kSin45, -kTestMagnitude * kSin45}},
            {.directionCartesian = {1, -1, kSqrt2},                         .magnitudeComponents = {kTestMagnitude * kCos45 * kCos45, -kTestMagnitude * kCos45 * kSin45, kTestMagnitude * kSin45}},
            {.directionCartesian = {-1, -1, -kSqrt2},                       .magnitudeComponents = {-kTestMagnitude * kCos45 * kCos45, -kTestMagnitude * kCos45 * kSin45, -kTestMagnitude * kSin45}},

            // Three direction components, complex
            {.directionCartesian = {1, kSqrt3, kSqrt3 * 2},                 .magnitudeComponents = {kTestMagnitude * kCos60 * kCos60, kTestMagnitude * kCos60 * kSin60, kTestMagnitude * kSin60}},
            {.directionCartesian = {kSqrt3, 1, kSqrt3 * 2},                 .magnitudeComponents = {kTestMagnitude * kCos60 * kCos30, kTestMagnitude * kCos60 * kSin30, kTestMagnitude * kSin60}},
            {.directionCartesian = {1, kSqrt3, 2 / kSqrt3},                 .magnitudeComponents = {kTestMagnitude * kCos30 * kCos60, kTestMagnitude * kCos30 * kSin60, kTestMagnitude * kSin30}},
            {.directionCartesian = {kSqrt3, 1, 2 / kSqrt3},                 .magnitudeComponents = {kTestMagnitude * kCos30 * kCos30, kTestMagnitude * kCos30 * kSin30, kTestMagnitude * kSin30}}
        };

        for (const auto& testData : kTestData)
            DirectionVectorMagnitudeComponentsTest(kTestMagnitude, testData);
    }

    // Exercises various ways of setting directions using invalid coordinates.
    // All invocations are expected to fail.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_InvalidCoordinates)
    {
        constexpr TEffectValue kInvalidAngleCoordinates[] = {-1, -1000, 36000, 50000};

        DirectionVector vector;
        std::array<TEffectValue, kEffectAxesMaximumNumber + 1> inputCoordinates = {};

        // Various ways of sending invalid Cartesian coordinates.
        // The only way for Cartesian coordinates to be invalid is to send in 0 coordinates.
        inputCoordinates = {1000};
        TEST_ASSERT(false == vector.SetDirectionUsingCartesian(&inputCoordinates[0], 0));
        inputCoordinates = {};

        // Various ways of sending invalid polar coordinates.
        // First we use some valid angle values but with an invalid number of coordinates (the only allowed number is 1).
        // Then we use invalid angles that are either negative or out of range.
        for (int i = 0; i <= (int)inputCoordinates.size(); ++i)
        {
            if (1 == i)
                continue;

            TEST_ASSERT(false == vector.SetDirectionUsingPolar(&inputCoordinates[0], i));
        }

        for (const auto invalidAngleCoordinate : kInvalidAngleCoordinates)
            TEST_ASSERT(false == vector.SetDirectionUsingPolar(&invalidAngleCoordinate, 1));

        // Various ways of sending invalid spherical coordinates.
        // First we use some valid angle values but with an invalid number of coordinates (the allowed range is 0 to one less than maximum allowed axes).
        // Then we use some invalid angles that are either negative or out of range.
        for (int i = kEffectAxesMaximumNumber; i <= (int)inputCoordinates.size(); ++i)
            TEST_ASSERT(false == vector.SetDirectionUsingSpherical(&inputCoordinates[0], i));

        for (const auto invalidAngleCoordinate : kInvalidAngleCoordinates)
            TEST_ASSERT(false == vector.SetDirectionUsingSpherical(&invalidAngleCoordinate, 1));

        // Finally, verify that the vector reports not having any direction set, since all of the above attempts should have failed.
        TEST_ASSERT(false == vector.HasDirection());
    }

    // Verifies that direction vector objects correctly report their original coordinate system once a direction is set.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_OriginalCoordinateSystem)
    {
        constexpr TEffectValue kTestCoordinates[] = {5566};

        DirectionVector vector;
        TEST_ASSERT(false == vector.HasDirection());

        TEST_ASSERT(true == vector.SetDirectionUsingCartesian(kTestCoordinates, _countof(kTestCoordinates)));
        TEST_ASSERT(ECoordinateSystem::Cartesian == vector.GetOriginalCoordinateSystem());

        TEST_ASSERT(true == vector.SetDirectionUsingPolar(kTestCoordinates, _countof(kTestCoordinates)));
        TEST_ASSERT(ECoordinateSystem::Polar == vector.GetOriginalCoordinateSystem());

        TEST_ASSERT(true == vector.SetDirectionUsingSpherical(kTestCoordinates, _countof(kTestCoordinates)));
        TEST_ASSERT(ECoordinateSystem::Spherical == vector.GetOriginalCoordinateSystem());
    }

    // Verifies that direction vector objects correctly enter omnidirectional mode and, when in this mode, broadcast force components without transformation.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_Omnidirectional)
    {
        constexpr TEffectValue kTestCoordinates[] = {0, 0, 0};

        DirectionVector vector;
        TEST_ASSERT(false == vector.HasDirection());

        TEST_ASSERT(true == vector.SetDirectionUsingCartesian(kTestCoordinates, _countof(kTestCoordinates)));
        TEST_ASSERT(true == vector.HasDirection());
        TEST_ASSERT(true == vector.IsOmnidirectional());

        constexpr TEffectValue kTestMagnitude = 5432;
        const TMagnitudeComponents expectedMagnitudeComponents = {kTestMagnitude, kTestMagnitude, kTestMagnitude};
        const TMagnitudeComponents actualMagnitudeComponents = vector.ComputeMagnitudeComponents(kTestMagnitude);
        TEST_ASSERT(actualMagnitudeComponents == expectedMagnitudeComponents);
    }

    // Verifies that direction vector objects exit omnidirectional mode once the direction is changed to something else.
    TEST_CASE(ForceFeedbackParameters_DirectionVector_OmnidirectionalExit)
    {
        DirectionVector vector;
        TEST_ASSERT(false == vector.HasDirection());

        constexpr TEffectValue kTestCoordinatesOmnidirectional[] = {0, 0, 0};
        TEST_ASSERT(true == vector.SetDirectionUsingCartesian(kTestCoordinatesOmnidirectional, _countof(kTestCoordinatesOmnidirectional)));
        TEST_ASSERT(true == vector.HasDirection());
        TEST_ASSERT(true == vector.IsOmnidirectional());

        constexpr TEffectValue kTestCoordinatesUnidirectional[] = {1, 1, 1};
        TEST_ASSERT(true == vector.SetDirectionUsingCartesian(kTestCoordinatesUnidirectional, _countof(kTestCoordinatesUnidirectional)));
        TEST_ASSERT(true == vector.HasDirection());
        TEST_ASSERT(false == vector.IsOmnidirectional());
    }
}
