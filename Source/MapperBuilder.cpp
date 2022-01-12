/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MapperBuilder.cpp
 *   Implementation of functionality for building new mapper objects
 *   piece-wise at runtime.
 *****************************************************************************/

#include "Mapper.h"
#include "MapperBuilder.h"
#include "MapperParser.h"
#include "Message.h"

#include <map>
#include <memory>
#include <set>
#include <string_view>
#include <vector>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Retrieves and returns a safe string view that can be used to represent mapper names and template names.
        /// This function maintains the memory needed to store mapper names permanently and deduplicates to ensure only one copy of each mapper name is ever stored.
        /// @param [in] mapperName Name of the mapper for which a safe view is needed.
        /// @return Safe string view of the mapper name that will remain valid until program termination.
        static inline std::wstring_view SafeMapperNameString(std::wstring_view mapperName)
        {
            static std::set<std::wstring> mapperNames;
            return *mapperNames.emplace(mapperName).first;
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "MapperBuilder.h" for documentation.

        bool MapperBuilder::Build(void)
        {
            for (const auto& blueprintItem : blueprints)
            {
                if ((true == blueprintItem.second.buildAttempted) || (false == blueprintItem.second.buildCanAttempt))
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
                Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Internal error: A mapper already exists with this name.", mapperName.data());
                return nullptr;
            }

            SBlueprint& blueprint = blueprints.at(mapperName);

            if (false == blueprint.buildCanAttempt)
            {
                // If the blueprint was previously invalidated, then it cannot be built.
                Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Mapper configuration is invalid.", mapperName.data());
                return nullptr;
            }

            if (true == blueprint.buildAttempted)
            {
                // If the build started but was never completed, then this indicates a cycle in the dependency graph, which is an error.
                Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Circular template dependency.", mapperName.data());
                return nullptr;
            }

            blueprint.buildAttempted = true;

            Mapper::UElementMap mapperElements;
            Mapper::UForceFeedbackActuatorMap mapperForceFeedbackActuators;

            if (false == blueprint.templateName.empty())
            {
                // If a template is specified, then the mapper element starting point comes from an existing mapper object.
                // If the mapper object named in the template does not exist, try to build it. It is an error if that dependent build operation fails.
                if (false == Mapper::IsMapperNameKnown(blueprint.templateName))
                {
                    // The purpose of this check is to make error messages easier to understand by making it immediately obvious why a template build operation failed.
                    // Without it, the user would see an attempt to build the template take place, fail, and then this mapper would fail to build due to a template dependency build failure, so either 2 or 3 messages total when 1 would suffice and be more concise.
                    if (false == DoesBlueprintNameExist(blueprint.templateName))
                    {
                        Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Unknown template dependency %s.", mapperName.data(), blueprint.templateName.data());
                        return nullptr;
                    }

                    Message::OutputFormatted(Message::ESeverity::Info, L"Mapper %s uses mapper %s as a template. Attempting to build it.", mapperName.data(), blueprint.templateName.data());
                    
                    if (nullptr == Build(blueprint.templateName))
                    {
                        Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Template dependency %s failed to build.", mapperName.data(), blueprint.templateName.data());
                        return nullptr;
                    }

                    if (false == Mapper::IsMapperNameKnown(blueprint.templateName))
                    {
                        Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Internal error: Successfully built template dependency %s but failed to register the resulting mapper object.", mapperName.data(), blueprint.templateName.data());
                        return nullptr;
                    }
                }

                // Since the template name is known, the registered mapper object should be obtainable.
                // It is an internal error if this fails.
                const Mapper* const kTemplateMapper = Mapper::GetByName(blueprint.templateName);
                if (nullptr == kTemplateMapper)
                {
                    Message::OutputFormatted(Message::ESeverity::Error, L"Error while building mapper %s: Internal error: Failed to locate the mapper object for template dependency %s.", mapperName.data(), blueprint.templateName.data());
                    return nullptr;
                }

                mapperElements = Mapper::GetByName(blueprint.templateName)->CloneElementMap();
                mapperForceFeedbackActuators = Mapper::GetByName(blueprint.templateName)->GetForceFeedbackActuatorMap();
            }

            // Loop through all the changes that the blueprint describes and apply them to the starting point.
            // If the starting point is empty then this is essentially building a new element map from scratch.
            for (auto& changeFromTemplate : blueprint.changesFromTemplate)
                mapperElements.all[changeFromTemplate.first] = std::move(changeFromTemplate.second);

            Message::OutputFormatted(Message::ESeverity::Info, L"Successfully built mapper %s.", mapperName.data());
            return new Mapper(mapperName, std::move(mapperElements.named), mapperForceFeedbackActuators.named);
        }

        // --------

        bool MapperBuilder::ClearBlueprintElementMapper(std::wstring_view mapperName, unsigned int elementIndex)
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.end() == blueprintIter)
                return false;

            if (elementIndex >= _countof(Mapper::UElementMap::all))
                return false;

            if (false == blueprintIter->second.changesFromTemplate.contains(elementIndex))
                return false;

            blueprintIter->second.changesFromTemplate.erase(elementIndex);
            return true;
        }

        // --------

        bool MapperBuilder::ClearBlueprintElementMapper(std::wstring_view mapperName, std::wstring_view elementString)
        {
            const std::optional<unsigned int> kMaybeControllerElementIndex = MapperParser::FindControllerElementIndex(elementString);
            if (false == kMaybeControllerElementIndex.has_value())
                return false;

            return ClearBlueprintElementMapper(mapperName, kMaybeControllerElementIndex.value());
        }

        // --------

        bool MapperBuilder::CreateBlueprint(std::wstring_view mapperName)
        {
            if (true == Mapper::IsMapperNameKnown(mapperName))
                return false;

            return blueprints.emplace(std::make_pair(SafeMapperNameString(mapperName), SBlueprint())).second;
        }

        // --------

        bool MapperBuilder::DoesBlueprintNameExist(std::wstring_view mapperName) const
        {
            return blueprints.contains(mapperName);
        }

        // --------

        const MapperBuilder::TElementMapSpec* MapperBuilder::GetBlueprintElementMapSpec(std::wstring_view mapperName) const
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.cend() == blueprintIter)
                return nullptr;

            return &blueprintIter->second.changesFromTemplate;
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

        bool MapperBuilder::InvalidateBlueprint(std::wstring_view mapperName)
        {
            auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.cend() == blueprintIter)
                return false;

            blueprintIter->second.buildCanAttempt = false;
            return true;
        }

        // --------

        bool MapperBuilder::SetBlueprintElementMapper(std::wstring_view mapperName, unsigned int elementIndex, std::unique_ptr<IElementMapper>&& elementMapper)
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.end() == blueprintIter)
                return false;

            if (elementIndex >= _countof(Mapper::UElementMap::all))
                return false;

            blueprintIter->second.changesFromTemplate[elementIndex] = std::move(elementMapper);
            return true;
        }

        // --------

        bool MapperBuilder::SetBlueprintElementMapper(std::wstring_view mapperName, std::wstring_view elementString, std::unique_ptr<IElementMapper>&& elementMapper)
        {
            const std::optional<unsigned int> kMaybeControllerElementIndex = MapperParser::FindControllerElementIndex(elementString);
            if (false == kMaybeControllerElementIndex.has_value())
                return false;

            return SetBlueprintElementMapper(mapperName, kMaybeControllerElementIndex.value(), std::move(elementMapper));
        }

        // --------

        bool MapperBuilder::SetBlueprintTemplate(std::wstring_view mapperName, std::wstring_view newTemplateName)
        {
            const auto blueprintIter = blueprints.find(mapperName);
            if (blueprints.end() == blueprintIter)
                return false;

            blueprintIter->second.templateName = SafeMapperNameString(newTemplateName);
            return true;
        }
    }
}
