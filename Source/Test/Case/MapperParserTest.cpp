/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MapperParserTest.cpp
 *   Unit tests for run-time mapper object parsing functionality.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ForceFeedbackTypes.h"
#include "Keyboard.h"
#include "Mapper.h"
#include "MapperParser.h"
#include "Mouse.h"
#include "TestCase.h"

#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>


namespace XidiTest
{
    using namespace ::Xidi::Controller;
    using ::Xidi::Controller::ForceFeedback::EActuatorMode;
    using ::Xidi::Controller::ForceFeedback::SActuatorElement;
    using ::Xidi::Controller::MapperParser::ElementMapperOrError;
    using ::Xidi::Controller::MapperParser::ForceFeedbackActuatorOrError;
    using ::Xidi::Controller::MapperParser::SStringParts;
    using ::Xidi::Controller::MapperParser::SElementMapperParseResult;
    using ::Xidi::Controller::MapperParser::SParamStringParts;
    using ::Xidi::Keyboard::TKeyIdentifier;
    using ::Xidi::Mouse::EMouseAxis;
    using ::Xidi::Mouse::EMouseButton;


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Checks if the supplied element mapper pointers are equivalent and flags a test failure if not.
    /// Only works for simple element mappers that uniquely target zero or one specific controller elements and have no side effects.
    /// @param [in] elementMapperA First of the two element mapper pointers to compare.
    /// @param [in] elementMapperB Second of the two element mapper pointers to compare.
    static void VerifyElementMapperPointersAreEquivalent(const IElementMapper* elementMapperA, const IElementMapper* elementMapperB)
    {
        if (nullptr == elementMapperA)
        {
            TEST_ASSERT(nullptr == elementMapperB);
        }
        else
        {
            TEST_ASSERT(nullptr != elementMapperB);
            TEST_ASSERT(elementMapperA->GetTargetElementCount() == elementMapperB->GetTargetElementCount());
            for (int i = 0; i < elementMapperA->GetTargetElementCount(); ++i)
                TEST_ASSERT(elementMapperA->GetTargetElementAt(i) == elementMapperB->GetTargetElementAt(i));
        }
    }

    /// Checks if the supplied element mapper parse results are equivalent and flags a test failure if not.
    /// Only works for simple element mappers that uniquely target zero or one specific controller elements and have no side effects.
    /// @param [in] resultA First of the two parse result structures to compare.
    /// @param [in] resultB Second of the two parse result structures to compare.
    static void VerifyParseResultsAreEquivalent(const SElementMapperParseResult& resultA, const SElementMapperParseResult& resultB)
    {
        TEST_ASSERT(resultA.maybeElementMapper.HasValue() == resultB.maybeElementMapper.HasValue());
        TEST_ASSERT(resultA.remainingString == resultB.remainingString);

        if (resultA.maybeElementMapper.HasValue())
            VerifyElementMapperPointersAreEquivalent(resultA.maybeElementMapper.Value().get(), resultB.maybeElementMapper.Value().get());
    }

    /// Checks if the supplied split mapper pointers are equivalent and flags a test failure if not.
    /// Supports recursively descending into sub-mappers that happen to be split mappers.
    /// @param [in] splitMapperA First of the two split mapper pointers to compare.
    /// @param [in] splitMapperB Second of the two split mapper pointers to compare.
    static inline void VerifySplitMapperPointersAreEquivalent(const SplitMapper* splitMapperA, const SplitMapper* splitMapperB)
    {
        if (nullptr == splitMapperA)
        {
            TEST_ASSERT(nullptr == splitMapperB);
        }
        else
        {
            const IElementMapper* subMappersA[] = {splitMapperA->GetPositiveMapper(), splitMapperA->GetNegativeMapper()};
            const IElementMapper* subMappersB[] = {splitMapperB->GetPositiveMapper(), splitMapperB->GetNegativeMapper()};
            static_assert(_countof(subMappersA) == _countof(subMappersB), "Sub mapper array size mismatch.");

            for (int i = 0; i < _countof(subMappersA); ++i)
            {
                const SplitMapper* subSplitMapperA = dynamic_cast<const SplitMapper*>(subMappersA[i]);
                const SplitMapper* subSplitMapperB = dynamic_cast<const SplitMapper*>(subMappersB[i]);

                if (nullptr == subSplitMapperA)
                {
                    TEST_ASSERT(nullptr == subSplitMapperB);
                    VerifyElementMapperPointersAreEquivalent(subMappersA[i], subMappersB[i]);
                }
                else
                {
                    TEST_ASSERT(nullptr != subSplitMapperB);
                    VerifySplitMapperPointersAreEquivalent(subSplitMapperA, subSplitMapperB);
                }
            }
        }
    }


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies correct identification of valid controller element strings.
    TEST_CASE(MapperParser_ControllerElementString_Valid)
    {
        constexpr std::pair<unsigned int, std::wstring_view> kControllerElements[] = {
            {ELEMENT_MAP_INDEX_OF(stickLeftY), L"StickLeftY"},
            {ELEMENT_MAP_INDEX_OF(dpadDown), L"DpadDown"},
            {ELEMENT_MAP_INDEX_OF(triggerLT), L"TriggerLT"},
            {ELEMENT_MAP_INDEX_OF(buttonRB), L"ButtonRB"},
            {ELEMENT_MAP_INDEX_OF(buttonStart), L"ButtonStart"}
        };

        for (const auto& controllerElement : kControllerElements)
        {
            TEST_ASSERT(true == MapperParser::IsControllerElementStringValid(controllerElement.second));
            TEST_ASSERT(controllerElement.first == MapperParser::FindControllerElementIndex(controllerElement.second));
        }
    }

    // Verifies correct identification of invalid controller element strings.
    TEST_CASE(MapperParser_ControllerElementString_Invalid)
    {
        constexpr std::wstring_view kControllerElementStrings[] = {L"stickLeftZ", L"dpadDown", L"random_string"};

        for (auto controllerElementString : kControllerElementStrings)
        {
            TEST_ASSERT(false == MapperParser::IsControllerElementStringValid(controllerElementString));
            TEST_ASSERT(false == MapperParser::FindControllerElementIndex(controllerElementString).has_value());
        }
    }

    // Verifies correct identification of valid force feedback actuator strings.
    TEST_CASE(MapperParser_ForceFeedbackActuatorString_Valid)
    {
        constexpr std::pair<unsigned int, std::wstring_view> kForceFeedbackActuators[] = {
            {FFACTUATOR_MAP_INDEX_OF(leftMotor), L"ForceFeedback.LeftMotor"},
            {FFACTUATOR_MAP_INDEX_OF(rightMotor), L"ForceFeedback.RightMotor"}
        };

        for (const auto& ffActuator : kForceFeedbackActuators)
        {
            TEST_ASSERT(true == MapperParser::IsForceFeedbackActuatorStringValid(ffActuator.second));
            TEST_ASSERT(ffActuator.first == MapperParser::FindForceFeedbackActuatorIndex(ffActuator.second));
        }
    }

