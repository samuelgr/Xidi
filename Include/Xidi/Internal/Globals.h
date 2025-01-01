/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file Globals.h
 *   Declaration of a namespace for storing and retrieving global data. Intended for
 *   miscellaneous data elements with no other suitable place.
 **************************************************************************************************/

#pragma once

#include <string>
#include <string_view>

#include <Infra/Core/Configuration.h>

#include "ApiWindows.h"

namespace Xidi
{
  namespace Globals
  {
    /// Determines if this process has input focus based on whether or not a window it owns is at
    /// the foreground.
    /// @return `true` if so, `false` if not.
    bool DoesCurrentProcessHaveInputFocus(void);

    /// Retrieves the configuration object that represents the data read from a configuration file.
    /// @return Read-only configuration object reference.
    const Infra::Configuration::ConfigurationData& GetConfigurationData(void);

    /// Performs run-time initialization.
    /// This function only performs operations that are safe to perform within a DLL entry point.
    void Initialize(void);
  } // namespace Globals
} // namespace Xidi
