/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Message.h
 *   Message output interface declaration.
 *****************************************************************************/

#pragma once


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

            // Boundary value between forced interactive and optional output
            ForcedInteractiveBoundaryValue,                             ///< Not used as a value, but separates forced interactive output from optional output.

            // Optional output
            Error,                                                      ///< Error. Causes a change in behavior if encountered, possibly leading to application termination.
            Warning,                                                    ///< Warning. May cause a change in behavior but is not critical and will not terminate the application.
            Info,                                                       ///< Informational. Useful status-related remarks for tracking application behavior.
            Debug,                                                      ///< Debug. Only output in debug mode, and then only written to an attached debugger.
            SuperDebug,                                                 ///< Super Debug. Includes detailed messages as above but goes beyond to include calls that happen frequently, making the logs large.

            // Upper sentinel value
            UpperBoundValue,                                            ///< Not used as a value. One higher than the maximum possible value in this enumeration.
        };


        // -------- CONSTANTS ---------------------------------------------- //

        /// Specifies the default minimum severity required to output a message.
        /// Messages below the configured minimum severity severity (i.e. above the integer value that represents this severity) are not output.
#if _DEBUG
        static constexpr ESeverity kDefaultMinimumSeverityForOutput = ESeverity::Debug;
#else
        static constexpr ESeverity kDefaultMinimumSeverityForOutput = ESeverity::Error;
#endif

        /// Specifies the maximum severity that requires a non-interactive mode of output be used.
        /// Messages at this severity or lower will be skipped unless a non-intneractive output mode is being used.
        static constexpr ESeverity kMaximumSeverityToRequireNonInteractiveOutput = ESeverity::Warning;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Attempts to create and enable the log file.
        /// Will generate an error message on failure.
        /// Once logging to a file is enabled, it cannot be disabled.
        void CreateAndEnableLogFile(void);

        /// Outputs the specified message.
        /// Requires both a severity and a message string.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        void Output(const ESeverity severity, const wchar_t* message);

        /// Formats and outputs the specified message.
        /// Requires a severity, a message string with standard format specifiers, and values to be formatted.
        /// @param [in] severity Severity of the message.
        /// @param [in] format Message string, possibly with format specifiers.
        void OutputFormatted(const ESeverity severity, const wchar_t* format, ...);

        /// Sets the minimum message severity required for a message to be output.
        /// Compares the supplied severity level to ESeverity::ForcedInteractiveBoundaryValue and, if not greater, sums them together and adds 1.
        /// The effect of doing this is to map from forced severities to unforced severities (i.e. input is ESeverity::ForcedInteractiveError, actual minimum severity is ESeverity::Error).
        /// @param [in] severity New minimum severity setting.
        void SetMinimumSeverityForOutput(const ESeverity severity);

        /// Determines if a message of the specified severity will be output.
        /// Compares the supplied severity level to the configured minimum severity level.
        /// @param [in] severity Severity to test.
        /// @return `true` if a message of the specified severity should be output, `false` otherwise.
        bool WillOutputMessageOfSeverity(const ESeverity severity);
    }
}
