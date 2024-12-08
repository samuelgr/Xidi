/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ControllerMathTest.cpp
 *   Unit tests for internal math.
 **************************************************************************************************/

#include "ControllerMath.h"

#include <algorithm>

#include <Infra/Test/TestCase.h>

#include "ControllerTypes.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller::Math;

  /// Compares two integer values and determines if they are "sufficiently equal" or not. The
  /// comparison computes the absolute value of the difference and ensures it is within a very tight
  /// threshold.
  /// @tparam IntegerType Type of integers to compare. Can be signed or unsigned.
  /// @param a First integer to compare.
  /// @param b Second integer to compare.
  /// @return Whether or not the two integers are considered "sufficiently equal" by being within a
  /// specific difference threshold of one another.
  template <typename IntegerType> static bool SufficientlyEqual(IntegerType a, IntegerType b)
  {
    constexpr IntegerType kMaxDifference = 1;
    return (std::max(a, b) - std::min(a, b)) <= kMaxDifference;
  }

  // Verifies that no transformation is applied to analog stick readings for a deadzone and
  // saturation of 0 and 100, respectively. These settings indicate no deadzone or saturation point.
  TEST_CASE(ControllerMath_AnalogTransformNominal)
  {
    constexpr unsigned int kDeadzonePercent = 0;
    constexpr unsigned int kSaturationPercent = 100;

    constexpr int16_t kTestValues[] = {-32768, -100, 0, 100, 32767};

    for (const auto& testValue : kTestValues)
    {
      TEST_ASSERT(
          testValue == ApplyRawAnalogTransform(testValue, kDeadzonePercent, kSaturationPercent));
    }
  }

  // Verifies that deadzone transformations are applied correctly in isolation for analog sticks.
  TEST_CASE(ControllerMath_AnalogTransformWithDeadzone)
  {
    constexpr unsigned int kDeadzonePercent = 50;
    constexpr unsigned int kSaturationPercent = 100;

    constexpr struct
    {
      int16_t rawInput;
      int16_t expectedOutput;
    } kTestValues[] = {
        {.rawInput = -32768, .expectedOutput = -32768},
        {.rawInput = 32767, .expectedOutput = 32767},
        {.rawInput = 16383, .expectedOutput = 0},
        {.rawInput = -16383, .expectedOutput = 0},
        {.rawInput = (16383 + (16384 * 1 / 4)), .expectedOutput = (32768 * 1 / 4)},
        {.rawInput = -(16383 + (16384 * 1 / 4)), .expectedOutput = -(32768 * 1 / 4)},
        {.rawInput = (16383 + (16384 * 1 / 2)), .expectedOutput = (32768 * 1 / 2)},
        {.rawInput = -(16383 + (16384 * 1 / 2)), .expectedOutput = -(32768 * 1 / 2)},
        {.rawInput = (16383 + (16384 * 3 / 4)), .expectedOutput = (32768 * 3 / 4)},
        {.rawInput = -(16383 + (16384 * 3 / 4)), .expectedOutput = -(32768 * 3 / 4)},
    };

    for (const auto& testValue : kTestValues)
    {
      const auto actualOutput =
          ApplyRawAnalogTransform(testValue.rawInput, kDeadzonePercent, kSaturationPercent);
      TEST_ASSERT(SufficientlyEqual(actualOutput, testValue.expectedOutput));
    }
  }

  // Verifies that saturation transformations are applied correctly in isolation for analog sticks.
  TEST_CASE(ControllerMath_AnalogTransformWithSaturation)
  {
    constexpr unsigned int kDeadzonePercent = 0;
    constexpr unsigned int kSaturationPercent = 50;

    constexpr struct
    {
      int16_t rawInput;
      int16_t expectedOutput;
    } kTestValues[] = {
        {.rawInput = -32768, .expectedOutput = -32768},
        {.rawInput = 32767, .expectedOutput = 32767},
        {.rawInput = 16383, .expectedOutput = 32767},
        {.rawInput = -16383, .expectedOutput = -32767},
        {.rawInput = (0 + (16384 * 1 / 4)), .expectedOutput = (32768 * 1 / 4)},
        {.rawInput = -(0 + (16384 * 1 / 4)), .expectedOutput = -(32768 * 1 / 4)},
        {.rawInput = (0 + (16384 * 1 / 2)), .expectedOutput = (32768 * 1 / 2)},
        {.rawInput = -(0 + (16384 * 1 / 2)), .expectedOutput = -(32768 * 1 / 2)},
        {.rawInput = (0 + (16384 * 3 / 4)), .expectedOutput = (32768 * 3 / 4)},
        {.rawInput = -(0 + (16384 * 3 / 4)), .expectedOutput = -(32768 * 3 / 4)},
    };

    for (const auto& testValue : kTestValues)
    {
      const auto actualOutput =
          ApplyRawAnalogTransform(testValue.rawInput, kDeadzonePercent, kSaturationPercent);
      TEST_ASSERT(SufficientlyEqual(actualOutput, testValue.expectedOutput));
    }
  }

  // Verifies that deadzone and saturation transformations are applied correctly in combination for
  // analog sticks.
  TEST_CASE(ControllerMath_AnalogTransformWithDeadzoneAndSaturation)
  {
    constexpr unsigned int kDeadzonePercent = 25;
    constexpr unsigned int kSaturationPercent = 75;

    constexpr struct
    {
      int16_t rawInput;
      int16_t expectedOutput;
    } kTestValues[] = {
        {.rawInput = -32768, .expectedOutput = -32768},
        {.rawInput = 32767, .expectedOutput = 32767},
        {.rawInput = 16383, .expectedOutput = 16384},
        {.rawInput = -16383, .expectedOutput = -16384},
        {.rawInput = (8191 + (16384 * 1 / 4)), .expectedOutput = (32768 * 1 / 4)},
        {.rawInput = -(8191 + (16384 * 1 / 4)), .expectedOutput = -(32768 * 1 / 4)},
        {.rawInput = (8191 + (16384 * 1 / 2)), .expectedOutput = (32768 * 1 / 2)},
        {.rawInput = -(8191 + (16384 * 1 / 2)), .expectedOutput = -(32768 * 1 / 2)},
        {.rawInput = (8191 + (16384 * 3 / 4)), .expectedOutput = (32768 * 3 / 4)},
        {.rawInput = -(8191 + (16384 * 3 / 4)), .expectedOutput = -(32768 * 3 / 4)},
    };

    for (const auto& testValue : kTestValues)
    {
      const auto actualOutput =
          ApplyRawAnalogTransform(testValue.rawInput, kDeadzonePercent, kSaturationPercent);
      TEST_ASSERT(SufficientlyEqual(actualOutput, testValue.expectedOutput));
    }
  }

  // Verifies that no transformation is applied to trigger readings for a deadzone and saturation of
  // 0 and 100, respectively. These settings indicate no deadzone or saturation point.
  TEST_CASE(ControllerMath_TriggerTransformNominal)
  {
    constexpr unsigned int kDeadzonePercent = 0;
    constexpr unsigned int kSaturationPercent = 100;

    constexpr uint8_t kTestValues[] = {0, 31, 63, 127, 159, 191, 223, 255};

    for (const auto& testValue : kTestValues)
    {
      TEST_ASSERT(
          testValue == ApplyRawTriggerTransform(testValue, kDeadzonePercent, kSaturationPercent));
    }
  }

  // Verifies that deadzone transformations are applied correctly in isolation for triggers.
  TEST_CASE(ControllerMath_TriggerTransformWithDeadzone)
  {
    constexpr unsigned int kDeadzonePercent = 50;
    constexpr unsigned int kSaturationPercent = 100;

    constexpr struct
    {
      uint8_t rawInput;
      uint8_t expectedOutput;
    } kTestValues[] = {
        {.rawInput = 0, .expectedOutput = 0},
        {.rawInput = 255, .expectedOutput = 255},
        {.rawInput = (255 * 1 / 8), .expectedOutput = 0},
        {.rawInput = (255 * 1 / 4), .expectedOutput = 0},
        {.rawInput = (255 * 1 / 2), .expectedOutput = 0},
        {.rawInput = (255 * 3 / 4), .expectedOutput = (255 * 1 / 2)},
        {.rawInput = (255 * 7 / 8), .expectedOutput = (255 * 3 / 4)},
    };

    for (const auto& testValue : kTestValues)
    {
      const auto actualOutput =
          ApplyRawTriggerTransform(testValue.rawInput, kDeadzonePercent, kSaturationPercent);
      TEST_ASSERT(SufficientlyEqual(actualOutput, testValue.expectedOutput));
    }
  }

  // Verifies that saturation transformations are applied correctly in isolation for triggers.
  TEST_CASE(ControllerMath_TriggerTransformWithSaturation)
  {
    constexpr unsigned int kDeadzonePercent = 0;
    constexpr unsigned int kSaturationPercent = 50;

    constexpr struct
    {
      uint8_t rawInput;
      uint8_t expectedOutput;
    } kTestValues[] = {
        {.rawInput = 0, .expectedOutput = 0},
        {.rawInput = 255, .expectedOutput = 255},
        {.rawInput = (255 * 1 / 8), .expectedOutput = (255 * 1 / 4)},
        {.rawInput = (255 * 1 / 4), .expectedOutput = (255 * 1 / 2)},
        {.rawInput = (255 * 1 / 2), .expectedOutput = 255},
        {.rawInput = (255 * 3 / 4), .expectedOutput = 255},
        {.rawInput = (255 * 7 / 8), .expectedOutput = 255},
    };

    for (const auto& testValue : kTestValues)
    {
      const auto actualOutput =
          ApplyRawTriggerTransform(testValue.rawInput, kDeadzonePercent, kSaturationPercent);
      TEST_ASSERT(SufficientlyEqual(actualOutput, testValue.expectedOutput));
    }
  }

  // Verifies that deadzone and saturation transformations are applied correctly in combination for
  // triggers.
  TEST_CASE(ControllerMath_TriggerTransformWithDeadzoneAndSaturation)
  {
    constexpr unsigned int kDeadzonePercent = 25;
    constexpr unsigned int kSaturationPercent = 75;

    constexpr struct
    {
      uint8_t rawInput;
      uint8_t expectedOutput;
    } kTestValues[] = {
        {.rawInput = 0, .expectedOutput = 0},
        {.rawInput = 255, .expectedOutput = 255},
        {.rawInput = (255 * 1 / 8), .expectedOutput = 0},
        {.rawInput = (255 * 1 / 4), .expectedOutput = 0},
        {.rawInput = (255 * 1 / 2), .expectedOutput = (255 * 1 / 2)},
        {.rawInput = (255 * 3 / 4), .expectedOutput = 255},
        {.rawInput = (255 * 7 / 8), .expectedOutput = 255},
    };

    for (const auto& testValue : kTestValues)
    {
      const auto actualOutput =
          ApplyRawTriggerTransform(testValue.rawInput, kDeadzonePercent, kSaturationPercent);
      TEST_ASSERT(SufficientlyEqual(actualOutput, testValue.expectedOutput));
    }
  }

  // Verifies that analog sticks are correctly identified as "pressed" as a digital button if
  // sufficiently pressed in the positive direction. Only checks extreme values to avoid enforcing a
  // specific threshold value requirement.
  TEST_CASE(ControllerMath_IsAnalogPressed_PositiveThreshold)
  {
    constexpr struct
    {
      int16_t rawInput;
      bool expectedIsPressed;
    } kTestValues[] = {
        {.rawInput = -32768, .expectedIsPressed = false},
        {.rawInput = 0, .expectedIsPressed = false},
        {.rawInput = 32767, .expectedIsPressed = true}};

    for (const auto& testValue : kTestValues)
    {
      const auto actualIsPressed = IsAnalogPressedPositive(testValue.rawInput);
      TEST_ASSERT(actualIsPressed == testValue.expectedIsPressed);
    }
  }

  // Verifies that analog sticks are correctly identified as "pressed" as a digital button if
  // sufficiently pressed in the negative direction. Only checks extreme values to avoid enforcing
  // a specific threshold value requirement.
  TEST_CASE(ControllerMath_IsAnalogPressed_NegativeThreshold)
  {
    constexpr struct
    {
      int16_t rawInput;
      bool expectedIsPressed;
    } kTestValues[] = {
        {.rawInput = -32768, .expectedIsPressed = true},
        {.rawInput = 0, .expectedIsPressed = false},
        {.rawInput = 32767, .expectedIsPressed = false}};

    for (const auto& testValue : kTestValues)
    {
      const auto actualIsPressed = IsAnalogPressedNegative(testValue.rawInput);
      TEST_ASSERT(actualIsPressed == testValue.expectedIsPressed);
    }
  }

  // Verifies that analog sticks are correctly identified as "pressed" as a digital button if
  // sufficiently pressed in either direction. Only checks extreme values to avoid enforcing a
  // specific threshold value requirement.
  TEST_CASE(ControllerMath_IsAnalogPressed_BidirectionalThreshold)
  {
    constexpr struct
    {
      int16_t rawInput;
      bool expectedIsPressed;
    } kTestValues[] = {
        {.rawInput = -32768, .expectedIsPressed = true},
        {.rawInput = 0, .expectedIsPressed = false},
        {.rawInput = 32767, .expectedIsPressed = true}};

    for (const auto& testValue : kTestValues)
    {
      const auto actualIsPressed = IsAnalogPressed(testValue.rawInput);
      TEST_ASSERT(actualIsPressed == testValue.expectedIsPressed);
    }
  }

  // Verifies that triggers sticks are correctly identified as "pressed" as a digital button if
  // sufficiently pressed. Only checks extreme values to avoid enforcing a specific threshold value
  // requirement.
  TEST_CASE(ControllerMath_IsTriggerPressed_UnidirectionalThreshold)
  {
    constexpr struct
    {
      uint8_t rawInput;
      bool expectedIsPressed;
    } kTestValues[] = {
        {.rawInput = 0, .expectedIsPressed = false}, {.rawInput = 255, .expectedIsPressed = true}};

    for (const auto& testValue : kTestValues)
    {
      const auto actualIsPressed = IsTriggerPressed(testValue.rawInput);
      TEST_ASSERT(actualIsPressed == testValue.expectedIsPressed);
    }
  }

  // Verifies correct application of the square correction transformation, using input coordinates
  // along only a single axis at a time. The expected result is that there should be no change in
  // the input.
  TEST_CASE(ControllerMath_TransformCoordinatesCircleToSquare_OneDimensional)
  {
    constexpr double kAmountFraction = 1.0;
    constexpr SAnalogStickCoordinates kTestValues[] = {
        {.x = 0, .y = 0},
        {.x = 32767, .y = 0},
        {.x = -32767, .y = 0},
        {.x = 0, .y = 32767},
        {.x = 0, .y = -32767},
        {.x = 100, .y = 0},
        {.x = -100, .y = 0},
        {.x = 0, .y = 100},
        {.x = 0, .y = -100}};

    for (const auto& testValue : kTestValues)
    {
      TEST_ASSERT(testValue == TransformCoordinatesCircleToSquare(testValue, kAmountFraction));
    }
  }

  // Verifies correct application of the square correction transformation, using input coordinates
  // along two axes simultaneously, but with the amount set to 0 so the transformation should be a
  // no-op.
  TEST_CASE(ControllerMath_TransformCoordinatesCircleToSquare_TwoDimensionalDisabled)
  {
    constexpr double kAmountFraction = 0.0;
    constexpr SAnalogStickCoordinates kTestValues[] = {
        {.x = 0, .y = 0}, {.x = 100, .y = 4199}, {.x = -5000, .y = 22}};

    for (const auto& testValue : kTestValues)
    {
      TEST_ASSERT(testValue == TransformCoordinatesCircleToSquare(testValue, kAmountFraction));
    }
  }

  // Verifies correct application of the square correction transformation, using input coordinates
  // along two axes simultaneously.
  TEST_CASE(ControllerMath_TransformCoordinatesCircleToSquare_TwoDimensional)
  {
    constexpr double kAmountFraction = 1.0;

    // Extreme diagonal coordinate within a circular range of motion. Represents the absolute value
    // of both X and Y coordinates at a 45-degree angle from either horizontal or vertical axes.
    // Equal to 32768 * (1 / sqrt(2)).
    constexpr int16_t kExtremeDiagonalCircleCoord = 23170;

    constexpr struct
    {
      SAnalogStickCoordinates rawInputCircleCoords;
      SAnalogStickCoordinates expectedOutputSquareCoords;
    } kTestValues[] = {
        {.rawInputCircleCoords =
             {.x = kExtremeDiagonalCircleCoord, .y = kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = 32767, .y = 32767}},
        {.rawInputCircleCoords =
             {.x = kExtremeDiagonalCircleCoord, .y = -kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = 32767, .y = -32767}},
        {.rawInputCircleCoords =
             {.x = -kExtremeDiagonalCircleCoord, .y = kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = -32767, .y = 32767}},
        {.rawInputCircleCoords =
             {.x = -kExtremeDiagonalCircleCoord, .y = -kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = -32767, .y = -32767}},
        {.rawInputCircleCoords =
             {.x = kExtremeDiagonalCircleCoord / 2, .y = kExtremeDiagonalCircleCoord / 2},
         .expectedOutputSquareCoords = {.x = 16383, .y = 16383}},
    };

    for (const auto& testValue : kTestValues)
    {
      SAnalogStickCoordinates actualOutputSquareCoords =
          TransformCoordinatesCircleToSquare(testValue.rawInputCircleCoords, kAmountFraction);
      TEST_ASSERT(
          SufficientlyEqual(actualOutputSquareCoords.x, testValue.expectedOutputSquareCoords.x));
      TEST_ASSERT(
          SufficientlyEqual(actualOutputSquareCoords.y, testValue.expectedOutputSquareCoords.y));
    }
  }

  // Verifies correct application of the square correction transformation, using input coordinates
  // along two axes simultaneously. Verifies that the calculations behave predictably when the input
  // coordinates are not possible in a completely circular range of motion.
  TEST_CASE(ControllerMath_TransformCoordinatesCircleToSquare_TwoDimensionalWithImperfectCircle)
  {
    constexpr double kAmountFraction = 1.0;

    // Extreme diagonal coordinate within a circular range of motion, scaled up slightly so that the
    // radius is too high for a circle of radius 32768.
    constexpr int16_t kExtremeDiagonalCircleCoord = 26000;

    constexpr struct
    {
      SAnalogStickCoordinates rawInputCircleCoords;
      SAnalogStickCoordinates expectedOutputSquareCoords;
    } kTestValues[] = {
        {.rawInputCircleCoords =
             {.x = kExtremeDiagonalCircleCoord, .y = kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = 32767, .y = 32767}},
        {.rawInputCircleCoords =
             {.x = kExtremeDiagonalCircleCoord, .y = -kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = 32767, .y = -32767}},
        {.rawInputCircleCoords =
             {.x = -kExtremeDiagonalCircleCoord, .y = kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = -32767, .y = 32767}},
        {.rawInputCircleCoords =
             {.x = -kExtremeDiagonalCircleCoord, .y = -kExtremeDiagonalCircleCoord},
         .expectedOutputSquareCoords = {.x = -32767, .y = -32767}},
    };

    for (const auto& testValue : kTestValues)
    {
      SAnalogStickCoordinates actualOutputSquareCoords =
          TransformCoordinatesCircleToSquare(testValue.rawInputCircleCoords, kAmountFraction);
      TEST_ASSERT(
          SufficientlyEqual(actualOutputSquareCoords.x, testValue.expectedOutputSquareCoords.x));
      TEST_ASSERT(
          SufficientlyEqual(actualOutputSquareCoords.y, testValue.expectedOutputSquareCoords.y));
    }
  }

} // namespace XidiTest
