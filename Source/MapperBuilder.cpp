/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperBuilder.cpp
 *   Implementation of functionality for building new mapper objects
 *   piece-wise at runtime.
 *****************************************************************************/

#include "Mapper.h"
#include "MapperBuilder.h"
#include "Message.h"

#include <map>
#include <memory>
#include <string_view>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL CONSTANTS ------------------------------------- //

        /// Map of strings representing XInput controller elements to indices within the element map data structure.
        /// One pair exists per field in #SElementMap.
        static const std::map<std::wstring_view, unsigned int> kControllerElementStrings = {
            {L"StickLeftX", ELEMENT_MAP_INDEX_OF(stickLeftX)},
            {L"StickLeftY", ELEMENT_MAP_INDEX_OF(stickLeftY)},
            {L"StickRightX", ELEMENT_MAP_INDEX_OF(stickRightX)},
            {L"StickRightY", ELEMENT_MAP_INDEX_OF(stickRightY)},
            {L"DpadUp", ELEMENT_MAP_INDEX_OF(dpadUp)},
            {L"DpadDown", ELEMENT_MAP_INDEX_OF(dpadDown)},
            {L"DpadLeft", ELEMENT_MAP_INDEX_OF(dpadLeft)},
            {L"DpadRight", ELEMENT_MAP_INDEX_OF(dpadRight)},
            {L"TriggerLT", ELEMENT_MAP_INDEX_OF(triggerLT)},
            {L"TriggerRT", ELEMENT_MAP_INDEX_OF(triggerRT)},
            {L"ButtonA", ELEMENT_MAP_INDEX_OF(buttonA)},
            {L"ButtonB", ELEMENT_MAP_INDEX_OF(buttonB)},
            {L"ButtonX", ELEMENT_MAP_INDEX_OF(buttonX)},
            {L"ButtonY", ELEMENT_MAP_INDEX_OF(buttonY)},
            {L"ButtonLB", ELEMENT_MAP_INDEX_OF(buttonLB)},
            {L"ButtonRB", ELEMENT_MAP_INDEX_OF(buttonRB)},
            {L"ButtonBack", ELEMENT_MAP_INDEX_OF(buttonBack)},
            {L"ButtonStart", ELEMENT_MAP_INDEX_OF(buttonStart)},
            {L"ButtonLS", ELEMENT_MAP_INDEX_OF(buttonLS)},
            {L"ButtonRS", ELEMENT_MAP_INDEX_OF(buttonRS)}
        };


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Attempts to look up the specified string in the map of known controller element strings.
        /// If found, returns the corresponding index within the element map data structure.
        static std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString)
        {
            const auto controllerElementIter = kControllerElementStrings.find(controllerElementString);

            if (kControllerElementStrings.cend() == controllerElementIter)
                return std::nullopt;
            else
                return controllerElementIter->second;
        }


        // -------- CLASS METHODS ------------------------------------------ //
        // See "MapperBuilder.h" for documentation.

        bool MapperBuilder::IsControllerElementStringValid(std::wstring_view element)
        {
            return FindControllerElementIndex(element).has_value();
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "MapperBuilder.h" for documentation.

        bool MapperBuilder::Build(void)
        {
            // TODO
            return false;
        }

        // --------

        bool MapperBuilder::CreateBlueprint(std::wstring_view mapperName)
        {
            if (true == Mapper::IsMapperNameKnown(mapperName))
                return false;

            return blueprints.emplace(std::make_pair(mapperName, SBlueprint())).second;
        }

        // --------

        bool MapperBuilder::DoesBlueprintNameExist(std::wstring_view mapperName) const
        {
            return blueprints.contains(mapperName);
        }

        // --------

        const Mapper::UElementMap* MapperBuilder::GetBlueprintElementMap(std::wstring_view mapperName) const
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.cend() == blueprintIter)
                return nullptr;

            return &blueprintIter->second.deltaFromTemplate;
        }

        // --------

        std::optional<std::wstring_view> MapperBuilder::GetBlueprintTemplate(std::wstring_view mapperName) const
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.cend() == blueprintIter)
                return std::nullopt;

            return blueprintIter->second.templateName;
        }

        // --------

        bool MapperBuilder::SetBlueprintElementMapper(std::wstring_view mapperName, std::wstring_view element, std::unique_ptr<IElementMapper>&& elementMapper)
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.end() == blueprintIter)
                return false;

            const std::optional<unsigned int> kMaybeControllerElementIndex = FindControllerElementIndex(element);
            if (false == kMaybeControllerElementIndex.has_value())
                return false;

            const unsigned int kControllerElementIndex = kMaybeControllerElementIndex.value();
            if (kControllerElementIndex >= _countof(Mapper::UElementMap::all))
                return false;

            blueprintIter->second.deltaFromTemplate.all[kControllerElementIndex] = std::move(elementMapper);
            return true;
        }

        // --------

        bool MapperBuilder::SetBlueprintTemplate(std::wstring_view mapperName, std::wstring_view newTemplateName)
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.end() == blueprintIter)
                return false;

            blueprintIter->second.templateName = newTemplateName;
            return true;
        }
    }
}
