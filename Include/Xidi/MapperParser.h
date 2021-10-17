/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file MapperParser.h
 *   Declaration of functionality for parsing pieces of mapper objects from
 *   strings, typically supplied in a configuration file.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Mapper.h"

#include <memory>
#include <optional>
#include <string_view>


namespace Xidi
{
    namespace Controller
    {
        namespace MapperParser
        {
            // -------- TYPE DEFINITIONS ----------------------------------- //

            /// Holds a partially-separated representation of an element mapper string.
            /// This view of the element mapper string is separated into type and parameter portions.
            /// For example, the string "Axis(RotY, +)" would be separated into "Axis" as the type and "RotY, +" (the entire contents of the parentheses) as the parameters.
            struct SElementMapperStringParts
            {
                std::wstring_view type;                                             ///< String identifying the element mapper type.
                std::wstring_view params;                                           ///< String holding all of the parameters without the enclosing parentheses.
                std::wstring_view remaining;                                        ///< Remaining part of the string that was not separated into parts.

                /// Simple check for equality by field-by-field comparison.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                inline bool operator==(const SElementMapperStringParts& other) const
                {
                    return ((other.type == type) && (other.params == params) && (other.remaining == remaining));
                }
            };

            /// Holds the result of parsing and consuming a single element mapper worth of input string.
            struct SElementMapperParseResult
            {
                std::optional<std::unique_ptr<IElementMapper>> maybeElementMapper;  ///< Element mapper object, if the parse was successful. Note that `nullptr` indicates successful parse of a null element mapper.
                std::wstring_view remainingString;                                  ///< Remaining unparsed part of the string. Either empty or contains additional element mapper strings to be parsed.
            };

            /// Holds a partially-separated representation of a parameter string.
            struct SParamStringParts
            {
                std::wstring_view first;                                            ///< String representing just the first parameter.
                std::wstring_view remaining;                                        ///< String representing all the remaining parameters.

                /// Simple check for equality by field-by-field comparison.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                inline bool operator==(const SParamStringParts& other) const
                {
                    return ((other.first == first) && (other.remaining == remaining));
                }
            };


            // -------- FUNCTIONS ------------------------------------------ //

            /// Attempts to identify the index within the `all` member of #UElementMap that corresponds to the controller element identified by the input string.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] controllerElementString String to parse that supposedly identifies a controller element.
            /// @return Element map array index, if it could be identified based on the input string.
            std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString);

            /// Attempts to build an element mapper using the supplied string.
            /// This is the main entry point intended for use when parsing element mappers from strings.
            /// @param [in] elementMapperString Input string supposedly representing an element mapper.
            /// @return Pointer to the new mapper object if successful.
            std::optional<std::unique_ptr<IElementMapper>> ElementMapperFromString(std::wstring_view elementMapperString);

            /// Determines if the specified controller element string is valid and recognized as identifying a controller element.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] controllerElementString String to be checked.
            /// @return `true` if the input string is recognized, `false` otherwise.
            bool IsControllerElementStringValid(std::wstring_view controllerElementString);


            // -------- HELPERS -------------------------------------------- //
            // Internal functions exposed for testing.

            /// Computes the recursion depth of the specified element mapper string.
            /// Some element mappers contain other embedded element mappers, which introduces a recursive aspect to parsing element mapper strings.
            /// For simple mapper types that take parameters identifying a controller element, the recursion depth is 1..
            /// For a null mapper identified without any parameters, the recursion depth is 0.
            /// For more complex mapper types, the recursion depth can be arbitrary.
            /// If the input string does not contain an even number of parameter list starting and ending characters, the recursion is unbalanced and the depth cannot be determined.
            /// @param [in] elementMapperString Input string supposedly representing an element mapper.
            /// @return Recursion depth if the recursion is balanced and therefore the depth can be determined.
            std::optional<unsigned int> ComputeRecursionDepth(std::wstring_view elementMapperString);

            /// Separates the supplied element mapper string into type and parameter parts, with a possible remainder, and returns the result.
            /// Example: "Axis(X)" would be split into "Axis" and "X" as type and parameters respectively.
            /// Example: "Split( Split(Button(1), Button(2)), Split(Button(3), Button(4)) )" would be split into "Split" and "Split(Button(1), Button(2)), Split(Button(3), Button(4))" as type and parameters respectively.
            /// Example: "Split(Button(1), Button(2)), Split(Button(3), Button(4))" would be split into "Split" and "Button(1), Button(2)" as type and parameters respectively, with "Split(Button(3), Button(4))" indicated as remaining.
            /// @param [in] elementMapperString Input string supposedly representing an element mapper.
            /// @return Structure of separated string parts, if successful.
            std::optional<SElementMapperStringParts> ExtractElementMapperStringParts(std::wstring_view elementMapperString);

            /// Partially parses the supplied parameter list string by extracting the first parameter and leaving behind the rest of the string.
            /// Example: "A, B, C, D" would result in "A" as the first parameter and "B, C, D" as the remaining part of the string.
            /// Example: "RotY, +" would result in "RotY" as the first parameter and "+" as the remaining part of the string.
            /// Example: "Split(Button(1), Button(2)), Split(Button(3), Button(4))" would result in "Split(Button(1), Button(2))" as the first parameter and "Split(Button(3), Button(4))" as the remaining part of the string.
            /// @param [in] elementMapperString Input string supposedly representing an element mapper.
            /// @return Structure of separated parameter list string parts, if successful.
            std::optional<SParamStringParts> ExtractParameterListStringParts(std::wstring_view paramListString);

            /// Attempts to build an #AxisMapper using the supplied parameters.
            /// Parameter string should consist of a string representing an axis and optionally a second string representing an axis direction.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful.
            std::optional<std::unique_ptr<IElementMapper>> MakeAxisMapper(std::wstring_view params);

            /// Attempts to build a #ButtonMapper using the supplied parameters.
            /// Parameter string should consist of a single integer identifying the button number.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful.
            std::optional<std::unique_ptr<IElementMapper>> MakeButtonMapper(std::wstring_view params);

            /// Attempts to build a #KeyboardMapper using the supplied parameters.
            /// Parameter string should consist of a single integer identifying the keyboard scan code.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful.
            std::optional<std::unique_ptr<IElementMapper>> MakeKeyboardMapper(std::wstring_view params);

            /// Attempts to build a null mapper (i.e. `nullptr`) using the supplied parameters.
            /// Parameter string should be empty.
            /// @param [in] params Parameter string.
            /// @return `nullptr` if successful.
            std::optional<std::unique_ptr<IElementMapper>> MakeNullMapper(std::wstring_view params);

            /// Attempts to build a #PovMapper using the supplied parameters.
            /// Parameter string should consist of a string representing a POV direction (for positive "pressed" contributions) and optionally a second string representing a POV direction (for negative "pressed" contributions).
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful.
            std::optional<std::unique_ptr<IElementMapper>> MakePovMapper(std::wstring_view params);

            /// Consumes part or all of the input string and attempts to parse it into an element mapper object.
            /// @param [in] elementMapperString Input string supposedly containing the representation of an element mapper.
            /// @return Result of the parse. Failure is indicated by the absence of an element mapper object.
            SElementMapperParseResult ParseSingleElementMapper(std::wstring_view elementMapperString);
        }
    }
}
