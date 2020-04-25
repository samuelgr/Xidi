/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Configuration.h
 *   Declaration of configuration file functionality.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <string>
#include <unordered_map>


namespace Xidi
{
    /// Enumerates all supported configuration value types.
    /// Used to specify how to parse a configuration value.
    enum EConfigurationValueType
    {
        ConfigurationValueTypeInteger,                                  ///< Signed integer
        ConfigurationValueTypeBoolean,                                  ///< Boolean
        ConfigurationValueTypeString                                    ///< String
    };

    /// Enumerates all possible types of configuration file lines.
    /// Used during parsing to classify each line encountered.
    enum EConfigurationLineType
    {
        ConfigurationLineTypeIgnore,                                    ///< Line should be ignored, either because it is just whitespace or because it is a comment
        ConfigurationLineTypeSection,                                   ///< Line begins a section, whose name appears in square brackets
        ConfigurationLineTypeValue,                                     ///< Line is a value within the current section and so should be parsed
        ConfigurationLineTypeError                                      ///< Line could not be parsed
    };

    /// Holds the type and applicator function for configuration values.
    struct SConfigurationValueApplyInfo
    {
        EConfigurationValueType type;                                   ///< Type of the value, used to specify how to interpret it.
        void* applyFunc;                                                ///< Pointer to the function to call when applying the setting. Must match the correct signature for the specified type.
    };

    /// Specifies the signature for a function that accepts an integer-valued setting.
    typedef bool(*TFuncApplyIntSetting)(int64_t value);

    /// Specifies the signature for a function that accepts a Boolean-valued setting.
    typedef bool(*TFuncApplyBoolSetting)(bool value);

    /// Specifies the signature for a function that accepts a string-valued setting.
    typedef bool(*TFuncApplyStringSetting)(const std::wstring& value);


    /// Encapsulates all configuration-related functionality.
    /// All methods are class methods.
    class Configuration
    {
    private:
        // -------- CONSTANTS ------------------------------------------------------ //

        /// Specifies the maximum length of a full configuration file path.
        static const size_t kMaximumConfigurationFilePathLength = 2048;

        /// Specifies the maximum length of a configuration file's file name (i.e. the part after the directory name).
        static const size_t kMaximumConfigurationFileNameLength = 32;

        /// Specifies the maximum length of a line in the configuration file.
        static const size_t kMaximumConfigurationLineLength = 2048;


        // -------- CLASS VARIABLES ------------------------------------------------ //

        /// Defines the supported values in the "Import" section of the configuration file.
        static std::unordered_map<std::wstring, SConfigurationValueApplyInfo> importSettings;

        /// Defines the supported values in the "Log" section of the configuration file.
        static std::unordered_map<std::wstring, SConfigurationValueApplyInfo> logSettings;

        /// Defines the supported values in the "Mapper" section of the configuration file.
        static std::unordered_map<std::wstring, SConfigurationValueApplyInfo> mapperSettings;

        /// Defines the supported sections of the configuration file.
        static std::unordered_map<std::wstring, std::unordered_map<std::wstring, SConfigurationValueApplyInfo>*> configurationSections;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        Configuration(void);


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        /// Parses and applies a configuration file, whose location is determined internally.
        static void ParseAndApplyConfigurationFile(void);


    private:
        // -------- HELPERS -------------------------------------------------------- //

        /// Classifies the provided configuration file line and returns a value indicating the result.
        /// @param [in] buf Buffer containing the configuration file line.
        /// @param [in] length Number of characters in the buffer.
        /// @return Configuration line classification.
        static EConfigurationLineType ClassifyConfigurationFileLine(LPCWSTR buf, const size_t length);

        /// Extracts a name and a value for the specified configuration file line, which must first have been classified as containing a value.
        /// Modifies the buffer by changing a value to the NULL character, indicating the end of the portion of interest in the input configuration file line.
        /// @param [out] name Filled with the name of the configuration setting.
        /// @param [out] value Filled with the value specified for the configuration setting.
        /// @param [in] configFileLine Buffer containing the configuration file line.
        static void ExtractNameValuePairFromConfigurationFileLine(std::wstring& name, std::wstring& value, LPWSTR configFileLine);

        /// Extracts a section name from the specified configuration file line, which must first have been classified as containing a section name.
        /// Modifies the buffer by changing a value to the NULL character, indicating the end of the portion of interest in the input configuration file line.
        /// @param [out] sectionName Filled with the name of the configuration section.
        /// @param [in] configFileLine Buffer containing the configuration file line.
        static void ExtractSectionNameFromConfigurationFileLine(std::wstring& sectionName, LPWSTR configFileLine);

        /// Tests if the supplied character is allowed as a value name (the part before the '=' sign in the configuration file).
        /// @param [in] charToTest Character to test.
        /// @return Nonzero if true or zero if false (similar interface to cctype functions).
        static int IsAllowedValueNameCharacter(const wchar_t charToTest);

        /// Tests if the supplied character is allowed as a value setting (the part after the '=' sign in the configuration file).
        /// @param [in] charToTest Character to test.
        /// @return Nonzero if true or zero if false (similar interface to cctype functions).
        static int IsAllowedValueSettingCharacter(const wchar_t charToTest);

