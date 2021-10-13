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
#include <vector>


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
            for (const auto& blueprintItem : blueprints)
            {
                if (true == blueprintItem.second.buildAttempted)
                    continue;

                if (nullptr == Build(blueprintItem.first))
                    return false;
            }

            return true;
        }

        // --------

        const Mapper* MapperBuilder::Build(std::wstring_view mapperName)
        {
            if (false == DoesBlueprintNameExist(mapperName))
            {
                Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Unrecognized name.", mapperName.data());
                return nullptr;
            }

            if (true == Mapper::IsMapperNameKnown(mapperName))
            {
                Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Internal error due to a mapper already existing with this name.", mapperName.data());
                return nullptr;
            }

            SBlueprint& blueprint = blueprints.at(mapperName);

            if (true == blueprint.buildAttempted)
            {
                // If the build started but was never completed, then this indicates a cycle in the dependency graph, which is an error.
                Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Cycle detected in the template dependency graph.", mapperName.data());
                return nullptr;
            }

            blueprint.buildAttempted = true;

            Mapper::UElementMap mapperElements;

            if (false == blueprint.templateName.empty())
            {
                // If a template is specified, then the mapper element starting point comes from an existing mapper object.
                // If the mapper object named in the template does not exist, try to build it. It is an error if that dependent build operation fails.
                if (false == Mapper::IsMapperNameKnown(blueprint.templateName))
                {
                    Message::OutputFormatted(Message::ESeverity::Info, L"Mapper %s uses mapper %s as a template. Attempting to build it.", mapperName.data(), blueprint.templateName.data());
                    
                    if (nullptr == Build(blueprint.templateName))
                    {
                        Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Template dependency %s failed to build.", mapperName.data(), blueprint.templateName.data());
                        return nullptr;
                    }

                    if (false == Mapper::IsMapperNameKnown(blueprint.templateName))
                    {
                        Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Internal error due to successful build of template dependency %s but failure to register the resulting mapper object.", mapperName.data(), blueprint.templateName.data());
                        return nullptr;
                    }
                }

                // Since the template name is known, the registered mapper object should be obtainable.
                // It is an internal error if this fails.
                const Mapper* const kTemplateMapper = Mapper::GetByName(blueprint.templateName);
                if (nullptr == kTemplateMapper)
                {
                    Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Internal error due to failure to locate the mapper object for template dependency %s.", mapperName.data(), blueprint.templateName.data());
                    return nullptr;
                }

                mapperElements = Mapper::GetByName(blueprint.templateName)->CloneElementMap();
            }

            // Loop through all the changes that the blueprint describes and apply them to the starting point.
            // If the starting point is empty then this is essentially building a new element map from scratch.
            for (int i = 0; i < _countof(mapperElements.all); ++i)
            {
                if (nullptr != blueprint.deltaFromTemplate.all[i])
                    mapperElements.all[i] = std::move(blueprint.deltaFromTemplate.all[i]);
            }

            Message::OutputFormatted(Message::ESeverity::Info, L"Successfully built mapper %s.", mapperName.data());
            return new Mapper(mapperName, std::move(mapperElements.named));
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
