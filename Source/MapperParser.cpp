/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperParser.cpp
 *   Implementation of functionality for parsing pieces of mapper objects
 *   from strings, typically supplied in a configuration file.
 *****************************************************************************/


#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"
#include "MapperParser.h"

#include <climits>
#include <cstddef>
#include <cwchar>
#include <cwctype>
#include <map>
#include <optional>
#include <string_view>


namespace Xidi
{
    namespace Controller
    {
        namespace MapperParser
        {
            // -------- INTERNAL CONSTANTS --------------------------------- //

            /// Character used inside an element mapper string to indicate the beginning of a parameter list.
            static constexpr wchar_t kCharElementMapperBeginParams = '(';

            /// Character used inside an element mapper string to indicate the end of a parameter list.
            static constexpr wchar_t kCharElementMapperEndParams = L')';

            /// Character used inside an element mapper string to indicate a separation between parameters.
            static constexpr wchar_t kCharElementMapperParamSeparator = L',';

            /// Set of characters that are considered whitespace for the purpose of parsing element mapper strings.
            static constexpr wchar_t kCharSetWhitespace[] = L" \t";

            /// Set of characters that separate an element mapper type from the rest of the input string.
            /// For example, if parsing "SplitMapper ( Null, Axis(X, +) )" then we need to stop at the comma when extracting the type that corresponds to the "Null" portion of the string.
            /// As another example, if parsing "SplitMapper ( Axis(X, +), Null )" then we need to stop at the close parenthesis when extracting the type that corresponds to the "Null" portion of the string.
            /// Terminating null character is needed so that this array acts as a null-terminated string.
            static constexpr wchar_t kCharSetElementMapperTypeSeparator[] = {kCharElementMapperBeginParams, kCharElementMapperEndParams, kCharElementMapperParamSeparator, L'\0'};


            // -------- INTERNAL TYPES ------------------------------------- //

            /// Type for all functions that attempt to build individual element mappers given a parameter string.
            typedef std::optional<std::unique_ptr<IElementMapper>>(*TMakeElementMapperFunc)(std::wstring_view);


            // -------- INTERNAL FUNCTIONS --------------------------------- //

            /// Identifies the end position of the parameter list given a string that starts a parameter list.
            /// For example, if the element mapper string is "Axis(RotY, +)" then the input string should be "(RotY, +)" and this function will identify the position of the closing parenthesis.
            /// @param [in] paramListString Parameter list input string to search.
            /// @return Position of the end of the parameter list if the input string is valid and contains correct balance, or `npos` otherwise.
            static size_t FindParamListEndPosition(std::wstring_view paramListString)
            {
                unsigned int depth = 1;

                for (size_t pos = 1; pos < paramListString.length(); ++pos)
                {
                    switch (paramListString[pos])
                    {
                    case kCharElementMapperBeginParams:
                        depth += 1;
                        break;

                    case kCharElementMapperEndParams:
                        depth -= 1;
                        if (0 == depth)
                            return pos;
                        break;

                    default:
                        break;
                    }
                }

                return std::wstring_view::npos;
            }

            /// Parses a relatively small unsigned integer value from the supplied input string.
            /// A maximum of 8 characters are permitted, meaning any parsed values are guaranteed to fit into 32 bits.
            /// This function will fail if the input string is too long or if it does not entirely represent an unsigned integer value.
            /// @param [in] uintString String from which to parse.
            /// @param [in] base Representation base of the number, which defaults to decimal.
            /// @return Parsed integer value if successful.
            static std::optional<unsigned int> ParseUnsignedInteger(std::wstring_view uintString, int base = 10)
            {
                constexpr size_t kMaxChars = 8;

                if (true == uintString.empty())
                    return std::nullopt;

                if (uintString.length() > kMaxChars)
                    return std::nullopt;

                // Create a null-terminated version of the number by copying the digits into a small buffer.
                wchar_t convertBuffer[1 + kMaxChars];
                convertBuffer[kMaxChars] = L'\0';
                for (size_t i = 0; i < kMaxChars; ++i)
                {
                    if (i < uintString.length())
                    {
                        if (iswxdigit(uintString[i]))
                            convertBuffer[i] = uintString[i];
                        else
                            return std::nullopt;
                    }
                    else
                    {
                        convertBuffer[i] = L'\0';
                        break;
                    }
                }
                
                wchar_t* endptr = nullptr;
                unsigned int parsedValue = (unsigned int)wcstoul(convertBuffer, &endptr, base);

                // Verify that the whole string was consumed.
                if (L'\0' != *endptr)
                    return std::nullopt;

                return parsedValue;
            }

