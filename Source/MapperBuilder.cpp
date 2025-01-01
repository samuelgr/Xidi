/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MapperBuilder.cpp
 *   Implementation of functionality for building new mapper objects piece-wise at runtime.
 **************************************************************************************************/

#include "MapperBuilder.h"

#include <deque>
#include <map>
#include <memory>

#include <Infra/Core/Message.h>

#include "Mapper.h"
#include "MapperParser.h"

namespace Xidi
{
  namespace Controller
  {
    /// Retrieves and returns a safe string view that can be used to represent mapper names and
    /// template names. This function maintains the memory needed to store mapper names permanently
    /// and deduplicates to ensure only one copy of each mapper name is ever stored. Internally, the
    /// data structure is heap-allocated so it is not destroyed until right at program termination,
    /// existing even past the lifetime of typical static objects.
    /// @param [in] mapperName Name of the mapper for which a safe view is needed.
    /// @return Safe string view of the mapper name that will remain valid until program
    /// termination.
    static std::wstring_view SafeMapperNameString(std::wstring_view mapperName)
    {
      static std::deque<std::wstring>* mapperNames = new std::deque<std::wstring>();
      return mapperNames->emplace_back(mapperName);
    }

    bool MapperBuilder::Build(void)
    {
      for (const auto& blueprintItem : blueprints)
      {
        if ((true == blueprintItem.second.buildAttempted) ||
            (false == blueprintItem.second.buildCanAttempt))
          continue;

        if (nullptr == Build(blueprintItem.first)) return false;
      }

      return true;
    }

    const Mapper* MapperBuilder::Build(std::wstring_view mapperName)
    {
      if (false == DoesBlueprintNameExist(mapperName))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
            L"Error while building mapper %s: Unrecognized name.",
            mapperName.data());
        return nullptr;
      }