    // Verifies correct identification of invalid force feedback actuator strings.
    TEST_CASE(MapperParser_ForceFeedbackActuatorString_Invalid)
    {
        constexpr std::wstring_view kForceFeedbackActuatorStrings[] = { L"leftMotor", L"RightMotor", L"random_string" };

        for (auto ffActuatorString : kForceFeedbackActuatorStrings)
        {
            TEST_ASSERT(false == MapperParser::IsForceFeedbackActuatorStringValid(ffActuatorString));
            TEST_ASSERT(false == MapperParser::FindForceFeedbackActuatorIndex(ffActuatorString).has_value());
        }
    }

    // Verifies correct determination of recursion depth, given a set of input strings that are all properly balanced.
    TEST_CASE(MapperParser_RecursionDepth_Balanced)
    {
        constexpr std::pair<unsigned int, std::wstring_view> kRecursionTestItems[] = {
            {0, L" MapperStringNoParams  "},
            {1, L"   OuterMapper  (   Param1, Param2 )"},
            {2, L"OuterMapper( InnerMapper1( Param), InnerMapper2(Param234))"},
            {3, L"Split(    Split( Button(1), Button(2)), Split(Button(3), Button(4)), Axis(Z))"},
            {4, L" ( ()  (  ()   (  ()) () ))"}
        };

        for (auto& recursionTestItem : kRecursionTestItems)
            TEST_ASSERT(recursionTestItem.first == MapperParser::ComputeRecursionDepth(recursionTestItem.second));
    }

    // Verifies inability to compute recursion depth, given a set of input strings that are not properly balanced.
    TEST_CASE(MapperParser_RecursionDepth_Unbalanced)
    {
        constexpr std::wstring_view kRecursionTestStrings[] = {
            L")",
            L"(",
            L"    )  (",
            L"   (    (    )",
            L"   (   )    (    ",
            L"   OuterMapper    Param1, Param2 )",
            L"Split(    Split( Button(1), Button(2)), Split(Button(3), Button(4)"
        };

        for (auto& recursionTestString : kRecursionTestStrings)
            TEST_ASSERT(std::nullopt == MapperParser::ComputeRecursionDepth(recursionTestString));
    }

    // Verifies correct separation of an input element mapper string into type and parameter substrings.
    // Exercises several different simple cases in which the element mapper string contains one type and one set of parameters.
    // The whole string is consumed, so there is no remainder.
    TEST_CASE(MapperParser_ExtractElementMapperStringParts_Simple)
    {
        constexpr std::pair<std::wstring_view, SStringParts> kExtractPartsTestItems[] = {
            {L"Axis(Y)",                                {.type = L"Axis",   .params = L"Y"}},
            {L"   Axis       (    Y    ,    + )",       {.type = L"Axis",   .params = L"Y    ,    +"}},
            {L"   Null  ",                              {.type = L"Null"}}
        };

        for (auto& extractPartsTestItem : kExtractPartsTestItems)
            TEST_ASSERT(extractPartsTestItem.second == MapperParser::ExtractElementMapperStringParts(extractPartsTestItem.first));
    }

    // Verifies correct separation of an input element mapper string into type and parameter substrings.
    // Exercises several different nested cases in which an element mapper string has other element mapper strings as parameters.
    // The whole string is consumed, so there is no remainder.
    TEST_CASE(MapperParser_ExtractElementMapperStringParts_Nested)
    {
        constexpr std::pair<std::wstring_view, SStringParts> kExtractPartsTestItems[] = {
            {L"  Split ( Button(2), Button(3)   )",                                 {.type = L"Split",  .params = L"Button(2), Button(3)"}},
            {L"Split( Split(Button(1), Button(2)), Split(Button(3), Button(4)) )",  {.type = L"Split",  .params = L"Split(Button(1), Button(2)), Split(Button(3), Button(4))"}}
        };

        for (auto& extractPartsTestItem : kExtractPartsTestItems)
            TEST_ASSERT(extractPartsTestItem.second == MapperParser::ExtractElementMapperStringParts(extractPartsTestItem.first));
    }

    // Verifies correct separation of an input mapper element string into type, parameter, and remaining substrings.
    // Exercises situations in which the whole string is not consumed, so there is a remaining part of the string left behind.
    TEST_CASE(MapperParser_ExtractElementMapperStringParts_PartialWithRemainder)
    {
        constexpr std::pair<std::wstring_view, SStringParts> kExtractPartsTestItems[] = {
            {L"  Null      ,   Button(2) ",                                         {.type = L"Null",                                       .remaining = L"Button(2)"}},
            {L"  Null,   Button(2) ",                                               {.type = L"Null",                                       .remaining = L"Button(2)"}},
            {L"Split(Button(1), Button(2)), Split(Button(3), Button(4))",           {.type = L"Split",  .params = L"Button(1), Button(2)",  .remaining = L"Split(Button(3), Button(4))"}}
        };

        for (auto& extractPartsTestItem : kExtractPartsTestItems)
            TEST_ASSERT(extractPartsTestItem.second == MapperParser::ExtractElementMapperStringParts(extractPartsTestItem.first));
    }

    // Verifies correct rejection of invalid element mapper strings when attempting to split into type, parameter, and remaining substrings.
    TEST_CASE(MapperParser_ExtractElementMapperStringParts_Invalid)
    {
        constexpr std::wstring_view kExtractPartsTestStrings[] = {
            L"  Null   )  ",
            L"Null,",
            L"  Null   , ",
            L"Split(Button(1), Button(2)))   ",
            L"Axis(RotZ",
            L"Axis(RotZ),"
        };

        for (auto& extractPartsTestString : kExtractPartsTestStrings)
            TEST_ASSERT(false == MapperParser::ExtractElementMapperStringParts(extractPartsTestString).has_value());
    }

    TEST_CASE(MapperParser_ExtractForceFeedbackActuatorStringParts_Valid)
    {
        constexpr std::pair<std::wstring_view, SStringParts> kExtractPartsTestItems[] = {
            {L"  SingleAxis( X )         ",                                         {.type = L"SingleAxis",             .params = L"X"}},
            {L"SingleAxis(Y, +)",                                                   {.type = L"SingleAxis",             .params = L"Y, +"}},
            {L"     MagnitudeProjection   (   Z,    RotZ   )           ",           {.type = L"MagnitudeProjection",    .params = L"Z,    RotZ"}},
            {L" None ",                                                             {.type = L"None"}}
        };

        for (auto& extractPartsTestItem : kExtractPartsTestItems)
            TEST_ASSERT(extractPartsTestItem.second == MapperParser::ExtractForceFeedbackActuatorStringParts(extractPartsTestItem.first));
    }

