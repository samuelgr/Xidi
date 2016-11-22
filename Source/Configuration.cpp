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

#include "ApiStdString.h"
#include "Configuration.h"
#include "Globals.h"
#include "Log.h"

#include <cctype>
#include <cstdio>
#include <unordered_set>
#include <unordered_map>


using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Configuration.h" for documentation.

std::unordered_map<StdString, EConfigurationValueType> Configuration::logSettings = {
    {_T("Enabled"),                             EConfigurationValueType::ConfigurationValueTypeBoolean},
    {_T("Level"),                               EConfigurationValueType::ConfigurationValueTypeInteger}
};

std::unordered_map<StdString, EConfigurationValueType> Configuration::mapperSettings = {
    {_T("Type"),                                EConfigurationValueType::ConfigurationValueTypeString},
};

std::unordered_map<StdString, std::unordered_map<StdString, EConfigurationValueType>*> Configuration::configurationSections = {
    {_T("Log"),                                 &logSettings},
    {_T("Mapper"),                              &mapperSettings},
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

    // Parse the configuration file, one line at a time.
    {
        std::unordered_set<StdString> seenConfigurationSections;
        std::unordered_set<StdString> seenConfigurationValuesInCurrentSection;
        std::unordered_map<StdString, EConfigurationValueType>* currentConfigurationSection = NULL;
        
        TCHAR configurationLineBuffer[1024];
        int configurationLineLength = readAndTrimSingleLine(configurationLineBuffer, _countof(configurationLineBuffer), configurationFileHandle);
        unsigned int configurationLineNumber = 1;
        
        while (configurationLineLength >= 0)
        {
            StdString extractedName;
            StdString extractedValue;
			EConfigurationValueType extractedValueType;
            
            switch (classifyConfigurationFileLine(configurationLineBuffer, configurationLineLength))
            {
            case EConfigurationLineType::ConfigurationLineTypeIgnore:
                // Skip lines that should be ignored.
                break;

            case EConfigurationLineType::ConfigurationLineTypeSection:
                extractSectionNameFromConfigurationFileLine(extractedName, configurationLineBuffer);

                if (0 != seenConfigurationSections.count(extractedName))
                {
                    // Error: section name is duplicated.
					handleErrorDuplicateConfigurationSection(configurationFilePath, extractedName.c_str());
					fclose(configurationFileHandle);
					return;
                }
                else if (0 == configurationSections.count(extractedName))
                {
                    // Error: section name is unsupported.
					handleErrorUnsupportedConfigurationSection(configurationFilePath, extractedName.c_str());
					fclose(configurationFileHandle);
					return;
                }
                else
                {
                    // Insert the current configuration section into the set of those already seen.
                    seenConfigurationSections.insert(extractedName);

                    // Set the current section to the one named in the configuration file and reset the set of all values seen.
                    currentConfigurationSection = configurationSections[extractedName];
                    seenConfigurationValuesInCurrentSection.clear();
                }
                break;

            case EConfigurationLineType::ConfigurationLineTypeValue:
				if (NULL == currentConfigurationSection)
				{
					// Error: value specified outside a section (i.e. before the first section header in the configuration file).
					handleErrorValueOutsideSection(configurationFilePath, configurationLineNumber);
					fclose(configurationFileHandle);
					return;
				}
				
				// Extract out the name and value from the current line.
				extractNameValuePairFromConfigurationFileLine(extractedName, extractedValue, configurationLineBuffer);

				if (0 != seenConfigurationValuesInCurrentSection.count(extractedName))
				{
					// Error: duplicate value within the current section.
					handleErrorDuplicateValue(configurationFilePath, configurationLineNumber, extractedName.c_str());
					fclose(configurationFileHandle);
					return;
				}

				// Verify the name is recognized.
				if (0 == currentConfigurationSection->count(extractedName))
				{
					// Error: unsupported value in current section.
					handleErrorUnsupportedValue(configurationFilePath, configurationLineNumber, extractedName.c_str());
					fclose(configurationFileHandle);
					return;
				}

				// Extract the value type.
				extractedValueType = (*currentConfigurationSection)[extractedName];

				// Parse the value according to its specified and supposed type.
				// TODO
				
				seenConfigurationValuesInCurrentSection.insert(extractedName);

                break;

            default:
                // Error: unable to parse the current configuration file line.
				handleErrorCannotParseConfigurationFileLine(configurationFilePath, configurationLineNumber);
                fclose(configurationFileHandle);
                return;
            }
            
            configurationLineLength = readAndTrimSingleLine(configurationLineBuffer, _countof(configurationLineBuffer), configurationFileHandle);
            configurationLineNumber += 1;
        }
        
        if (!feof(configurationFileHandle))
        {
            // Stopped reading the configuration file early due to some condition other than end-of-file.
            // This indicates an error.

            if (ferror(configurationFileHandle))
            {
                // Error: file I/O problem.
				handleErrorFileIO();
				fclose(configurationFileHandle);
				return;

            }
            else if (configurationLineLength < 0)
            {
                // Error: line is too long.
				handleErrorLineTooLong(configurationFilePath, configurationLineNumber);
				fclose(configurationFileHandle);
				return;
            }
        }
    }
    
    fclose(configurationFileHandle);
}