            /// Trims all whitespace from the back of the supplied string.
            /// @param [in] stringToTrim Input string to be trimmed.
            /// @return Trimmed version of the input string, which may be empty if the input string is entirely whitespace.
            static inline std::wstring_view TrimWhitespaceBack(std::wstring_view stringToTrim)
            {
                const size_t kLastNonWhitespacePosition = stringToTrim.find_last_not_of(kCharSetWhitespace);
                if (std::wstring_view::npos == kLastNonWhitespacePosition)
                    return std::wstring_view();

                return stringToTrim.substr(0, 1 + kLastNonWhitespacePosition);
            }

            /// Trims all whitespace from the front of the supplied string.
            /// @param [in] stringToTrim Input string to be trimmed.
            /// @return Trimmed version of the input string, which may be empty if the input string is entirely whitespace.
            static inline std::wstring_view TrimWhitespaceFront(std::wstring_view stringToTrim)
            {
                const size_t kFirstNonWhitespacePosition = stringToTrim.find_first_not_of(kCharSetWhitespace);
                if (std::wstring_view::npos == kFirstNonWhitespacePosition)
                    return std::wstring_view();

                return stringToTrim.substr(kFirstNonWhitespacePosition);
            }

            /// Trims all whitespace from the front and back of the supplied string.
            /// @param [in] stringToTrim Input string to be trimmed.
            /// @return Trimmed version of the input string, which may be empty if the input string is entirely whitespace.
            static inline std::wstring_view TrimWhitespace(std::wstring_view stringToTrim)
            {
                return TrimWhitespaceBack(TrimWhitespaceFront(stringToTrim));
            }


            // -------- FUNCTIONS ------------------------------------------ //
            // See "MapperParser.h" for documentation.

            std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString)
            {
                // Map of strings representing controller elements to indices within the element map data structure.
                // One pair exists per field in the SElementMap structure.
                static const std::map<std::wstring_view, unsigned int> kControllerElementStrings = {
                    {L"StickLeftX",     ELEMENT_MAP_INDEX_OF(stickLeftX)},
                    {L"StickLeftY",     ELEMENT_MAP_INDEX_OF(stickLeftY)},
                    {L"StickRightX",    ELEMENT_MAP_INDEX_OF(stickRightX)},
                    {L"StickRightY",    ELEMENT_MAP_INDEX_OF(stickRightY)},
                    {L"DpadUp",         ELEMENT_MAP_INDEX_OF(dpadUp)},
                    {L"DpadDown",       ELEMENT_MAP_INDEX_OF(dpadDown)},
                    {L"DpadLeft",       ELEMENT_MAP_INDEX_OF(dpadLeft)},
                    {L"DpadRight",      ELEMENT_MAP_INDEX_OF(dpadRight)},
                    {L"TriggerLT",      ELEMENT_MAP_INDEX_OF(triggerLT)},
                    {L"TriggerRT",      ELEMENT_MAP_INDEX_OF(triggerRT)},
                    {L"ButtonA",        ELEMENT_MAP_INDEX_OF(buttonA)},
                    {L"ButtonB",        ELEMENT_MAP_INDEX_OF(buttonB)},
                    {L"ButtonX",        ELEMENT_MAP_INDEX_OF(buttonX)},
                    {L"ButtonY",        ELEMENT_MAP_INDEX_OF(buttonY)},
                    {L"ButtonLB",       ELEMENT_MAP_INDEX_OF(buttonLB)},
                    {L"ButtonRB",       ELEMENT_MAP_INDEX_OF(buttonRB)},
                    {L"ButtonBack",     ELEMENT_MAP_INDEX_OF(buttonBack)},
                    {L"ButtonStart",    ELEMENT_MAP_INDEX_OF(buttonStart)},
                    {L"ButtonLS",       ELEMENT_MAP_INDEX_OF(buttonLS)},
                    {L"ButtonRS",       ELEMENT_MAP_INDEX_OF(buttonRS)}
                };

                const auto controllerElementIter = kControllerElementStrings.find(controllerElementString);

                if (kControllerElementStrings.cend() == controllerElementIter)
                    return std::nullopt;
                else
                    return controllerElementIter->second;
            }

