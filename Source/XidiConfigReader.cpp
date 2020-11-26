/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file XidiConfigReader.cpp
 *   Implementation of Xidi-specific configuration reading functionality.
 *****************************************************************************/

#include "Configuration.h"
#include "MapperType.h"
#include "Strings.h"
#include "TemporaryBuffer.h"
#include "XidiConfigReader.h"

#include <unordered_map>
#include <string>
#include <string_view>


namespace Xidi
{
    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Holds the layout of the Xidi configuration file that is known statically.
    static Configuration::TConfigurationFileLayout configurationFileLayout = {
        ConfigurationFileLayoutSection(Strings::kStrConfigurationSectionImport, {
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingImportDirectInput, Configuration::EValueType::String),
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingImportDirectInput8, Configuration::EValueType::String),
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingImportWinMM, Configuration::EValueType::String),
        }),
        ConfigurationFileLayoutSection(Strings::kStrConfigurationSectionLog, {
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingLogEnabled, Configuration::EValueType::Boolean),
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingLogLevel, Configuration::EValueType::Integer),
        }),
        ConfigurationFileLayoutSection(Strings::kStrConfigurationSectionMapper, {
            ConfigurationFileLayoutNameAndValueType(Strings::kStrConfigurationSettingMapperType, Configuration::EValueType::String),
        }),
    };


    // -------- CONCRETE INSTANCE METHODS ---------------------------------- //
    // See "Configuration.h" for documentation.

    Configuration::ESectionAction XidiConfigReader::ActionForSection(std::wstring_view section)
    {
        if (0 != configurationFileLayout.count(section))
            return Configuration::ESectionAction::Read;

        return Configuration::ESectionAction::Error;
    }

    // --------

    bool XidiConfigReader::CheckValue(std::wstring_view section, std::wstring_view name, const Configuration::TIntegerValue& value)
    {
        return (value >= 0);
    }

    // --------

    bool XidiConfigReader::CheckValue(std::wstring_view section, std::wstring_view name, const Configuration::TBooleanValue& value)
    {
        return true;
    }

    // --------

    bool XidiConfigReader::CheckValue(std::wstring_view section, std::wstring_view name, const Configuration::TStringValue& value)
    {
        if ((Strings::kStrConfigurationSectionMapper == section) && (Strings::kStrConfigurationSettingMapperType == name))
            return (EMapperType::Invalid != MapperTypeFromString(value));

        return true;
    }

    // --------

    Configuration::EValueType XidiConfigReader::TypeForValue(std::wstring_view section, std::wstring_view name)
    {
        auto sectionLayout = configurationFileLayout.find(section);
        if (configurationFileLayout.end() == sectionLayout)
            return Configuration::EValueType::Error;

        auto settingInfo = sectionLayout->second.find(name);
        if (sectionLayout->second.end() == settingInfo)
            return Configuration::EValueType::Error;

        return settingInfo->second;
    }
}