    TEST_CASE(MapperParser_ExtractForceFeedbackActuatorStringParts_Invalid)
    {
        constexpr std::wstring_view kExtractPartsTestStrings[] = {
            L"  Null   )  ",
            L"Null,",
            L"  Null   , ",
            L"MagnitudeProjection(X, Y), SingleAxis(X)",
            L"MagnitudeProjection(X, Y",
            L"SingleAxis(Y, +),"
        };

        for (auto& extractPartsTestString : kExtractPartsTestStrings)
            TEST_ASSERT(false == MapperParser::ExtractForceFeedbackActuatorStringParts(extractPartsTestString).has_value());
    }

    // Verifies correct separation of a parameter string into first parameter and remainder substrings.
    TEST_CASE(MapperParser_ExtractParameterListStringParts_Valid)
    {
        constexpr std::pair<std::wstring_view, SParamStringParts> kExtractPartsTestItems[] = {
            {L"Param1",                                                     {.first = L"Param1"}},
            {L"Param1, Param2",                                             {.first = L"Param1",                        .remaining = L"Param2"}},
            {L"A, B, C, D",                                                 {.first = L"A",                             .remaining = L"B, C, D"}},
            {L"A(0), B(1, 2), C(3, 4), D(5, 6)",                            {.first = L"A(0)",                          .remaining = L"B(1, 2), C(3, 4), D(5, 6)"}},
            {L"   RotY   ,   +  ",                                          {.first = L"RotY",                          .remaining = L"+"}},
            {L"Split(Button(1), Button(2)), Split(Button(3), Button(4))",   {.first = L"Split(Button(1), Button(2))",   .remaining = L"Split(Button(3), Button(4))"}}
        };

        for (auto& extractPartsTestItem : kExtractPartsTestItems)
            TEST_ASSERT(extractPartsTestItem.second == MapperParser::ExtractParameterListStringParts(extractPartsTestItem.first));
    }

    // Verifies correct rejection of invalid parameter list strings when attempting to split into first parameter and remainder substrings.
    TEST_CASE(MapperParser_ExtractParameterListStringParts_Invalid)
    {
        constexpr std::wstring_view kExtractPartsTestStrings[] = {
            L"  Param1  )  ",
            L"  Param2   , ",
            L"Split(Button(1), Button(2), Split(Button(3), Button(4))"
        };

        for (auto& extractPartsTestString : kExtractPartsTestStrings)
            TEST_ASSERT(false == MapperParser::ExtractParameterListStringParts(extractPartsTestString).has_value());
    }

