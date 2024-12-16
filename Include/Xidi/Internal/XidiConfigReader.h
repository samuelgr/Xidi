/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file XidiConfigReader.h
 *   Declaration of Xidi-specific configuration reading functionality.
 **************************************************************************************************/

#pragma once

#include <Infra/Core/Configuration.h>

#ifndef XIDI_SKIP_MAPPERS
#include "MapperBuilder.h"
#endif

#include <string_view>

namespace Xidi
{
  using namespace ::Infra::Configuration;

  class XidiConfigReader : public ConfigurationFileReader
  {
#ifndef XIDI_SKIP_MAPPERS

  public:

    /// Sets the mapper builder object to be filled with custom mapper blueprints during the next
    /// configuration file read attempt. Upon completion of the next read attempt the pointer held
    /// by this object is automatically cleared.
    /// @param [in] newCustomMapperBuilder Pointer to the object that will receive custom mapper
    /// blueprints during configuration file read.
    inline void SetMapperBuilder(Controller::MapperBuilder* newCustomMapperBuilder)
    {
      customMapperBuilder = newCustomMapperBuilder;
    }

#endif

  protected:

    // ConfigurationFileReader
    Action ActionForSection(std::wstring_view section) override;
    Action ActionForValue(
        std::wstring_view section, std::wstring_view name, TIntegerView value) override;
    Action ActionForValue(
        std::wstring_view section, std::wstring_view name, TBooleanView value) override;
    Action ActionForValue(
        std::wstring_view section, std::wstring_view name, TStringView value) override;
    void BeginRead(void) override;
    void EndRead(void) override;
    EValueType TypeForValue(std::wstring_view section, std::wstring_view name) override;

#ifndef XIDI_SKIP_MAPPERS

  private:

    /// Holds custom mapper blueprints parsed from configuration files.
    Controller::MapperBuilder* customMapperBuilder;

#endif
  };
} // namespace Xidi