      if (true == Mapper::IsMapperNameKnown(mapperName))
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
            L"Error while building mapper %s: Internal error: A mapper already exists with this name.",
            mapperName.data());
        return nullptr;
      }

      SBlueprint& blueprint = blueprints.at(mapperName);

      if (false == blueprint.buildCanAttempt)
      {
        // If the blueprint was previously invalidated, then it cannot be built.
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
            L"Error while building mapper %s: Mapper configuration is invalid.",
            mapperName.data());
        return nullptr;
      }

      if (true == blueprint.buildAttempted)
      {
        // If the build started but was never completed, then this indicates a cycle in the
        // dependency graph, which is an error.
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
            L"Error while building mapper %s: Circular template dependency.",
            mapperName.data());
        return nullptr;
      }

      blueprint.buildAttempted = true;

      Mapper::UElementMap mapperElements;
      Mapper::UForceFeedbackActuatorMap mapperForceFeedbackActuators;

      if (false == blueprint.templateName.empty())
      {
        // If a template is specified, then the mapper element starting point comes from an existing
        // mapper object. If the mapper object named in the template does not exist, try to build
        // it. It is an error if that dependent build operation fails.
        if (false == Mapper::IsMapperNameKnown(blueprint.templateName))
        {
          // The purpose of this check is to make error messages easier to understand by making it
          // immediately obvious why a template build operation failed. Without it, the user would
          // see an attempt to build the template take place, fail, and then this mapper would fail
          // to build due to a template dependency build failure, so either 2 or 3 messages total
          // when 1 would suffice and be more concise.
          if (false == DoesBlueprintNameExist(blueprint.templateName))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Error,
                L"Error while building mapper %s: Unknown template dependency %s.",
                mapperName.data(),
                blueprint.templateName.data());
            return nullptr;
          }

          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Info,
              L"Mapper %s uses mapper %s as a template. Attempting to build it.",
              mapperName.data(),
              blueprint.templateName.data());

          if (nullptr == Build(blueprint.templateName))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Error,
                L"Error while building mapper %s: Template dependency %s failed to build.",
                mapperName.data(),
                blueprint.templateName.data());
            return nullptr;
          }

          if (false == Mapper::IsMapperNameKnown(blueprint.templateName))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Error,
                L"Error while building mapper %s: Internal error: Successfully built template dependency %s but failed to register the resulting mapper object.",
                mapperName.data(),
                blueprint.templateName.data());
            return nullptr;
          }
        }

        // Since the template name is known, the registered mapper object should be obtainable.
        // It is an internal error if this fails.
        const Mapper* const kTemplateMapper = Mapper::GetByName(blueprint.templateName);
        if (nullptr == kTemplateMapper)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Error,
              L"Error while building mapper %s: Internal error: Failed to locate the mapper object for template dependency %s.",
              mapperName.data(),
              blueprint.templateName.data());
          return nullptr;
        }

        mapperElements = Mapper::GetByName(blueprint.templateName)->CloneElementMap();
        mapperForceFeedbackActuators =
            Mapper::GetByName(blueprint.templateName)->GetForceFeedbackActuatorMap();
      }

      // Loop through all the changes that the blueprint describes and apply them to the starting
      // point. If the starting point is empty then this is essentially building a new element map
      // from scratch.
      for (auto& elementChangeFromTemplate : blueprint.elementChangesFromTemplate)
        mapperElements.all[elementChangeFromTemplate.first] =
            std::move(elementChangeFromTemplate.second);

      // If the actuator map is empty, then no template was specified and no actuators were parsed
      // out of the configuration file. This means that the default actuator map should be used.
      // Otherwise the logic is the same as for the element changes.
      if (true == blueprint.ffActuatorChangesFromTemplate.empty())
      {
        mapperForceFeedbackActuators = Mapper::kDefaultForceFeedbackActuatorMap;
      }
      else
      {
        for (auto& ffActuatorChangeFromTemplate : blueprint.ffActuatorChangesFromTemplate)
          mapperForceFeedbackActuators.all[ffActuatorChangeFromTemplate.first] =
              ffActuatorChangeFromTemplate.second;
      }

      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Info, L"Successfully built mapper %s.", mapperName.data());
      return new Mapper(
          mapperName, std::move(mapperElements.named), mapperForceFeedbackActuators.named);
    }

    bool MapperBuilder::ClearBlueprintElementMapper(
        std::wstring_view mapperName, unsigned int elementIndex)
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.end() == blueprintIter) return false;

      if (elementIndex >= _countof(Mapper::UElementMap::all)) return false;

      if (false == blueprintIter->second.elementChangesFromTemplate.contains(elementIndex))
        return false;

      blueprintIter->second.elementChangesFromTemplate.erase(elementIndex);
      return true;
    }

    bool MapperBuilder::ClearBlueprintElementMapper(
        std::wstring_view mapperName, std::wstring_view elementString)
    {
      const std::optional<unsigned int> maybeControllerElementIndex =
          MapperParser::FindControllerElementIndex(elementString);
      if (false == maybeControllerElementIndex.has_value()) return false;

      return ClearBlueprintElementMapper(mapperName, maybeControllerElementIndex.value());
    }

    bool MapperBuilder::ClearBlueprintForceFeedbackActuator(
        std::wstring_view mapperName, unsigned int ffActuatorIndex)
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.end() == blueprintIter) return false;

      if (ffActuatorIndex >= _countof(Mapper::UForceFeedbackActuatorMap::all)) return false;

      if (false == blueprintIter->second.ffActuatorChangesFromTemplate.contains(ffActuatorIndex))
        return false;

      blueprintIter->second.ffActuatorChangesFromTemplate.erase(ffActuatorIndex);
      return true;
    }

    bool MapperBuilder::ClearBlueprintForceFeedbackActuator(
        std::wstring_view mapperName, std::wstring_view ffActuatorString)
    {
      const std::optional<unsigned int> maybeForceFeedbackActuatorIndex =
          MapperParser::FindForceFeedbackActuatorIndex(ffActuatorString);
      if (false == maybeForceFeedbackActuatorIndex.has_value()) return false;

      return ClearBlueprintForceFeedbackActuator(
          mapperName, maybeForceFeedbackActuatorIndex.value());
    }

    bool MapperBuilder::CreateBlueprint(std::wstring_view mapperName)
    {
      if (true == Mapper::IsMapperNameKnown(mapperName)) return false;

      return blueprints.emplace(std::make_pair(SafeMapperNameString(mapperName), SBlueprint()))
          .second;
    }

    bool MapperBuilder::DoesBlueprintNameExist(std::wstring_view mapperName) const
    {
      return blueprints.contains(mapperName);
    }

    const MapperBuilder::TElementMapSpec* MapperBuilder::GetBlueprintElementMapSpec(
        std::wstring_view mapperName) const
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.cend() == blueprintIter) return nullptr;

      return &blueprintIter->second.elementChangesFromTemplate;
    }

    const MapperBuilder::TForceFeedbackActuatorSpec*
        MapperBuilder::GetBlueprintForceFeedbackActuatorSpec(std::wstring_view mapperName) const
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.cend() == blueprintIter) return nullptr;

      return &blueprintIter->second.ffActuatorChangesFromTemplate;
    }

    std::optional<std::wstring_view> MapperBuilder::GetBlueprintTemplate(
        std::wstring_view mapperName) const
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.cend() == blueprintIter) return std::nullopt;

      return blueprintIter->second.templateName;
    }

    bool MapperBuilder::InvalidateBlueprint(std::wstring_view mapperName)
    {
      auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.cend() == blueprintIter) return false;

      blueprintIter->second.buildCanAttempt = false;
      return true;
    }

    bool MapperBuilder::SetBlueprintElementMapper(
        std::wstring_view mapperName,
        unsigned int elementIndex,
        std::unique_ptr<IElementMapper>&& elementMapper)
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.end() == blueprintIter) return false;

      if (elementIndex >= _countof(Mapper::UElementMap::all)) return false;

      blueprintIter->second.elementChangesFromTemplate[elementIndex] = std::move(elementMapper);
      return true;
    }

    bool MapperBuilder::SetBlueprintElementMapper(
        std::wstring_view mapperName,
        std::wstring_view elementString,
        std::unique_ptr<IElementMapper>&& elementMapper)
    {
      const std::optional<unsigned int> maybeControllerElementIndex =
          MapperParser::FindControllerElementIndex(elementString);
      if (false == maybeControllerElementIndex.has_value()) return false;

      return SetBlueprintElementMapper(
          mapperName, maybeControllerElementIndex.value(), std::move(elementMapper));
    }

    bool MapperBuilder::SetBlueprintForceFeedbackActuator(
        std::wstring_view mapperName,
        unsigned int ffActuatorIndex,
        ForceFeedback::SActuatorElement ffActuator)
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.end() == blueprintIter) return false;

      if (ffActuatorIndex >= _countof(Mapper::UForceFeedbackActuatorMap::all)) return false;

      blueprintIter->second.ffActuatorChangesFromTemplate[ffActuatorIndex] = ffActuator;
      return true;
    }

    bool MapperBuilder::SetBlueprintForceFeedbackActuator(
        std::wstring_view mapperName,
        std::wstring_view ffActuatorString,
        ForceFeedback::SActuatorElement ffActuator)
    {
      const std::optional<unsigned int> maybeForceFeedbackActuatorIndex =
          MapperParser::FindForceFeedbackActuatorIndex(ffActuatorString);
      if (false == maybeForceFeedbackActuatorIndex.has_value()) return false;

      return SetBlueprintForceFeedbackActuator(
          mapperName, maybeForceFeedbackActuatorIndex.value(), ffActuator);
    }

    bool MapperBuilder::SetBlueprintTemplate(
        std::wstring_view mapperName, std::wstring_view newTemplateName)
    {
      const auto blueprintIter = blueprints.find(mapperName);
      if (blueprints.end() == blueprintIter) return false;

      blueprintIter->second.templateName = SafeMapperNameString(newTemplateName);
      return true;
    }
  } // namespace Controller
} // namespace Xidi
