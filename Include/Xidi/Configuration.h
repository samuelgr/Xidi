/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * Configuration.h
 *      Declaration of configuration file functionality.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "ApiStdString.h"

#include <unordered_map>


namespace Xidi
{
    // Enumerates all supported configuration value types.
    // Used to specify how to parse a configuration value.
    enum EConfigurationValueType
    {
        ConfigurationValueTypeInteger,                                  // Signed integer
        ConfigurationValueTypeBoolean,                                  // Boolean
        ConfigurationValueTypeString                                    // String
    };
    
    // Enumerates all possible types of configuration file lines.
    // Used during parsing to classify each line encountered.
    enum EConfigurationLineType
    {
        ConfigurationLineTypeIgnore,                                    // Line should be ignored, either because it is just whitespace or because it is a comment
        ConfigurationLineTypeSection,                                   // Line begins a section, whose name appears in square brackets
        ConfigurationLineTypeValue,                                     // Line is a value within the current section and so should be parsed
        ConfigurationLineTypeError                                      // Line could not be parsed
    };

    // Holds the type and applicator function for configuration values.
    struct SConfigurationValueApplyInfo
    {
        EConfigurationValueType type;                                   // Type of the value, used to specify how to interpret it.
        void* applyFunc;                                                // Pointer to the function to call when applying the setting. Must match the correct signature for the specified type.
    };
    
    // Specifies the signature for a function that accepts an integer-valued setting.
    typedef bool(*TFuncApplyIntSetting)(int64_t value);

    // Specifies the signature for a function that accepts a Boolean-valued setting.
    typedef bool(*TFuncApplyBoolSetting)(bool value);

    // Specifies the signature for a function that accepts a string-valued setting.
    typedef bool(*TFuncApplyStringSetting)(const StdString& value);


    // Encapsulates all configuration-related functionality.
    // All methods are class methods.
    class Configuration
    {
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Defines the supported values in the "Log" section of the configuration file.
        static std::unordered_map<StdString, SConfigurationValueApplyInfo> logSettings;

        // Defines the supported values in the "Mapper" section of the configuration file.
        static std::unordered_map<StdString, SConfigurationValueApplyInfo> mapperSettings;

        // Defines the supported sections of the configuration file.
        static std::unordered_map<StdString, std::unordered_map<StdString, SConfigurationValueApplyInfo>*> configurationSections;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        Configuration();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Parses and applies a configuration file, whose location is determined internally.
        static void ParseAndApplyConfigurationFile(void);


    private:
        // -------- HELPERS -------------------------------------------------------- //
        
        // Classifies the provided configuration file line and returns a value indicating the result.
        static EConfigurationLineType ClassifyConfigurationFileLine(LPCTSTR buf, const size_t length);

        // Extracts a name and a value for the specified configuration file line, which must first have been classified as containing a value.
        // Modifies the buffer by changing a value to the NULL character, indicating the end of the portion of interest in the input configuration file line.
        static void ExtractNameValuePairFromConfigurationFileLine(StdString& name, StdString& value, LPTSTR configFileLine);
        
        // Extracts a section name from the specified configuration file line, which must first have been classified as containing a section name.
        // Modifies the buffer by changing a value to the NULL character, indicating the end of the portion of interest in the input configuration file line.
        static void ExtractSectionNameFromConfigurationFileLine(StdString& sectionName, LPTSTR configFileLine);

        // Parses a signed integer value from the supplied input string.
        // Returns TRUE if the parse was successful and able to consume the whole string, FALSE otherwise.
        // Sets the destination parameter's value to the result of the parse, although it remains unmodified on parse failure.
        static bool ParseIntegerValue(int64_t& dest, const StdString& source);

        // Parses a Boolean from the supplied input string.
        // Returns TRUE if the parse was successful and able to consume the whole string, FALSE otherwise.
        // Sets the destination parameter's value to the result of the parse, although it remains unmodified on parse failure.
        static bool ParseBooleanValue(bool& dest, const StdString& source);

        // Reads a single line from the specified file handle, verifies that it fits within the specified buffer, and removes the trailing newline.
        // Returns the length of the string that was read, with negative indicating an error condition.
        static int ReadAndTrimSingleLine(LPTSTR buf, const size_t count, FILE* filehandle);


        // -------- APPLICATION-SPECIFIC METHODS ----------------------------------- //
        
        // Fills in the specified buffer with the file name of the configuration file to use.
        // Returns the number of characters written to the buffer.
        static size_t GetConfigurationFilePath(LPTSTR buf, const size_t count);

        // Handles an error related to being unable to open a configuration file.
        // The filename is passed as a parameter for the purpose of generating a suitable error message.
        static void HandleErrorCannotOpenConfigurationFile(LPCTSTR filename);

        // Handles an error related to being unable to parse a specific line of the configuration file.
        // The filename and line number are both passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorCannotParseConfigurationFileLine(LPCTSTR filename, const DWORD linenum);

        // Handles an error related to a section present in the configuration file multiple times.
        // The filename and section name are passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorDuplicateConfigurationSection(LPCTSTR filename, LPCTSTR section);

        // Handles an error related to a section present in the configuration file that is unsupported by this application.
        // The filename and section name are passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorUnsupportedConfigurationSection(LPCTSTR filename, LPCTSTR section);

        // Handles an error due to a configuration file's line length being too long.
        // The filename and line number are both passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorLineTooLong(LPCTSTR filename, const DWORD linenum);

        // Handles a semantic error in which a value is specified outside of a section.
        // The filename and line number are both passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorValueOutsideSection(LPCTSTR filename, const DWORD linenum);

        // Handles a semantic error in which a value is specified multiple times in a section.
        // The filename, line number, section name, and value name are all passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorDuplicateValue(LPCTSTR filename, const DWORD linenum, LPCTSTR section, LPCTSTR value);

        // Handles a semantic error in which a value is specified in a section and its name is recognized but its type is malformed and the value could not be parsed.
        // The file name, line number, section name, and value name are all passed as parameters for teh purpose of generating a suitable error message.
        static void HandleErrorMalformedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR section, LPCTSTR value);
        
        // Handles a semantic error in which a value is specified in a section that does not recognize that value.
        // The filename, line number, section name, and value name are all passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorUnsupportedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR section, LPCTSTR value);

        // Handles a semantic error in which a value is parsed correctly but is rejected by the function that is supposed to apply it.
        // The filename, line number, setting value, section name, and value are all passed as parameters for the purpose of generating a suitable error message.
        static void HandleErrorCannotApplyValue(LPCTSTR filename, const DWORD linenum, LPCTSTR setting, LPCTSTR section, LPCTSTR value);
        
        // Handles file I/O errors while reading the configuration file.
        static void HandleErrorFileIO(LPCTSTR filename);

        // Handles a miscellaneous internal error related to being unable to read the configuration file.
        // The code should be presented to the user.
        static void HandleErrorInternal(const DWORD code);
        
        // Handles a success case in which a value is parsed correctly and successfully applied.
        // The filename, line number, setting value, section name, and value are all passed as parameters for the purpose of generating a suitable message.
        static void HandleSuccessAppliedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR setting, LPCTSTR section, LPCTSTR value);
    };
}
