/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Configuration.cpp
 *      Implementation of configuration file functionality.
 *****************************************************************************/

#include "Configuration.h"
#include "Globals.h"
#include "Log.h"

#include <cstdio>
#include <unordered_map>


using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Configuration.h" for documentation.

std::unordered_map<std::string, EConfigurationValueType> Configuration::logSettings = {
    {"Enabled",                                 EConfigurationValueType::ConfigurationValueTypeBoolean},
    {"Level",                                   EConfigurationValueType::ConfigurationValueTypeInteger}
};

std::unordered_map<std::string, EConfigurationValueType> Configuration::mapperSettings = {
    {"Type",                                    EConfigurationValueType::ConfigurationValueTypeString},
};

std::unordered_map<std::string, std::unordered_map<std::string, EConfigurationValueType>&> Configuration::configurationLayout = {
    {"Log",                                     logSettings},
    {"Mapper",                                  mapperSettings},
};


// -------- CLASS METHODS -------------------------------------------------- //
// See "Configuration.h" for documentation.

void Configuration::parseAndApplyConfigurationFile(void)
{
    TCHAR configurationFilePath[1024];
    FILE* configurationFileHandle = NULL;

    // Get the configuration file path.
    if (0 == getConfigurationFilePath(configurationFilePath, _countof(configurationFilePath)))
    {
        handleErrorInternal(__LINE__);
        return;
    }

    // Attempt to open the configuration file.
    _tfopen_s(&configurationFileHandle, configurationFilePath, _T("r"));
    if (NULL == configurationFileHandle)
    {
        handleErrorCannotOpenConfigurationFile(configurationFilePath);
        return;
    }
    
    fclose(configurationFileHandle);
}


// -------- HELPERS -------------------------------------------------------- //
// See "Configuration.h" for documentation.

size_t Configuration::getConfigurationFilePath(LPTSTR buf, const size_t count)
{
    TCHAR configurationFileName[128];
    size_t lenConfigurationFileName = (size_t)LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_NAME, configurationFileName, _countof(configurationFileName));
    size_t lenInstancePath = (size_t)GetModuleFileName(Globals::GetInstanceHandle(), buf, count);
    
    // Search for the final '\' character in the instance path, by working backwards, and truncate the entire length of the string after.
    // This extracts the directory name from the module file name.
    for (lenInstancePath -= 1; lenInstancePath >= 0 && _T('\\') != buf[lenInstancePath]; --lenInstancePath);
    if (0 == lenInstancePath)
        return 0;
    
    lenInstancePath += 1;
    buf[lenInstancePath] = _T('\0');

    // If there is room, concatenate the configuration file name to the directory name.
    if (lenConfigurationFileName + lenInstancePath >= count)
        return 0;

    _tcscat_s(buf, count, configurationFileName);
    
    return lenInstancePath + lenConfigurationFileName - 1;
}

// ---------

void Configuration::handleErrorCannotOpenConfigurationFile(LPTSTR filename)
{
    TCHAR configurationErrorMessageFormat[1024];
    
    if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_CANNOT_OPEN_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, filename);
}

// ---------

void Configuration::handleErrorCannotParseConfigurationFileLine(LPTSTR filename, const DWORD linenum)
{
    TCHAR configurationErrorMessageFormat[1024];

    if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_CANNOT_PARSE_LINE_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, configurationErrorMessageFormat, linenum, filename);
}

// ---------

void Configuration::handleErrorInternal(const DWORD code)
{
    TCHAR configurationErrorMessageFormat[1024];

    if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_INTERNAL_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, code);
}
