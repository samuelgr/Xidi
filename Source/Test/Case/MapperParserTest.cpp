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

#include "MapperParser.h"
#include "TestCase.h"

#include <map>
#include <optional>
#include <string_view>


namespace XidiTest
{
    using namespace ::Xidi::Controller;
    using ::Xidi::Controller::MapperParser::SElementMapperStringParts;


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies correct identification of valid controller element strings.
    TEST_CASE(MapperParser_ControllerElementString_Valid)
    {
        const std::map<unsigned int, std::wstring_view> kControllerElements = {
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
        const std::map<unsigned int, std::wstring_view> kRecursionTestItems = {
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
    // Exercises several different nominal cases in which the input string is valid and actually contains both of these parts.
    TEST_CASE(MapperParser_ExtractParts_Nominal)
    {
        const std::map<std::wstring_view, SElementMapperStringParts> kExtractPartsTestItems = {
            {L"Axis(Y)",                                {.type = L"Axis",   .params = L"Y"}},
            {L"   Axis       (    Y    ,    + )",       {.type = L"Axis",   .params = L"Y    ,    +"}},
            {L"  Split ( Button(2), Button(3)   )",     {.type = L"Split",  .params = L"Button(2), Button(3)"}},
            {L"   Null  ",                              {.type = L"Null"}},
            {L"  Button( 2 ),  Button ( 3 )  ",         {.type = L"Button", .params = L"2"}},
            {L" Null, Button(3) ",                      {.type = L"Null",   .params = L""}}
        };

        for (auto& extractPartsTestItem : kExtractPartsTestItems)
            TEST_ASSERT(extractPartsTestItem.second == MapperParser::ExtractParts(extractPartsTestItem.first));
    }
}
