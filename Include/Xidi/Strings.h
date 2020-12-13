/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Strings.h
 *   Declaration of common strings and functions to manipulate them.
 *****************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>


namespace Xidi
{
    namespace Strings
    {
        // -------- COMPILE-TIME CONSTANTS --------------------------------- //
        // Can safely be used at any time, including to perform static initialization.

        /// Base name of the DirectInput library to import.
        inline constexpr std::wstring_view kStrLibraryNameDirectInput = L"dinput.dll";

        /// Base name of the DirectInput8 library to import.
        inline constexpr std::wstring_view kStrLibraryNameDirectInput8 = L"dinput8.dll";

        /// Base name of the WinMM library to import.
        inline constexpr std::wstring_view kStrLibraryNameWinMM = L"winmm.dll";
        
        /// Configuration file section name for overriding import libraries.
        inline constexpr std::wstring_view kStrConfigurationSectionImport = L"Import";

        /// Configuration file setting for overriding import for DirectInput.
        inline constexpr std::wstring_view kStrConfigurationSettingImportDirectInput = kStrLibraryNameDirectInput;

        /// Configuration file setting for overriding import for DirectInput8.
        inline constexpr std::wstring_view kStrConfigurationSettingImportDirectInput8 = kStrLibraryNameDirectInput8;

        /// Configuration file setting for overriding import for WinMM.
        inline constexpr std::wstring_view kStrConfigurationSettingImportWinMM = kStrLibraryNameWinMM;

        /// Configuration file section name for log-related settings.
        inline constexpr std::wstring_view kStrConfigurationSectionLog = L"Log";

        /// Configuration file setting for specifying if the log is enabled.
        inline constexpr std::wstring_view kStrConfigurationSettingLogEnabled = L"Enabled";

        /// Configuration file setting for specifying the logging verbosity level.
        inline constexpr std::wstring_view kStrConfigurationSettingLogLevel = L"Level";

        /// Configuration file section name for mapper-related settings.
        inline constexpr std::wstring_view kStrConfigurationSectionMapper = L"Mapper";

        /// Configuration file setting for specifying the mapper type.
        inline constexpr std::wstring_view kStrConfigurationSettingMapperType = L"Type";


        // -------- RUN-TIME CONSTANTS ------------------------------------- //
        // Not safe to access before run-time, and should not be used to perform dynamic initialization.

        /// Product name.
        /// Use this to identify Xidi in areas of user interaction.
        extern const std::wstring_view kStrProductName;

        /// Version name.
        /// Use this to identify Xidi's version (dinput, dinput8, winmm) in areas of user interaction.
        extern const std::wstring_view kStrVersionName;

        /// Complete path and filename of the currently-running executable.
        extern const std::wstring_view kStrExecutableCompleteFilename;

        /// Base name of the currently-running executable.
        extern const std::wstring_view kStrExecutableBaseName;

        /// Directory name of the currently-running executable, including trailing backslash if available.
        extern const std::wstring_view kStrExecutableDirectoryName;

        /// Directory name in which system-supplied libraries are found.
        extern const std::wstring_view kStrSystemDirectoryName;

        /// Complete path and filename of the system-supplied DirectInput library.
        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput;

        /// Complete path and filename of the system-supplied DirectInput8 library.
        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput8;

        /// Complete path and filename of the system-supplied WinMM library.
        extern const std::wstring_view kStrSystemLibraryFilenameWinMM;

        /// Expected filename of a configuration file.
        /// Xidi configuration filename = (executable directory)\Xidi.ini
        extern const std::wstring_view kStrConfigurationFilename;

        /// Expected filename for the log file.
        /// Xidi log filename = (current user's desktop)\Xidi_(Xidi Version)_(base name of the running executable)_(process ID).log
        extern const std::wstring_view kStrLogFilename;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Generates a string representation of a system error code.
        /// @param [in] systemErrorCode System error code for which to generate a string.
        /// @return String representation of the system error code.
        std::wstring SystemErrorCodeString(const unsigned long systemErrorCode);
    }
}