            // --------

            std::optional<std::unique_ptr<IElementMapper>> ElementMapperFromString(std::wstring_view elementMapperString)
            {
                static constexpr unsigned int kMaxRecursionDepth = 3;

                const unsigned int kRecursionDepth = ComputeRecursionDepth(elementMapperString).value_or(UINT_MAX);
                if (kRecursionDepth > kMaxRecursionDepth)
                    return std::nullopt;

                SElementMapperParseResult parseResult = ParseSingleElementMapper(elementMapperString);
                if ((false == parseResult.maybeElementMapper.has_value()) || (false == parseResult.remainingString.empty()))
                    return std::nullopt;

                return std::move(parseResult.maybeElementMapper.value());
            }

            // --------

            bool IsControllerElementStringValid(std::wstring_view element)
            {
                return FindControllerElementIndex(element).has_value();
            }


            // -------- HELPERS -------------------------------------------- //
            // See "MapperParser.h" for documentation.

            std::optional<unsigned int> ComputeRecursionDepth(std::wstring_view elementMapperString)
            {
                unsigned int recursionDepth = 0;
                unsigned int maxRecursionDepth = 0;

                for (auto elementMapperChar : elementMapperString)
                {
                    switch (elementMapperChar)
                    {
                    case kCharElementMapperBeginParams:
                        recursionDepth += 1;
                        if (recursionDepth > maxRecursionDepth)
                            maxRecursionDepth = recursionDepth;
                        break;

                    case kCharElementMapperEndParams:
                        if (0 == recursionDepth)
                            return std::nullopt;
                        recursionDepth -= 1;
                        break;

                    default:
                        break;
                    }
                }

                if (0 != recursionDepth)
                    return std::nullopt;

                return maxRecursionDepth;
            }

            // --------

            std::optional<SElementMapperStringParts> ExtractElementMapperStringParts(std::wstring_view elementMapperString)
            {
                // First, look for the end of the "type" part of the string. This just means looking for the first possible separator character.
                // Example: "Axis(X)" means the type is "Axis"
                // Example: "Split(Null, Pov(Up))" means the type is "Split"
                // Example: "Null, Pov(Up)" (parameters in the above example) means the type is "Null"
                const size_t kSeparatorPosition = elementMapperString.find_first_of(kCharSetElementMapperTypeSeparator);

                if (std::wstring_view::npos == kSeparatorPosition)
                {
                    // No separator characters were found at all.
                    // Example: "Null" in which case we got to the end of the string with no separator character identified.
                    
                    // The entire string is consumed and is the type. There are no parameters and no remaining parts to the string.
                    return SElementMapperStringParts({.type = TrimWhitespace(elementMapperString)});
                }
                else if (kCharElementMapperBeginParams != elementMapperString[kSeparatorPosition])
                {
                    // A separator character was found but it does not begin a parameter list.
                    // Example: "Null, Pov(Up)" in which case we parsed the "Null" and stopped at the comma.

                    // The only possible separator character in this situation is a comma, otherwise it is an error.
                    if (kCharElementMapperParamSeparator != elementMapperString[kSeparatorPosition])
                        return std::nullopt;

                    // Type string goes up to the separator character and the remaining part of the string comes afterwards.
                    const std::wstring_view kTypeString = TrimWhitespace(elementMapperString.substr(0, kSeparatorPosition));
                    const std::wstring_view kRemainingString = TrimWhitespace(elementMapperString.substr(1 + kSeparatorPosition));

                    // If the remaining string is empty, it means the comma is a dangling comma which is a syntax error.
                    if (true == kRemainingString.empty())
                        return std::nullopt;

                    return SElementMapperStringParts({.type = kTypeString, .remaining = kRemainingString});
                }
                else
                {
                    // A separator character was found and it does begin a parameter list.
                    // Example: "Axis(X)" and "Split(Null, Pov(Up))" in both of which cases we stopped at the open parenthesis character.
                    const size_t kParamListStartPos = 1 + kSeparatorPosition;
                    const size_t kParamListLength = FindParamListEndPosition(elementMapperString.substr(kParamListStartPos));
                    const size_t kParamListEndPos = kParamListLength + kParamListStartPos;

                    // It is an error if a parameter list starting character was found with no matching end character.
                    if (std::wstring_view::npos == kParamListLength)
                        return std::nullopt;
                                        
                    // Figure out what part of the string is remaining. It is possible that there is nothing left or that we stopped at a comma.
                    // Example: "Axis(X)" in which case "Axis" is the type, "X" is the parameter string, and there is nothing left as a remainder. This is true whether or not there are dangling whitespace characters at the end of the input.
                    // Example: "Split(Button(1), Button(2)), Split(Button(3), Button(4))" in which case "Split" is the type, "Button(1), Button(2)" is the parameter string, and we need to skip over the comma to obtain "Split(Button(3), Button(4))" as the remaining string.
                    std::wstring_view possibleRemainingString = TrimWhitespaceFront(elementMapperString.substr(1 + kParamListEndPos));
                    if (false == possibleRemainingString.empty())
                    {
                        // The only possible separator that would have given rise to this situation is a comma.
                        // Any other character at this point indicates an error.
                        if (kCharElementMapperParamSeparator != possibleRemainingString.front())
                            return std::nullopt;

                        // If after skipping over the comma there is nothing left, then the comma is a dangling comma which is an error.
                        possibleRemainingString = TrimWhitespace(possibleRemainingString.substr(1));
                        if (true == possibleRemainingString.empty())
                            return std::nullopt;
                    }
                    
                    const std::wstring_view kTypeString = TrimWhitespace(elementMapperString.substr(0, kSeparatorPosition));
                    const std::wstring_view kParamString = TrimWhitespace(elementMapperString.substr(kParamListStartPos, kParamListLength));
                    const std::wstring_view kRemainingString = possibleRemainingString;

                    return SElementMapperStringParts({.type = kTypeString, .params = kParamString, .remaining = kRemainingString});
                }
            }

