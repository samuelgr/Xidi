/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Configuration.cpp
 *   Implementation of configuration file functionality.
 *****************************************************************************/

#include "Configuration.h"
#include "Globals.h"
#include "ImportApiDirectInput.h"
#include "ImportApiWinMM.h"
#include "Log.h"
#include "MapperFactory.h"

#include <cstdio>
#include <string>
#include <unordered_set>
#include <unordered_map>

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "Configuration.h" for documentation.

std::unordered_map<std::wstring, SConfigurationValueApplyInfo> Configuration::importSettings = {
    {L"dinput.dll",                             {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&Globals::ApplyOverrideImportDirectInput}},
    {L"dinput8.dll",                            {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&Globals::ApplyOverrideImportDirectInput8}},
    {L"winmm.dll",                              {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&Globals::ApplyOverrideImportWinMM}},
};

std::unordered_map<std::wstring, SConfigurationValueApplyInfo> Configuration::logSettings = {
    {L"Enabled",                                {EConfigurationValueType::ConfigurationValueTypeBoolean,    (void*)&Log::ApplyConfigurationLogEnabled}},
    {L"Level",                                  {EConfigurationValueType::ConfigurationValueTypeInteger,    (void*)&Log::ApplyConfigurationLogLevel}}
};

std::unordered_map<std::wstring, SConfigurationValueApplyInfo> Configuration::mapperSettings = {
    {L"Type",                                   {EConfigurationValueType::ConfigurationValueTypeString,     (void*)&MapperFactory::ApplyConfigurationMapperType}},
};

std::unordered_map<std::wstring, std::unordered_map<std::wstring, SConfigurationValueApplyInfo>*> Configuration::configurationSections = {
    {L"Import",                                 &importSettings},
    {L"Log",                                    &logSettings},
    {L"Mapper",                                 &mapperSettings},
};


// -------- CLASS METHODS -------------------------------------------------- //
// See "Configuration.h" for documentation.

