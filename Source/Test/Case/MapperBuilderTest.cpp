/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperBuilderTest.cpp
 *   Unit tests for run-time mapper object building functionality.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"
#include "MapperBuilder.h"
#include "TestCase.h"

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- TEST CASES ------------------------------------------------- //

    // Verifies correct identification of valid controller element strings.
    TEST_CASE(MapperBuilder_ControllerElementString_Valid)
    {
        constexpr std::wstring_view kControllerElementStrings[] = {L"StickLeftY", L"DpadDown", L"TriggerLT", L"ButtonRB", L"ButtonStart"};

        for (auto controllerElementString : kControllerElementStrings)
            TEST_ASSERT(true == MapperBuilder::IsControllerElementStringValid(controllerElementString));
    }

    // Verifies correct identification of invalid controller element strings.
    TEST_CASE(MapperBuilder_ControllerElementString_Invalid)
    {
        constexpr std::wstring_view kControllerElementStrings[] = {L"stickLeftY", L"dpadDown", L"random_string"};

        for (auto controllerElementString : kControllerElementStrings)
            TEST_ASSERT(false == MapperBuilder::IsControllerElementStringValid(controllerElementString));
    }

    // Verifies that blueprints can be created and successfully identified.
    TEST_CASE(MapperBuilder_BlueprintName_Nominal)
    {
        constexpr std::wstring_view kMapperNames[] = {L"TestMapper", L"testMapper", L"TestMapper2", L"testMapper2"};

        MapperBuilder builder;

        for (auto mapperName : kMapperNames)
            TEST_ASSERT(false == builder.DoesBlueprintNameExist(mapperName));

        for (auto mapperName : kMapperNames)
            TEST_ASSERT(true == builder.CreateBlueprint(mapperName));

        for (auto mapperName : kMapperNames)
            TEST_ASSERT(true == builder.DoesBlueprintNameExist(mapperName));
    }

    // Verifies that attempts to create blueprints with the same name are rejected.
    TEST_CASE(MapperBuilder_BlueprintName_DuplicatesRejected)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";
        constexpr int kRepeatTimes = 10;

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        for (int i = 0; i < kRepeatTimes; ++i)
        {
            TEST_ASSERT(false == builder.CreateBlueprint(kMapperName));
            TEST_ASSERT(true == builder.DoesBlueprintNameExist(kMapperName));
        }
    }

    // Verifies that attempts to create blueprints with the same name as existing mapper objects are rejected.
    // This test uses the names of known documented mappers.
    TEST_CASE(MapperBuilder_BlueprintName_ExistingMapperNameRejected)
    {
        constexpr std::wstring_view kMapperNames[] = {L"StandardGamepad", L"DigitalGamepad", L"ExtendedGamepad", L"XInputNative", L"XInputSharedTriggers"};

        MapperBuilder builder;

        for (auto mapperName : kMapperNames)
            TEST_ASSERT(false == builder.DoesBlueprintNameExist(mapperName));

        for (auto mapperName : kMapperNames)
            TEST_ASSERT(false == builder.CreateBlueprint(mapperName));
    }

    // Verifies that new blueprints are empty upon creation.
    TEST_CASE(MapperBuilder_CreateBlueprint_Empty)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        const Mapper::UElementMap* const kElementMap = builder.GetBlueprintElementMap(kMapperName);
        TEST_ASSERT(nullptr != kElementMap);
        for (int i = 0; i < _countof(kElementMap->all); ++i)
            TEST_ASSERT(nullptr == kElementMap->all[i]);

        const std::optional<std::wstring_view> kMaybeTemplateName = builder.GetBlueprintTemplate(kMapperName);
        TEST_ASSERT(true == kMaybeTemplateName.has_value());
        TEST_ASSERT(true == kMaybeTemplateName.value().empty());
    }

    // Verifies that element mappers can be set in the nominal case of valid controller elements being specified.
    TEST_CASE(MapperBuilder_ElementMap_Nominal)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";
        constexpr AxisMapper kTestElementMapper(EAxis::X);
        const std::unordered_map<int, std::wstring_view> kControllerElements = {
            {ELEMENT_MAP_INDEX_OF(stickLeftY), L"StickLeftY"},
            {ELEMENT_MAP_INDEX_OF(triggerLT), L"TriggerLT"}
        };

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        for (const auto& controllerElement : kControllerElements)
            TEST_ASSERT(true == builder.SetBlueprintElementMapper(kMapperName, controllerElement.second, kTestElementMapper.Clone()));

        const Mapper::UElementMap* const kElementMap = builder.GetBlueprintElementMap(kMapperName);
        TEST_ASSERT(nullptr != kElementMap);
        for (int i = 0; i < _countof(kElementMap->all); ++i)
        {
            if (kControllerElements.contains(i))
            {
                TEST_ASSERT(nullptr != kElementMap->all[i]);
                TEST_ASSERT(nullptr != dynamic_cast<decltype(kTestElementMapper)*>(kElementMap->all[i].get()));
                TEST_ASSERT(kTestElementMapper.GetTargetElementCount() == kElementMap->all[i]->GetTargetElementCount());

                for (int j = 0; j < kTestElementMapper.GetTargetElementCount(); ++j)
                    TEST_ASSERT(kTestElementMapper.GetTargetElementAt(j) == kElementMap->all[i]->GetTargetElementAt(j));
            }
            else
            {
                TEST_ASSERT(nullptr == kElementMap->all[i]);
            }
        }
    }

    // Verifies that element mappers can be set with some being valid and some being invalid.
    TEST_CASE(MapperBuilder_ElementMap_SomeInvalid)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";
        constexpr AxisMapper kTestElementMapper(EAxis::X);

        // Same as above, but with negative indices to indicate invalid controller elements.
        // The condition for successful insertion uses comparison-with-0 to decide whether to expect success or failure.
        // Similarly, the loop that verifies null vs non-null element mappers skips over all indices less than 0.
        const std::unordered_map<int, std::wstring_view> kControllerElements = {
            {ELEMENT_MAP_INDEX_OF(stickLeftY), L"StickLeftY"},
            {-1, L"InvalidControllerElement1"},
            {ELEMENT_MAP_INDEX_OF(triggerLT), L"TriggerLT"},
            {-2, L"InvalidControllerElement2"},
        };

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        for (const auto& controllerElement : kControllerElements)
            TEST_ASSERT((controllerElement.first >= 0) == builder.SetBlueprintElementMapper(kMapperName, controllerElement.second, kTestElementMapper.Clone()));

        const Mapper::UElementMap* const kElementMap = builder.GetBlueprintElementMap(kMapperName);
        TEST_ASSERT(nullptr != kElementMap);
        for (int i = 0; i < _countof(kElementMap->all); ++i)
        {
            if (kControllerElements.contains(i))
            {
                TEST_ASSERT(nullptr != kElementMap->all[i]);
                TEST_ASSERT(nullptr != dynamic_cast<decltype(kTestElementMapper)*>(kElementMap->all[i].get()));
                TEST_ASSERT(kTestElementMapper.GetTargetElementCount() == kElementMap->all[i]->GetTargetElementCount());

                for (int j = 0; j < kTestElementMapper.GetTargetElementCount(); ++j)
                    TEST_ASSERT(kTestElementMapper.GetTargetElementAt(j) == kElementMap->all[i]->GetTargetElementAt(j));
            }
            else
            {
                TEST_ASSERT(nullptr == kElementMap->all[i]);
            }
        }
    }

    // Verifies that element mappers cannot be set on unknown mappers.
    // The element mappers themselves are valid, but the mapper name is unknown.
    TEST_CASE(MapperBuilder_ElementMap_UnknownMapper)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";
        constexpr std::wstring_view kUnknownMapperName = L"UnknownMapper";
        constexpr AxisMapper kTestElementMapper(EAxis::X);
        const std::wstring_view kControllerElements[] = {L"StickLeftY", L"TriggerLT"};

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        for (const auto& controllerElement : kControllerElements)
            TEST_ASSERT(false == builder.SetBlueprintElementMapper(kUnknownMapperName, controllerElement, kTestElementMapper.Clone()));

        TEST_ASSERT(nullptr == builder.GetBlueprintElementMap(kUnknownMapperName));

        const Mapper::UElementMap* const kElementMap = builder.GetBlueprintElementMap(kMapperName);
        TEST_ASSERT(nullptr != kElementMap);
        for (int i = 0; i < _countof(kElementMap->all); ++i)
            TEST_ASSERT(nullptr == kElementMap->all[i]);
    }

    // Verifies that template names can be set regardless of whether or not they refer to existing mappers, mapper blueprints, or even the mapper blueprint itself.
    // These should all be successful because template names are not checked for semantic correctness until an attempt is made to construct a mapper object.
    TEST_CASE(MapperBuilder_TemplateName_Nominal)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";
        constexpr std::wstring_view kTemplateNames[] = {kMapperName, L"RandomMapper", L"StandardGamepad"};

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        for (auto templateName : kTemplateNames)
        {
            TEST_ASSERT(true == builder.SetBlueprintTemplate(kMapperName, templateName));
            TEST_ASSERT(true == builder.GetBlueprintTemplate(kMapperName).has_value());
            TEST_ASSERT(templateName == builder.GetBlueprintTemplate(kMapperName).value());
        }
    }

    // Verifies that template name setting attempts are rejected if the mapper name is unknown.
    TEST_CASE(MapperBuilder_TemplateName_UnknownMapper)
    {
        constexpr std::wstring_view kMapperName = L"TestMapper";
        constexpr std::wstring_view kUnknownMapperName = L"UnknownMapper";
        constexpr std::wstring_view kTemplateNames[] = {kMapperName, L"RandomMapper", L"StandardGamepad"};

        MapperBuilder builder;
        TEST_ASSERT(true == builder.CreateBlueprint(kMapperName));

        for (auto templateName : kTemplateNames)
        {
            TEST_ASSERT(false == builder.SetBlueprintTemplate(kUnknownMapperName, templateName));
            TEST_ASSERT(false == builder.GetBlueprintTemplate(kUnknownMapperName).has_value());
        }

        TEST_ASSERT(true == builder.GetBlueprintTemplate(kMapperName).has_value());
        TEST_ASSERT(true == builder.GetBlueprintTemplate(kMapperName).value().empty());
    }
}
