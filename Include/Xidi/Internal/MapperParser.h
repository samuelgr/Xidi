/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file MapperParser.h
 *   Declaration of functionality for parsing pieces of mapper objects from
 *   strings, typically supplied in a configuration file.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "ForceFeedbackTypes.h"
#include "Mapper.h"
#include "ValueOrError.h"

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

            /// Type alias for representing either an element mapper pointer or an error message.
            /// Intended to be returned from functions that parse element mapper strings and can be used to hold semantically-rich error messages for the user.
            typedef ValueOrError<std::unique_ptr<IElementMapper>, std::wstring> ElementMapperOrError;

            /// Type alias for representing either a force feedback actuator or an error message.
            /// Intended to be returned from functions that parse force feedback actuator strings and can be used to hold semantically-rich error messages for the user.
            typedef ValueOrError<ForceFeedback::SActuatorElement, std::wstring> ForceFeedbackActuatorOrError;

            /// Holds a partially-separated representation of a string that has been parsed at the very highest level.
            /// This view of the input string is separated into type and parameter portions.
            /// For example, the string "Axis(RotY, +)" would be separated into "Axis" as the type and "RotY, +" (the entire contents of the parentheses) as the parameters.
            struct SStringParts
            {
                std::wstring_view type;                                             ///< String identifying the top-level type.
                std::wstring_view params;                                           ///< String holding all of the parameters without the enclosing parentheses.
                std::wstring_view remaining;                                        ///< Remaining part of the string that was not separated into parts.

                /// Simple check for equality by field-by-field comparison.
                /// Primarily useful during testing.
                /// @param [in] other Object with which to compare.
                /// @return `true` if this object is equal to the other object, `false` otherwise.
                constexpr inline bool operator==(const SStringParts& other) const = default;
            };

            /// Holds the result of parsing and consuming a single element mapper worth of input string.
            struct SElementMapperParseResult
            {
                ElementMapperOrError maybeElementMapper;                            ///< Element mapper object, if the parse was successful. Note that `nullptr` indicates successful parse of a null element mapper.
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
                constexpr inline bool operator==(const SParamStringParts& other) const = default;
            };


            // -------- FUNCTIONS ------------------------------------------ //

            /// Attempts to identify the index within the `all` member of #UElementMap that corresponds to the controller element identified by the input string.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] controllerElementString String to parse that supposedly identifies a controller element.
            /// @return Element map array index, if it could be identified based on the input string.
            std::optional<unsigned int> FindControllerElementIndex(std::wstring_view controllerElementString);

            /// Attempts to identify the index within the `all` member of #UForceFeedbackActuatorMap that corresponds to the force feedback actuator identified by the input string.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] ffActuatorString String to parse that supposedly identifies a controller element.
            /// @return Force feedback actuator map array index, if it could be identified based on the input string.
            std::optional<unsigned int> FindForceFeedbackActuatorIndex(std::wstring_view ffActuatorString);

            /// Attempts to build an element mapper using the supplied string.
            /// This is the main entry point intended for use when parsing element mappers from strings.
            /// @param [in] elementMapperString Input string supposedly representing an element mapper.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError ElementMapperFromString(std::wstring_view elementMapperString);

            /// Attempts to build a force feedback actuator using the supplied string.
            /// This is the main entry point intended for use when parsing force feedback actuators from strings.
            /// @param [in] ffActuatorString Input string supposedly representing a force feedback actuator.
            /// @return Force feedback actuator descriptor object if successful, error message string otherwise.
            ForceFeedbackActuatorOrError ForceFeedbackActuatorFromString(std::wstring_view ffActuatorString);

            /// Determines if the specified controller element string is valid and recognized as identifying a controller element.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] controllerElementString String to be checked.
            /// @return `true` if the input string is recognized, `false` otherwise.
            inline bool IsControllerElementStringValid(std::wstring_view controllerElementString)
            {
                return FindControllerElementIndex(controllerElementString).has_value();
            }

            /// Determines if the specified force feedback actuator string is valid and recognized as identifying a physical force feedback actuator.
            /// See "MapperParser.cpp" for strings that will be recognized as valid.
            /// @param [in] ffActuatorString String to be checked.
            /// @return `true` if the input string is recognized, `false` otherwise.
            inline bool IsForceFeedbackActuatorStringValid(std::wstring_view ffActuatorString)
            {
                return FindForceFeedbackActuatorIndex(ffActuatorString).has_value();
            }


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
            std::optional<SStringParts> ExtractElementMapperStringParts(std::wstring_view elementMapperString);

            /// Separates the supplied force feedback actuator string into type and parameter parts and returns the result.
            /// Force feedback actuator strings are structured like element mapper strings, except they are not allowed to have a remainder.
            /// @param [in] ffActuatorString Input string supposedly representing an element mapper.
            /// @return Structure of separated string parts, if successful.
            std::optional<SStringParts> ExtractForceFeedbackActuatorStringParts(std::wstring_view ffActuatorString);

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
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeAxisMapper(std::wstring_view params);

            /// Attempts to build a #ButtonMapper using the supplied parameters.
            /// Parameter string should consist of a single integer identifying the button number.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeButtonMapper(std::wstring_view params);

            /// Attempts to build a #CompoundMapper using the supplied parameters.
            /// Parameter string should consist of a comma-separated list of element mappers, up to the maximum number allowed.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeCompoundMapper(std::wstring_view params);

            /// Attempts to build a #DigitalAxisMapper using the supplied parameters.
            /// Parameter string should consist of a string representing an axis and optionally a second string representing an axis direction.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeDigitalAxisMapper(std::wstring_view params);

            /// Attempts to build an #InvertMapper using the supplied parameters.
            /// Parameter string should consist of a string representing an element mapper.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeInvertMapper(std::wstring_view params);

            /// Attempts to build a #KeyboardMapper using the supplied parameters.
            /// Parameter string should consist of a string identifying the keyboard scan code.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeKeyboardMapper(std::wstring_view params);

            /// Attempts to build a #MouseAxisMapper using the supplied parameters.
            /// Parameter string should consist of a string representing a mouse axis and optionally a second string representing an axis direction.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeMouseAxisMapper(std::wstring_view params);

            /// Attempts to build a #MouseButtonMapper using the supplied parameters.
            /// Parameter string should consist of a string identifying the target mouse button.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeMouseButtonMapper(std::wstring_view params);

            /// Attempts to build a null mapper (i.e. `nullptr`) using the supplied parameters.
            /// Parameter string should be empty.
            /// @param [in] params Parameter string.
            /// @return `nullptr` if successful, error message string otherwise.
            ElementMapperOrError MakeNullMapper(std::wstring_view params);

            /// Attempts to build a #PovMapper using the supplied parameters.
            /// Parameter string should consist of a string representing a POV direction (for positive "pressed" contributions) and optionally a second string representing a POV direction (for negative "pressed" contributions).
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakePovMapper(std::wstring_view params);

            /// Attempts to build a #SplitMapper using the supplied parameters.
            /// Parameter string should consist of two comma-separated strings representing element mappers.
            /// @param [in] params Parameter string.
            /// @return Pointer to the new mapper object if successful, error message string otherwise.
            ElementMapperOrError MakeSplitMapper(std::wstring_view params);

            /// Attempts to build a force feedback actuator description object that matches the default actuator configuration.
            /// No parameters are allowed.
            /// @param [in] params Parameter string.
            /// @return Force feedback actuator description object if successful, error message string otherwise.
            ForceFeedbackActuatorOrError MakeForceFeedbackActuatorDefault(std::wstring_view params);

            /// Attempts to build a force feedback actuator description object that disables its associated physical actuator.
            /// No parameters are allowed.
            /// @param [in] params Parameter string.
            /// @return Force feedback actuator description object if successful, error message string otherwise.
            ForceFeedbackActuatorOrError MakeForceFeedbackActuatorDisabled(std::wstring_view params);

            /// Attempts to build a force feedback actuator description object in single-axis mode using the supplied parameters.
            /// Parameter string should consist of a string representing an axis and optionally a second string representing an axis direction.
            /// @param [in] params Parameter string.
            /// @return Force feedback actuator description object if successful, error message string otherwise.
            ForceFeedbackActuatorOrError MakeForceFeedbackActuatorSingleAxis(std::wstring_view params);

            /// Attempts to build a force feedback actuator description object in magnitude projection mode using the supplied parameters.
            /// Parameter string should consist of two comma-separated strings representing axes.
            /// @param [in] params Parameter string.
            /// @return Force feedback actuator description object if successful, error message string otherwise.
            ForceFeedbackActuatorOrError MakeForceFeedbackActuatorMagnitudeProjection(std::wstring_view params);

            /// Consumes part or all of the input string and attempts to parse it into an element mapper object.
            /// @param [in] elementMapperString Input string supposedly containing the representation of an element mapper.
            /// @return Result of the parse. Failure is indicated by the absence of an element mapper object.
            SElementMapperParseResult ParseSingleElementMapper(std::wstring_view elementMapperString);

            /// Consumes all of the input string and attempts to parse it into a force feedback actuator description object.
            /// @param [in] ffActuatorString Input string supposedly containing the representation of a force feedback actuator.
            /// @return Force feedback actuator description object if successful, error message string otherwise.
            ForceFeedbackActuatorOrError ParseForceFeedbackActuator(std::wstring_view ffActuatorString);
        }
    }
}