void Configuration::ParseAndApplyConfigurationFile(void)
{
    wchar_t configurationFilePath[kMaximumConfigurationFilePathLength];
    FILE* configurationFileHandle = nullptr;

    // Get the configuration file path.
    if (0 == GetConfigurationFilePath(configurationFilePath, _countof(configurationFilePath)))
    {
        HandleErrorInternal(__LINE__);
        return;
    }

    // Attempt to open the configuration file.
    _wfopen_s(&configurationFileHandle, configurationFilePath, L"r");
    if (nullptr == configurationFileHandle)
    {
        HandleErrorCannotOpenConfigurationFile(configurationFilePath);
        return;
    }

    // Parse the configuration file, one line at a time.
    {
        std::unordered_set<std::wstring> seenConfigurationSections;
        std::unordered_set<std::wstring> seenConfigurationValuesInCurrentSection;
        std::unordered_map<std::wstring, SConfigurationValueApplyInfo>* currentConfigurationSection = nullptr;
        std::wstring currentConfigurationSectionName = L"";

        wchar_t configurationLineBuffer[kMaximumConfigurationLineLength];
        int configurationLineLength = ReadAndTrimSingleLine(configurationLineBuffer, _countof(configurationLineBuffer), configurationFileHandle);
        unsigned int configurationLineNumber = 1;

        while (configurationLineLength >= 0)
        {
            std::wstring extractedName;
            std::wstring extractedValue;
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
                if (nullptr == currentConfigurationSection)
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
                    std::wstring* stringValue;
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
                    if ((nullptr == extractedValueInfo.applyFunc) || (false == ((TFuncApplyIntSetting)extractedValueInfo.applyFunc)(parsedValue.integerValue)))
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
                    if ((nullptr == extractedValueInfo.applyFunc) || (false == ((TFuncApplyBoolSetting)extractedValueInfo.applyFunc)(parsedValue.booleanValue)))
                        HandleErrorCannotApplyValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());
                    else
                        HandleSuccessAppliedValue(configurationFilePath, configurationLineNumber, extractedValue.c_str(), currentConfigurationSectionName.c_str(), extractedName.c_str());

                    break;

                case EConfigurationValueType::ConfigurationValueTypeString:

                    // No special parsing operation is required for a string-typed value.
                    parsedValue.stringValue = &extractedValue;

                    // Attempt to apply the value.
                    if ((nullptr == extractedValueInfo.applyFunc) || (false == ((TFuncApplyStringSetting)extractedValueInfo.applyFunc)(*parsedValue.stringValue)))
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

EConfigurationLineType Configuration::ClassifyConfigurationFileLine(LPCWSTR buf, const size_t length)
{
    // Skip over all whitespace at the start of the input line.
    LPCWSTR realBuf = buf;
    size_t realLength = length;
    while (realLength != 0 && iswblank(realBuf[0]))
    {
        realLength -= 1;
        realBuf += 1;
    }

    // Sanity check: zero-length and all-whitespace lines can be safely ignored.
    // Also filter out comments this way.
    if (0 == realLength || L';' == realBuf[0] || L'#' == realBuf[0])
        return EConfigurationLineType::ConfigurationLineTypeIgnore;

    // Non-comments must, by definition, have at least three characters in them, excluding all whitespace.
    // For section headers, this must mean '[' + section name + ']'.
    // For values, this must mean name + '=' + value.
    if (realLength < 3)
        return EConfigurationLineType::ConfigurationLineTypeError;

    if (L'[' == realBuf[0])
    {
        // The line cannot be a section header unless the second character is alphanumeric (there must be at least one character in the name of the section).
        if (!iswalnum(realBuf[1]))
            return EConfigurationLineType::ConfigurationLineTypeError;

        // Verify that the line is a valid section header by checking for alphanumeric characters between two square brackets.
        size_t i = 2;
        for (; i < realLength && L']' != realBuf[i]; ++i)
        {
            if (!iswalnum(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }
        if (L']' != realBuf[i])
            return EConfigurationLineType::ConfigurationLineTypeError;

        // Verify that the remainder of the line is just whitespace.
        for (i += 1; i < realLength; ++i)
        {
            if (!iswblank(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        return EConfigurationLineType::ConfigurationLineTypeSection;
    }
    else if (IsAllowedValueNameCharacter(realBuf[0]))
    {
        // Search for whitespace or an equals sign, with all characters in between needing to be allowed as value name characters.
        size_t i = 1;
        for (; i < realLength && L'=' != realBuf[i] && !iswblank(realBuf[i]); ++i)
        {
            if (!IsAllowedValueNameCharacter(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        // Skip over any whitespace present, then check for an equals sign.
        for (; i < realLength && iswblank(realBuf[i]); ++i);
        if (L'=' != realBuf[i])
            return EConfigurationLineType::ConfigurationLineTypeError;

        // Skip over any whitespace present, then verify the next character is allowed to start a value setting.
        for (i += 1; i < realLength && iswblank(realBuf[i]); ++i);
        if (!IsAllowedValueSettingCharacter(realBuf[i]))
            return EConfigurationLineType::ConfigurationLineTypeError;

        // Skip over the value setting characters that follow.
        for (i += 1; i < realLength && IsAllowedValueSettingCharacter(realBuf[i]); ++i);

        // Verify that the remainder of the line is just whitespace.
        for (; i < realLength; ++i)
        {
            if (!iswblank(realBuf[i]))
                return EConfigurationLineType::ConfigurationLineTypeError;
        }

        return EConfigurationLineType::ConfigurationLineTypeValue;
    }

    return EConfigurationLineType::ConfigurationLineTypeError;
}

// ---------

void Configuration::ExtractNameValuePairFromConfigurationFileLine(std::wstring& name, std::wstring& value, LPWSTR configFileLine)
{
    // Skip to the start of the configuration name.
    LPWSTR configBuf = configFileLine;
    while (iswblank(configBuf[0]))
        configBuf += 1;

    // Find the length of the configuration name.
    size_t configLength = 1;
    while (IsAllowedValueNameCharacter(configBuf[configLength]))
        configLength += 1;

    // nullptr-terminate the configuration name, then assign to the output string.
    configBuf[configLength] = L'\0';
    name = configBuf;

    // Advance to the value portion of the string.
    configBuf = &configBuf[configLength + 1];

    // Skip over whitespace and the '=' sign.
    while ((L'=' == configBuf[0]) || (iswblank(configBuf[0])))
        configBuf += 1;

    // Find the length of the configuration value.
    configLength = 1;
    while (IsAllowedValueSettingCharacter(configBuf[configLength]))
        configLength += 1;

    // Trim off any dangling whitespace.
    while ((1 < configLength) && (iswblank(configBuf[configLength - 1])))
        configLength -= 1;

    configBuf[configLength] = L'\0';
    value = configBuf;
}

// ---------

int Configuration::IsAllowedValueNameCharacter(const wchar_t charToTest)
{
    switch (charToTest)
    {
    case L'.':
        return true;

    default:
        return iswalnum(charToTest);
    }
}

// ---------

int Configuration::IsAllowedValueSettingCharacter(const wchar_t charToTest)
{
    switch (charToTest)
    {
    case L',':
    case L'.':
    case L';':
    case L':':
    case L'\'':
    case L'\\':
    case L'{':
    case L'[':
    case L'}':
    case L']':
    case L'-':
    case L'_':
    case L' ':
    case L'+':
    case L'=':
    case L'!':
    case L'@':
    case L'#':
    case L'$':
    case L'%':
    case L'^':
    case L'&':
    case L'(':
    case L')':
        return true;

    default:
        return iswalnum(charToTest);
    }
}

// ---------

bool Configuration::ParseIntegerValue(int64_t& dest, const std::wstring& source)
{
    int64_t value = 0ll;
    LPWSTR endptr = nullptr;

    // Parse out a number in any representable base.
    value = wcstoll(source.c_str(), &endptr, 0);

    // Verify that the number is not out of range.
    if (ERANGE == errno && (LLONG_MIN == value || LLONG_MAX == value))
        return false;

    // Verify that the whole string was consumed.
    if (L'\0' != *endptr)
        return false;

    // Set the output.
    dest = value;
    return true;
}

// ---------

bool Configuration::ParseBooleanValue(bool& dest, const std::wstring& source)
{
    static const std::wstring trueStrings[] = { L"t", L"true", L"on", L"y", L"yes", L"enabled", L"1" };
    static const std::wstring falseStrings[] = { L"f", L"false", L"off", L"n", L"no", L"disabled", L"0" };

    // Check if the string represents a value of TRUE.
    for (size_t i = 0; i < _countof(trueStrings); ++i)
    {
        if (0 == _wcsicmp(source.c_str(), trueStrings[i].c_str()))
        {
            dest = true;
            return true;
        }
    }

    // Check if the string represents a value of FALSE.
    for (size_t i = 0; i < _countof(falseStrings); ++i)
    {
        if (0 == _wcsicmp(source.c_str(), falseStrings[i].c_str()))
        {
            dest = false;
            return true;
        }
    }

    return false;
}

// ---------

void Configuration::ExtractSectionNameFromConfigurationFileLine(std::wstring& sectionName, LPWSTR configFileLine)
{
    // Skip to the '[' character.
    LPWSTR realBuf = configFileLine;
    while (L'[' != realBuf[0])
        realBuf += 1;
    realBuf += 1;

    // Find the length of the section name.
    size_t realLength = 1;
    while (L']' != realBuf[realLength])
        realLength += 1;

    // nullptr-terminate the section name, then assign to the output string.
    realBuf[realLength] = L'\0';
    sectionName = realBuf;
}

// ---------

int Configuration::ReadAndTrimSingleLine(LPWSTR buf, const int count, FILE* filehandle)
{
    if (buf != fgetws(buf, count, filehandle))
        return -1;

    // Verify that the line fits within the buffer, otherwise the line is too long.
    int linelength = (int)wcsnlen(buf, count);
    if (count - 1 == linelength && L'\n' != buf[linelength - 1])
    {
        // Line is too long.
        return -1;
    }

    // Remove the trailing newline character from the configuration line that was read.
    if (L'\n' == buf[linelength - 1])
    {
        linelength -= 1;
        buf[linelength] = L'\0';
    }

    return linelength;
}


// -------- APPLICATION-SPECIFIC METHODS ----------------------------------- //
// See "Configuration.h" for documentation.

size_t Configuration::GetConfigurationFilePath(LPWSTR buf, const DWORD count)
{
    wchar_t configurationFileName[kMaximumConfigurationFileNameLength];
    DWORD lenConfigurationFileName = (size_t)LoadString(Globals::GetInstanceHandle(), IDS_XIDI_CONFIGURATION_FILE_NAME, configurationFileName, _countof(configurationFileName));
    DWORD lenInstancePath = (size_t)GetModuleFileName(Globals::GetInstanceHandle(), buf, count);

    // Search for the final '\' character in the instance path, by working backwards, and truncate the entire length of the string after.
    // This extracts the directory name from the module file name.
    for (lenInstancePath -= 1; lenInstancePath >= 0 && L'\\' != buf[lenInstancePath]; --lenInstancePath);
    if (0 == lenInstancePath)
        return 0;

    lenInstancePath += 1;
    buf[lenInstancePath] = L'\0';

    // If there is room, concatenate the configuration file name to the directory name.
    if (lenConfigurationFileName + lenInstancePath >= count)
        return 0;

    wcscat_s(buf, count, configurationFileName);

    return lenInstancePath + lenConfigurationFileName - 1;
}

// ---------

void Configuration::HandleErrorCannotOpenConfigurationFile(LPCWSTR filename)
{
    // Do nothing.
    // In this scenario, the default settings will be applied.
}

// ---------

void Configuration::HandleErrorCannotParseConfigurationFileLine(LPCWSTR filename, const DWORD linenum)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Unable to parse line %u of configuration file \"%s\".", linenum, filename);
}

// ---------

void Configuration::HandleErrorDuplicateConfigurationSection(LPCWSTR filename, LPCWSTR section)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Duplicate section \"%s\" in configuration file \"%s\".", section, filename);
}

// ---------

void Configuration::HandleErrorUnsupportedConfigurationSection(LPCWSTR filename, LPCWSTR section)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Unsupported section \"%s\" in configuration file \"%s\".", section, filename);
}

// ---------

void Configuration::HandleErrorLineTooLong(LPCWSTR filename, const DWORD linenum)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Line %u is too long in configuration file \"%s\".", linenum, filename);
}

// ---------

void Configuration::HandleErrorValueOutsideSection(LPCWSTR filename, const DWORD linenum)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Value at line %u specified outside of a section in configuration file \"%s\".", linenum, filename);
}

