/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Configuration.cpp
 *   Implementation of configuration file functionality.
 *****************************************************************************/

#include "Configuration.h"
#include "TemporaryBuffer.h"

#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>


namespace Xidi
{
    namespace Configuration
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Enumerates all possible classifications of configuration file lines.
        /// Used during parsing to classify each line encountered.
        enum class ELineClassification
        {
            Error,                                                          ///< Line could not be parsed.
            Ignore,                                                         ///< Line should be ignored, either because it is just whitespace or because it is a comment.
            Section,                                                        ///< Line begins a section, whose name appears in square brackets.
            Value,                                                          ///< Line is a value within the current section and so should be parsed.
        };

        /// Wrapper around a standard file handle.
        /// Attempts to open the specified file on construction and close it on destruction.
        struct FileHandle
        {
            FILE* fileHandle = nullptr;

            inline operator FILE*(void) const
            {
                return fileHandle;
            }

            inline FILE** operator&(void)
            {
                return &fileHandle;
            }

            inline ~FileHandle(void)
            {
                if (nullptr != fileHandle)
                    fclose(fileHandle);
            }
        };


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Tests if the supplied character is allowed as a configuration setting name (the part before the '=' sign in the configuration file).
        /// @param [in] charToTest Character to test.
        /// @return `true` if so, `false` if not.
        static bool IsAllowedNameCharacter(const wchar_t charToTest)
        {
            switch (charToTest)
            {
            case L'.':
                return true;

            default:
                return (iswalnum(charToTest) ? true : false);
            }
        }

