/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file Configuration.cpp
 *   Implementation of configuration file functionality.
 *****************************************************************************/

#include "ApiCharacterType.h"
#include "ApiStdString.h"
#include "Configuration.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "ImportApiWinMM.h"
#include "Log.h"
#include "MapperFactory.h"

#include <cstdio>
#include <unordered_set>
#include <unordered_map>

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Configuration.h" for documentation.

std::unordered_map<StdString, SConfigurationValueApplyInfo> Configuration::importSettings = {
    {_T("dinput.dll"),                          {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&Globals::ApplyOverrideImportDirectInput}},
    {_T("dinput8.dll"),                         {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&Globals::ApplyOverrideImportDirectInput8}},
    {_T("winmm.dll"),                           {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&Globals::ApplyOverrideImportWinMM}},
};

std::unordered_map<StdString, SConfigurationValueApplyInfo> Configuration::logSettings = {
    {_T("Enabled"),                             {EConfigurationValueType::ConfigurationValueTypeBoolean,    (void*)&Log::ApplyConfigurationLogEnabled}},
    {_T("Level"),                               {EConfigurationValueType::ConfigurationValueTypeInteger,    (void*)&Log::ApplyConfigurationLogLevel}}
};

std::unordered_map<StdString, SConfigurationValueApplyInfo> Configuration::mapperSettings = {
    {_T("Type"),                                {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&MapperFactory::ApplyConfigurationMapperType}},
};

std::unordered_map<StdString, std::unordered_map<StdString, SConfigurationValueApplyInfo>*> Configuration::configurationSections = {
    {_T("Import"),                              &importSettings},
    {_T("Log"),                                 &logSettings},
    {_T("Mapper"),                              &mapperSettings},
};


// -------- CLASS METHODS -------------------------------------------------- //
// See "Configuration.h" for documentation.

