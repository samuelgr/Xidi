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
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Version information structure.
        struct SVersionInfo
        {
            uint16_t major;                                             ///< Major version number.
            uint16_t minor;                                             ///< Minor version number.
            uint16_t patch;                                             ///< Patch level.

            union
            {
                uint16_t flags;                                         ///< Complete view of the flags element of structured version information.

                // Per Microsoft documentation, bit fields are ordered from low bit to high bit.
                // See https://docs.microsoft.com/en-us/cpp/cpp/cpp-bit-fields for more information.
                struct
                {
                    uint16_t isDirty : 1;                               ///< Whether or not the working directory was dirty when the binary was built.
                    uint16_t reserved : 3;                              ///< Unused bits, reserved for future use.
                    uint16_t commitDistance : 12;                       ///< Number of commits since the most recent official version tag.
                };
            };

            std::wstring_view string;                                   ///< String representation of the version information, including any suffixes. Guaranteed to be null-terminated.
        };
        static_assert(sizeof(SVersionInfo) == ((4 * sizeof(uint16_t)) + sizeof(std::wstring_view)), "Version information structure size constraint violation.");


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Retrieves the configuration object that represents the contents of a configuration file.
        /// @return Read-only configuration object reference.
        const Configuration::ConfigurationFile& GetConfiguration(void);
        
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

        /// Retrieves and returns version information for this running binary.
        /// @return Version information structure.
        constexpr SVersionInfo GetVersion(void)
        {
            constexpr uint16_t kVersionStructured[] = {GIT_VERSION_STRUCT};
            static_assert(4 == _countof(kVersionStructured), "Invalid structured version information.");

            return {.major = kVersionStructured[0], .minor = kVersionStructured[1], .patch = kVersionStructured[2], .flags = kVersionStructured[3], .string = _CRT_WIDE(GIT_VERSION_STRING)};
        }

        /// Performs run-time initialization.
        /// This function only performs operations that are safe to perform within a DLL entry point.
        void Initialize(void);
    }
}
