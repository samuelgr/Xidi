/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file XidiConfigReader.h
 *   Declaration of Xidi-specific configuration reading functionality.
 *****************************************************************************/

#pragma once

#include "Configuration.h"

#ifndef XIDI_SKIP_MAPPERS
#include "MapperBuilder.h"
#endif

#include <string_view>


namespace Xidi
{
    using namespace ::Xidi::Configuration;


    class XidiConfigReader : public ConfigurationFileReader
    {
    private:
#ifndef XIDI_SKIP_MAPPERS
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Holds custom mapper blueprints parsed from configuration files.
        Controller::MapperBuilder* customMapperBuilder;


    public:
        // -------- INSTANCE METHODS --------------------------------------- //

        /// Sets the mapper builder object to be filled with custom mapper blueprints during the next configuration file read attempt.
        /// Upon completion of the next read attempt the pointer held by this object is automatically cleared.
        /// @param [in] newCustomMapperBuilder Pointer to the object that will receive custom mapper blueprints during configuration file read.
        inline void SetMapperBuilder(Controller::MapperBuilder* newCustomMapperBuilder)
        {
            customMapperBuilder = newCustomMapperBuilder;
        }


#endif
    protected:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //
        // See "Configuration.h" for documentation.

        EAction ActionForSection(std::wstring_view section) override;
        EAction ActionForValue(std::wstring_view section, std::wstring_view name, TIntegerView value) override;
        EAction ActionForValue(std::wstring_view section, std::wstring_view name, TBooleanView value) override;
        EAction ActionForValue(std::wstring_view section, std::wstring_view name, TStringView value) override;
        void BeginRead(void) override;
        void EndRead(void) override;
        EValueType TypeForValue(std::wstring_view section, std::wstring_view name) override;
    };
}
