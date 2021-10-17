/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperParserTest.cpp
 *   Unit tests for run-time mapper object parsing functionality.
 *****************************************************************************/

#include "KeyboardTypes.h"
#include "MapperParser.h"
#include "TestCase.h"

#include <memory>
#include <optional>
#include <string_view>
#include <utility>


namespace XidiTest
{
    using namespace ::Xidi::Controller;
    using ::Xidi::Controller::MapperParser::SElementMapperStringParts;
    using ::Xidi::Controller::MapperParser::SElementMapperParseResult;
    using ::Xidi::Controller::MapperParser::SParamStringParts;
    using ::Xidi::Keyboard::TKeyIdentifier;


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Checks if the supplied element mapper pointers are equivalent and flags a test failure if not.
    /// Only works for simple element mappers that uniquely target zero or one specific controller elements and have no side effects.
    /// @param [in] elementMapperA First of the two element mapper pointers to compare.
    /// @param [in] elementMapperB Second of the two element mapper pointers to compare.
    static void VerifyElementMapperPointersAreEquivalent(const std::unique_ptr<IElementMapper>& elementMapperA, const std::unique_ptr<IElementMapper>& elementMapperB)
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
        TEST_ASSERT(resultA.maybeElementMapper.has_value() == resultB.maybeElementMapper.has_value());
        TEST_ASSERT(resultA.remainingString == resultB.remainingString);

