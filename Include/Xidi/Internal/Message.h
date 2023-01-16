/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file Message.h
 *   Message output interface declaration.
 *****************************************************************************/

#pragma once

#include <sal.h>


namespace Xidi
{
    namespace Message
    {
        // -------- TYPE DEFINITIONS --------------------------------------- //

        /// Enumerates all supported severity levels for messages.
        /// These are primarily used to assist with output formatting.
        enum class ESeverity
        {
            // Forced interactive output
            ForcedInteractiveError,                                     ///< Error, always output interactively.
            ForcedInteractiveWarning,                                   ///< Warning, always output interactively.
            ForcedInteractiveInfo,                                      ///< Informational, always output interactively.

            // Lower bound value that is allowed to be configured.
            LowerBoundConfigurableValue,                                ///< Not used as a value, but separates forced interactive output from the configurable levels of optional output.

            // Optional output
            Error,                                                      ///< Error. Causes a change in behavior if encountered, possibly leading to application termination.
            Warning,                                                    ///< Warning. May cause a change in behavior but is not critical and will not terminate the application.
            Info,                                                       ///< Informational. Useful status-related remarks for tracking application behavior.
            Debug,                                                      ///< Debug. Detailed messages showing internal operations and state.
            SuperDebug,                                                 ///< Super Debug. Includes detailed messages as above but goes beyond to include message that can appear frequently and make the generated output large.

            // Upper sentinel value
            UpperBoundValue,                                            ///< Not used as a value. One higher than the maximum possible value in this enumeration.
        };


        // -------- CONSTANTS ---------------------------------------------- //

        /// Specifies the default minimum severity required to output a message.
        /// Messages below the configured minimum severity severity (i.e. above the integer value that represents this severity) are not output.
#ifdef _DEBUG
        static constexpr ESeverity kDefaultMinimumSeverityForOutput = ESeverity::Debug;
#else
        static constexpr ESeverity kDefaultMinimumSeverityForOutput = ESeverity::Error;
#endif

        /// Specifies the maximum severity that requires a non-interactive mode of output be used.
        /// Messages at this severity or lower will be skipped unless a non-interactive output mode is being used.
        static constexpr ESeverity kMaximumSeverityToRequireNonInteractiveOutput = ESeverity::Warning;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Attempts to create and enable the log file.
        /// Will generate an error message on failure.
        /// Once logging to a file is enabled, it cannot be disabled.
        void CreateAndEnableLogFile(void);

        /// Checks if logging to a file is enabled.
        /// @return `true` if so, `false` if not.
        bool IsLogFileEnabled(void);

        /// Outputs the specified message.
        /// Requires both a severity and a message string.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        void Output(const ESeverity severity, const wchar_t* message);

        /// Formats and outputs the specified message.
        /// Requires a severity, a message string with standard format specifiers, and values to be formatted.
        /// @param [in] severity Severity of the message.
        /// @param [in] format Message string, possibly with format specifiers.
        void OutputFormatted(const ESeverity severity, _Printf_format_string_ const wchar_t* format, ...);

        /// Sets the minimum message severity required for a message to be output.
        /// Does nothing if the requested severity level is less than the minimum value allowed to be configured.
        /// @param [in] severity New minimum severity setting.
        void SetMinimumSeverityForOutput(const ESeverity severity);

        /// Determines if a message of the specified severity will be output.
        /// Compares the supplied severity level to the configured minimum severity level.
        /// @param [in] severity Severity to test.
        /// @return `true` if a message of the specified severity should be output, `false` otherwise.
        bool WillOutputMessageOfSeverity(const ESeverity severity);
    }
}
