/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file XidiConfigReader.cpp
 *   Implementation of Xidi-specific configuration reading functionality.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Configuration.h"
#include "Strings.h"
#include "TemporaryBuffer.h"
#include "XidiConfigReader.h"

#ifndef XIDI_SKIP_MAPPERS
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"
#include "MapperBuilder.h"
#include "MapperParser.h"
#endif

#include <map>
#include <mutex>
#include <optional>
#include <string_view>


namespace Xidi
{
    using namespace ::Xidi::Configuration;


#ifndef XIDI_SKIP_MAPPERS
    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Default name for a custom mapper whose name is not specified.
    static constexpr std::wstring_view kDefaultCustomMapperName = L"Custom";


    // -------- INTERNAL TYPES --------------------------------------------- //

    /// Enumerates the possible operations to be performed on a custom mapper blueprint.
    enum class EBlueprintOperation
    {
        Error,                                                              ///< Indicates an error.
        SetElementMapper,                                                   ///< Set an element mapper for a controller element.
        SetForceFeedbackActuator,                                           ///< Set a force feedback actuator configuration for a physical force feedback actuator.
        SetTemplate,                                                        ///< Set the blueprint template.
    };
#endif


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Holds the layout of the Xidi configuration file that is known statically.
    static TConfigurationFileLayout configurationFileLayout = {
        ConfigurationFileLayoutSection(Strings::kStrConfigurationSectionImport, {
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingImportDirectInput, EValueType::String),
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingImportDirectInput8, EValueType::String),
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingImportWinMM, EValueType::String),
        }),
        ConfigurationFileLayoutSection(Strings::kStrConfigurationSectionLog, {
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingLogEnabled, EValueType::Boolean),
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingLogLevel, EValueType::Integer),
        }),
        ConfigurationFileLayoutSection(Strings::kStrConfigurationSectionMapper, {
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingMapperType, EValueType::String),
        }),
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

#ifndef XIDI_SKIP_MAPPERS
    /// Determines the operation that should be performed on a mapper blueprint based on the name of a configuration setting.
    /// @param [in] name Configuration setting name.
    /// @return Operation to perform.
    static EBlueprintOperation BlueprintOperationFromName(std::wstring_view name)
    {
        /// Map of supported configuration setting names to associated blueprint operations.
        static const std::map<std::wstring_view, EBlueprintOperation> kBlueprintOperationsMap = {
            {Strings::kStrConfigurationSettingCustomMapperTemplate,         EBlueprintOperation::SetTemplate}
        };

        // If the configuration setting name identifies a valid controller element, then the value should be parsed for an element mapper to be assigned to that controller element.
        if (true == Controller::MapperParser::IsControllerElementStringValid(name))
            return EBlueprintOperation::SetElementMapper;

        // If the configuration setting name identifies a valid force feedback actuator, then the value should be parsed for a configuration to be assigned to that actuator.
        if (true == Controller::MapperParser::IsForceFeedbackActuatorStringValid(name))
            return EBlueprintOperation::SetForceFeedbackActuator;

        // If the configuration setting name is known and contained within the map above, simply return it.
        const auto blueprintOperationIter = kBlueprintOperationsMap.find(name);
        if (kBlueprintOperationsMap.cend() != blueprintOperationIter)
            return blueprintOperationIter->second;

        return EBlueprintOperation::Error;
    }
#endif

    /// Checks if the specified section name could correspond with a section that defines a custom mapper.
    /// This is simply based on whether or not the section name begins with the right prefix.
    /// @param [in] section Section name to check.
    /// @return `true` if the section name begins with the custom mapper section name prefix, `false` otherwise.
    static inline bool IsCustomMapperSectionName(std::wstring_view section)
    {
        return (section.starts_with(Strings::kStrConfigurationSectionCustomMapperPrefix));
    }