        if (resultA.maybeElementMapper.has_value())
            VerifyElementMapperPointersAreEquivalent(resultA.maybeElementMapper.value(), resultB.maybeElementMapper.value());
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
        constexpr std::pair<std::wstring_view, SElementMapperStringParts> kExtractPartsTestItems[] = {
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
        constexpr std::pair<std::wstring_view, SElementMapperStringParts> kExtractPartsTestItems[] = {
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
        constexpr std::pair<std::wstring_view, SElementMapperStringParts> kExtractPartsTestItems[] = {
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
            std::optional<std::unique_ptr<IElementMapper>> maybeAxisMapper = MapperParser::MakeAxisMapper(axisMapperTestItem.first);

            TEST_ASSERT(true == maybeAxisMapper.has_value());
            TEST_ASSERT(1 == maybeAxisMapper.value()->GetTargetElementCount());
            TEST_ASSERT(axisMapperTestItem.second == maybeAxisMapper.value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create axis mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeAxisMapper_Invalid)
    {
        const std::wstring_view kAxisMapperTestStrings[] = {L"A", L"3", L"x, anydir", L"rotz, +, morestuff"};

        for (auto& axisMapperTestString : kAxisMapperTestStrings)
        {
            std::optional<std::unique_ptr<IElementMapper>> maybeAxisMapper = MapperParser::MakeAxisMapper(axisMapperTestString);
            TEST_ASSERT(false == maybeAxisMapper.has_value());
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
            std::optional<std::unique_ptr<IElementMapper>> maybeButtonMapper = MapperParser::MakeButtonMapper(buttonMapperTestItem.first);

            TEST_ASSERT(true == maybeButtonMapper.has_value());
            TEST_ASSERT(1 == maybeButtonMapper.value()->GetTargetElementCount());
            TEST_ASSERT(buttonMapperTestItem.second == maybeButtonMapper.value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create button mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeButtonMapper_Invalid)
    {
        const std::wstring_view kButtonMapperTestStrings[] = {L"0", L"B1", L"1B", L"asdf", L""};

        for (auto& buttonMapperTestString : kButtonMapperTestStrings)
        {
            std::optional<std::unique_ptr<IElementMapper>> maybeButtonMapper = MapperParser::MakeButtonMapper(buttonMapperTestString);
            TEST_ASSERT(false == maybeButtonMapper.has_value());
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
            std::optional<std::unique_ptr<IElementMapper>> maybeDigitalAxisMapper = MapperParser::MakeDigitalAxisMapper(digitalAxisMapperTestItem.first);

            TEST_ASSERT(true == maybeDigitalAxisMapper.has_value());
            TEST_ASSERT(1 == maybeDigitalAxisMapper.value()->GetTargetElementCount());
            TEST_ASSERT(digitalAxisMapperTestItem.second == maybeDigitalAxisMapper.value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create digital axis mapper objects when the parameter strings are invalid.
    // Same as the corresponding axis mapper test but with a different target type.
    TEST_CASE(MapperParser_MakeDigitalAxisMapper_Invalid)
    {
        const std::wstring_view kDigitalAxisMapperTestStrings[] = { L"A", L"3", L"x, anydir", L"rotz, +, morestuff" };

        for (auto& digitalAxisMapperTestString : kDigitalAxisMapperTestStrings)
        {
            std::optional<std::unique_ptr<IElementMapper>> maybeDigitalAxisMapper = MapperParser::MakeDigitalAxisMapper(digitalAxisMapperTestString);
            TEST_ASSERT(false == maybeDigitalAxisMapper.has_value());
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
            std::optional<std::unique_ptr<IElementMapper>> maybeKeyboardMapper = MapperParser::MakeKeyboardMapper(keyboardMapperTestItem.first);

            TEST_ASSERT(true == maybeKeyboardMapper.has_value());
            TEST_ASSERT(0 == maybeKeyboardMapper.value()->GetTargetElementCount());

            TEST_ASSERT(nullptr != dynamic_cast<KeyboardMapper*>(maybeKeyboardMapper.value().get()));

            const TKeyIdentifier kExpectedTargetKey = keyboardMapperTestItem.second;
            const TKeyIdentifier kActualTargetKey = dynamic_cast<KeyboardMapper*>(maybeKeyboardMapper.value().get())->GetTargetKey();
            TEST_ASSERT(kActualTargetKey == kExpectedTargetKey);
        }
    }

    // Verifies correct failure to create button mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakeKeyboardMapper_Invalid)
    {
        const std::wstring_view kKeyboardMapperTestStrings[] = {L"256", L"0x101", L"DIK_INVALID", L"Invalid", L""};

        for (auto& keyboardMapperTestString : kKeyboardMapperTestStrings)
        {
            std::optional<std::unique_ptr<IElementMapper>> maybeKeyboardMapper = MapperParser::MakeKeyboardMapper(keyboardMapperTestString);
            TEST_ASSERT(false == maybeKeyboardMapper.has_value());
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
            std::optional<std::unique_ptr<IElementMapper>> maybeNullMapper = MapperParser::MakeNullMapper(nullMapperTestString);
            TEST_ASSERT(false == maybeNullMapper.has_value());
        }
    }

    // Verifies correct construction of POV mapper objects in the nominal case of valid parameter strings being passed.
    // This test does not check POV directon, just target virtual controller element.
    TEST_CASE(MapperParser_MakePovMapper_Nominal)
    {
        constexpr std::pair<std::wstring_view, SElementIdentifier> kPovMapperTestItems[] = {
            {L"UP",         {.type = EElementType::Pov}},
            {L"Dn",         {.type = EElementType::Pov}},
            {L"Down, Up",   {.type = EElementType::Pov}},
            {L"Left",       {.type = EElementType::Pov}},
            {L"r, r",       {.type = EElementType::Pov}},
        };

        for (auto& povMapperTestItem : kPovMapperTestItems)
        {
            std::optional<std::unique_ptr<IElementMapper>> maybePovMapper = MapperParser::MakePovMapper(povMapperTestItem.first);

            TEST_ASSERT(true == maybePovMapper.has_value());
            TEST_ASSERT(1 == maybePovMapper.value()->GetTargetElementCount());
            TEST_ASSERT(povMapperTestItem.second == maybePovMapper.value()->GetTargetElementAt(0));
        }
    }

    // Verifies correct failure to create axis mapper objects when the parameter strings are invalid.
    TEST_CASE(MapperParser_MakePovMapper_Invalid)
    {
        const std::wstring_view kPovMapperTestStrings[] = {L"Up, Left, Right", L"WhoKnows", L",", L""};

        for (auto& povMapperTestString : kPovMapperTestStrings)
        {
            std::optional<std::unique_ptr<IElementMapper>> maybePovMapper = MapperParser::MakeAxisMapper(povMapperTestString);
            TEST_ASSERT(false == maybePovMapper.has_value());
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
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::Y, AxisMapper::EDirection::Both)},
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::Z, AxisMapper::EDirection::Positive)},
            {.maybeElementMapper = std::make_unique<AxisMapper>(EAxis::RotX, AxisMapper::EDirection::Negative), .remainingString = L"Button(3)"},
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<AxisMapper*>(actualParseResult.maybeElementMapper.value().get()));

            const AxisMapper::EDirection kExpectedDirection = dynamic_cast<AxisMapper*>(kExpectedParseResults[i].maybeElementMapper.value().get())->GetAxisDirection();
            const AxisMapper::EDirection kActualDirection = dynamic_cast<AxisMapper*>(actualParseResult.maybeElementMapper.value().get())->GetAxisDirection();
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
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::Y, AxisMapper::EDirection::Both)},
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::Z, AxisMapper::EDirection::Positive)},
            {.maybeElementMapper = std::make_unique<DigitalAxisMapper>(EAxis::RotX, AxisMapper::EDirection::Negative), .remainingString = L"Button(3)"},
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<DigitalAxisMapper*>(actualParseResult.maybeElementMapper.value().get()));

