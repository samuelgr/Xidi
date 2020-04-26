/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper.cpp
 *   Implementation of functionality common to all mappers.
 *****************************************************************************/

#include "Configuration.h"
#include "Globals.h"
#include "Mapper.h"
#include "Mapper/ExtendedGamepad.h"
#include "Mapper/XInputNative.h"
#include "Mapper/XInputSharedTriggers.h"
#include "Mapper/StandardGamepad.h"
#include "Strings.h"

#include <mutex>
#include <string_view>
#include <unordered_map>


namespace Xidi
{
    // -------- INTERNAL VARIABLES ----------------------------------------- //

    static std::unordered_map<std::wstring_view, EMapperType> mapperTypeStrings = {
        {L"ExtendedGamepad",                     EMapperType::ExtendedGamepad},
        {L"StandardGamepad",                     EMapperType::StandardGamepad},
        {L"XInputNative",                        EMapperType::XInputNative},
        {L"XInputSharedTriggers",                EMapperType::XInputSharedTriggers}
    };


    // -------- INTERNAL FUNCTIONS ----------------------------------------- //

    /// Creates a new mapper of the specified type, using the `new` operator.
    /// @return Pointer to the newly-created mapper, or `nullptr` in the event of an error.
    static Mapper* CreateMapperOfType(const EMapperType type)
    {
        Mapper* newMapper = nullptr;

        switch (type)
        {
        case EMapperType::XInputNative:
            newMapper = new XInputNativeMapper();
            break;

        case EMapperType::XInputSharedTriggers:
            newMapper = new XInputSharedTriggersMapper();
            break;

        case EMapperType::StandardGamepad:
            newMapper = new StandardGamepadMapper();
            break;

        case EMapperType::ExtendedGamepad:
            newMapper = new ExtendedGamepadMapper();
            break;
        }

        return newMapper;
    }


    // -------- CLASS METHODS -------------------------------------------------- //
    // See "Mapper.h" for documentation.

    Mapper* Mapper::Create(void)
    {
        static EMapperType configuredMapperType = kDefaultMapperType;

        // Mappers might be created multiple times, but always of the same type, so check the configuration once and cache the result.
        static std::once_flag getConfiguredTypeFlag;
        std::call_once(getConfiguredTypeFlag, []() {
            const Configuration::Configuration& config = Globals::GetConfiguration();

            if ((true == config.IsDataValid()) && (true == config.GetData().SectionNamePairExists(Strings::kStrConfigurationSectionMapper, Strings::kStrConfigurationSettingMapperType)))
            {
                const EMapperType requestedMapperType = TypeFromString(config.GetData()[Strings::kStrConfigurationSectionMapper][Strings::kStrConfigurationSettingMapperType].FirstValue().GetStringValue());

                if (EMapperType::Invalid != requestedMapperType)
                    configuredMapperType = requestedMapperType;
            }
        });

        return CreateMapperOfType(configuredMapperType);
    }

    // --------

    EMapperType Mapper::TypeFromString(std::wstring_view typeString)
    {
        auto it = mapperTypeStrings.find(typeString);

        if (mapperTypeStrings.end() == it)
            return EMapperType::Invalid;
        else
            return it->second;
    }
}