// -------- HELPERS -------------------------------------------------------- //
// See "Configuration.h" for documentation.

EConfigurationLineType Configuration::classifyConfigurationFileLine(LPCTSTR buf, const size_t length)
{
    // Skip over all whitespace at the start of the input line.
    LPCTSTR realBuf = buf;
    size_t realLength = length;
    while (realLength != 0 && isblank(realBuf[0]))
    {
        realLength -= 1;
        realBuf += 1;
    }
    
    // Sanity check: zero-length and all-whitespace lines can be safely ignored.
    // Also filter out comments this way.
    if (0 == realLength || _T(';') == realBuf[0] || _T('#') == realBuf[0])
        return EConfigurationLineType::ConfigurationLineTypeIgnore;

    // Non-comments must, by definition, have at least three characters in them, excluding all whitespace.
    // For section headers, this must mean '[' + section name + ']'.
    // For values, this must mean name + '=' + value.
    if (realLength < 3)
        return EConfigurationLineType::ConfigurationLineTypeError;
    
    if (_T('[') == realBuf[0])
    {
        // The line cannot be a section header unless the second character is alphanumeric (there must be at least one character in the name of the section).
        if (!isalnum(realBuf[1]))
            return EConfigurationLineType::ConfigurationLineTypeError;
        
        // Verify that the line is a valid section header by checking for alphanumeric characters between two square brackets.
        size_t i = 2;
        for (; i < realLength && _T(']') != realBuf[i]; ++i)
        {
            if (!isalnum(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }
        if (_T(']') != realBuf[i])
            return EConfigurationLineType::ConfigurationLineTypeError;

        // Verify that the remainder of the line is either a comment or just whitespace.
        for (i += 1; i < realLength && _T(';') != realBuf[i] && _T('#') != realBuf[i]; ++i)
        {
            if (!isblank(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        return EConfigurationLineType::ConfigurationLineTypeSection;
    }
    else if (isalnum(realBuf[0]))
    {
        // Search for whitespace or an equals sign, with all characters in between needing to be alphanumeric.
        size_t i = 1;
        for (; i < realLength && _T('=') != realBuf[i] && !isblank(realBuf[i]); ++i)
        {
            if (!isalnum(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        // Skip over any whitespace present, then check for an equals sign.
        for (; i < realLength && isblank(realBuf[i]); ++i);
        if (_T('=') != realBuf[i])
            return EConfigurationLineType::ConfigurationLineTypeError;
        
        // Skip over any whitespace present, then verify the next character is alphanumeric to start a value.
        for (i += 1; i < realLength && isblank(realBuf[i]); ++i);
        if (!isalnum(realBuf[i]))
            return EConfigurationLineType::ConfigurationLineTypeError;
        
        // Skip over the alphanumeric characters that follow, effectively skipping over the value itself.
        for (i += 1; i < realLength && isalnum(realBuf[i]); ++i);

        // Verify that the remainder of the line is either a comment or just whitespace.
        for (; i < realLength && _T(';') != realBuf[i] && _T('#') != realBuf[i]; ++i)
        {
            if (!isblank(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }
        
        return EConfigurationLineType::ConfigurationLineTypeValue;
    }

    return EConfigurationLineType::ConfigurationLineTypeError;
}

// ---------

void Configuration::extractSectionNameFromConfigurationFileLine(StdString& sectionName, LPTSTR configFileLine)
{
    // Skip to the '[' character.
    LPTSTR realBuf = configFileLine;
    while (_T('[') != realBuf[0])
        realBuf += 1;
    realBuf += 1;
    
    // Find the length of the section name.
    size_t realLength = 1;
    while (_T(']') != realBuf[realLength])
        realLength += 1;

    // NULL-terminate the section name, then assign to the output string.
    realBuf[realLength] = _T('\0');
    sectionName = realBuf;
}

// ---------

void Configuration::extractNameValuePairFromConfigurationFileLine(StdString& name, StdString& value, LPTSTR configFileLine)
{
	// Skip to the start of the configuration name.
	LPTSTR configBuf = configFileLine;
	while (isblank(configBuf[0]))
		configBuf += 1;

	// Find the length of the configuration name.
	size_t configLength = 1;
	while (!isblank(configBuf[configLength]) && !(_T('=') == configBuf[configLength]))
		configLength += 1;
	
	// NULL-terminate the configuration name, then assign to the output string.
	configBuf[configLength] = _T('\0');
	name = configBuf;

	// Same process for configuration value.
	configBuf = &configBuf[configLength + 1];
	
	while (!isalnum(configBuf[0]))
		configBuf += 1;
	
	configLength = 1;
	while (isalnum(configBuf[configLength]))
		configLength += 1;

	configBuf[configLength] = _T('\0');
	value = configBuf;
}

// ---------

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

void Configuration::handleErrorCannotOpenConfigurationFile(LPCTSTR filename)
{
    TCHAR configurationErrorMessageFormat[1024];
    
    if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_CANNOT_OPEN_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, filename);
}

// ---------

void Configuration::handleErrorCannotParseConfigurationFileLine(LPCTSTR filename, const DWORD linenum)
{
    TCHAR configurationErrorMessageFormat[1024];

    if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_CANNOT_PARSE_LINE_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
        Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, linenum, filename);
}

// ---------

void Configuration::handleErrorDuplicateConfigurationSection(LPCTSTR filename, LPCTSTR section)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_DUPLICATED_SECTION_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, section, filename);
}

// ---------

void Configuration::handleErrorUnsupportedConfigurationSection(LPCTSTR filename, LPCTSTR section)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_UNSUPPORTED_SECTION_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, section, filename);
}

// ---------

void Configuration::handleErrorLineTooLong(LPCTSTR filename, const DWORD linenum)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_LINE_TOO_LONG_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, linenum, filename);
}

// ---------

void Configuration::handleErrorValueOutsideSection(LPCTSTR filename, const DWORD linenum)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_VALUE_WITHOUT_SECTION_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, linenum, filename);
}