            const AxisMapper::EDirection kExpectedDirection = dynamic_cast<DigitalAxisMapper*>(kExpectedParseResults[i].maybeElementMapper.value().get())->GetAxisDirection();
            const AxisMapper::EDirection kActualDirection = dynamic_cast<DigitalAxisMapper*>(actualParseResult.maybeElementMapper.value().get())->GetAxisDirection();
            TEST_ASSERT(kActualDirection == kExpectedDirection);
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

            TEST_ASSERT(nullptr != dynamic_cast<KeyboardMapper*>(actualParseResult.maybeElementMapper.value().get()));

            const TKeyIdentifier kExpectedTargetKey = dynamic_cast<KeyboardMapper*>(kExpectedParseResults[i].maybeElementMapper.value().get())->GetTargetKey();
            const TKeyIdentifier kActualTargetKey = dynamic_cast<KeyboardMapper*>(actualParseResult.maybeElementMapper.value().get())->GetTargetKey();
            TEST_ASSERT(kActualTargetKey == kExpectedTargetKey);
        }
    }

    // Verifies correct parsing of single POV element mappers from a valid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Pov)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L"PovHat(Up)",
            L"Pov(Left, Right)",
            L"POV(Dn, Down)"
        };
        const SElementMapperParseResult kExpectedParseResults[] = {
            {.maybeElementMapper = std::make_unique<PovMapper>(EPovDirection::Up)},
            {.maybeElementMapper = std::make_unique<PovMapper>(EPovDirection::Left, EPovDirection::Right)},
            {.maybeElementMapper = std::make_unique<PovMapper>(EPovDirection::Down, EPovDirection::Down)},
        };
        static_assert(_countof(kExpectedParseResults) == _countof(kTestStrings), "Mismatch between input and expected output array lengths.");

        for (int i = 0; i < _countof(kTestStrings); ++i)
        {
            SElementMapperParseResult actualParseResult = MapperParser::ParseSingleElementMapper(kTestStrings[i]);
            VerifyParseResultsAreEquivalent(actualParseResult, kExpectedParseResults[i]);

            TEST_ASSERT(nullptr != dynamic_cast<PovMapper*>(actualParseResult.maybeElementMapper.value().get()));

            const EPovDirection kExpectedDirectionPositive = dynamic_cast<PovMapper*>(kExpectedParseResults[i].maybeElementMapper.value().get())->GetPositiveDirection();
            const EPovDirection kActualDirectionPositive = dynamic_cast<PovMapper*>(actualParseResult.maybeElementMapper.value().get())->GetPositiveDirection();
            TEST_ASSERT(kActualDirectionPositive == kExpectedDirectionPositive);

            const std::optional<EPovDirection> kExpectedDirectionNegative = dynamic_cast<PovMapper*>(kExpectedParseResults[i].maybeElementMapper.value().get())->GetNegativeDirection();
            const std::optional<EPovDirection> kActualDirectionNegative = dynamic_cast<PovMapper*>(actualParseResult.maybeElementMapper.value().get())->GetNegativeDirection();
            TEST_ASSERT(kActualDirectionNegative == kExpectedDirectionNegative);
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


    // Verifies failure to parse a single element mapper from an invalid supplied input string.
    TEST_CASE(MapperParser_ParseSingleElementMapper_Invalid)
    {
        constexpr std::wstring_view kTestStrings[] = {
            L" UnknownMapperType ",
            L" Axis(R)",
            L"  Axis(X, +-)",
            L" DigitalAxis(U)",
            L"  DigitalAxis(z, -+)",
            L"  Button(4) ) ",
            L"  Button(4) , ",
            L"Button(4,5)",
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
        const SElementMapperParseResult kExpectedParseResult = {.maybeElementMapper = std::nullopt};

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
            const std::optional<std::unique_ptr<IElementMapper>> kMaybeActualElementMapper = MapperParser::ElementMapperFromString(kTestStrings[i]);
            TEST_ASSERT(true == kMaybeActualElementMapper.has_value());
            VerifyElementMapperPointersAreEquivalent(kMaybeActualElementMapper.value(), kExpectedElementMappers[i]);
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
            const std::optional<std::unique_ptr<IElementMapper>> kMaybeActualElementMapper = MapperParser::ElementMapperFromString(kTestStrings[i]);
            TEST_ASSERT(false == kMaybeActualElementMapper.has_value());
        }
    }
}
