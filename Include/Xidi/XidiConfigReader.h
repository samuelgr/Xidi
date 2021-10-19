/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file XidiConfigReader.h
 *   Declaration of Xidi-specific configuration reading functionality.
 *****************************************************************************/

#pragma once

#include "Configuration.h"

#include <string_view>


namespace Xidi
{
    using namespace ::Xidi::Configuration;


    class XidiConfigReader : public ConfigurationFileReader
    {
    private:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //
        // See "Configuration.h" for documentation.

        
        EAction ActionForSection(std::wstring_view section) override;
        EAction ActionForValue(std::wstring_view section, std::wstring_view name, TIntegerView value) override;
        EAction ActionForValue(std::wstring_view section, std::wstring_view name, TBooleanView value) override;
        EAction ActionForValue(std::wstring_view section, std::wstring_view name, TStringView value) override;
        EValueType TypeForValue(std::wstring_view section, std::wstring_view name) override;
        void PrepareForRead(void) override;
    };
}
