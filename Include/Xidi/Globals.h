/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Globals.h
 *   Declaration of a namespace for storing and retrieving global data.
 *   Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "Configuration.h"

#include <string>
#include <string_view>


namespace Xidi
{
    namespace Globals
    {
        // -------- FUNCTIONS ---------------------------------------------- //

        /// Retrieves the configuration object that represents the contents of a configuration file.
        /// @return Read-only configuration object reference.
        const Configuration::Configuration& GetConfiguration(void);
        
        /// Retrieves a pseudohandle to the current process.
        /// @return Current process pseudohandle.
        HANDLE GetCurrentProcessHandle(void);

        /// Retrieves the PID of the current process.
        /// @return Current process PID.
        DWORD GetCurrentProcessId(void);

        /// Retrieves the handle of the instance that represents the current running form of this code.
        /// @return Instance handle for this code.
        HINSTANCE GetInstanceHandle(void);

        /// Retrieves information on the current system. This includes architecture, page size, and so on.
        /// @return Reference to a read-only structure containing system information.
        const SYSTEM_INFO& GetSystemInformation(void);

        /// Performs run-time initialization.
        /// This function only performs operations that are safe to perform within a DLL entry point.
        void Initialize(void);
    }
}