// ---------

void Configuration::HandleErrorDuplicateValue(LPCWSTR filename, const DWORD linenum, LPCWSTR section, LPCWSTR value)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Duplicate value \"%s\" in section \"%s\" on line %u of configuration file \"%s\".", value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorMalformedValue(LPCWSTR filename, const DWORD linenum, LPCWSTR section, LPCWSTR value)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Malformed setting for value \"%s\" in section \"%s\" on line %u of configuration file \"%s\".", value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorUnsupportedValue(LPCWSTR filename, const DWORD linenum, LPCWSTR section, LPCWSTR value)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Unsupported value \"%s\" in section \"%s\" on line %u of configuration file \"%s\".", value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorCannotApplyValue(LPCWSTR filename, const DWORD linenum, LPCWSTR setting, LPCWSTR section, LPCWSTR value)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Cannot apply setting \"%s\" for value \"%s\" in section \"%s\" on line %u of configuration file \"%s\".", setting, value, section, linenum, filename);
}

// ---------

void Configuration::HandleErrorFileIO(LPCWSTR filename)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"I/O error while attempting to read configuration file \"%s\".", filename);
}

// ---------

void Configuration::HandleErrorInternal(const DWORD code)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Internal error %u while attempting to read configuration file.", code);
}

// ---------

void Configuration::HandleSuccessAppliedValue(LPCWSTR filename, const DWORD linenum, LPCWSTR setting, LPCWSTR section, LPCWSTR value)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, L"Successfully applied setting \"%s\" for value \"%s\" in section \"%s\" on line %u of configuration file \"%s\".", setting, value, section, linenum, filename);
}