void Configuration::ParseAndApplyConfigurationFile(void)
{
    TCHAR configurationFilePath[kMaximumConfigurationFilePathLength];
    FILE* configurationFileHandle = NULL;

    // Get the configuration file path.
    if (0 == GetConfigurationFilePath(configurationFilePath, _countof(configurationFilePath)))
    {
        HandleErrorInternal(__LINE__);
        return;
    }

    // Attempt to open the configuration file.
    _tfopen_s(&configurationFileHandle, configurationFilePath, _T("r"));
    if (NULL == configurationFileHandle)
    {
        HandleErrorCannotOpenConfigurationFile(configurationFilePath);
        return;
    }

    // Parse the configuration file, one line at a time.
    {
        std::unordered_set<StdString> seenConfigurationSections;
        std::unordered_set<StdString> seenConfigurationValuesInCurrentSection;
        std::unordered_map<StdString, SConfigurationValueApplyInfo>* currentConfigurationSection = NULL;
        StdString currentConfigurationSectionName = _T("");
        
        TCHAR configurationLineBuffer[kMaximumConfigurationLineLength];
        int configurationLineLength = ReadAndTrimSingleLine(configurationLineBuffer, _countof(configurationLineBuffer), configurationFileHandle);
        unsigned int configurationLineNumber = 1;
        
        while (configurationLineLength >= 0)
        {
            StdString extractedName;
            StdString extractedValue;
            SConfigurationValueApplyInfo extractedValueInfo;
            
            switch (ClassifyConfigurationFileLine(configurationLineBuffer, configurationLineLength))
            {
            case EConfigurationLineType::ConfigurationLineTypeIgnore:
                // Skip lines that should be ignored.
                break;

            case EConfigurationLineType::ConfigurationLineTypeSection:
                ExtractSectionNameFromConfigurationFileLine(extractedName, configurationLineBuffer);

                if (0 != seenConfigurationSections.count(extractedName))
                {
                    // Error: section name is duplicated.
                    HandleErrorDuplicateConfigurationSection(configurationFilePath, extractedName.c_str());
                    fclose(configurationFileHandle);
                    return;
                }
                else if (0 == configurationSections.count(extractedName))
                {
                    // Error: section name is unsupported.
                    HandleErrorUnsupportedConfigurationSection(configurationFilePath, extractedName.c_str());
                    fclose(configurationFileHandle);
                    return;
                }
                else
                {
                    // Insert the current configuration section into the set of those already seen.
                    seenConfigurationSections.insert(extractedName);

                    // Set the current section to the one named in the configuration file and reset the set of all values seen.
                    currentConfigurationSection = configurationSections[extractedName];
                    currentConfigurationSectionName = extractedName;
                    seenConfigurationValuesInCurrentSection.clear();
                }
                break;

            case EConfigurationLineType::ConfigurationLineTypeValue:
                if (NULL == currentConfigurationSection)
                {
                    // Warning: value specified outside a section (i.e. before the first section header in the configuration file).
                    HandleErrorValueOutsideSection(configurationFilePath, configurationLineNumber);
                    break;
                }
                
                // Extract out the name and value from the current line.
                ExtractNameValuePairFromConfigurationFileLine(extractedName, extractedValue, configurationLineBuffer);

                if (0 != seenConfigurationValuesInCurrentSection.count(extractedName))
                {
                    // Warning: duplicate value within the current section.
                    HandleErrorDuplicateValue(configurationFilePath, configurationLineNumber, currentConfigurationSectionName.c_str(), extractedName.c_str());
                }

                // Verify the name is recognized.
                if (0 == currentConfigurationSection->count(extractedName))
                {
                    // Warning: unsupported value in current section.
                    HandleErrorUnsupportedValue(configurationFilePath, configurationLineNumber, currentConfigurationSectionName.c_str(), extractedName.c_str());
                    break;
                }

                // Extract information on the value.
                extractedValueInfo = (*currentConfigurationSection)[extractedName];
                
                // Parse the value according to its specified and supposed type.
                // Apply the setting immediately after parsing.
                union
                {
                    int64_t integerValue;
                    bool booleanValue;
                    StdString* stringValue;
                } parsedValue;
                
                switch (extractedValueInfo.type)
                {
                case EConfigurationValueType::ConfigurationValueTypeInteger:
                    
                    // Parse the value.
                    if (false == ParseIntegerValue(parsedValue.integerValue, extractedValue))
                    {
                        // Warning: unable to parse the value.
                        HandleErrorMalformedValue(configurationFilePath, configurationLineNumber, currentConfigurationSectionName.c_str(), extractedName.c_str());
                        break;
                    }
                    
                    // Attempt to apply the value.
                    if ((NULL == extractedValueInfo.applyFunc) || (false == ((TFuncApplyIntSetting)extractedValueInfo.applyFunc)(parsedValue.integerValue)))
                        HandleErrorCannotApplyValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    else
                        HandleSuccessAppliedValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    
                    break;

                case EConfigurationValueType::ConfigurationValueTypeBoolean:
                    
                    // Parse the value.
                    if (false == ParseBooleanValue(parsedValue.booleanValue, extractedValue))
                    {
                        // Warning: unable to parse the value.
                        HandleErrorMalformedValue(configurationFilePath, configurationLineNumber, currentConfigurationSectionName.c_str(), extractedName.c_str());
                        break;
                    }
                    
                    // Attempt to apply the value.
                    if ((NULL == extractedValueInfo.applyFunc) || (false == ((TFuncApplyBoolSetting)extractedValueInfo.applyFunc)(parsedValue.booleanValue)))
                        HandleErrorCannotApplyValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    else
                        HandleSuccessAppliedValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    
                    break;

                case EConfigurationValueType::ConfigurationValueTypeString:
                    
                    // No special parsing operation is required for a string-typed value.
                    parsedValue.stringValue = &extractedValue;
                    
                    // Attempt to apply the value.
                    if ((NULL == extractedValueInfo.applyFunc) || (false == ((TFuncApplyStringSetting)extractedValueInfo.applyFunc)(*parsedValue.stringValue)))
                        HandleErrorCannotApplyValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    else
                        HandleSuccessAppliedValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    
                    break;

                default:
                    // This should never happen and is an internal error.
                    HandleErrorInternal(__LINE__);
                    fclose(configurationFileHandle);
                    return;
                }
                
                seenConfigurationValuesInCurrentSection.insert(extractedName);
                break;

            default:
                // Error: unable to parse the current configuration file line.
                HandleErrorCannotParseConfigurationFileLine(configurationFilePath, configurationLineNumber);
                fclose(configurationFileHandle);
                return;
            }
            
            configurationLineLength = ReadAndTrimSingleLine(configurationLineBuffer, _countof(configurationLineBuffer), configurationFileHandle);
            configurationLineNumber += 1;
        }
        
        if (!feof(configurationFileHandle))
        {
            // Stopped reading the configuration file early due to some condition other than end-of-file.
            // This indicates an error.

            if (ferror(configurationFileHandle))
            {
                // Error: file I/O problem.
                HandleErrorFileIO(configurationFilePath);
                fclose(configurationFileHandle);
                return;

            }
            else if (configurationLineLength < 0)
            {
                // Error: line is too long.
                HandleErrorLineTooLong(configurationFilePath, configurationLineNumber);
                fclose(configurationFileHandle);
                return;
            }
        }
    }
    
    fclose(configurationFileHandle);
}


// -------- HELPERS -------------------------------------------------------- //
// See "Configuration.h" for documentation.

