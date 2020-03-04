/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file Log.cpp
 *   Logging functionality implementation.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"
#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <ShlObj.h>

using namespace Xidi;


// -------- INTERNAL CONSTANTS --------------------------------------------- //

/// Buffer size, in characters, for the temporary buffer to hold string messages read using a resource identifier.
/// When writing log messages using a resource identifier (rather than a raw string), a temporary buffer is created to hold the loaded resource string.
static const size_t kLogResourceBufferSize = 1024;

/// Suffix for the log file name.
static const WCHAR kLogFileNameSuffix[] = L".log";

/// Separator string to use within the log file.
static const TCHAR kLogSeparator[] = _T("-------------------------");


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
        GetModuleFileName(NULL, logFileNameBuffer, _countof(logFileNameBuffer));
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
        logFilePath += kLogFileNameSuffix;
        
        // Open the log file for writing.
        _wfopen_s(&fileHandle, logFilePath.c_str(), L"w, ccs=UTF-8");
        if (NULL == fileHandle) return;

        // Write out the log file header.
        TCHAR logHeaderStringBuffer[1024];
        
        // Header part 1: Xidi version name.
        LoadString(Globals::GetInstanceHandle(), IDS_XIDI_VERSION_NAME, logHeaderStringBuffer, _countof(logHeaderStringBuffer));
        OutputText(logHeaderStringBuffer);
        OutputText(_T("\n"));

        // Header part 2: application file name
        if (0 != GetModuleFileName(NULL, logHeaderStringBuffer, _countof(logHeaderStringBuffer)))
        {
            OutputText(logHeaderStringBuffer);
            OutputText(_T("\n"));
        }

        // Header part 3: separator
        OutputText(kLogSeparator);
        OutputText(_T("\n"));
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

void Log::WriteFormattedLogMessage(ELogLevel severity, LPCTSTR format, ...)
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

void Log::WriteLogMessage(ELogLevel severity, LPCTSTR message)
{
    if (ShouldOutputLogMessageOfSeverity(severity))
        LogLineOutputString(severity, message);
}


// -------- HELPERS -------------------------------------------------------- //
// See "Log.h" for documentation.

bool Log::IsLogReady(void)
{
    return (NULL != fileHandle);
}

// ---------

void Log::LogLineOutputString(ELogLevel severity, LPCTSTR message)
{
    OutputStamp(severity);
    OutputText(message);
    OutputText(_T("\n"));
}

// ---------

void Log::LogLineOutputFormat(ELogLevel severity, LPCTSTR format, va_list args)
{
    OutputStamp(severity);
    OutputFormattedText(format, args);
    OutputText(_T("\n"));
}

// ---------

void Log::OutputFormattedText(LPCTSTR format, va_list args)
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

void Log::OutputText(LPCTSTR message)
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
    OutputText(_T("["));

    // Stamp part 2: date
    if (0 != GetDateFormat(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, stampStringBuffer, _countof(stampStringBuffer)))
    {
        OutputText(stampStringBuffer);
        OutputText(_T(" "));
    }

    // Stamp part 3: time
    if (0 != GetTimeFormat(LOCALE_NAME_USER_DEFAULT, 0, NULL, NULL, stampStringBuffer, _countof(stampStringBuffer)))
    {
        OutputText(stampStringBuffer);
    }

    // Stamp part 4: separation between date/time and severity
    OutputText(_T("]("));
    
    // Stamp part 5: severity
    switch (severity)
    {
    case ELogLevel::LogLevelForced:
        OutputText(_T("F"));
        break;

    case ELogLevel::LogLevelError:
        OutputText(_T("E"));
        break;

    case ELogLevel::LogLevelWarning:
        OutputText(_T("W"));
        break;
    
    case ELogLevel::LogLevelInfo:
        OutputText(_T("I"));
        break;

    case ELogLevel::LogLevelDebug:
        OutputText(_T("D"));
        break;

    default:
        OutputText(_T("U"));
        break;
    }

    // Stamp part 6: close round bracket and space to the actual message
    OutputText(_T(") "));
}

// ---------

bool Log::ShouldOutputLogMessageOfSeverity(ELogLevel severity)
{
    return (severity <= GetMinimumSeverity());
}
