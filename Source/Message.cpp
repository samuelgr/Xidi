/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Message.cpp
 *   Message output implementation.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"
#include "Message.h"
#include "Strings.h"
#include "TemporaryBuffer.h"

#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <psapi.h>
#include <sal.h>
#include <shlobj.h>
#include <sstream>
#include <string>


namespace Xidi
{
    namespace Message
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Enumerates all supported modes of outputting messages.
        enum class EOutputMode
        {
            // Non-interactive output modes
            DebugString,                                                ///< Message is output using a debug string, which debuggers will display.
            LogFile,                                                    ///< Message is output to a log file.

            // Boundary value between non-interactive and interactive modes
            InteractiveBoundaryValue,                                   ///< Not used as a value, but separates non-interactive output modes from interactive output modes.

            // Interactive output modes
            GraphicalMessageBox,                                        ///< Message is output using a graphical message box.

            // Upper sentinel value
            UpperBoundValue,                                            ///< Not used as a value. One higher than the maximum possible value in this enumeration.
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// Handle to the log file, if enabled.
        static FILE* logFileHandle = nullptr;

        /// Specifies the minimum severity required to output a message.
        /// Messages below this severity (i.e. above the integer value that represents this severity) are not output.
        static ESeverity minimumSeverityForOutput = kDefaultMinimumSeverityForOutput;


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Checks if logging to a file is enabled.
        /// @return `true` if so, `false` if not.
        static inline bool IsLogFileEnabled(void)
        {
            return (nullptr != logFileHandle);
        }

        /// Checks if the specified output mode is interactive or non-interactive.
        /// @return `true` if the mode is interactive, `false` otherwise.
        static inline bool IsOutputModeInteractive(const EOutputMode outputMode)
        {
            return (outputMode > EOutputMode::InteractiveBoundaryValue);
        }

        /// Checks if the specified severity is forced interactive (i.e. one of the elements that will always cause a message to be emitted interactively).
        /// @return `true` if the severity is forced interactive, `false` otherwise.
        static inline bool IsSeverityForcedInteractive(const ESeverity severity)
        {
            return (severity < ESeverity::LowerBoundConfigurableValue);
        }

        /// Selects a character to represent each level of severity, for use when outputting messages.
        /// @param [in] severity Message severity.
        /// @return Character to use to represent it.
        static wchar_t CharacterForSeverity(const ESeverity severity)
        {
            switch (severity)
            {
            case ESeverity::ForcedInteractiveError:
            case ESeverity::Error:
                return L'E';

            case ESeverity::ForcedInteractiveWarning:
            case ESeverity::Warning:
                return L'W';

            case ESeverity::ForcedInteractiveInfo:
            case ESeverity::Info:
                return L'I';

            case ESeverity::Debug:
            case ESeverity::SuperDebug:
                return L'D';

            default:
                return L'?';
            }
        }

        /// Determines the appropriate modes of output based on the current configuration and message severity.
        /// @param [in] severity Severity of the message for which an output mode is being chosen.
        /// @param [out] selectedOutputModes Filled with the output modes that are selected. Array should have EOutputMode::UpperBoundValue elements.
        /// @return Number of output modes selected.
        static int DetermineOutputModes(const ESeverity severity, EOutputMode* selectedOutputModes)
        {
            int numOutputModes = 0;

            if (IsSeverityForcedInteractive(severity))
            {
                // If the severity level is forced interactive, then unconditionally enable an interactive output mode.
                // Other modes might also be enabled depending on the configuration.

                selectedOutputModes[numOutputModes++] = EOutputMode::GraphicalMessageBox;

                if (IsDebuggerPresent())
                    selectedOutputModes[numOutputModes++] = EOutputMode::DebugString;

                if (IsLogFileEnabled())
                    selectedOutputModes[numOutputModes++] = EOutputMode::LogFile;
            }
            else if (IsDebuggerPresent())
            {
                // If a debugger is present, #WillOutputMessageOfSeverity will always return `true`.
                // The goal is tn ensure that debug strings are sent for all messages irrespective of severity.
                // For other configured output modes, it is necessary to filter based on severity.

                selectedOutputModes[numOutputModes++] = EOutputMode::DebugString;

                if (severity <= minimumSeverityForOutput)
                {
                    if (IsLogFileEnabled())
                        selectedOutputModes[numOutputModes++] = EOutputMode::LogFile;
                }
            }
            else
            {
                // Since a debugger is not present, #WillOutputMessageOfSeverity has already validated that the severity of the message justifies outputting it.
                // It is therefore sufficient just to pick appropriate output modes depending on message subsystem configuration.
                // Prefer a log file if enabled, otherwise display an interactive message box.

                if (IsLogFileEnabled())
                    selectedOutputModes[numOutputModes++] = EOutputMode::LogFile;
                else
                    selectedOutputModes[numOutputModes++] = EOutputMode::GraphicalMessageBox;
            }

            return numOutputModes;
        }

