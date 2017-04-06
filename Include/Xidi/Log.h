/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * Log.h
 *      Logging interface declaration.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>


namespace Xidi
{
    // Enumerates all supported levels for logging messages.
    // Higher values indicate increased verbosity.
    // Lower values indicate increased severity.
    enum ELogLevel : int
    {
        LogLevelForced                          = -1,                   // Forced message. Anything at or below this level can cause a log file to be created even if it otherwise would not be.

        LogLevelDisabled                        = 0,                    // Logging is disabled. Should not be used for individual log messages.
        LogLevelError                           = 1,                    // Error. Causes a change in behavior if encountered, possibly leading to application termination. Anything at or above this level is only written to a log file if it has otherwise been created.
        LogLevelWarning                         = 2,                    // Warning. May cause a change in behavior but is not critical and will not terminate the application.
        LogLevelInfo                            = 3,                    // Informational. Useful status-related remarks for tracking application and Xidi behavior.
        LogLevelDebug                           = 4,                    // Debug. Includes detailed messages to aid in troubleshooting application and Xidi behavior.

        LogLevelMaxConfigurableValue            = LogLevelDebug,        // Maximum configurable severity value for logging.
        LogLevelMinConfigurableValue            = LogLevelError,        // Minimum configurable severity value for logging.
    };
    
    // Encapsulates all log-related functionality.
    // All methods are class methods.
    class Log
    {
    private:
        // -------- CONSTANTS ------------------------------------------------------ //
        
        // Buffer size, in characters, for the temporary buffer to hold string messages read using a resource identifier.
        // When writing log messages using a resource identifier (rather than a raw string), a temporary buffer is created to hold the loaded resource string.
        static const size_t kLogResourceBufferSize = 1024;

        
        // -------- CLASS VARIABLES ------------------------------------------------ //
        
        // Log file handle. Used to write to the log file.
        static FILE* fileHandle;

        // Configured minimum severity for log messages to be output.
        static ELogLevel configuredSeverity;

        // Configured log mode, either enabled or disabled.
        static bool logEnabled;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        Log();
        
        
    public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        // Applies a configuration setting that enables or disables the log.
        static bool ApplyConfigurationLogEnabled(bool value);

        // Applies a configuration setting that sets the log level.
        static bool ApplyConfigurationLogLevel(int64_t value);
        
        // Flushes and closes the log file.
        // Idempotent.
        static void FinalizeLog(void);
        
        // Retrieves the currently-configured minimum log severity.
        static ELogLevel GetMinimumSeverity(void);
        
        // Creates a log file and initializes this class.
        // File name is determined internally.
        // Called automatically before any log messages are written.
        // Idempotent.
        static void InitializeAndCreateLog(void);
        
        // Sets the currently-configured minimum log severity.
        static void SetMinimumSeverity(ELogLevel severity);
        
        // Formats and writes the specified log message to the log, filtering based on specified and configured minimum severity.
        // Requires a severity, a message string with standard format specifiers, and values to be formatted.
        // Adds a timestamp to the start of the message and a line break at the end.
        static void WriteFormattedLogMessage(ELogLevel severity, LPTSTR format, ...);

        // Formats and writes the specified log message to the log, filtering based on specified and configured minimum severity.
        // Requires a severity, a resource identifier that refers to a string containing standard format specifiers, and values to be formatted.
        // Adds a timestamp to the start of the message and a line break at the end.
        static void WriteFormattedLogMessageFromResource(ELogLevel severity, unsigned int resourceIdentifier, ...);
        
        // Writes the specified log message to the log, filtering based on specified and configured minimum severity.
        // Requires both a severity and a message string.
        // Adds a timestamp to the start of the message and a line break at the end.
        static void WriteLogMessage(ELogLevel severity, LPTSTR message);

        // Writes the specified log message to the log, filtering based on specified and configured minimum severity.
        // Requires both a severity and a resource identifier, which identifies the string resource that contains the message to be logged.
        // Adds a timestamp to the start of the message and a line break at the end.
        static void WriteLogMessageFromResource(ELogLevel severity, unsigned int resourceIdentifier);
        
        
    private:
        // -------- HELPERS -------------------------------------------------------- //
        
        // Specifies if the log file is created and initialized.
        static bool IsLogReady(void);
        
        // Formats and outputs a single log line, given a severity and a string to output as the message.
        static void LogLineOutputString(ELogLevel severity, LPTSTR message);

        // Formats and outputs a single log line, with support for format specifiers, given a severity, a format string, and a list of arguments.
        static void LogLineOutputFormat(ELogLevel severity, LPTSTR format, va_list args);
        
        // Formats and outputs a log message.
        // Will cause lazy initialization of the log if invoked when the log is not yet created or initialized.
        static void OutputFormattedLogMessage(LPTSTR format, va_list args);
        
        // Outputs a log message.
        // Will cause lazy initialization of the log if invoked when the log is not yet created or initialized.
        static void OutputLogMessage(LPTSTR message);
        
        // Outputs a log stamp, which includes a date and time plus a severity indicator.
        // Invoked to write the beginning part of each log message line.
        static void OutputStamp(ELogLevel severity);
        
        // Determines if a message of the specified severity should be output to the log.
        static bool ShouldOutputLogMessageOfSeverity(ELogLevel severity);
    };
}
