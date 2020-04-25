/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file XidiConfigReader.h
 *   Declaration of Xidi-specific configuration reading functionality.
 *****************************************************************************/

#pragma once

#include "Configuration.h"

#include <string_view>


namespace Xidi
{
    class XidiConfigReader : public Configuration::ConfigurationFileReader
    {
    private:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //
        // See "Configuration.h" for documentation.

        Configuration::ESectionAction ActionForSection(std::wstring_view section) override;
        bool CheckValue(std::wstring_view section, std::wstring_view name, const Configuration::TIntegerValue& value) override;
        bool CheckValue(std::wstring_view section, std::wstring_view name, const Configuration::TBooleanValue& value) override;
        bool CheckValue(std::wstring_view section, std::wstring_view name, const Configuration::TStringValue& value) override;
        Configuration::EValueType TypeForValue(std::wstring_view section, std::wstring_view name) override;
    };
}