        /// Outputs the specified message using a debug string.
        /// Requires both a severity and a message string.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        static void OutputInternalUsingDebugString(const ESeverity severity, const wchar_t* message)
        {
            std::wstringstream outputString;

            // First compose the output string stamp.
            // Desired format is "(product name)-(form name): [(severity)]"
            outputString << Strings::kStrProductName << L'-' << Strings::kStrFormName << L": [" << CharacterForSeverity(severity) << L"] ";

            // Append the message itself.
            outputString << message << L"\n";

            // Output to the debugger.
            OutputDebugString(outputString.str().c_str());
        }

        /// Outputs the specified message to the log file.
        /// Requires both a severity and a message string.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        static void OutputInternalUsingLogFile(const ESeverity severity, const wchar_t* message)
        {
            std::wstringstream outputString;

            // First compose the output string stamp.
            // Desired format is "[(current date) (current time)] [(severity)]"
            outputString << L'[';

            TemporaryBuffer<wchar_t> bufferTimestamp;

            if (0 != GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, nullptr, L"MM'/'dd'/'yyyy", bufferTimestamp, bufferTimestamp.Count(), nullptr))
                outputString << (wchar_t*)bufferTimestamp;
            else
                outputString << L"(date not available)";

            if (0 != GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, nullptr, L"HH':'mm':'ss", bufferTimestamp, bufferTimestamp.Count()))
                outputString << L' ' << (wchar_t*)bufferTimestamp;
            else
                outputString << L" (time not available)";

            // Finish up the stamp and append the message itself.
            outputString << L"] [" << CharacterForSeverity(severity) << "] " << message << L'\n';