#ifndef XIDI_SKIP_MAPPERS
    /// Extracts the name of a custom mapper from a section name and performs no error checks whatsoever.
    /// @param [in] section Section name from which to extract the custom mapper name.
    /// @return Custom mapper name.
    static inline std::wstring_view QuickExtractCustomMapperName(std::wstring_view section)
    {
        if (section.length() == Strings::kStrConfigurationSectionCustomMapperPrefix.length())
            return kDefaultCustomMapperName;

        return section.substr(1 + Strings::kStrConfigurationSectionCustomMapperPrefix.length());
    }

    /// Extracts the name of a custom mapper from a section name with the appropriate prefix.
    /// @param [in] section Section name from which to extract the custom mapper name.
    /// @return Custom mapper name, if it was successfully extracted from the section name.
    static std::optional<std::wstring_view> ExtractCustomMapperName(std::wstring_view section)
    {
        if (false == IsCustomMapperSectionName(section))
            return std::nullopt;

        // Section name contains just the prefix, in which case it identifies the default name.
        if (section.length() == Strings::kStrConfigurationSectionCustomMapperPrefix.length())
            return kDefaultCustomMapperName;

        // Separator character needs to be present otherwise the section name is invalid.
        if (Strings::kCharConfigurationSectionCustomMapperSeparator != section[Strings::kStrConfigurationSectionCustomMapperPrefix.length()])
            return std::nullopt;

        std::wstring_view customMapperName = QuickExtractCustomMapperName(section);
        
        // It is explicitly forbidden to specify a custom mapper name equal to the default.
        if (customMapperName == kDefaultCustomMapperName)
            return std::nullopt;

        return customMapperName;
    }
