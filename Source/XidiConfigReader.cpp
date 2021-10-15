/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file XidiConfigReader.cpp
 *   Implementation of Xidi-specific configuration reading functionality.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Configuration.h"
#include "ControllerTypes.h"
#include "Mapper.h"
#include "Strings.h"
#include "TemporaryBuffer.h"
#include "XidiConfigReader.h"

#include <mutex>
#include <string_view>


namespace Xidi
{
    using namespace ::Xidi::Configuration;


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


    // -------- CONCRETE INSTANCE METHODS ---------------------------------- //
    // See "Configuration.h" for documentation.

    ESectionAction XidiConfigReader::ActionForSection(std::wstring_view section)
    {
        if (0 != configurationFileLayout.count(section))
            return ESectionAction::Read;

        return ESectionAction::Error;
    }

    // --------

    bool XidiConfigReader::CheckValue(std::wstring_view section, std::wstring_view name, const TIntegerValue& value)
    {
        return (value >= 0);
    }

    // --------

    bool XidiConfigReader::CheckValue(std::wstring_view section, std::wstring_view name, const TBooleanValue& value)
    {
        return true;
    }

    // --------

    bool XidiConfigReader::CheckValue(std::wstring_view section, std::wstring_view name, const TStringValue& value)
    {
        return true;
    }

    // --------

    EValueType XidiConfigReader::TypeForValue(std::wstring_view section, std::wstring_view name)
    {
        auto sectionLayout = configurationFileLayout.find(section);
        if (configurationFileLayout.end() == sectionLayout)
            return EValueType::Error;

        auto settingInfo = sectionLayout->second.find(name);
        if (sectionLayout->second.end() == settingInfo)
            return EValueType::Error;

        return settingInfo->second;
    }

    // --------

    void XidiConfigReader::PrepareForRead(void)
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
}
