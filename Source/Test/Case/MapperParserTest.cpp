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
}
