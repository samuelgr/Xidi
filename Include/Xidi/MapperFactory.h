/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file MapperFactory.h
 *   Declaration of a class that creates mappers of different types.
 *****************************************************************************/

#pragma once

#include "Mapper/Base.h"

#include <string_view>


namespace Xidi
{
    namespace Mapper
    {
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Enumerates the known types of mappers that can be created.
        enum class EType
        {
            Invalid = -1,

            ExtendedGamepad,
            StandardGamepad,
            XInputNative,
            XInputSharedTriggers,
        };


        // -------- CONSTANTS ---------------------------------------------- //

        /// Default mapper type.
        /// Mappers of this type will be created unless overridden by a configuration file.
        inline constexpr EType kDefaultMapperType = EType::StandardGamepad;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Creates a new mapper of the configured type, using the `new` operator.
        /// @return Pointer to the newly-created mapper, or `nullptr` in the event of an error.
        Mapper::Base* Create(void);

        /// Convert the specified string into a mapper type enumerator, which can be invalid if the string is not recognized.
        /// Valid strings are the same as the names of the enumerators themselves.
        /// @param [in] typeString String representation of the mapper type enumerator.
        /// @return Enumerator from the string.
        EType TypeFromString(std::wstring_view typeString);
    }
}