// ---------

void Configuration::handleErrorDuplicateValue(LPCTSTR filename, const DWORD linenum, LPCTSTR value)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_DUPLICATE_VALUE_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, value, linenum, filename);
}

// ---------

void Configuration::handleErrorUnsupportedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR value)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_UNSUPPORTED_VALUE_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, value, linenum, filename);
}

// ---------

void Configuration::handleErrorFileIO(void)
{
	TCHAR configurationErrorMessage[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_IO_FORMAT, configurationErrorMessage, _countof(configurationErrorMessage)))
		Log::WriteLogMessage(ELogLevel::LogLevelForced, configurationErrorMessage);
}

// ---------

void Configuration::handleErrorInternal(const DWORD code)
{
	TCHAR configurationErrorMessageFormat[1024];

	if (0 != LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_ERROR_INTERNAL_FORMAT, configurationErrorMessageFormat, _countof(configurationErrorMessageFormat)))
		Log::WriteFormattedLogMessage(ELogLevel::LogLevelForced, configurationErrorMessageFormat, code);
}

// ---------

int Configuration::readAndTrimSingleLine(LPTSTR buf, const size_t count, FILE* filehandle)
{
    if (buf != _fgetts(buf, count, filehandle))
        return -1;
    
    // Verify that the line fits within the buffer, otherwise the line is too long.
    size_t linelength = _tcsnlen(buf, count);
    if (count - 1 == linelength && _T('\n') != buf[linelength - 1])
    {
        // Line is too long.
        return -1;
    }

    // Remove the trailing newline character from the configuration line that was read.
    if (_T('\n') == buf[linelength - 1])
    {
        linelength -= 1;
        buf[linelength] = _T('\0');
    }

    return linelength;
}