            // --------

            std::optional<std::unique_ptr<IElementMapper>> MakeButtonMapper(std::wstring_view params)
            {
                const std::optional<unsigned int> kMaybeButtonNumber = ParseUnsignedInteger(params);
                if (false == kMaybeButtonNumber.has_value())
                    return std::nullopt;

                const unsigned int kButtonNumber = kMaybeButtonNumber.value() - 1;
                if (kButtonNumber >= (unsigned int)EButton::Count)
                    return std::nullopt;

                return std::make_unique<ButtonMapper>((EButton)kButtonNumber);
            }

            // --------

            std::optional<std::unique_ptr<IElementMapper>> MakeNullMapper(std::wstring_view params)
            {
                if (false == params.empty())
                    return std::nullopt;

                return nullptr;
            }

            // --------

            SElementMapperParseResult ParseSingleElementMapper(std::wstring_view elementMapperString)
            {
                static const std::map<std::wstring_view, TMakeElementMapperFunc> kMakeElementMapperFunctions = {
                    {L"button",         &MakeButtonMapper},
                    {L"Button",         &MakeButtonMapper},

                    {L"null",           &MakeNullMapper},
                    {L"Null",           &MakeNullMapper},
                    {L"nothing",        &MakeNullMapper},
                    {L"Nothing",        &MakeNullMapper},
                    {L"none",           &MakeNullMapper},
                    {L"None",           &MakeNullMapper},
                    {L"empty",          &MakeNullMapper},
                    {L"Empty",          &MakeNullMapper},
                };

                const std::optional<SElementMapperStringParts> kMaybeElementMapperStringParts = ExtractElementMapperStringParts(elementMapperString);
                if (false == kMaybeElementMapperStringParts.has_value())
                    return {.maybeElementMapper = std::nullopt};

                const SElementMapperStringParts& kElementMapperStringParts = kMaybeElementMapperStringParts.value();

                const auto kMakeElementMapperIter = kMakeElementMapperFunctions.find(kElementMapperStringParts.type);
                if (kMakeElementMapperFunctions.cend() == kMakeElementMapperIter)
                    return {.maybeElementMapper = std::nullopt};

                return {.maybeElementMapper = kMakeElementMapperIter->second(kElementMapperStringParts.params), .remainingString = kElementMapperStringParts.remaining};
            }
        }
    }
}
