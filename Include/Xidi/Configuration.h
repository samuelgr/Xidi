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

#include <string>
#include <unordered_map>


namespace Xidi
{
    // Enumerates all supported configuration value types.
    // Used to specify how to parse a configuration value.
    enum EConfigurationValueType
    {
        ConfigurationValueTypeInteger           = 1,                    // Signed integer
        ConfigurationValueTypeBoolean,                                  // Boolean
        ConfigurationValueTypeString                                    // String
    };

    // Encapsulates all configuration-related functionality.
    // All methods are class methods.
    class Configuration
    {
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Defines the supported values in the "Log" section of the configuration file.
        static std::unordered_map<std::string, EConfigurationValueType> logSettings;

        // Defines the supported values in the "Mapper" section of the configuration file.
        static std::unordered_map<std::string, EConfigurationValueType> mapperSettings;

        // Defines the supported sections of the configuration file.
        static std::unordered_map<std::string, std::unordered_map<std::string, EConfigurationValueType>&> configurationLayout;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        Configuration();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Parses and applies a configuration file, whose location is determined internally.
        static void parseAndApplyConfigurationFile(void);


    //private:
        // -------- HELPERS -------------------------------------------------------- //

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
    };
}
