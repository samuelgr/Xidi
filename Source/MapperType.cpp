/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper.cpp
 *    Implementation of helper functions for obtaining mapper metadata.
 *****************************************************************************/

#include "MapperType.h"

#include <string_view>
#include <unordered_map>


namespace Xidi
{
    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Maps from string to internal mapper type enumerator.
    static const std::unordered_map<std::wstring_view, EMapperType> mapperTypeStrings = {
        {L"ExtendedGamepad",                     EMapperType::ExtendedGamepad},
        {L"StandardGamepad",                     EMapperType::StandardGamepad},
        {L"XInputNative",                        EMapperType::XInputNative},
        {L"XInputSharedTriggers",                EMapperType::XInputSharedTriggers}
    };

    // -------- FUNCTIONS -------------------------------------------------- //
    // See "MapperType.h" for documentation.

    EMapperType MapperTypeFromString(std::wstring_view typeString)
    {
        auto it = mapperTypeStrings.find(typeString);

        if (mapperTypeStrings.end() == it)
            return EMapperType::Invalid;
        else
            return it->second;
    }

}
