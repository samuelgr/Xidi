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

#include <cstddef>
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

            /// Map of strings representing XInput controller elements to indices within the element map data structure.
            /// One pair exists per field in #SElementMap.
            static const std::map<std::wstring_view, unsigned int> kControllerElementStrings = {
                {L"StickLeftX", ELEMENT_MAP_INDEX_OF(stickLeftX)},
                {L"StickLeftY", ELEMENT_MAP_INDEX_OF(stickLeftY)},
                {L"StickRightX", ELEMENT_MAP_INDEX_OF(stickRightX)},
                {L"StickRightY", ELEMENT_MAP_INDEX_OF(stickRightY)},
                {L"DpadUp", ELEMENT_MAP_INDEX_OF(dpadUp)},
                {L"DpadDown", ELEMENT_MAP_INDEX_OF(dpadDown)},
                {L"DpadLeft", ELEMENT_MAP_INDEX_OF(dpadLeft)},
                {L"DpadRight", ELEMENT_MAP_INDEX_OF(dpadRight)},
                {L"TriggerLT", ELEMENT_MAP_INDEX_OF(triggerLT)},
                {L"TriggerRT", ELEMENT_MAP_INDEX_OF(triggerRT)},
                {L"ButtonA", ELEMENT_MAP_INDEX_OF(buttonA)},
                {L"ButtonB", ELEMENT_MAP_INDEX_OF(buttonB)},
                {L"ButtonX", ELEMENT_MAP_INDEX_OF(buttonX)},
                {L"ButtonY", ELEMENT_MAP_INDEX_OF(buttonY)},
                {L"ButtonLB", ELEMENT_MAP_INDEX_OF(buttonLB)},
                {L"ButtonRB", ELEMENT_MAP_INDEX_OF(buttonRB)},
                {L"ButtonBack", ELEMENT_MAP_INDEX_OF(buttonBack)},
                {L"ButtonStart", ELEMENT_MAP_INDEX_OF(buttonStart)},
                {L"ButtonLS", ELEMENT_MAP_INDEX_OF(buttonLS)},
                {L"ButtonRS", ELEMENT_MAP_INDEX_OF(buttonRS)}
            };


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

            /// Trims all whitespace from the front and end of the supplied string.
            /// @param [in] stringToTrim Input string to be trimmed.
            /// @return Trimmed version of the input string, which may be empty if the input string is entirely whitespace.
            static std::wstring_view TrimWhitespace(std::wstring_view stringToTrim)
            {
                const size_t kFirstNonWhitespacePosition = stringToTrim.find_first_not_of(kCharSetWhitespace);
                if (std::wstring_view::npos == kFirstNonWhitespacePosition)
                    return std::wstring_view();

                const size_t kLastNonWhitespacePosition = stringToTrim.find_last_not_of(kCharSetWhitespace);
                return stringToTrim.substr(kFirstNonWhitespacePosition, (1 + kLastNonWhitespacePosition - kFirstNonWhitespacePosition));
            }


            // -------- FUNCTIONS ------------------------------------------ //
            // See "MapperParser.h" for documentation.

            std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString)
            {
                const auto controllerElementIter = kControllerElementStrings.find(controllerElementString);

                if (kControllerElementStrings.cend() == controllerElementIter)
                    return std::nullopt;
                else
                    return controllerElementIter->second;
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

            std::optional<SElementMapperStringParts> ExtractParts(std::wstring_view elementMapperString)
            {
                std::wstring_view trimmedElementMapperString = TrimWhitespace(elementMapperString);
                if (true == trimmedElementMapperString.empty())
                    return std::nullopt;

                const size_t kOnePastTypeEndPosition = trimmedElementMapperString.find_first_of(kCharSetElementMapperTypeSeparator);
                if (std::wstring_view::npos == kOnePastTypeEndPosition)
                {
                    // No separator characters were found at all.
                    // Example: input string "     Null    MoreStuff    " was trimmed to "Null    MoreStuff" and then returned as the type because there is no separator.
                    return SElementMapperStringParts({.type = trimmedElementMapperString});
                }
                else if (kCharElementMapperBeginParams != trimmedElementMapperString[kOnePastTypeEndPosition])
                {
                    // A separator character was found but it does not begin a parameter list.
                    // Example: "Null,   Axis(X, +)" would result in the position of the comma, so the type is "Null" and the parameter list is empty.
                    return SElementMapperStringParts({.type = TrimWhitespace(trimmedElementMapperString.substr(0, kOnePastTypeEndPosition - 1))});
                }

                const size_t kParamListEndPosition = FindParamListEndPosition(trimmedElementMapperString.substr(kOnePastTypeEndPosition));
                if (std::wstring_view::npos == kParamListEndPosition)
                {
                    // A parameter list starting character was found with no matching end character. This is an error.
                    // Example: "Axis(X, +" which is missing the closing parenthesis.
                    return std::nullopt;
                }

                const size_t kParamListPos = 1 + kOnePastTypeEndPosition;
                const size_t kParamListLength = kParamListEndPosition - kParamListPos;
                return SElementMapperStringParts({.type = TrimWhitespace(trimmedElementMapperString.substr(0, kOnePastTypeEndPosition - 1)), .params = trimmedElementMapperString.substr(kParamListPos, kParamListLength)});
            }
        }
    }
}
