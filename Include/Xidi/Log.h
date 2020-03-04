/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file Log.h
 *   Logging interface declaration.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>


/// Produces a nicely-formatted string representation of the name of the current function.
/// Intended to be used for generating log messages.
#define XIDI_LOG_FORMATTED_FUNCTION_NAME        _T(__FUNCTION__ "()")


namespace Xidi
{
    /// Enumerates all supported levels for logging messages.
    /// Higher values indicate increased verbosity.
    /// Lower values indicate increased severity.
    enum ELogLevel : int
    {
        LogLevelForced                          = -1,                       ///< Forced message. Anything at or below this level can cause a log file to be created even if it otherwise would not be.

        LogLevelDisabled                        = 0,                        ///< Logging is disabled. Should not be used for individual log messages.
        LogLevelError                           = 1,                        ///< Error. Causes a change in behavior if encountered, possibly leading to application termination. Anything at or above this level is only written to a log file if it has otherwise been created.
        LogLevelWarning                         = 2,                        ///< Warning. May cause a change in behavior but is not critical and will not terminate the application.
        LogLevelInfo                            = 3,                        ///< Informational. Useful status-related remarks for tracking application and Xidi behavior.
        LogLevelDebug                           = 4,                        ///< Debug. Includes detailed messages to aid in troubleshooting application and Xidi behavior.

        LogLevelMaxConfigurableValue            = LogLevelDebug,            ///< Maximum configurable severity value for logging.
        LogLevelMinConfigurableValue            = LogLevelError,            ///< Minimum configurable severity value for logging.
    };
    
    /// Encapsulates all log-related functionality.
    /// All methods are class methods.
    class Log
    {
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //
        
        /// Log file handle. Used to write to the log file.
        static FILE* fileHandle;

        /// Configured minimum severity for log messages to be output.
        static ELogLevel configuredSeverity;

        /// Configured log mode, either enabled or disabled.
        static bool logEnabled;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        Log(void);
        
        
    public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        /// Applies a configuration setting that enables or disables the log.
        /// @param [in] value Setting value to be applied.
        /// @return `true` if setting could be applied, `false` otherwise.
        static bool ApplyConfigurationLogEnabled(bool value);

        /// Applies a configuration setting that sets the log level.
        /// @param [in] value Setting value to be applied.
        /// @return `true` if setting could be applied, `false` otherwise.
        static bool ApplyConfigurationLogLevel(int64_t value);
        
        /// Flushes and closes the log file.
        /// Idempotent.
        static void FinalizeLog(void);
        
        /// Retrieves the currently-configured minimum log severity.
        /// @return Configured minimum severity.
        static ELogLevel GetMinimumSeverity(void);
        
        /// Creates a log file and initializes this class.
        /// File name is determined internally.
        /// Called automatically before any log messages are written.
        /// Idempotent.
        static void InitializeAndCreateLog(void);
        
        /// Sets the currently-configured minimum log severity.
        /// @param [in] Severity to set.
        static void SetMinimumSeverity(ELogLevel severity);
        
        /// Determines if a message of the specified severity should be output to the log.
        /// Compares the supplied severity level to the configured minimum severity level.
        /// @param [in] severity Severity to test.
        /// @return `true` if a message of the specified severity should be written to the log, `false` otherwise.
        static bool WillOutputLogMessageOfSeverity(ELogLevel severity);
        
        /// Formats and writes the specified log message to the log, filtering based on specified and configured minimum severity.
        /// Requires a severity, a message string with standard format specifiers, and values to be formatted.
        /// Adds a timestamp to the start of the message and a line break at the end.
        /// @param [in] severity Severity of the message.
        /// @param [in] format Message string, possibly with format specifiers.
        static void WriteFormattedLogMessage(ELogLevel severity, LPCTSTR format, ...);
        
        /// Writes the specified log message to the log, filtering based on specified and configured minimum severity.
        /// Requires both a severity and a message string.
        /// Adds a timestamp to the start of the message and a line break at the end.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        static void WriteLogMessage(ELogLevel severity, LPCTSTR message);
        
        
    private:
        // -------- HELPERS -------------------------------------------------------- //
        
        /// Specifies if the log file is created and initialized.
        /// @return `true` if so, `false` if not.
        static bool IsLogReady(void);
        
        /// Formats and outputs a single log line.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        static void LogLineOutputString(ELogLevel severity, LPCTSTR message);

        /// Formats and outputs a single log line, with support for format specifiers.
        /// @param [in] severity Severity of the message.
        /// @param [in] format Message string, possibly with format specifiers.
        /// @param [in] args Variable-length list of arguments to be used for any format specifiers in the message string.
        static void LogLineOutputFormat(ELogLevel severity, LPCTSTR format, va_list args);
        
        /// Formats and outputs some text to the log.
        /// Will cause lazy initialization of the log if invoked when the log is not yet created or initialized.
        /// @param [in] format Message string, possibly with format specifiers.
        /// @param [in] args Variable-length list of arguments to be used for any format specifiers in the message string.
        static void OutputFormattedText(LPCTSTR format, va_list args);
        
        /// Outputs some text to the log.
        /// Will cause lazy initialization of the log if invoked when the log is not yet created or initialized.
        /// @param [in] message Message text.
        static void OutputText(LPCTSTR message);
        
        /// Outputs a log stamp, which includes a date and time plus a severity indicator.
        /// Invoked to write the beginning part of each log message line.
        /// @param [in] severity Severity of the stamp to generate.
        static void OutputStamp(ELogLevel severity);
    };
}