            // Write to the log file.
            fputws(outputString.str().c_str(), logFileHandle);
            fflush(logFileHandle);
        }

        /// Outputs the specified message using a graphical message box.
        /// Requires both a severity and a message string.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        static void OutputInternalUsingMessageBox(const ESeverity severity, const wchar_t* message)
        {
            UINT messageBoxType = MB_SETFOREGROUND;

            switch (severity)
            {
            case ESeverity::ForcedInteractiveError:
            case ESeverity::Error:
                messageBoxType |= MB_ICONERROR;
                break;

            case ESeverity::ForcedInteractiveWarning:
            case ESeverity::Warning:
                messageBoxType |= MB_ICONWARNING;
                break;

            case ESeverity::ForcedInteractiveInfo:
            case ESeverity::Info:
                messageBoxType |= MB_ICONINFORMATION;
                break;

            default:
                messageBoxType |= MB_OK;
                break;
            }

            MessageBox(nullptr, message, Strings::kStrProductName.data(), messageBoxType);
        }

        /// Outputs the specified message.
        /// Requires both a severity and a message string.
        /// Concurrency-safe.
        /// @param [in] severity Severity of the message.
        /// @param [in] message Message text.
        static void OutputInternal(const ESeverity severity, const wchar_t* message)
        {
            static std::mutex outputGuard;
            
            EOutputMode outputModes[(int)EOutputMode::UpperBoundValue];
            const int numOutputModes = DetermineOutputModes(severity, outputModes);

            if (numOutputModes > 0)
            {
                std::scoped_lock lock(outputGuard);

                for (int i = 0; i < numOutputModes; ++i)
                {
                    switch (outputModes[i])
                    {
                    case EOutputMode::DebugString:
                        OutputInternalUsingDebugString(severity, message);
                        break;

                    case EOutputMode::LogFile:
                        OutputInternalUsingLogFile(severity, message);
                        break;

                    case EOutputMode::GraphicalMessageBox:
                        OutputInternalUsingMessageBox(severity, message);
                        break;

                    default:
                        break;
                    }
                }
            }
        }

        /// Formats and outputs some text of the given severity.
        /// @param [in] severity Severity of the message.
        /// @param [in] format Message string, possibly with format specifiers.
        /// @param [in] args Variable-length list of arguments to be used for any format specifiers in the message string.
        static void OutputFormattedInternal(const ESeverity severity, const wchar_t* format, va_list args)
        {
            TemporaryBuffer<wchar_t> messageBuf;

            vswprintf_s(messageBuf, messageBuf.Count(), format, args);
            OutputInternal(severity, messageBuf);
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Message.h" for documentation.

        void CreateAndEnableLogFile(void)
        {
            if (false == IsLogFileEnabled())
            {
                // Open the log file.
                if (0 != _wfopen_s(&logFileHandle, Strings::kStrLogFilename.data(), L"w"))
                {
                    logFileHandle = nullptr;
                    OutputFormatted(ESeverity::Error, L"%s - Unable to create log file.", Strings::kStrLogFilename.data());
                    return;
                }

                // Output the log file header.
                static constexpr wchar_t kLogHeaderSeparator[] = L"---------------------------------------------";
                fwprintf_s(logFileHandle, L"%s\n", kLogHeaderSeparator);
                fwprintf_s(logFileHandle, L"%s Log\n", Strings::kStrProductName.data());
                fwprintf_s(logFileHandle, L"%s\n", kLogHeaderSeparator);
                fwprintf_s(logFileHandle, L"Version:   %s\n", Globals::GetVersion().string.data());
                fwprintf_s(logFileHandle, L"Form:      %s\n", Strings::kStrFormName.data());
                fwprintf_s(logFileHandle, L"Program:   %s\n", Strings::kStrExecutableCompleteFilename.data());
                fwprintf_s(logFileHandle, L"PID:       %d\n", Globals::GetCurrentProcessId());
                fwprintf_s(logFileHandle, L"%s\n", kLogHeaderSeparator);
            }
        }

        // --------

        void Output(const ESeverity severity, const wchar_t* message)
        {
            const DWORD lastError = GetLastError();

            if (false == WillOutputMessageOfSeverity(severity))
            {
                SetLastError(lastError);
                return;
            }

            OutputInternal(severity, message);
            SetLastError(lastError);
        }

        // ---------

        void OutputFormatted(const ESeverity severity, _Printf_format_string_ const wchar_t* format, ...)
        {
            const DWORD lastError = GetLastError();

            if (false == WillOutputMessageOfSeverity(severity))
            {
                SetLastError(lastError);
                return;
            }

            va_list args;
            va_start(args, format);

            OutputFormattedInternal(severity, format, args);

            va_end(args);

            SetLastError(lastError);
        }

        // --------

        void SetMinimumSeverityForOutput(const ESeverity severity)
        {
            if (severity > ESeverity::LowerBoundConfigurableValue)
                minimumSeverityForOutput = severity;
        }

        // --------

        bool WillOutputMessageOfSeverity(const ESeverity severity)
        {
            // Messages of all severities are output unconditionally if a debugger is present.
            // #DetermineOutputModes takes care of selecting the appropriate modes, given the message severity.
            if (IsDebuggerPresent())
                return true;

            if ((severity < ESeverity::LowerBoundConfigurableValue) || (severity <= minimumSeverityForOutput))
            {
                // Counter-intuitive: severity *values* increase as severity *levels* decrease.
                // This is checking if the actual severity *level* is above (*value* is below) the highest severity *level* that requires output be non-interactive.
                // If so, then there is no requirement that output be non-interactive.
                if (severity < kMaximumSeverityToRequireNonInteractiveOutput)
                    return true;

                // Check all the selected output modes.
                // If any are interactive, then this message is skipped over.
                EOutputMode outputModes[(int)EOutputMode::UpperBoundValue];
                const int numOutputModes = DetermineOutputModes(severity, outputModes);

                for (int i = 0; i < numOutputModes; ++i)
                {
                    if (true == IsOutputModeInteractive(outputModes[i]))
                        return false;
                }

                return true;
            }
            else
                return false;
        }
    }
}