EConfigurationLineType Configuration::ClassifyConfigurationFileLine(LPCTSTR buf, const size_t length)
{
    // Skip over all whitespace at the start of the input line.
    LPCTSTR realBuf = buf;
    size_t realLength = length;
    while (realLength != 0 && istblank(realBuf[0]))
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
        if (!istalnum(realBuf[1]))
            return EConfigurationLineType::ConfigurationLineTypeError;
        
        // Verify that the line is a valid section header by checking for alphanumeric characters between two square brackets.
        size_t i = 2;
        for (; i < realLength && _T(']') != realBuf[i]; ++i)
        {
            if (!istalnum(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }
        if (_T(']') != realBuf[i])
            return EConfigurationLineType::ConfigurationLineTypeError;

        // Verify that the remainder of the line is just whitespace.
        for (i += 1; i < realLength; ++i)
        {
            if (!istblank(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        return EConfigurationLineType::ConfigurationLineTypeSection;
    }
    else if (IsAllowedValueNameCharacter(realBuf[0]))
    {
        // Search for whitespace or an equals sign, with all characters in between needing to be allowed as value name characters.
        size_t i = 1;
        for (; i < realLength && _T('=') != realBuf[i] && !istblank(realBuf[i]); ++i)
        {
            if (!IsAllowedValueNameCharacter(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        // Skip over any whitespace present, then check for an equals sign.
        for (; i < realLength && istblank(realBuf[i]); ++i);
        if (_T('=') != realBuf[i])
            return EConfigurationLineType::ConfigurationLineTypeError;
        
        // Skip over any whitespace present, then verify the next character is allowed to start a value setting.
        for (i += 1; i < realLength && istblank(realBuf[i]); ++i);
        if (!IsAllowedValueSettingCharacter(realBuf[i]))
            return EConfigurationLineType::ConfigurationLineTypeError;
        
        // Skip over the value setting characters that follow.
        for (i += 1; i < realLength && IsAllowedValueSettingCharacter(realBuf[i]); ++i);

        // Verify that the remainder of the line is just whitespace.
        for (; i < realLength; ++i)
        {
            if (!istblank(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }
        
        return EConfigurationLineType::ConfigurationLineTypeValue;
    }

    return EConfigurationLineType::ConfigurationLineTypeError;
}

// ---------

void Configuration::ExtractNameValuePairFromConfigurationFileLine(StdString& name, StdString& value, LPTSTR configFileLine)
{
    // Skip to the start of the configuration name.
    LPTSTR configBuf = configFileLine;
    while (istblank(configBuf[0]))
        configBuf += 1;

    // Find the length of the configuration name.
    size_t configLength = 1;
    while (IsAllowedValueNameCharacter(configBuf[configLength]))
        configLength += 1;

    // NULL-terminate the configuration name, then assign to the output string.
    configBuf[configLength] = _T('\0');
    name = configBuf;

    // Advance to the value portion of the string.
    configBuf = &configBuf[configLength + 1];

    // Skip over whitespace and the '=' sign.
    while ((_T('=') == configBuf[0]) || (istblank(configBuf[0])))
        configBuf += 1;

    // Find the length of the configuration value.
    configLength = 1;
    while (IsAllowedValueSettingCharacter(configBuf[configLength]))
        configLength += 1;

    // Trim off any dangling whitespace.
    while ((1 < configLength) && (istblank(configBuf[configLength - 1])))
        configLength -= 1;
    
    configBuf[configLength] = _T('\0');
    value = configBuf;
}

// ---------

int Configuration::IsAllowedValueNameCharacter(const TCHAR charToTest)
{
    switch (charToTest)
    {
    case _T('.'):
        return true;

    default:
        return istalnum(charToTest);
    }
}

// ---------

int Configuration::IsAllowedValueSettingCharacter(const TCHAR charToTest)
{
    switch (charToTest)
    {
    case _T(','):
    case _T('.'):
    case _T(';'):
    case _T(':'):
    case _T('\''):
    case _T('\\'):
    case _T('{'):
    case _T('['):
    case _T('}'):
    case _T(']'):
    case _T('-'):
    case _T('_'):
    case _T(' '):
    case _T('+'):
    case _T('='):
    case _T('!'):
    case _T('@'):
    case _T('#'):
    case _T('$'):
    case _T('%'):
    case _T('^'):
    case _T('&'):
    case _T('('):
    case _T(')'):
        return true;

    default:
        return istalnum(charToTest);
    }
}

// ---------

bool Configuration::ParseIntegerValue(int64_t& dest, const StdString& source)
{
    int64_t value = 0ll;
    LPTSTR endptr = NULL;

    // Parse out a number in any representable base.
    value = _tcstoll(source.c_str(), &endptr, 0);

    // Verify that the number is not out of range.
    if (ERANGE == errno && (LLONG_MIN == value || LLONG_MAX == value))
        return false;
    
    // Verify that the whole string was consumed.
    if (_T('\0') != *endptr)
        return false;
    
    // Set the output.
    dest = value;
    return true;
}

// ---------

bool Configuration::ParseBooleanValue(bool& dest, const StdString& source)
{
    static const StdString trueStrings[] = { _T("t"), _T("true"), _T("on"), _T("y"), _T("yes"), _T("enabled"), _T("1") };
    static const StdString falseStrings[] = { _T("f"), _T("false"), _T("off"), _T("n"), _T("no"), _T("disabled"), _T("0") };
    
    // Check if the string represents a value of TRUE.
    for (size_t i = 0; i < _countof(trueStrings); ++i)
    {
        if (0 == _tcsicmp(source.c_str(), trueStrings[i].c_str()))
        {
            dest = true;
            return true;
        }
    }

    // Check if the string represents a value of FALSE.
    for (size_t i = 0; i < _countof(falseStrings); ++i)
    {
        if (0 == _tcsicmp(source.c_str(), falseStrings[i].c_str()))
        {
            dest = false;
            return true;
        }
    }
    
    return false;
}

// ---------

void Configuration::ExtractSectionNameFromConfigurationFileLine(StdString& sectionName, LPTSTR configFileLine)
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

int Configuration::ReadAndTrimSingleLine(LPTSTR buf, const int count, FILE* filehandle)
{
    if (buf != _fgetts(buf, count, filehandle))
        return -1;

    // Verify that the line fits within the buffer, otherwise the line is too long.
    int linelength = (int)_tcsnlen(buf, count);
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


// -------- APPLICATION-SPECIFIC METHODS ----------------------------------- //
// See "Configuration.h" for documentation.

size_t Configuration::GetConfigurationFilePath(LPTSTR buf, const DWORD count)
{
    TCHAR configurationFileName[kMaximumConfigurationFileNameLength];
    DWORD lenConfigurationFileName = (size_t)LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_NAME, configurationFileName, _countof(configurationFileName));
    DWORD lenInstancePath = (size_t)GetModuleFileName(Globals::GetInstanceHandle(), buf, count);
    
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

void Configuration::HandleErrorCannotOpenConfigurationFile(LPCTSTR filename)
{
    // Do nothing.
    // In this scenario, the default settings will be applied.
}

// ---------

void Configuration::HandleErrorCannotParseConfigurationFileLine(LPCTSTR filename, const DWORD linenum)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_CONFIGURATION_FILE_ERROR_CANNOT_PARSE_LINE_FORMAT, linenum, filename);
}

// ---------

void Configuration::HandleErrorDuplicateConfigurationSection(LPCTSTR filename, LPCTSTR section)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_CONFIGURATION_FILE_ERROR_DUPLICATED_SECTION_FORMAT, section, filename);
}

// ---------

void Configuration::HandleErrorUnsupportedConfigurationSection(LPCTSTR filename, LPCTSTR section)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_CONFIGURATION_FILE_ERROR_UNSUPPORTED_SECTION_FORMAT, section, filename);
}

// ---------

void Configuration::HandleErrorLineTooLong(LPCTSTR filename, const DWORD linenum)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_CONFIGURATION_FILE_ERROR_LINE_TOO_LONG_FORMAT, linenum, filename);
}

// ---------

void Configuration::HandleErrorValueOutsideSection(LPCTSTR filename, const DWORD linenum)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_CONFIGURATION_FILE_ERROR_VALUE_WITHOUT_SECTION_FORMAT, linenum, filename);
}

