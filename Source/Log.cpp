/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Log.cpp
 *      Logging functionality implementation.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"
#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <ShlObj.h>


using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Log.h" for documentation.

FILE* Log::fileHandle = NULL;

ELogLevel Log::configuredSeverity = LogLevelDisabled;


// -------- CLASS METHODS -------------------------------------------------- //
// See "Log.h" for documentation.

void Log::FinalizeLog(void)
{
    if (IsLogReady())
    {
        fclose(fileHandle);
        fileHandle = NULL;
    }
}

// ---------

ELogLevel Log::GetMinimumSeverity(void)
{
    return configuredSeverity;
}

// ---------

void Log::InitializeAndCreateLog(void)
{
    if (!IsLogReady())
    {
        // Determine the file path for the log file.
        // It will be placed on the current user's desktop.
        PWSTR folderPath;
        HRESULT result = SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &folderPath);
        if (S_OK != result) return;
        
        WCHAR logFileName[32];
        WCHAR logFilePath[2048];
        wcscpy_s(logFilePath, _countof(logFilePath) - _countof(logFileName) - 2, folderPath);
        CoTaskMemFree(folderPath);

        LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_LOG_FILE_NAME, logFileName, _countof(logFileName));
        wcsncat_s(logFilePath, _countof(logFilePath) - _countof(logFileName) - 1, L"\\", 1);
        wcsncat_s(logFilePath, _countof(logFilePath), logFileName, _countof(logFileName));

        // Open the log file for writing.
        _wfopen_s(&fileHandle, logFilePath, L"w");
        if (NULL == fileHandle) return;

        // Write out the log file header.
        TCHAR logHeaderStringBuffer[1024];
        
        // Header part 1: Xidi version name.
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_VERSION_NAME, logHeaderStringBuffer, _countof(logHeaderStringBuffer));
        OutputLogMessage(logHeaderStringBuffer);
        OutputLogMessage(_T("\n"));

        // Header part 2: application file name
        if (0 != GetModuleFileName(NULL, logHeaderStringBuffer, _countof(logHeaderStringBuffer)))
        {
            OutputLogMessage(logHeaderStringBuffer);
            OutputLogMessage(_T("\n"));
        }

        // Header part 3: separator
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_FILE_SEPARATOR, logHeaderStringBuffer, _countof(logHeaderStringBuffer));
        OutputLogMessage(logHeaderStringBuffer);
        OutputLogMessage(_T("\n"));
    }
}

// ---------

void Log::SetMinimumSeverity(ELogLevel severity)
{
    if (severity >= LogLevelMinConfigurableValue && severity <= LogLevelMaxConfigurableValue)
    {
        configuredSeverity = severity;
    }
}

// ---------

void Log::WriteFormattedLogMessage(ELogLevel severity, LPTSTR format, ...)
{
    if (ShouldOutputLogMessageOfSeverity(severity))
    {
        va_list args;
        va_start(args, format);

        OutputStamp(severity);
        OutputFormattedLogMessage(format, args);
        OutputLogMessage(_T("\n"));

        va_end(args);
    }
}

// ---------

void Log::WriteLogMessage(ELogLevel severity, LPTSTR message)
{
    if (ShouldOutputLogMessageOfSeverity(severity))
    {
        OutputStamp(severity);
        OutputLogMessage(message);
        OutputLogMessage(_T("\n"));
    }
}


// -------- HELPERS -------------------------------------------------------- //
// See "Log.h" for documentation.

bool Log::IsLogReady(void)
{
    return (NULL != fileHandle);
}

// ---------

void Log::OutputFormattedLogMessage(LPTSTR format, va_list args)
{
    if (!IsLogReady())
    {
        InitializeAndCreateLog();
    }

    if (IsLogReady())
    {
        _vftprintf_s(fileHandle, format, args);
    }
}

// ---------

void Log::OutputLogMessage(LPTSTR message)
{
    if (!IsLogReady())
    {
        InitializeAndCreateLog();
    }

    if (IsLogReady())
    {
        _fputts(message, fileHandle);
    }
}

// ---------

void Log::OutputStamp(ELogLevel severity)
{
    TCHAR stampStringBuffer[1024];

    // Stamp part 1: open square bracket
    OutputLogMessage(_T("["));

    // Stamp part 2: date
    if (0 != GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, stampStringBuffer, _countof(stampStringBuffer), NULL))
    {
        OutputLogMessage(stampStringBuffer);
        OutputLogMessage(_T(" "));
    }

    // Stamp part 3: time
    if (0 != GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, NULL, NULL, stampStringBuffer, _countof(stampStringBuffer)))
    {
        OutputLogMessage(stampStringBuffer);
    }

    // Stamp part 4: separation between date/time and severity
    OutputLogMessage(_T("]("));
    
    // Stamp part 5: severity
    switch (severity)
    {
    case ELogLevel::LogLevelForced:
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_SEVERITY_FORCED, stampStringBuffer, _countof(stampStringBuffer));
        break;

    case ELogLevel::LogLevelError:
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_SEVERITY_ERROR, stampStringBuffer, _countof(stampStringBuffer));
        break;

    case ELogLevel::LogLevelWarning:
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_SEVERITY_WARNING, stampStringBuffer, _countof(stampStringBuffer));
        break;
    
    case ELogLevel::LogLevelInfo:
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_SEVERITY_INFO, stampStringBuffer, _countof(stampStringBuffer));
        break;

    default:
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_SEVERITY_UNKNOWN, stampStringBuffer, _countof(stampStringBuffer));
        break;
    }
    OutputLogMessage(stampStringBuffer);

    // Stamp part 6: close round bracket and space to the actual message
    OutputLogMessage(_T(") "));
}

// ---------

bool Log::ShouldOutputLogMessageOfSeverity(ELogLevel severity)
{
    return (LogLevelDisabled != severity && severity <= configuredSeverity);
}
