/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ForceFeedbackEffectTest.cpp
 *   Unit tests for functionality common to all force feedback effects.
 *****************************************************************************/

#include "ForceFeedbackParameters.h"
#include "TestCase.h"

#include <array>
#include <cmath>


namespace XidiTest
{
    using namespace ::Xidi::ForceFeedback;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Square root of 3.
    /// Used as a vector coordinate.
    static const TEffectValue kSqrt3 = (TEffectValue)sqrt(3);


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


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Compares two sets of Cartesian coordinates for direction equivalence.
    /// @param [in] coordinatesA First array of the comparison.
    /// @param [in] coordinatesB Second array of the comparison.
    /// @param [in] numCoordinates Number of coordinates contained in both arrays.
    static void CheckCartesianDirectionEquivalence(const TEffectValue* coordinatesA, const TEffectValue* coordinatesB, int numCoordinates)
    {
        std::optional<double> expectedRatio;

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
                if (false == expectedRatio.has_value())
                    expectedRatio = coordinatesB[i] / coordinatesA[i];
                else
                {
                    const double kExpectedRatio = expectedRatio.value();
                    const double kActualRatio = coordinatesB[i] / coordinatesA[i];

                    // There could be some imprecision, so allow a maximum error of about 1%.
                    const double kRatioSimilarity = kActualRatio / kExpectedRatio;
                    TEST_ASSERT(kRatioSimilarity >= 0.99 && kRatioSimilarity <= 1.01);
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
            const auto kExpectedCartesianToPolar = testData.polar.value();
            TEffectValue actualCartesianToPolar = 0;
            TEST_ASSERT(1 == vectorCartesian.GetPolarCoordinates(&actualCartesianToPolar, 1));
            TEST_ASSERT(actualCartesianToPolar == kExpectedCartesianToPolar);
        }

        const auto kExpectedCartesianToSpherical = testData.spherical;
        decltype(testData.spherical) actualCartesianToSpherical;
        TEST_ASSERT((int)actualCartesianToSpherical.size() == vectorCartesian.GetSphericalCoordinates(&actualCartesianToSpherical[0], (int)actualCartesianToSpherical.size()));
        TEST_ASSERT(actualCartesianToSpherical == kExpectedCartesianToSpherical);

        // Conversion from polar (optional, not always valid)
        if (testData.polar.has_value())
        {
            DirectionVector vectorPolar;
            TEST_ASSERT(true == vectorPolar.SetDirectionUsingPolar(&testData.polar.value(), 1));

            const auto kExpectedPolarToCartesian = testData.cartesian;
            decltype(testData.cartesian) actualPolarToCartesian;
            TEST_ASSERT((int)actualPolarToCartesian.size() == vectorPolar.GetCartesianCoordinates(&actualPolarToCartesian[0], (int)actualPolarToCartesian.size()));
            CheckCartesianDirectionEquivalence(&kExpectedPolarToCartesian[0], &actualPolarToCartesian[0], (int)kNumAxes);

            const auto kExpectedPolarToSpherical = testData.spherical;
            decltype(testData.spherical) actualPolarToSpherical;
            TEST_ASSERT((int)actualPolarToSpherical.size() == vectorPolar.GetSphericalCoordinates(&actualPolarToSpherical[0], (int)actualPolarToSpherical.size()));
            TEST_ASSERT(actualPolarToSpherical == kExpectedPolarToSpherical);
        }

        // Conversion from spherical
        DirectionVector vectorSpherical;
        TEST_ASSERT(true == vectorSpherical.SetDirectionUsingSpherical(&testData.spherical[0], (int)testData.spherical.size()));

        const auto kExpectedSphericalToCartesian = testData.cartesian;
        decltype(testData.cartesian) actualSphericalToCartesian;
        TEST_ASSERT((int)actualSphericalToCartesian.size() == vectorSpherical.GetCartesianCoordinates(&actualSphericalToCartesian[0], (int)actualSphericalToCartesian.size()));
        CheckCartesianDirectionEquivalence(&kExpectedSphericalToCartesian[0], &actualSphericalToCartesian[0], (int)kNumAxes);

        if (true == testData.polar.has_value())
        {
            const auto kExpectedSphericalToPolar = testData.polar.value();
            TEffectValue actualSphericalToPolar = 0;
            TEST_ASSERT(1 == vectorSpherical.GetPolarCoordinates(&actualSphericalToPolar, 1));
            TEST_ASSERT(actualSphericalToPolar == kExpectedSphericalToPolar);
        }
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Exercises coordinate system setting, getting, and converting with single-axis direction vectors.
    // The only possible input coordinate system is Cartesian, and all of these attempted conversions should fail.
    TEST_CASE(ForceFeedbackDirectionVector_1D_Conversions)
    {
        constexpr TEffectValue kTestCoordinates[] = {-100000000, -10000, -100, -1, 1, 100, 10000, 100000000};
        
        for (const auto kTestCoordinate : kTestCoordinates)
        {
            DirectionVector vector;
            TEST_ASSERT(true == vector.SetDirectionUsingCartesian(&kTestCoordinate, 1));

            // Simple retrieval should succeed without any transformation.
            std::array<TEffectValue, kEffectAxesMaximumNumber> actualOutputCoordinates = {0};
            TEST_ASSERT(1 == vector.GetCartesianCoordinates(&actualOutputCoordinates[0], (int)actualOutputCoordinates.size()));
            TEST_ASSERT(actualOutputCoordinates[0] == kTestCoordinate);

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
    TEST_CASE(ForceFeedbackDirectionVector_1D_MagnitudeComponents)
    {
        constexpr TEffectValue kTestMagnitudes[] = {-1000, -10, 0, 100, 10000};
        constexpr TEffectValue kTestCoordinates[] = {-100000000, -10000, -100, -1, 1, 100, 10000, 100000000};

        for (const auto kTestMagnitude : kTestMagnitudes)
        {
            for (const auto kTestCoordinate : kTestCoordinates)
            {
                DirectionVector vector;
                TEST_ASSERT(true == vector.SetDirectionUsingCartesian(&kTestCoordinate, 1));

                const TMagnitudeComponents kExpectedOutput = {((kTestCoordinate > 0) ? kTestMagnitude : -kTestMagnitude)};
                const TMagnitudeComponents kActualOutput = vector.ComputeMagnitudeComponents(kTestMagnitude);
                TEST_ASSERT(kActualOutput == kExpectedOutput);
            }
        }
    }

    // Exercises coordinate system setting, getting, and converting with two-axis direction vectors.
    TEST_CASE(ForceFeedbackDirectionVector_2D_Conversions)
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

        for (const auto& kTest : kTestData)
            DirectionVectorCoordinateConversionTest(kTest);
    }
}
