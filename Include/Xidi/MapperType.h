/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file MapperType.h
 *   Declaration of types and helpers for obtaining mapper metadata.
 *****************************************************************************/

#pragma once

#include <string_view>


namespace Xidi
{
    // -------- TYPE DEFINITIONS ------------------------------------------- //

    /// Enumerates the known types of mappers that can be created.
    enum class EMapperType
    {
        Invalid = -1,

        ExtendedGamepad,
        StandardGamepad,
        XInputNative,
        XInputSharedTriggers,
    };


    // -------- CONSTANTS -------------------------------------------------- //

    /// Default mapper type.
    /// Mappers of this type will be created unless overridden by a configuration file.
    inline constexpr EMapperType kDefaultMapperType = EMapperType::StandardGamepad;


    // -------- FUNCTIONS -------------------------------------------------- //

    /// Convert the specified string into a mapper type enumerator, which can be invalid if the string is not recognized.
    /// Valid strings are the same as the names of the enumerators themselves.
    /// @param [in] typeString String representation of the mapper type enumerator.
    /// @return Enumerator from the string.
    EMapperType MapperTypeFromString(std::wstring_view typeString);
}