    // Verifies correct construction of axis mapper objects in the nominal case of valid parameter strings being passed.
    // This test does not check axis directon, just target virtual controller element.
    TEST_CASE(MapperParser_MakeAxisMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, SElementIdentifier> kAxisMapperTestItems[] = {
            {L"x",              {.type = EElementType::Axis, .axis = EAxis::X}},
            {L"rX",             {.type = EElementType::Axis, .axis = EAxis::RotX}},
            {L"RotY",           {.type = EElementType::Axis, .axis = EAxis::RotY}},
            {L"rotz, +",        {.type = EElementType::Axis, .axis = EAxis::RotZ}},
            {L"y, NEGATIVE",    {.type = EElementType::Axis, .axis = EAxis::Y}},
        };

        for (auto& axisMapperTestItem : kAxisMapperTestItems)
        {
            ElementMapperOrError maybeAxisMapper = MapperParser::MakeAxisMapper(axisMapperTestItem.first);

            TEST_ASSERT(true == maybeAxisMapper.HasValue());
            TEST_ASSERT(1 == maybeAxisMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(axisMapperTestItem.second == maybeAxisMapper.Value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create axis mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeAxisMapper_Invalid)
    {
        const std::wstring_view kAxisMapperTestStrings[] = {L"A", L"3", L"x, anydir", L"rotz, +, morestuff"};

        for (auto& axisMapperTestString : kAxisMapperTestStrings)
        {
            ElementMapperOrError maybeAxisMapper = MapperParser::MakeAxisMapper(axisMapperTestString);
            TEST_ASSERT(false == maybeAxisMapper.HasValue());
        }
    }

    // Verifies correct construction of button mapper objects in the nominal case of valid parameter strings being passed.
    TEST_CASE(MapperParser_MakeButtonMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, SElementIdentifier> kButtonMapperTestItems[] = {
            {L"1",      {.type = EElementType::Button, .button = EButton::B1}},
            {L"2",      {.type = EElementType::Button, .button = EButton::B2}},
            {L"6",      {.type = EElementType::Button, .button = EButton::B6}},
            {L"12",     {.type = EElementType::Button, .button = EButton::B12}}
        };

        for (auto& buttonMapperTestItem : kButtonMapperTestItems)
        {
            ElementMapperOrError maybeButtonMapper = MapperParser::MakeButtonMapper(buttonMapperTestItem.first);

            TEST_ASSERT(true == maybeButtonMapper.HasValue());
            TEST_ASSERT(1 == maybeButtonMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(buttonMapperTestItem.second == maybeButtonMapper.Value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create button mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeButtonMapper_Invalid)
    {
        const std::wstring_view kButtonMapperTestStrings[] = {L"0", L"B1", L"1B", L"asdf", L""};

        for (auto& buttonMapperTestString : kButtonMapperTestStrings)
        {
            ElementMapperOrError maybeButtonMapper = MapperParser::MakeButtonMapper(buttonMapperTestString);
            TEST_ASSERT(false == maybeButtonMapper.HasValue());
        }
    }

    // Verifies correct construction of compound mapper objects in the nominal case of valid parameter strings being passed.
    // In this test only simple underlying element mapper types are used.
    TEST_CASE(MapperParser_MakeCompoundMapper_Nominal)
    {
        constexpr std::wstring_view kCompoundMapperTestStrings[] = {
            L"Null, Null, Null, Null",
            L"Button(1), Button(2), Button(3), Button(4)",
            L"Axis(X), Axis(RotX)",
            L" Pov(  Up  )  ,   Button  ( 10   ) ",
            L"Axis(RotX, +), Pov(Down)",
            L"Pov(Up), Pov(Down), Pov(Left), Pov(Right)"
        };
        const std::vector<SElementIdentifier> kExpectedElements[] = {
            {},
            {{.type = EElementType::Button, .button = EButton::B1}, {.type = EElementType::Button, .button = EButton::B2}, {.type = EElementType::Button, .button = EButton::B3}, {.type = EElementType::Button, .button = EButton::B4}},
            {{.type = EElementType::Axis, .axis = EAxis::X}, {.type = EElementType::Axis, .axis = EAxis::RotX}},
            {{.type = EElementType::Pov}, {.type = EElementType::Button, .button = EButton::B10}},
            {{.type = EElementType::Axis, .axis = EAxis::RotX}, {.type = EElementType::Pov}},
            {{.type = EElementType::Pov}, {.type = EElementType::Pov}, {.type = EElementType::Pov}, {.type = EElementType::Pov}}
        };
        static_assert(_countof(kExpectedElements) == _countof(kCompoundMapperTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kCompoundMapperTestStrings); ++i)
        {
            ElementMapperOrError maybeCompoundMapper = MapperParser::MakeCompoundMapper(kCompoundMapperTestStrings[i]);

            TEST_ASSERT(true == maybeCompoundMapper.HasValue());
            TEST_ASSERT(kExpectedElements[i].size() == maybeCompoundMapper.Value()->GetTargetElementCount());

            for (unsigned int j = 0; j < kExpectedElements[i].size(); ++j)
            {
                const SElementIdentifier kExpectedElement = kExpectedElements[i][j];

                const std::optional<SElementIdentifier> kMaybeActualElement = maybeCompoundMapper.Value()->GetTargetElementAt(j);
                TEST_ASSERT(true == kMaybeActualElement.has_value());

                const SElementIdentifier kActualElement = kMaybeActualElement.value();
                TEST_ASSERT(kActualElement == kExpectedElement);
            }
        }
    }

    // Verifies correct failure to create compound mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeCompoundMapper_Invalid)
    {
        constexpr std::wstring_view kCompoundMapperTestStrings[] = {
            L"Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null",
            L"Axis(X), Axis(Y), Axis(Z), Axis(RotX), Axis(RotY), Axis(RotZ), Axis(X), Axis(Y), Axis(Z), Axis(RotX), Axis(RotY), Axis(RotZ)"
            L"Button(800)"
        };

        for (auto& compoundMapperTestString : kCompoundMapperTestStrings)
        {
            ElementMapperOrError maybeCompoundMapper = MapperParser::MakeCompoundMapper(compoundMapperTestString);
            TEST_ASSERT(false == maybeCompoundMapper.HasValue());
        }
        }

    // Verifies correct construction of digital axis mapper objects in the nominal case of valid parameter strings being passed.
    // Same as the corresponding axis mapper test but with a different target type.
    TEST_CASE(MapperParser_MakeDigitalAxisMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, SElementIdentifier> kDigitalAxisMapperTestItems[] = {
            {L"x",              {.type = EElementType::Axis, .axis = EAxis::X}},
            {L"rX",             {.type = EElementType::Axis, .axis = EAxis::RotX}},
            {L"RotY",           {.type = EElementType::Axis, .axis = EAxis::RotY}},
            {L"rotz, +",        {.type = EElementType::Axis, .axis = EAxis::RotZ}},
            {L"y, NEGATIVE",    {.type = EElementType::Axis, .axis = EAxis::Y}},
        };

        for (auto& digitalAxisMapperTestItem : kDigitalAxisMapperTestItems)
        {
            ElementMapperOrError maybeDigitalAxisMapper = MapperParser::MakeDigitalAxisMapper(digitalAxisMapperTestItem.first);

            TEST_ASSERT(true == maybeDigitalAxisMapper.HasValue());
            TEST_ASSERT(1 == maybeDigitalAxisMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(digitalAxisMapperTestItem.second == maybeDigitalAxisMapper.Value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create digital axis mapper objects when the parameter strings are invalid.
    // Same as the corresponding axis mapper test but with a different target type.
    TEST_CASE(MapperParser_MakeDigitalAxisMapper_Invalid)
    {
        const std::wstring_view kDigitalAxisMapperTestStrings[] = { L"A", L"3", L"x, anydir", L"rotz, +, morestuff" };

        for (auto& digitalAxisMapperTestString : kDigitalAxisMapperTestStrings)
        {
            ElementMapperOrError maybeDigitalAxisMapper = MapperParser::MakeDigitalAxisMapper(digitalAxisMapperTestString);
            TEST_ASSERT(false == maybeDigitalAxisMapper.HasValue());
        }
    }

    // Verifies correct construction of invert mapper objects in the nominal case of using very simple non-null inner element mappers represented by valid strings.
    TEST_CASE(MapperParser_MakeInvertMapper_Nominal)
    {
        constexpr std::wstring_view kInvertMapperTestStrings[] = {
            L"Axis(X)",
            L" Pov(  Up  )",
            L"   Button(10) ",
            L"Axis(RotX, +)"
        };
        constexpr SElementIdentifier kExpectedElements[] = {
            {.type = EElementType::Axis,   .axis = EAxis::X},
            {.type = EElementType::Pov},
            {.type = EElementType::Button, .button = EButton::B10},
            {.type = EElementType::Axis,   .axis = EAxis::RotX},
        };
        static_assert(_countof(kExpectedElements) == _countof(kInvertMapperTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kInvertMapperTestStrings); ++i)
        {
            ElementMapperOrError maybeInvertMapper = MapperParser::MakeInvertMapper(kInvertMapperTestStrings[i]);

            TEST_ASSERT(true == maybeInvertMapper.HasValue());
            TEST_ASSERT(1 == maybeInvertMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(kExpectedElements[i] == maybeInvertMapper.Value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create invert mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeInvertMapper_Invalid)
    {
        constexpr std::wstring_view kInvertMapperTestStrings[] = {
            L"Axis(X), Axis(RotX), Axis(Z)",
            L"Button(100)",
            L"Null, Null"
        };

        for (auto& invertMapperTestString : kInvertMapperTestStrings)
        {
            ElementMapperOrError maybeInvertMapper = MapperParser::MakeInvertMapper(invertMapperTestString);
            TEST_ASSERT(false == maybeInvertMapper.HasValue());
        }
    }

    // Verifies correct construction of keyboard mapper objects in the nominal case of valid parameter strings being passed.
    TEST_CASE(MapperParser_MakeKeyboardMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, TKeyIdentifier> kKeyboardMapperTestItems[] = {
            {L"100",        100},
            {L"0xcc",       0xcc},
            {L"070",        070},
            {L"DownArrow",  DIK_DOWNARROW},
            {L"DIK_RALT",   DIK_RALT}
        };

        for (auto& keyboardMapperTestItem : kKeyboardMapperTestItems)
        {
            ElementMapperOrError maybeKeyboardMapper = MapperParser::MakeKeyboardMapper(keyboardMapperTestItem.first);

            TEST_ASSERT(true == maybeKeyboardMapper.HasValue());
            TEST_ASSERT(0 == maybeKeyboardMapper.Value()->GetTargetElementCount());

            TEST_ASSERT(nullptr != dynamic_cast<KeyboardMapper*>(maybeKeyboardMapper.Value().get()));

            const TKeyIdentifier kExpectedTargetKey = keyboardMapperTestItem.second;
            const TKeyIdentifier kActualTargetKey = dynamic_cast<KeyboardMapper*>(maybeKeyboardMapper.Value().get())->GetTargetKey();
            TEST_ASSERT(kActualTargetKey == kExpectedTargetKey);
        }
    }

    // Verifies correct failure to create keyboard mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeKeyboardMapper_Invalid)
    {
        const std::wstring_view kKeyboardMapperTestStrings[] = {L"256", L"0x101", L"DIK_INVALID", L"Invalid", L""};

        for (auto& keyboardMapperTestString : kKeyboardMapperTestStrings)
        {
            ElementMapperOrError maybeKeyboardMapper = MapperParser::MakeKeyboardMapper(keyboardMapperTestString);
            TEST_ASSERT(false == maybeKeyboardMapper.HasValue());
        }
    }

    // Verifies correct construction of mouse axis mapper objects in the nominal case of valid parameter strings being passed.
    TEST_CASE(MapperParser_MakeMouseAxisMapper_Nominal)
    {
        constexpr struct {
            std::wstring_view params;
            EMouseAxis axis;
            EAxisDirection axisDirection;
        } kMouseAxisMapperTestItems[] = {
            {.params = L" X  ",             .axis = EMouseAxis::X},
            {.params = L" Horizontal,  -",  .axis = EMouseAxis::X,                  .axisDirection = EAxisDirection::Negative},
            {.params = L"WheelVertical, +", .axis = EMouseAxis::WheelVertical,      .axisDirection = EAxisDirection::Positive}
        };

        for (auto& mouseAxisMapperTestItem : kMouseAxisMapperTestItems)
        {
            ElementMapperOrError maybeMouseAxisMapper = MapperParser::MakeMouseAxisMapper(mouseAxisMapperTestItem.params);

            TEST_ASSERT(true == maybeMouseAxisMapper.HasValue());
            TEST_ASSERT(0 == maybeMouseAxisMapper.Value()->GetTargetElementCount());

            const MouseAxisMapper* const mouseAxisMapper = dynamic_cast<MouseAxisMapper*>(maybeMouseAxisMapper.Value().get());
            TEST_ASSERT(nullptr != mouseAxisMapper);

            TEST_ASSERT(mouseAxisMapperTestItem.axis == mouseAxisMapper->GetAxis());
            TEST_ASSERT(mouseAxisMapperTestItem.axisDirection == mouseAxisMapper->GetAxisDirection());
        }
    }

    // Verifies correct failure to create mouse button mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeMouseAxisMapper_Invalid)
    {
        const std::wstring_view kMouseAxisMapperTestStrings[] = { L"B", L"5", L"x, anydir", L"wheelX, +, morestuff" };

        for (auto& mouseAxisMapperTestString : kMouseAxisMapperTestStrings)
        {
            ElementMapperOrError maybeMouseAxisMapper = MapperParser::MakeMouseAxisMapper(mouseAxisMapperTestString);
            TEST_ASSERT(false == maybeMouseAxisMapper.HasValue());
        }
    }

    // Verifies correct construction of mouse button mapper objects in the nominal case of valid parameter strings being passed.
    TEST_CASE(MapperParser_MakeMouseButtonMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, EMouseButton> kMouseButtonMapperTestItems[] = {
            {L"left",           EMouseButton::Left},
            {L"Left",           EMouseButton::Left},
            {L"LeftButton",     EMouseButton::Left},
            {L"middle",         EMouseButton::Middle},
            {L"Middle",         EMouseButton::Middle},
            {L"Wheel",          EMouseButton::Middle},
            {L"right",          EMouseButton::Right},
            {L"Right",          EMouseButton::Right},
            {L"RightButton",    EMouseButton::Right},
            {L"X1",             EMouseButton::X1},
            {L"Back",           EMouseButton::X1},
            {L"X2",             EMouseButton::X2},
            {L"Forward",        EMouseButton::X2}

        };

        for (auto& mouseButtonMapperTestItem : kMouseButtonMapperTestItems)
        {
            ElementMapperOrError maybeMouseButtonMapper = MapperParser::MakeMouseButtonMapper(mouseButtonMapperTestItem.first);

            TEST_ASSERT(true == maybeMouseButtonMapper.HasValue());
            TEST_ASSERT(0 == maybeMouseButtonMapper.Value()->GetTargetElementCount());

            TEST_ASSERT(nullptr != dynamic_cast<MouseButtonMapper*>(maybeMouseButtonMapper.Value().get()));

            const EMouseButton kExpectedTargetMouseButton = mouseButtonMapperTestItem.second;
            const EMouseButton kActualTargetMouseButton = dynamic_cast<MouseButtonMapper*>(maybeMouseButtonMapper.Value().get())->GetMouseButton();
            TEST_ASSERT(kActualTargetMouseButton == kExpectedTargetMouseButton);
        }
    }

    // Verifies correct failure to create mouse button mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeMouseButtonMapper_Invalid)
    {
        const std::wstring_view kMouseButtonMapperTestStrings[] = {L"invalid", L"", L" "};

        for (auto& mouseButtonMapperTestString : kMouseButtonMapperTestStrings)
        {
            ElementMapperOrError maybeMouseButtonMapper = MapperParser::MakeKeyboardMapper(mouseButtonMapperTestString);
            TEST_ASSERT(false == maybeMouseButtonMapper.HasValue());
        }
    }

    // Verifies correct construction of null mappers in the nominal case of empty parameter strings being passed.
    TEST_CASE(MapperParser_MakeNullMapper_Nominal)
    {
        TEST_ASSERT(nullptr == MapperParser::MakeNullMapper(L""));
    }

    // Verifies correct failure to create null mappers when the parameter strings are non-empty.
    TEST_CASE(MapperParser_MakeNullMapper_Invalid)
    {
        const std::wstring_view kNullMapperTestStrings[] = {L"0", L"A", L"1,+", L"A, B"};

        for (auto& nullMapperTestString : kNullMapperTestStrings)
        {
            ElementMapperOrError maybeNullMapper = MapperParser::MakeNullMapper(nullMapperTestString);
            TEST_ASSERT(false == maybeNullMapper.HasValue());
        }
    }

    // Verifies correct construction of POV mapper objects in the nominal case of valid parameter strings being passed.
    // This test does not check POV directon, just target virtual controller element.
    TEST_CASE(MapperParser_MakePovMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, SElementIdentifier> kPovMapperTestItems[] = {
            {L"UP",         {.type = EElementType::Pov}},
            {L"Dn",         {.type = EElementType::Pov}},
            {L"Left",       {.type = EElementType::Pov}}
        };

        for (auto& povMapperTestItem : kPovMapperTestItems)
        {
            ElementMapperOrError maybePovMapper = MapperParser::MakePovMapper(povMapperTestItem.first);

            TEST_ASSERT(true == maybePovMapper.HasValue());
            TEST_ASSERT(1 == maybePovMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(povMapperTestItem.second == maybePovMapper.Value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create axis mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakePovMapper_Invalid)
    {
        const std::wstring_view kPovMapperTestStrings[] = {L"Up, Left, Right", L"WhoKnows", L",", L""};

        for (auto& povMapperTestString : kPovMapperTestStrings)
        {
            ElementMapperOrError maybePovMapper = MapperParser::MakeAxisMapper(povMapperTestString);
            TEST_ASSERT(false == maybePovMapper.HasValue());
        }
    }

    // Verifies correct construction of split mapper objects in the nominal case of using very simple non-null inner element mappers represented by valid strings.
    TEST_CASE(MapperParser_MakeSplitMapper_Nominal)
    {
        constexpr std::wstring_view kSplitMapperTestStrings[] = {
            L"Axis(X), Axis(RotX)",
            L" Pov(  Up  )  ,   Button  ( 10   ) ",
            L"Axis(RotX, +), Pov(Down)"
        };
        constexpr std::pair<SElementIdentifier, SElementIdentifier> kExpectedElements[] = {
            {{.type = EElementType::Axis,   .axis = EAxis::X},      {.type = EElementType::Axis,    .axis = EAxis::RotX}},
            {{.type = EElementType::Pov},                           {.type = EElementType::Button,  .button = EButton::B10}},
            {{.type = EElementType::Axis,   .axis = EAxis::RotX},   {.type = EElementType::Pov}}
        };
        static_assert(_countof(kExpectedElements) == _countof(kSplitMapperTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kSplitMapperTestStrings); ++i)
        {
            ElementMapperOrError maybeSplitMapper = MapperParser::MakeSplitMapper(kSplitMapperTestStrings[i]);

            TEST_ASSERT(true == maybeSplitMapper.HasValue());
            TEST_ASSERT(2 == maybeSplitMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(kExpectedElements[i].first == maybeSplitMapper.Value()->GetTargetElementAt(0));
            TEST_ASSERT(kExpectedElements[i].second == maybeSplitMapper.Value()->GetTargetElementAt(1));
        }
    }

    // Verifies correct failure to create split mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeSplitMapper_Invalid)
    {
        constexpr std::wstring_view kSplitMapperTestStrings[] = {
            L"Axis(X), Axis(RotX), Axis(Z)",
            L"Button(10)",
            L"Null, Null, Null, Null, Null"
        };

        for (auto& splitMapperTestString : kSplitMapperTestStrings)
        {
            ElementMapperOrError maybeSplitMapper = MapperParser::MakeSplitMapper(splitMapperTestString);
            TEST_ASSERT(false == maybeSplitMapper.HasValue());
        }
    }

    // Verifies correct construction of split mapper objects when both inner mappers are represented by valid strings but one is null.
    TEST_CASE(MapperParser_MakeSplitMapper_OneNull)
    {
        constexpr std::pair<std::wstring_view, SElementIdentifier> kSplitMapperTestItems[] = {
            {L"Axis(X), Null",          {.type = EElementType::Axis, .axis = EAxis::X}},
            {L"Null, Axis(X)",          {.type = EElementType::Axis, .axis = EAxis::X}},
            {L"Button(3), Null",        {.type = EElementType::Button, .button = EButton::B3}},
            {L"Null, Button(3)",        {.type = EElementType::Button, .button = EButton::B3}},
            {L"POV(Left), Null",        {.type = EElementType::Pov}},
            {L"Null, POV(Right)",       {.type = EElementType::Pov}}
        };

        for (auto& splitMapperTestItem : kSplitMapperTestItems)
        {
            ElementMapperOrError maybeSplitMapper = MapperParser::MakeSplitMapper(splitMapperTestItem.first);

            TEST_ASSERT(true == maybeSplitMapper.HasValue());
            TEST_ASSERT(1 == maybeSplitMapper.Value()->GetTargetElementCount());
            TEST_ASSERT(splitMapperTestItem.second == maybeSplitMapper.Value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct parsing of single axis element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Axis)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Axis(X)",
            L"Axis(Y, Both)",
            L"Axis(Z, +)",
            L"Axis(RX, negative), Button(3)"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::X)},
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::Y, EAxisDirection::Both)},
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::Z, EAxisDirection::Positive)},
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::RotX, EAxisDirection::Negative), .remainingString = L"Button(3)"},
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<AxisMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const EAxisDirection kExpectedDirection = dynamic_cast<AxisMapper*>(kExpectedParseResults[i].maybeElementMapper.Value().get())->GetAxisDirection();
            const EAxisDirection kActualDirection = dynamic_cast<AxisMapper*>(actualParseResult.maybeElementMapper.Value().get())->GetAxisDirection();
            TEST_ASSERT(kActualDirection == kExpectedDirection);
        }
    }

    // Verifies correct parsing of single button element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Button)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Button(10)",
            L"  Button   (    10   )  ",
            L"Button(1), Button(3)"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<ButtonMapper>(EButton::B10)},
            {.maybeElementMapper = std::make_unique<ButtonMapper>(EButton::B10)},
            {.maybeElementMapper = std::make_unique<ButtonMapper>(EButton::B1),     .remainingString = L"Button(3)"}
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);
        }
    }

    // Verifies correct parsing of single compound element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Compound)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Compound(Null, Null, Null, Null)",
            L"Compound(Button(10), Button(11))",
            L"Compound( Invert(Axis(X)), Pov(Up) )",
            L"Compound( Button(1), Button(2), Button(3), Button(4), Button(5), Button(6), Button(7), Button(8) )",
            L"Compound( Button(1), Button(2), Button(3), Button(4), Keyboard(5), Keyboard(6), Keyboard(7), Keyboard(8) )"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<CompoundMapper>(CompoundMapper::TElementMappers())},
            {.maybeElementMapper = std::make_unique<CompoundMapper>(CompoundMapper::TElementMappers({
                std::make_unique<ButtonMapper>(EButton::B10),
                std::make_unique<ButtonMapper>(EButton::B11)}))},
            {.maybeElementMapper = std::make_unique<CompoundMapper>(CompoundMapper::TElementMappers({
                std::make_unique<InvertMapper>(
                    std::make_unique<AxisMapper>(EAxis::X)),
                std::make_unique<PovMapper>(EPovDirection::Up)}))},
            {.maybeElementMapper = std::make_unique<CompoundMapper>(CompoundMapper::TElementMappers({
                std::make_unique<ButtonMapper>(EButton::B1),
                std::make_unique<ButtonMapper>(EButton::B2),
                std::make_unique<ButtonMapper>(EButton::B3),
                std::make_unique<ButtonMapper>(EButton::B4),
                std::make_unique<ButtonMapper>(EButton::B5),
                std::make_unique<ButtonMapper>(EButton::B6),
                std::make_unique<ButtonMapper>(EButton::B7),
                std::make_unique<ButtonMapper>(EButton::B8)}))},
            {.maybeElementMapper = std::make_unique<CompoundMapper>(CompoundMapper::TElementMappers({
                std::make_unique<ButtonMapper>(EButton::B1),
                std::make_unique<ButtonMapper>(EButton::B2),
                std::make_unique<ButtonMapper>(EButton::B3),
                std::make_unique<ButtonMapper>(EButton::B4),
                std::make_unique<KeyboardMapper>(DIK_5),
                std::make_unique<KeyboardMapper>(DIK_6),
                std::make_unique<KeyboardMapper>(DIK_7),
                std::make_unique<KeyboardMapper>(DIK_8)}))}
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<CompoundMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const CompoundMapper::TElementMappers& kExpectedElementMappers = dynamic_cast<CompoundMapper*>(kExpectedParseResults[i].maybeElementMapper.Value().get())->GetElementMappers();
            const CompoundMapper::TElementMappers& kActualElementMappers = dynamic_cast<CompoundMapper*>(actualParseResult.maybeElementMapper.Value().get())->GetElementMappers();

            for (size_t j = 0; j < kExpectedElementMappers.size(); ++j)
                VerifyElementMapperPointersAreEquivalent(kActualElementMappers[j].get(), kExpectedElementMappers[j].get());
        }
    }

    // Verifies correct parsing of single digital axis element mappers from a valid supplied input string.
    // Same as the axis mapper test but with a different target type.
    TEST_CASE(MapperParser_ParseSingleElementMapper_DigitalAxis)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"DigitalAxis(X)",
            L"DigitalAxis(Y, Both)",
            L"DigitalAxis(Z, +)",
            L"DigitalAxis(RX, negative), Button(3)"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::X)},
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::Y, EAxisDirection::Both)},
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::Z, EAxisDirection::Positive)},
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::RotX, EAxisDirection::Negative), .remainingString = L"Button(3)"},
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<DigitalAxisMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const EAxisDirection kExpectedDirection = dynamic_cast<DigitalAxisMapper*>(kExpectedParseResults[i].maybeElementMapper.Value().get())->GetAxisDirection();
            const EAxisDirection kActualDirection = dynamic_cast<DigitalAxisMapper*>(actualParseResult.maybeElementMapper.Value().get())->GetAxisDirection();
            TEST_ASSERT(kActualDirection == kExpectedDirection);
        }
    }

    // Verifies correct parsing of single invert element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Invert)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Invert(Axis(X))",
            L"Invert(Pov(Right))",
            L"Invert( Split(Button(10), Button(12)) )"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<InvertMapper>(std::make_unique<AxisMapper>(EAxis::X))},
            {.maybeElementMapper = std::make_unique<InvertMapper>(std::make_unique<PovMapper>(EPovDirection::Right))},
            {.maybeElementMapper = std::make_unique<InvertMapper>(std::make_unique<SplitMapper>(
                std::make_unique<ButtonMapper>(EButton::B10),
                std::make_unique<ButtonMapper>(EButton::B12)))}
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<InvertMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const IElementMapper* kExpectedElementMapper = kExpectedParseResults[i].maybeElementMapper.Value().get();
            const IElementMapper* kActualElementMapper = dynamic_cast<InvertMapper*>(actualParseResult.maybeElementMapper.Value().get())->GetElementMapper();
            VerifyElementMapperPointersAreEquivalent(kActualElementMapper, kExpectedElementMapper);
        }
    }

    // Verifies correct parsing of single keyboard element mappers from a valid supplied input string.
    // Exercises different scancode representations.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Keyboard)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Keyboard(10)",
            L"Keyboard(0xa)",
            L"Keyboard( 0XA )",
            L"Keyboard(UpArrow)",
            L"Keyboard(DIK_ESCAPE)",
            L"Keyboard(  A  )",
            L"Keyboard( 3  )",
            L"Keyboard(0)",
            L"Keyboard(0x0)",
            L"Keyboard(0x3)",
            L"Keyboard(00)",
            L"Keyboard(03)",
            L"Keyboard(012), Button(3)"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(10)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(10)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(10)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(DIK_UPARROW)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(DIK_ESCAPE)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(DIK_A)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(DIK_3)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(DIK_0)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(0)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(3)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(0)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(3)},
            {.maybeElementMapper = std::make_unique<KeyboardMapper>(10),            .remainingString = L"Button(3)"}
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<KeyboardMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const TKeyIdentifier kExpectedTargetKey = dynamic_cast<KeyboardMapper*>(kExpectedParseResults[i].maybeElementMapper.Value().get())->GetTargetKey();
            const TKeyIdentifier kActualTargetKey = dynamic_cast<KeyboardMapper*>(actualParseResult.maybeElementMapper.Value().get())->GetTargetKey();
            TEST_ASSERT(kActualTargetKey == kExpectedTargetKey);
        }
    }

    // Verifies correct parsing of single POV element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Pov)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"PovHat(Up)",
            L"Pov(Left)",
            L"POV(Dn)"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<PovMapper>(EPovDirection::Up)},
            {.maybeElementMapper = std::make_unique<PovMapper>(EPovDirection::Left)},
            {.maybeElementMapper = std::make_unique<PovMapper>(EPovDirection::Down)},
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<PovMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const EPovDirection kExpectedDirection = dynamic_cast<PovMapper*>(kExpectedParseResults[i].maybeElementMapper.Value().get())->GetDirection();
            const EPovDirection kActualDirection = dynamic_cast<PovMapper*>(actualParseResult.maybeElementMapper.Value().get())->GetDirection();
            TEST_ASSERT(kActualDirection == kExpectedDirection);
        }
    }

    // Verifies correct parsing of single null element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Null)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Null",
            L"  Null  ",
            L"  Null  , Null  , Button(2) "
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = nullptr},
            {.maybeElementMapper = nullptr},
            {.maybeElementMapper = nullptr,     .remainingString = L"Null  , Button(2)"}
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);
        }
    }

    // Verifies correct parsing of single split element mappers of varying complexity from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Split)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Split(    Axis(x),  Axis(y)    )",
            L"Split(    Split(  Button(1), Button(2)  ), Split(  Button(3), Button(4)  )    )",
            L"Split(    Split(  Null,      Button(2)  ), Null    )",
            L"Split( Null, Null )"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<SplitMapper>(
                std::make_unique<AxisMapper>(EAxis::X),
                std::make_unique<AxisMapper>(EAxis::Y))},
            {.maybeElementMapper = std::make_unique<SplitMapper>(
                std::make_unique<SplitMapper>(
                    std::make_unique<ButtonMapper>(EButton::B1),
                    std::make_unique<ButtonMapper>(EButton::B2)),
                std::make_unique<SplitMapper>(
                    std::make_unique<ButtonMapper>(EButton::B3),
                    std::make_unique<ButtonMapper>(EButton::B4)))},
            {.maybeElementMapper = std::make_unique<SplitMapper>(
                std::make_unique<SplitMapper>(
                    nullptr,
                    std::make_unique<ButtonMapper>(EButton::B2)),
                nullptr)},
            {.maybeElementMapper = std::make_unique<SplitMapper>(
                nullptr,
                nullptr)}
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<SplitMapper*>(actualParseResult.maybeElementMapper.Value().get()));

            const SplitMapper* const kExpectedSplitMapper = dynamic_cast<SplitMapper*>(kExpectedParseResults[i].maybeElementMapper.Value().get());
            const SplitMapper* const kActualSplitMapper = dynamic_cast<SplitMapper*>(actualParseResult.maybeElementMapper.Value().get());
            VerifySplitMapperPointersAreEquivalent(kActualSplitMapper, kExpectedSplitMapper);
        }
    }

    // Verifies failure to parse a single element mapper from an invalid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Invalid)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L" UnknownMapperType ",
            L" Axis(R)",
            L"  Axis(X, +-)",
            L"  Button(4) ) ",
            L"  Button(4) , ",
            L"Button(4,5)",
            L"Compound",
            L"Compound(Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null, Null)"
            L" DigitalAxis(U)",
            L"  DigitalAxis(z, -+)",
            L"  Invert(asdf) ",
            L"Keyboard(1000)",
            L"Keyboard(10,11)",
            L"Keyboard(0x a)",
            L"  Null , ",
            L"Null()",
            L"Null(   )",
            L"Null   (   )  ",
            L"Null(      ",
            L"Pov(Up, Left, Right)",
            L"Pov(AnyDir)"
        };
        const SElementMapperParseResult kExpectedParseResult = {.maybeElementMapper = L"Test error message."};

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResult);
        }
    }

    // Verifies successful parsing of element mapper strings to element mapper objects.
    // Only exercises simple cases in which element mappers are not nested within one another.
    TEST_CASE(MapperParser_ElementMapperFromString_Simple)
    {
        constexpr std::wstring_view kTestStrings[] = {
           L"Null",
           L"  Null  ",
           L"Button(3)",
           L"  Button   (    5    )  "
        };
        const std::unique_ptr<IElementMapper> kExpectedElementMappers[] = {
            nullptr,
            nullptr,
            std::make_unique<ButtonMapper>(EButton::B3),
            std::make_unique<ButtonMapper>(EButton::B5)
        };
        static_assert(_countof(kExpectedElementMappers) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            const ElementMapperOrError kMaybeActualElementMapper = MapperParser::ElementMapperFromString(kTestStrings[i]);
            TEST_ASSERT(true == kMaybeActualElementMapper.HasValue());
            VerifyElementMapperPointersAreEquivalent(kMaybeActualElementMapper.Value().get(), kExpectedElementMappers[i].get());
        }
    }

    // Verifies successful parsing of element mapper strings to element mapper objects.
    // Exercises more complex cases in which element mappers are nested within one another.
    TEST_CASE(MapperParser_ElementMapperFromString_Nested)
    {
        constexpr std::wstring_view kTestStrings[] = {
           L"Split(Null, Null)",
           L"Split(Split(Null, Null), Split(Button(3), Button(4)))"
        };
        const std::unique_ptr<IElementMapper> kExpectedElementMappers[] = {
            std::make_unique<SplitMapper>(
                nullptr,
                nullptr),
            std::make_unique<SplitMapper>(
                std::make_unique<SplitMapper>(
                    nullptr,
                    nullptr),
                std::make_unique<SplitMapper>(
                    std::make_unique<ButtonMapper>(EButton::B3),
                    std::make_unique<ButtonMapper>(EButton::B4)))
        };
        static_assert(_countof(kExpectedElementMappers) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            const ElementMapperOrError kMaybeActualElementMapper = MapperParser::ElementMapperFromString(kTestStrings[i]);
            TEST_ASSERT(true == kMaybeActualElementMapper.HasValue());
            VerifyElementMapperPointersAreEquivalent(kMaybeActualElementMapper.Value().get(), kExpectedElementMappers[i].get());
        }
    }

    // Verifies failure to parse element mapper strings that are invalid.
    TEST_CASE(MapperParser_ElementMapperFromString_Invalid)
    {
        constexpr std::wstring_view kTestStrings[] = {
           L"Null, Null",
           L"  UnknownMapperType  ",
           L"Button(3), Button(4)",
           L"Button((8))"
        };

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            const ElementMapperOrError kMaybeActualElementMapper = MapperParser::ElementMapperFromString(kTestStrings[i]);
            TEST_ASSERT(true == kMaybeActualElementMapper.HasError());
        }
    }

    // Verifies failure to parse element mapper strings that are syntactically correct but have recursion that goes too deep.
    TEST_CASE(MapperParser_ElementMapperFromString_RecursionTooDeep)
    {
        constexpr std::wstring_view kTestStrings[] = {
           L"Split(   Split(   Split(   Split(Split(Split(Null, Null), Null), Null), Null    ), Null), Null)"
        };

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            const ElementMapperOrError kMaybeActualElementMapper = MapperParser::ElementMapperFromString(kTestStrings[i]);
            TEST_ASSERT(true == kMaybeActualElementMapper.HasError());
        }
    }

    // Verifies successful parsing of force feedback actuator strings to force feedback actuator description objects.
    TEST_CASE(MapperParser_ForceFeedbackActuatorFromString_Valid)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"Default",
            L"Disabled",
            L"SingleAxis(RotX)",
            L"SingleAxis(RotZ, -)",
            L"MagnitudeProjection(Y, RotY)",
            L"MagnitudeProjection(RotY, Y)"
        };
        constexpr SActuatorElement kExpectedForceFeedbackActuators[] = {
            Mapper::kDefaultForceFeedbackActuator,
            {
                .isPresent = false
            },
            {
                .isPresent = true,
                .mode = EActuatorMode::SingleAxis,
                .singleAxis = {
                    .axis = EAxis::RotX,
                    .direction = EAxisDirection::Both
                }
            },
            {
                .isPresent = true,
                .mode = EActuatorMode::SingleAxis,
                .singleAxis = {
                    .axis = EAxis::RotZ,
                    .direction = EAxisDirection::Negative
                }
            },
            {
                .isPresent = true,
                .mode = EActuatorMode::MagnitudeProjection,
                .magnitudeProjection = {
                    .axisFirst = EAxis::Y,
                    .axisSecond = EAxis::RotY
                }
            },
            {
                .isPresent = true,
                .mode = EActuatorMode::MagnitudeProjection,
                .magnitudeProjection = {
                    .axisFirst = EAxis::RotY,
                    .axisSecond = EAxis::Y
                }
            }
        };
        static_assert(_countof(kExpectedForceFeedbackActuators) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            const ForceFeedbackActuatorOrError kMaybeActualForceFeedbackActuator = MapperParser::ForceFeedbackActuatorFromString(kTestStrings[i]);
            TEST_ASSERT(true == kMaybeActualForceFeedbackActuator.HasValue());

            const SActuatorElement kActualForceFeedbackActuator = kMaybeActualForceFeedbackActuator.Value();
            TEST_ASSERT(kActualForceFeedbackActuator == kExpectedForceFeedbackActuators[i]);
        }
    }

    // Verifies failure to parse force feedback actuator strings that are invalid.
    TEST_CASE(MapperParser_ForceFeedbackActuatorFromString_Invalid)
    {
        constexpr std::wstring_view kTestStrings[] = {
           L"Default(X)",
           L"  UnknownActuatorType  ",
           L"Disabled, Disabled",
           L"SingleAxis(X), SingleAxis(Y)",
           L"MagnitudeProjection(X, Y, Z)",
           L"MagnitudeProjection(X, X)",
           L"SingleAxis(X, +, -)"
        };

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            const ForceFeedbackActuatorOrError kMaybeActualForceFeedbackActuator = MapperParser::ForceFeedbackActuatorFromString(kTestStrings[i]);
            TEST_ASSERT(false == kMaybeActualForceFeedbackActuator.HasValue());
        }
    }
}