#endif


    // -------- CONCRETE INSTANCE METHODS ---------------------------------- //
    // See "Configuration.h" for documentation.

    EAction XidiConfigReader::ActionForSection(std::wstring_view section)
    {
#ifndef XIDI_SKIP_MAPPERS
        if ((nullptr != customMapperBuilder) && (true == IsCustomMapperSectionName(section)))
        {
            std::optional<std::wstring_view> customMapperName = ExtractCustomMapperName(section);
            if (false == customMapperName.has_value())
                return EAction::Error;

            if (false == customMapperBuilder->CreateBlueprint(customMapperName.value()))
            {
                SetErrorMessage(Strings::FormatString(L"%s: A mapper with this name already exists.", customMapperName.value().data()));
                return EAction::Error;
            }

            return EAction::Process;
        }
#else
        if (true == IsCustomMapperSectionName(section))
            return EAction::Skip;
#endif

        if (0 != configurationFileLayout.count(section))
            return EAction::Process;

        return EAction::Error;
    }

    // --------

    EAction XidiConfigReader::ActionForValue(std::wstring_view section, std::wstring_view name, TIntegerView value)
    {
        if (value >= 0)
            return EAction::Process;

        return EAction::Error;
    }

    // --------

    EAction XidiConfigReader::ActionForValue(std::wstring_view section, std::wstring_view name, TBooleanView value)
    {
        return EAction::Process;
    }

    // --------

    EAction XidiConfigReader::ActionForValue(std::wstring_view section, std::wstring_view name, TStringView value)
    {
#ifndef XIDI_SKIP_MAPPERS
        if ((nullptr != customMapperBuilder) && (true == IsCustomMapperSectionName(section)))
        {
            std::wstring_view customMapperName = QuickExtractCustomMapperName(section);

            switch (BlueprintOperationFromName(name))
            {
            case EBlueprintOperation::SetElementMapper:
                do {
                    Xidi::Controller::MapperParser::ElementMapperOrError maybeElementMapper = Controller::MapperParser::ElementMapperFromString(value);
                    if (false == maybeElementMapper.HasValue())
                    {
                        SetErrorMessage(Strings::FormatString(L"%s: Failed to parse element mapper: %s.", name.data(), maybeElementMapper.Error().c_str()));
                        customMapperBuilder->InvalidateBlueprint(customMapperName);
                        return EAction::Error;
                    }

                    if (false == customMapperBuilder->SetBlueprintElementMapper(customMapperName, name, std::move(maybeElementMapper.Value())))
                    {
                        SetErrorMessage(Strings::FormatString(L"%s: Internal error: Successfully parsed element mapper but failed to set it on the blueprint.", name.data()));
                        customMapperBuilder->InvalidateBlueprint(customMapperName);
                        return EAction::Error;
                    }
                } while (false);
                break;

            case EBlueprintOperation::SetForceFeedbackActuator:
                do {
                    Xidi::Controller::MapperParser::ForceFeedbackActuatorOrError maybeForceFeedbackActuator = Controller::MapperParser::ForceFeedbackActuatorFromString(value);
                    if (false == maybeForceFeedbackActuator.HasValue())
                    {
                        SetErrorMessage(Strings::FormatString(L"%s: Failed to parse force feedback actuator: %s.", name.data(), maybeForceFeedbackActuator.Error().c_str()));
                        customMapperBuilder->InvalidateBlueprint(customMapperName);
                        return EAction::Error;
                    }

                    if (false == customMapperBuilder->SetBlueprintForceFeedbackActuator(customMapperName, name, maybeForceFeedbackActuator.Value()))
                    {
                        SetErrorMessage(Strings::FormatString(L"%s: Internal error: Successfully parsed force feedback actuator but failed to set it on the blueprint.", name.data()));
                        customMapperBuilder->InvalidateBlueprint(customMapperName);
                        return EAction::Error;
                    }
                } while (false);
                break;

            case EBlueprintOperation::SetTemplate:
                do {
                    if (false == customMapperBuilder->SetBlueprintTemplate(customMapperName, value))
                    {
                        SetErrorMessage(Strings::FormatString(L"Internal error: Failed to set template for %s to %s.", customMapperName.data(), value.data()));
                        customMapperBuilder->InvalidateBlueprint(customMapperName);
                        return EAction::Error;
                    }
                } while (false);
                break;

            default:
                return EAction::Error;
            }

            // Custom mapper configuration settings are processed using the mapper builder.
            // They do not need to be inserted into the configuration data structure.
            return EAction::Skip;
        }
#endif

        return EAction::Process;
    }

    // --------

    void XidiConfigReader::BeginRead(void)
    {
        static std::once_flag initFlag;

        std::call_once(initFlag, []() -> void
            {
                // Create the per-controller mapper settings types and submit them to the configuration file layout.
                // These are gernerated dynamically based on the number of controllers the system supports.
                for (Controller::TControllerIdentifier i = 0; i < Controller::kPhysicalControllerCount; ++i)
                    configurationFileLayout[Strings::kStrConfigurationSectionMapper][Strings::MapperTypeConfigurationNameString(i)] = EValueType::String;
            }
        );
    }

    // --------

    void XidiConfigReader::EndRead(void)
    {
#ifndef XIDI_SKIP_MAPPERS
        customMapperBuilder = nullptr;
#endif
    }

    // --------

    EValueType XidiConfigReader::TypeForValue(std::wstring_view section, std::wstring_view name)
    {
#ifndef XIDI_SKIP_MAPPERS
        if ((nullptr != customMapperBuilder) && (true == IsCustomMapperSectionName(section)))
        {
            // All custom mapper operations use strings as input.
            // As long as an operation can be located for the specified configuration setting name the result is a string, otherwise it is an error.

            if (EBlueprintOperation::Error == BlueprintOperationFromName(name))
                return EValueType::Error;
            else
                return EValueType::String;
        }
#endif

        auto sectionLayout = configurationFileLayout.find(section);
        if (configurationFileLayout.end() == sectionLayout)
            return EValueType::Error;

        auto settingInfo = sectionLayout->second.find(name);
        if (sectionLayout->second.end() == settingInfo)
            return EValueType::Error;

        return settingInfo->second;
    }
}
