/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file Globals.h
 *   Declaration of a namespace for storing and retrieving global data. Intended for
 *   miscellaneous data elements with no other suitable place.
 **************************************************************************************************/

#pragma once

#include <string>
#include <string_view>

#include "ApiWindows.h"
#include "Configuration.h"

namespace Xidi
{
  namespace Globals
  {
    /// Version information structure.
    struct SVersionInfo
    {
      /// Major version number.
      uint16_t major;

      /// Minor version number.
      uint16_t minor;

      /// Patch level.
      uint16_t patch;

      union
      {
        /// Complete view of the flags element of structured version information.
        uint16_t flags;

        // Per Microsoft documentation, bit fields are ordered from low bit to high bit.
        // See https://docs.microsoft.com/en-us/cpp/cpp/cpp-bit-fields for more information.
        struct
        {
          /// Whether or not the working directory was dirty when the binary was built.
          uint16_t isDirty : 1;

          /// Unused bits, reserved for future use.
          uint16_t reserved : 3;

          /// Number of commits since the most recent official version tag.
          uint16_t commitDistance : 12;
        };
      };

      /// String representation of the version information, including any suffixes. Guaranteed to be
      /// null-terminated.
      std::wstring_view string;
    };

    static_assert(
        sizeof(SVersionInfo) == ((4 * sizeof(uint16_t)) + sizeof(std::wstring_view)),
        "Version information structure size constraint violation.");

    /// Retrieves the configuration object that represents the data read from a configuration file.
    /// @return Read-only configuration object reference.
    const Configuration::ConfigurationData& GetConfigurationData(void);

    /// Determines if this process has input focus based on whether or not a window it owns is at
    /// the foreground.
    /// @return `true` if so, `false` if not.
    bool DoesCurrentProcessHaveInputFocus(void);

    /// Retrieves a pseudohandle to the current process.
    /// @return Current process pseudohandle.
    HANDLE GetCurrentProcessHandle(void);

    /// Retrieves the PID of the current process.
    /// @return Current process PID.
    DWORD GetCurrentProcessId(void);

    /// Retrieves the handle of the instance that represents the current running form of this code.
    /// @return Instance handle for this code.
    HINSTANCE GetInstanceHandle(void);

    /// Retrieves information on the current system. This includes architecture, page size, and so
    /// on.
    /// @return Reference to a read-only structure containing system information.
    const SYSTEM_INFO& GetSystemInformation(void);

    /// Retrieves and returns version information for this running binary.
    /// @return Version information structure.
    SVersionInfo GetVersion(void);

    /// Performs run-time initialization.
    /// This function only performs operations that are safe to perform within a DLL entry point.
    void Initialize(void);
  } // namespace Globals
} // namespace Xidi
