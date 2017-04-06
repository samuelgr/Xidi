/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
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

ELogLevel Log::configuredSeverity = ELogLevel::LogLevelError;

bool Log::logEnabled = false;


// -------- CLASS METHODS -------------------------------------------------- //
// See "Log.h" for documentation.

bool Log::ApplyConfigurationLogEnabled(bool value)
{
    logEnabled = value;
    return true;
}

// --------

bool Log::ApplyConfigurationLogLevel(int64_t value)
{
    if ((ELogLevel)value >= LogLevelMinConfigurableValue && (ELogLevel)value <= LogLevelMaxConfigurableValue)
    {
        SetMinimumSeverity((ELogLevel)value);
        return true;
    }
    else
        return false;
}

// --------

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
    if (logEnabled)
        return configuredSeverity;
    else
        return ELogLevel::LogLevelDisabled;
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
        
        std::wstring logFilePath = folderPath;
        logFilePath += L"\\";
        CoTaskMemFree(folderPath);
        
        // The log file will be named according to the version of Xidi that is being used, plus the name of the executable and a suffix.
        WCHAR logFileNameBuffer[2048];
        
        // First is the name of the executable.
        GetModuleFileNameW(NULL, logFileNameBuffer, _countof(logFileNameBuffer));
        std::wstring executableFileName = logFileNameBuffer;
        const size_t executableStartPos = executableFileName.find_last_of(L'\\');
        const size_t executableEndPos = executableFileName.find_last_of(L'.');
        
        if ((std::wstring::npos != executableStartPos) && (std::wstring::npos != executableEndPos) && (executableStartPos < executableEndPos))
            logFilePath += executableFileName.substr(executableStartPos + 1, executableEndPos - executableStartPos - 1);

        logFilePath += L"_";
        
        // Next is the version of Xidi.
        LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_VERSION_NAME, logFileNameBuffer, _countof(logFileNameBuffer));
        logFilePath += logFileNameBuffer;
        
        // Finally is the suffix.
        LoadStringW(Globals::GetInstanceHandle(), IDS_XIDI_LOG_FILE_NAME_SUFFIX, logFileNameBuffer, _countof(logFileNameBuffer));
        logFilePath += logFileNameBuffer;
        
        // Open the log file for writing.
        _wfopen_s(&fileHandle, logFilePath.c_str(), L"w");
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

        LogLineOutputFormat(severity, format, args);

        va_end(args);
    }
}

// ---------

void Log::WriteLogMessage(ELogLevel severity, LPTSTR message)
{
    if (ShouldOutputLogMessageOfSeverity(severity))
        LogLineOutputString(severity, message);
}

// ---------

void Log::WriteFormattedLogMessageFromResource(ELogLevel severity, unsigned int resourceIdentifier, ...)
{
    if (ShouldOutputLogMessageOfSeverity(severity))
    {
        TCHAR logMessageFormat[kLogResourceBufferSize];
        
        if (0 != LoadString(Globals::GetInstanceHandle(), resourceIdentifier, logMessageFormat, _countof(logMessageFormat)))
        {
            va_list args;
            va_start(args, resourceIdentifier);

            LogLineOutputFormat(severity, logMessageFormat, args);

            va_end(args);
        }
    }
}

// ---------

void Log::WriteLogMessageFromResource(ELogLevel severity, unsigned int resourceIdentifier)
{
    if (ShouldOutputLogMessageOfSeverity(severity))
    {
        TCHAR logMessage[kLogResourceBufferSize];

        if (0 != LoadString(Globals::GetInstanceHandle(), resourceIdentifier, logMessage, _countof(logMessage)))
            LogLineOutputString(severity, logMessage);
    }
}


// -------- HELPERS -------------------------------------------------------- //
// See "Log.h" for documentation.

bool Log::IsLogReady(void)
{
    return (NULL != fileHandle);
}

// ---------

void Log::LogLineOutputString(ELogLevel severity, LPTSTR message)
{
    OutputStamp(severity);
    OutputLogMessage(message);
    OutputLogMessage(_T("\n"));
}

// ---------

void Log::LogLineOutputFormat(ELogLevel severity, LPTSTR format, va_list args)
{
    OutputStamp(severity);
    OutputFormattedLogMessage(format, args);
    OutputLogMessage(_T("\n"));
}

// ---------

void Log::OutputFormattedLogMessage(LPTSTR format, va_list args)
{
    if (!IsLogReady())
        InitializeAndCreateLog();
    
    if (IsLogReady())
    {
        _vftprintf_s(fileHandle, format, args);
        fflush(fileHandle);
    }
}

// ---------

void Log::OutputLogMessage(LPTSTR message)
{
    if (!IsLogReady())
        InitializeAndCreateLog();
    
    if (IsLogReady())
    {
        _fputts(message, fileHandle);
        fflush(fileHandle);
    }
}

// ---------

void Log::OutputStamp(ELogLevel severity)
{
    TCHAR stampStringBuffer[1024];

    // Stamp part 1: open square bracket
    OutputLogMessage(_T("["));

    // Stamp part 2: date
    if (0 != GetDateFormat(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, stampStringBuffer, _countof(stampStringBuffer)))
    {
        OutputLogMessage(stampStringBuffer);
        OutputLogMessage(_T(" "));
    }

    // Stamp part 3: time
    if (0 != GetTimeFormat(LOCALE_NAME_USER_DEFAULT, 0, NULL, NULL, stampStringBuffer, _countof(stampStringBuffer)))
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

    case ELogLevel::LogLevelDebug:
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_LOG_SEVERITY_DEBUG, stampStringBuffer, _countof(stampStringBuffer));
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
    return (severity <= GetMinimumSeverity());
}