        /// Tests if the supplied character is allowed as a section name (appears between square brackets in the configuration file).
        /// @param [in] charToTest Character to test.
        /// @return `true` if so, `false` if not.
        static bool IsAllowedSectionCharacter(const wchar_t charToTest)
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
            case L'}':
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
                return (iswalnum(charToTest) ? true : false);
            }
        }

        /// Tests if the supplied character is allowed as a configuration setting value (the part after the '=' sign in the configuration file).
        /// @param [in] charToTest Character to test.
        /// @return `true` if so, `false` if not.
        static bool IsAllowedValueCharacter(const wchar_t charToTest)
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
                return (iswalnum(charToTest) ? true : false);
            }
        }

        /// Classifies the provided configuration file line and returns a value indicating the result.
        /// @param [in] buf Buffer containing the configuration file line.
        /// @param [in] length Number of characters in the buffer.
        /// @return Configuration line classification.
        static ELineClassification ClassifyConfigurationFileLine(const wchar_t* const buf, const int length)
        {
            // Skip over all whitespace at the start of the input line.
            const wchar_t* realBuf = buf;
            int realLength = length;
            while (realLength != 0 && iswblank(realBuf[0]))
            {
                realLength -= 1;
                realBuf += 1;
            }

            // Sanity check: zero-length and all-whitespace lines can be safely ignored.
            // Also filter out comments this way.
            if (0 == realLength || L';' == realBuf[0] || L'#' == realBuf[0])
                return ELineClassification::Ignore;

            // Non-comments must, by definition, have at least three characters in them, excluding all whitespace.
            // For section headers, this must mean '[' + section name + ']'.
            // For values, this must mean name + '=' + value.
            if (realLength < 3)
                return ELineClassification::Error;

            if (L'[' == realBuf[0])
            {
                // The line cannot be a section header unless the second character is a valid section name character.
                if (!IsAllowedSectionCharacter(realBuf[1]))
                    return ELineClassification::Error;

                // Verify that the line is a valid section header by checking for valid section name characters between two square brackets.
                int i = 2;
                for (; i < realLength && L']' != realBuf[i]; ++i)
                {
                    if (!IsAllowedSectionCharacter(realBuf[i]))
                        return ELineClassification::Error;
                }
                if (L']' != realBuf[i])
                    return ELineClassification::Error;

                // Verify that the remainder of the line is just whitespace.
                for (i += 1; i < realLength; ++i)
                {
                    if (!iswblank(realBuf[i]))
                        return ELineClassification::Error;
                }

                return ELineClassification::Section;
            }
            else if (IsAllowedNameCharacter(realBuf[0]))
            {
                // Search for whitespace or an equals sign, with all characters in between needing to be allowed as value name characters.
                int i = 1;
                for (; i < realLength && L'=' != realBuf[i] && !iswblank(realBuf[i]); ++i)
                {
                    if (!IsAllowedNameCharacter(realBuf[i]))
                        return ELineClassification::Error;
                }

                // Skip over any whitespace present, then check for an equals sign.
                for (; i < realLength && iswblank(realBuf[i]); ++i);
                if (L'=' != realBuf[i])
                    return ELineClassification::Error;

                // Skip over any whitespace present, then verify the next character is allowed to start a value setting.
                for (i += 1; i < realLength && iswblank(realBuf[i]); ++i);
                if (!IsAllowedValueCharacter(realBuf[i]))
                    return ELineClassification::Error;

                // Skip over the value setting characters that follow.
                for (i += 1; i < realLength && IsAllowedValueCharacter(realBuf[i]); ++i);

                // Verify that the remainder of the line is just whitespace.
                for (; i < realLength; ++i)
                {
                    if (!iswblank(realBuf[i]))
                        return ELineClassification::Error;
                }

                return ELineClassification::Value;
            }

            return ELineClassification::Error;
        }

        /// Creates a string based on formatting another one.
        /// @param [out] dest String to which to output the result of formatting..
        /// @param [in] format String to format, possibly with format specifiers.
        static void FormatString(std::wstring& dest, const wchar_t* const format, ...)
        {
            TemporaryBuffer<wchar_t> buf;

            va_list args;
            va_start(args, format);

            vswprintf_s(buf, buf.Count(), format, args);

            va_end(args);

            dest = buf;
        }

        /// Parses a Boolean from the supplied input string.
        /// @param [in] source String from which to parse.
        /// @param [out] dest Filled with the result of the parse.
        /// @return `true` if the parse was successful and able to consume the whole string, `false` otherwise.
        static bool ParseBoolean(const TStringValue& source, TBooleanValue* const dest)
        {
            static const wchar_t* const trueStrings[] = { L"t", L"true", L"on", L"y", L"yes", L"enabled", L"1" };
            static const wchar_t* const falseStrings[] = { L"f", L"false", L"off", L"n", L"no", L"disabled", L"0" };

            // Check if the string represents a value of TRUE.
            for (int i = 0; i < _countof(trueStrings); ++i)
            {
                if (0 == _wcsicmp(source.c_str(), trueStrings[i]))
                {
                    *dest = (TBooleanValue)true;
                    return true;
                }
            }

            // Check if the string represents a value of FALSE.
            for (int i = 0; i < _countof(falseStrings); ++i)
            {
                if (0 == _wcsicmp(source.c_str(), falseStrings[i]))
                {
                    *dest = (TBooleanValue)false;
                    return true;
                }
            }

            return false;
        }

        /// Parses a signed integer value from the supplied input string.
        /// @param [in] source String from which to parse.
        /// @param [out] dest Filled with the result of the parse.
        /// @return `true` if the parse was successful and able to consume the whole string, `false` otherwise.
        static bool ParseInteger(const TStringValue& source, TIntegerValue* const dest)
        {
            intmax_t value = 0ll;
            wchar_t* endptr = nullptr;

            // Parse out a number in any representable base.
            value = wcstoll(source.c_str(), &endptr, 0);

            // Verify that the number is not out of range.
            if (ERANGE == errno && (LLONG_MIN == value || LLONG_MAX == value))
                return false;
            if (value > std::numeric_limits<TIntegerValue>::max() || value < std::numeric_limits<TIntegerValue>::min())
                return false;

            // Verify that the whole string was consumed.
            if (L'\0' != *endptr)
                return false;

            *dest = (TIntegerValue)value;
            return true;
        }

        /// Parses a name and a value for the specified configuration file line, which must first have been classified as containing a value.
        /// @param [in] configFileLine Buffer containing the configuration file line.
        /// @param [out] nameString Filled with the name of the configuration setting.
        /// @param [out] valueString Filled with the value specified for the configuration setting.
        static void ParseNameAndValue(const wchar_t* const configFileLine, std::wstring& nameString, TStringValue& valueString)
        {
            // Skip to the start of the name part of the line.
            const wchar_t* name = configFileLine;
            while (iswblank(name[0]))
                name += 1;

            // Find the length of the configuration name.
            int nameLength = 1;
            while (IsAllowedNameCharacter(name[nameLength]))
                nameLength += 1;

            // Advance to the value portion of the string.
            const wchar_t* value = &name[nameLength + 1];

            // Skip over whitespace and the '=' sign.
            while ((L'=' == value[0]) || (iswblank(value[0])))
                value += 1;

            // Find the length of the configuration value.
            int valueLength = 1;
            while (IsAllowedValueCharacter(value[valueLength]))
                valueLength += 1;

            nameString = std::move(std::wstring(name, nameLength));
            valueString = std::move(TStringValue(value, valueLength));
        }

        /// Parses a section name from the specified configuration file line, which must first have been classified as containing a section name.
        /// @param [in] configFileLine Buffer containing the configuration file line.
        /// @param [out] sectionString Filled with the name of the configuration section.
        static void ParseSectionName(const wchar_t* const configFileLine, std::wstring& sectionString)
        {
            // Skip to the '[' character and then advance once more to the section name itself.
            const wchar_t* section = configFileLine;
            while (L'[' != section[0])
                section += 1;
            section += 1;

            // Find the length of the section name.
            int sectionLength = 1;
            while (L']' != section[sectionLength])
                sectionLength += 1;

            sectionString = std::move(std::wstring(section, sectionLength));
        }

        /// Reads a single line from the specified file handle, verifies that it fits within the specified buffer, and removes trailing whitespace.
        /// @param [in] filehandle Handle to the file from which to read.
        /// @param [out] lineBuffer Filled with text read from the specified file.
        /// @param [in] lineBufferCount Length, in character units, of the line buffer.
        /// @return Length of the string that was read, with -1 indicating an error condition.
        static int ReadAndTrimLine(FILE* const filehandle, wchar_t* const lineBuffer, const int lineBufferCount)
        {
            // Results in a null-terminated string guaranteed, but might not be the whole line if the buffer is too small.
            if (lineBuffer != fgetws(lineBuffer, lineBufferCount, filehandle))
                return -1;

            // If the line fits in the buffer, then either its detected length is small by comparison to the buffer size or, if it perfectly fits in the buffer, then the last character is a newline.
            int lineLength = (int)wcsnlen(lineBuffer, lineBufferCount);
            if (((lineBufferCount - 1) == lineLength) && (L'\n' != lineBuffer[lineLength - 1]))
                return -1;

            // Trim off any whitespace on the end of the line.
            while (iswspace(lineBuffer[lineLength - 1]))
                lineLength -= 1;

            lineBuffer[lineLength] = L'\0';

            return lineLength;
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "Configuration.h" for documentation.

        EFileReadResult ConfigurationFileReader::ReadConfigurationFile(std::wstring_view configFileName, ConfigurationData& configToFill)
        {
            FileHandle configFileHandle;
            _wfopen_s(&configFileHandle, configFileName.data(), L"r");

            if (nullptr == configFileHandle)
            {
                FormatString(readErrorMessage, L"%s - Unable to open configuration file.", configFileName.data());
                return EFileReadResult::FileNotFound;
            }

            PrepareForRead();

            // Parse the configuration file, one line at a time.
            std::unordered_set<std::wstring> seenSections;
            std::wstring thisSection = kSectionNameGlobal;

            int configLineNumber = 1;
            TemporaryBuffer<wchar_t> configLineBuffer;
            int configLineLength = ReadAndTrimLine(configFileHandle, configLineBuffer, configLineBuffer.Count());
            bool skipValueLines = false;

            while (configLineLength >= 0)
            {
                switch (ClassifyConfigurationFileLine(configLineBuffer, configLineLength))
                {
                case ELineClassification::Error:
                    FormatString(readErrorMessage, L"%s:%d - Unable to parse line.", configFileName.data(), configLineNumber);
                    return EFileReadResult::Malformed;

                case ELineClassification::Ignore:
                    break;

                case ELineClassification::Section:
                {
                    std::wstring section;
                    ParseSectionName(configLineBuffer, section);

                    if (0 != seenSections.count(section))
                    {
                        FormatString(readErrorMessage, L"%s:%d - Section \"%s\" is duplicated.", configFileName.data(), configLineNumber, section.c_str());
                        return EFileReadResult::Malformed;
                    }

                    const ESectionAction sectionAction = ActionForSection(section);
                    switch (sectionAction)
                    {
                    case ESectionAction::Error:
                        FormatString(readErrorMessage, L"%s:%d - Section \"%s\" is invalid.", configFileName.data(), configLineNumber, section.c_str());
                        return EFileReadResult::Malformed;

                    case ESectionAction::Read:
                        thisSection = std::move(section);
                        seenSections.insert(thisSection);
                        skipValueLines = false;
                        break;

                    case ESectionAction::Skip:
                        skipValueLines = true;
                        break;

                    default:
                        FormatString(readErrorMessage, L"%s:%d - Internal error while processing section name.", configFileName.data(), configLineNumber);
                        return EFileReadResult::Malformed;
                    }
                }
                break;

                case ELineClassification::Value:
                    if (false == skipValueLines)
                    {
                        std::wstring name;
                        TStringValue value;
                        ParseNameAndValue(configLineBuffer, name, value);

                        const EValueType valueType = TypeForValue(thisSection, name);

                        // If the value type does not identify it as multi-valued, make sure this is the first time the setting is seen.
                        switch (valueType)
                        {
                        case EValueType::Integer:
                        case EValueType::Boolean:
                        case EValueType::String:
                            if (configToFill.SectionNamePairExists(thisSection, name))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Configuration setting \"%s\" only supports a single value.", configFileName.data(), configLineNumber, name.c_str());
                                return EFileReadResult::Malformed;
                            }

                        default:
                            break;
                        }

                        // Attempt to parse the value.
                        switch (valueType)
                        {
                        case EValueType::Error:
                            FormatString(readErrorMessage, L"%s:%d - Configuration setting \"%s\" is invalid.", configFileName.data(), configLineNumber, name.c_str());
                            return EFileReadResult::Malformed;

                        case EValueType::Integer:
                        case EValueType::IntegerMultiValue:
                        {
                            TIntegerValue intValue = (TIntegerValue)0;

                            if (false == ParseInteger(value, &intValue))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Value \"%s\" is not a valid integer.", configFileName.data(), configLineNumber, value.c_str());
                                return EFileReadResult::Malformed;
                            }

                            if (false == CheckValue(thisSection, name, intValue))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Configuration setting \"%s\" with value \"%s\" is invalid.", configFileName.data(), configLineNumber, name.c_str(), value.c_str());
                                return EFileReadResult::Malformed;
                            }

                            if (false == configToFill.Insert(thisSection, name, intValue))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Value \"%s\" for configuration setting \"%s\" is duplicated.", configFileName.data(), configLineNumber, value.c_str(), name.c_str());
                                return EFileReadResult::Malformed;
                            }
                        }
                        break;

                        case EValueType::Boolean:
                        case EValueType::BooleanMultiValue:
                        {
                            TBooleanValue boolValue = (TBooleanValue)false;

                            if (false == ParseBoolean(value, &boolValue))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Value \"%s\" is not a valid Boolean.", configFileName.data(), configLineNumber, value.c_str());
                                return EFileReadResult::Malformed;
                            }

                            if (false == CheckValue(thisSection, name, boolValue))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Configuration setting \"%s\" with value \"%s\" is invalid.", configFileName.data(), configLineNumber, name.c_str(), value.c_str());
                                return EFileReadResult::Malformed;
                            }

                            if (false == configToFill.Insert(thisSection, name, boolValue))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Value \"%s\" for configuration setting \"%s\" is duplicated.", configFileName.data(), configLineNumber, value.c_str(), name.c_str());
                                return EFileReadResult::Malformed;
                            }
                        }
                        break;

                        case EValueType::String:
                        case EValueType::StringMultiValue:
                            if (false == CheckValue(thisSection, name, value))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Configuration setting \"%s\" with value \"%s\" is invalid.", configFileName.data(), configLineNumber, name.c_str(), value.c_str());
                                return EFileReadResult::Malformed;
                            }

                            if (false == configToFill.Insert(thisSection, name, value))
                            {
                                FormatString(readErrorMessage, L"%s:%d - Value \"%s\" for configuration setting \"%s\" is duplicated.", configFileName.data(), configLineNumber, value.c_str(), name.c_str());
                                return EFileReadResult::Malformed;
                            }
                            break;

                        default:
                            FormatString(readErrorMessage, L"%s:%d - Internal error while processing configuration setting.", configFileName.data(), configLineNumber);
                            return EFileReadResult::Malformed;
                        }
                    }
                    break;

                default:
                    FormatString(readErrorMessage, L"%s:%d - Internal error while processing line.", configFileName.data(), configLineNumber);
                    return EFileReadResult::Malformed;
                }

                configLineLength = ReadAndTrimLine(configFileHandle, configLineBuffer, configLineBuffer.Count());
                configLineNumber += 1;
            }

            if (!feof(configFileHandle))
            {
                // Stopped reading the configuration file early due to some condition other than end-of-file.
                // This indicates an error.

                if (ferror(configFileHandle))
                {
                    FormatString(readErrorMessage, L"%s - I/O error while reading.", configFileName.data(), configLineNumber);
                    return EFileReadResult::Malformed;

                }
                else if (configLineLength < 0)
                {
                    FormatString(readErrorMessage, L"%s:%d - Line is too long.", configFileName.data(), configLineNumber);
                    return EFileReadResult::Malformed;
                }
            }

            return EFileReadResult::Success;
        }


        // -------- CONCRETE INSTANCE METHODS ------------------------------ //
        // See "Configuration.h" for documentation.

        void ConfigurationFileReader::PrepareForRead(void)
        {
            // Nothing to do here.
        }
    }
}