// ---------

void Configuration::HandleErrorDuplicateValue(LPCTSTR filename, const DWORD linenum, LPCTSTR section, LPCTSTR value)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_CONFIGURATION_FILE_ERROR_DUPLICATE_VALUE_FORMAT, value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorMalformedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR section, LPCTSTR value)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_CONFIGURATION_FILE_ERROR_MALFORMED_VALUE_FORMAT, value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorUnsupportedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR section, LPCTSTR value)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_CONFIGURATION_FILE_ERROR_UNSUPPORTED_VALUE_FORMAT, value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorCannotApplyValue(LPCTSTR filename, const DWORD linenum, LPCTSTR setting, LPCTSTR section, LPCTSTR value)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelWarning, IDS_XIDI_CONFIGURATION_FILE_ERROR_CANNOT_APPLY_VALUE_FORMAT, setting, value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorFileIO(LPCTSTR filename)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_CONFIGURATION_FILE_ERROR_IO_FORMAT, filename);
}

// ---------

void Configuration::HandleErrorInternal(const DWORD code)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_CONFIGURATION_FILE_ERROR_INTERNAL_FORMAT, code);
}

// ---------

void Configuration::HandleSuccessAppliedValue(LPCTSTR filename, const DWORD linenum, LPCTSTR setting, LPCTSTR section, LPCTSTR value)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_CONFIGURATION_FILE_SUCCESS_APPLY_FORMAT, setting, value, section, linenum, filename);
}