        /// Parses a signed integer value from the supplied input string.
        /// Sets the destination parameter's value to the result of the parse, although it remains unmodified on parse failure.
        /// @param [out] dest Filled with the result of the parse.
        /// @param [in] source String from which to parse.
        /// @return `TRUE` if the parse was successful and able to consume the whole string, `FALSE` otherwise.
        static bool ParseIntegerValue(int64_t& dest, const std::wstring& source);

        /// Parses a Boolean from the supplied input string.
        /// Sets the destination parameter's value to the result of the parse, although it remains unmodified on parse failure.
        /// @param [out] dest Filled with the result of the parse.
        /// @param [in] source String from which to parse.
        /// @return `TRUE` if the parse was successful and able to consume the whole string, `FALSE` otherwise.
        static bool ParseBooleanValue(bool& dest, const std::wstring& source);

        /// Reads a single line from the specified file handle, verifies that it fits within the specified buffer, and removes the trailing newline.
        /// @param [out] buf Filled with text from the specified file.
        /// @param [in] count Number of characters the buffer can hold.
        /// @param [in] filehandle Handle to the file from which to read.
        /// @return Length of the string that was read, with negative indicating an error condition.
        static int ReadAndTrimSingleLine(LPWSTR buf, const int count, FILE* filehandle);


        // -------- APPLICATION-SPECIFIC METHODS ----------------------------------- //

        /// Fills in the specified buffer with the file name of the configuration file to use.
        /// @param [out] buf Buffer to fill.
        /// @param [in] count Number of characters that fit in the buffer.
        /// @return Number of characters written to the buffer.
        static size_t GetConfigurationFilePath(LPWSTR buf, const DWORD count);

        /// Handles an error related to being unable to open a configuration file.
        /// @param [in] filename String containing the name of the configuration file.
        static void HandleErrorCannotOpenConfigurationFile(LPCWSTR filename);

        /// Handles an error related to being unable to parse a specific line of the configuration file.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        static void HandleErrorCannotParseConfigurationFileLine(LPCWSTR filename, const DWORD linenum);

        /// Handles an error related to a section present in the configuration file multiple times.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] section Name of the affected configuration section.
        static void HandleErrorDuplicateConfigurationSection(LPCWSTR filename, LPCWSTR section);

        /// Handles an error related to a section present in the configuration file that is unsupported by this application.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] section Name of the affected configuration section.
        static void HandleErrorUnsupportedConfigurationSection(LPCWSTR filename, LPCWSTR section);

        /// Handles an error due to a configuration file's line length being too long.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        static void HandleErrorLineTooLong(LPCWSTR filename, const DWORD linenum);

        /// Handles a semantic error in which a value is specified outside of a section.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        static void HandleErrorValueOutsideSection(LPCWSTR filename, const DWORD linenum);

        /// Handles a semantic error in which a value is specified multiple times in a section.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        /// @param [in] section Name of the affected configuration section.
        /// @param [in] value Value specified in the configuration file.
        static void HandleErrorDuplicateValue(LPCWSTR filename, const DWORD linenum, LPCWSTR section, LPCWSTR value);

        /// Handles a semantic error in which a value is specified in a section and its name is recognized but its type is malformed and the value could not be parsed.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        /// @param [in] section Name of the affected configuration section.
        /// @param [in] value Value specified in the configuration file.
        static void HandleErrorMalformedValue(LPCWSTR filename, const DWORD linenum, LPCWSTR section, LPCWSTR value);

        /// Handles a semantic error in which a value is specified in a section that does not recognize that value.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        /// @param [in] section Name of the affected configuration section.
        /// @param [in] value Value specified in the configuration file.
        static void HandleErrorUnsupportedValue(LPCWSTR filename, const DWORD linenum, LPCWSTR section, LPCWSTR value);

        /// Handles a semantic error in which a value is parsed correctly but is rejected by the function that is supposed to apply it.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        /// @param [in] setting Name of the affected configuration setting.
        /// @param [in] section Name of the affected configuration section.
        /// @param [in] value Value specified in the configuration file.
        static void HandleErrorCannotApplyValue(LPCWSTR filename, const DWORD linenum, LPCWSTR setting, LPCWSTR section, LPCWSTR value);

        /// Handles file I/O errors while reading the configuration file.
        /// @param [in] filename String containing the name of the configuration file.
        static void HandleErrorFileIO(LPCWSTR filename);

        /// Handles a miscellaneous internal error related to being unable to read the configuration file.
        /// The code should be presented to the user.
        /// @param [in] code Error code.
        static void HandleErrorInternal(const DWORD code);

        /// Handles a success case in which a value is parsed correctly and successfully applied.
        /// @param [in] filename String containing the name of the configuration file.
        /// @param [in] linenum Configuration file line number at which the error was encountered.
        /// @param [in] setting Name of the affected configuration setting.
        /// @param [in] section Name of the affected configuration section.
        /// @param [in] value Value specified in the configuration file.
        static void HandleSuccessAppliedValue(LPCWSTR filename, const DWORD linenum, LPCWSTR setting, LPCWSTR section, LPCWSTR value);
    };
}
