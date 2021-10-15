/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperParser.h
 *   Declaration of functionality for parsing pieces of mapper objects from
 *   strings, typically supplied in a configuration file.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"

#include <optional>
#include <string_view>


namespace Xidi
{
    namespace Controller
    {
        namespace MapperParser
        {
            // -------- FUNCTIONS ------------------------------------------ //

            /// Attempts to identify the index within the `all` member of #UElementMap that corresponds to the controller element identified by the input string.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] controllerElementString String to parse that supposedly identifies a controller element.
            /// @return Element map array index, if it could be identified based on the input string.
            std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString);

            /// Determines if the specified controller element string is valid and recognized as identifying a controller element.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] controllerElementString String to be checked.
            /// @return `true` if the input string is recognized, `false` otherwise.
            bool IsControllerElementStringValid(std::wstring_view controllerElementString);
        }
    }
}
