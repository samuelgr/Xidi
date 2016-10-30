/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
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

    // Encapsulates all configuration-related functionality.
    // All methods are class methods.
    class Configuration
    {
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Defines the supported values in the "Log" section of the configuration file.
        static std::unordered_map<StdString, EConfigurationValueType> logSettings;

        // Defines the supported values in the "Mapper" section of the configuration file.
        static std::unordered_map<StdString, EConfigurationValueType> mapperSettings;

        // Defines the supported sections of the configuration file.
        static std::unordered_map<StdString, std::unordered_map<StdString, EConfigurationValueType>*> configurationSections;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        Configuration();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Parses and applies a configuration file, whose location is determined internally.
        static void parseAndApplyConfigurationFile(void);


    private:
        // -------- HELPERS -------------------------------------------------------- //

        // Classifies the provided configuration file line and returns a value indicating the result.
        static EConfigurationLineType classifyConfigurationFileLine(LPTSTR buf, const size_t length);

        // Extracts a section name from the specified configuration file line, which must first have been classified as containing a section name.
        static void extractSectionNameFromConfigurationFileLine(StdString& sectionName, LPTSTR configFileLine);
        
        // Fills in the specified buffer with the file name of the configuration file to use.
        // Returns the number of characters written to the buffer.
        static size_t getConfigurationFilePath(LPTSTR buf, const size_t count);

        // Handles an error related to being unable to open a configuration file.
        // The filename is passed as a parameter for the purpose of generating a suitable error message.
        static void handleErrorCannotOpenConfigurationFile(LPTSTR filename);

        // Handles an error related to being unable to parse a specific line of the configuration file.
        // The filename and line number are both passed as parameters for the purpose of generating a suitable error message.
        static void handleErrorCannotParseConfigurationFileLine(LPTSTR filename, const DWORD linenum);

        // Handles a miscellaneous internal error related to being unable to read the configuration file.
        // The code should be presented to the user.
        static void handleErrorInternal(const DWORD code);

        // Reads a single line from the specified file handle, verifies that it fits within the specified buffer, and removes the trailing newline.
        // Returns the length of the string that was read, with negative indicating an error condition.
        static int readAndTrimSingleLine(LPTSTR buf, const size_t count, FILE* filehandle);
    };
}
