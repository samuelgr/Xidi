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
    namespace Mapper
    {
        // -------- INTERNAL VARIABLES --------------------------------------------- //

        static std::unordered_map<std::wstring_view, EType> mapperTypeStrings = {
            {L"ExtendedGamepad",                     EType::ExtendedGamepad},
            {L"StandardGamepad",                     EType::StandardGamepad},
            {L"XInputNative",                        EType::XInputNative},
            {L"XInputSharedTriggers",                EType::XInputSharedTriggers}
        };


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Creates a new mapper of the specified type, using the `new` operator.
        /// @return Pointer to the newly-created mapper, or `nullptr` in the event of an error.
        static IMapper* CreateMapperOfType(const EType type)
        {
            IMapper* newMapper = nullptr;

            switch (type)
            {
            case EType::XInputNative:
                newMapper = new Mapper::XInputNative();
                break;

            case EType::XInputSharedTriggers:
                newMapper = new Mapper::XInputSharedTriggers();
                break;

            case EType::StandardGamepad:
                newMapper = new Mapper::StandardGamepad();
                break;

            case EType::ExtendedGamepad:
                newMapper = new Mapper::ExtendedGamepad();
                break;
            }

            return newMapper;
        }


        // -------- FUNCTIONS ------------------------------------------------------ //
        // See "Mapper.h" for documentation.

        IMapper* Create(void)
        {
            static EType configuredMapperType = kDefaultMapperType;

            // Mappers might be created multiple times, but always of the same type, so check the configuration once and cache the result.
            static std::once_flag getConfiguredTypeFlag;
            std::call_once(getConfiguredTypeFlag, []() {
                const Configuration::Configuration& config = Globals::GetConfiguration();

                if ((true == config.IsDataValid()) && (true == config.GetData().SectionNamePairExists(Strings::kStrConfigurationSectionMapper, Strings::kStrConfigurationSettingMapperType)))
                {
                    const EType requestedMapperType = TypeFromString(config.GetData()[Strings::kStrConfigurationSectionMapper][Strings::kStrConfigurationSettingMapperType].FirstValue().GetStringValue());

                    if (EType::Invalid != requestedMapperType)
                        configuredMapperType = requestedMapperType;
                }
            });

            return CreateMapperOfType(configuredMapperType);
        }

        // --------

        EType TypeFromString(std::wstring_view typeString)
        {
            auto it = mapperTypeStrings.find(typeString);

            if (mapperTypeStrings.end() == it)
                return EType::Invalid;
            else
                return it->second;
        }
    }
}
