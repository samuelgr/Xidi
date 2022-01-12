/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MapperParser.cpp
 *   Implementation of functionality for parsing pieces of mapper objects
 *   from strings, typically supplied in a configuration file.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Keyboard.h"
#include "Mapper.h"
#include "MapperParser.h"
#include "Strings.h"
#include "ValueOrError.h"

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

            /// Maximum recursion depth allowed for an element mapper string.
            /// Should be at least one more than the total number of element mapper types that accept underlying element mappers.
            static constexpr unsigned int kElementMapperMaxRecursionDepth = 4;

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
            typedef ElementMapperOrError(*TMakeElementMapperFunc)(std::wstring_view);

            /// Holds parameters for creating #AxisMapper objects.
            /// Useful because both #AxisMapper and #DigitalAxisMapper follow the same parsing logic and parameters.
            /// See #AxisMapper for documentation on the fields.
            struct SAxisMapperParams
            {
                EAxis axis;
                EAxisDirection direction;
            };

            /// Type alias for enabling axis parameter parsing to indicate a semantically-rich error on parse failure.
            typedef ValueOrError<SAxisMapperParams, std::wstring> AxisMapperParamsOrError;


            // -------- INTERNAL FUNCTIONS --------------------------------- //

            /// Identifies the end position of the first parameter in the supplied string which should be a parameter list.
            /// Example: "RotY, +" would identify the position of the comma.
            /// Example: "Split(Button(1), Button(2)), Split(Button(3), Button(4))" would identify the position of the second comma.
            /// Example: "RotY" would return a value of 4, the length of the string, indicating the entire string is the first parameter.
            /// @return End position of the first parameter in the supplied string, or `npos` if the input string is invalid thus leading to a parse error.
            static size_t FindFirstParameterEndPosition(std::wstring_view paramListString)
            {
                unsigned int depth = 0;

                for (size_t pos = 0; pos < paramListString.length(); ++pos)
                {
                    switch (paramListString[pos])
                    {
                    case kCharElementMapperBeginParams:
                        depth += 1;
                        break;

                    case kCharElementMapperEndParams:
                        if (0 == depth)
                            return std::wstring_view::npos;
                        depth -= 1;
                        break;

                    case kCharElementMapperParamSeparator:
                        if (0 == depth)
                            return pos;
                        break;

                    default:
                        break;
                    }
                }

                if (0 != depth)
                    return std::wstring_view::npos;
                else
                    return paramListString.length();
            }

            /// Identifies the end position of the parameter list given a string that starts a parameter list.
            /// For example, if the element mapper string is "Axis(RotY, +)" then the input string should be "RotY, +)" and this function will identify the position of the closing parenthesis.
            /// @param [in] paramListString Parameter list input string to search.
            /// @return Position of the end of the parameter list if the input string is valid and contains correct balance, or `npos` otherwise.
            static size_t FindParamListEndPosition(std::wstring_view paramListString)
            {
                unsigned int depth = 1;

                for (size_t pos = paramListString.find_first_not_of(kCharSetWhitespace); pos < paramListString.length(); ++pos)
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

            /// Common logic for parsing axis mapper parameters from an axis mapper string.
            /// Used for creating both #AxisMapper and #DigitalAxisMapper objects.
            /// @param [in] params Parameter string.
            /// @return Structure containing the parsed parameters if parsing was successful, error message otherwise.
            static AxisMapperParamsOrError ParseAxisMapperParams(std::wstring_view params)
            {
                // Map of strings representing axes to axis enumerators.
                static const std::map<std::wstring_view, EAxis> kAxisStrings = {
                    {L"x",              EAxis::X},
                    {L"X",              EAxis::X},

                    {L"y",              EAxis::Y},
                    {L"Y",              EAxis::Y},

                    {L"z",              EAxis::Z},
                    {L"Z",              EAxis::Z},

                    {L"rx",             EAxis::RotX},
                    {L"Rx",             EAxis::RotX},
                    {L"rX",             EAxis::RotX},
                    {L"RX",             EAxis::RotX},
                    {L"rotx",           EAxis::RotX},
                    {L"rotX",           EAxis::RotX},
                    {L"Rotx",           EAxis::RotX},
                    {L"RotX",           EAxis::RotX},

                    {L"ry",             EAxis::RotY},
                    {L"Ry",             EAxis::RotY},
                    {L"rY",             EAxis::RotY},
                    {L"RY",             EAxis::RotY},
                    {L"roty",           EAxis::RotY},
                    {L"rotY",           EAxis::RotY},
                    {L"Roty",           EAxis::RotY},
                    {L"RotY",           EAxis::RotY},

                    {L"rz",             EAxis::RotZ},
                    {L"Rz",             EAxis::RotZ},
                    {L"rZ",             EAxis::RotZ},
                    {L"RZ",             EAxis::RotZ},
                    {L"rotz",           EAxis::RotZ},
                    {L"rotZ",           EAxis::RotZ},
                    {L"Rotz",           EAxis::RotZ},
                    {L"RotZ",           EAxis::RotZ}
                };

                // Map of strings representing axis directions to axis direction enumerators.
                static const std::map<std::wstring_view, EAxisDirection> kDirectionStrings = {
                    {L"bidir",          EAxisDirection::Both},
                    {L"Bidir",          EAxisDirection::Both},
                    {L"BiDir",          EAxisDirection::Both},
                    {L"BIDIR",          EAxisDirection::Both},
                    {L"bidirectional",  EAxisDirection::Both},
                    {L"Bidirectional",  EAxisDirection::Both},
                    {L"BiDirectional",  EAxisDirection::Both},
                    {L"BIDIRECTIONAL",  EAxisDirection::Both},
                    {L"both",           EAxisDirection::Both},
                    {L"Both",           EAxisDirection::Both},
                    {L"BOTH",           EAxisDirection::Both},

                    {L"+",              EAxisDirection::Positive},
                    {L"+ve",            EAxisDirection::Positive},
                    {L"pos",            EAxisDirection::Positive},
                    {L"Pos",            EAxisDirection::Positive},
                    {L"POS",            EAxisDirection::Positive},
                    {L"positive",       EAxisDirection::Positive},
                    {L"Positive",       EAxisDirection::Positive},
                    {L"POSITIVE",       EAxisDirection::Positive},

                    {L"-",              EAxisDirection::Negative},
                    {L"-ve",            EAxisDirection::Negative},
                    {L"neg",            EAxisDirection::Negative},
                    {L"Neg",            EAxisDirection::Negative},
                    {L"NEG",            EAxisDirection::Negative},
                    {L"negative",       EAxisDirection::Negative},
                    {L"Negative",       EAxisDirection::Negative},
                    {L"NEGATIVE",       EAxisDirection::Negative}
                };

                SParamStringParts paramParts = ExtractParameterListStringParts(params).value_or(SParamStringParts());

                // First parameter is required. It is a string that specifies the target axis.
                if (true == paramParts.first.empty())
                    return L"Missing or unparseable axis";

                const auto kAxisIter = kAxisStrings.find(paramParts.first);
                if (kAxisStrings.cend() == kAxisIter)
                    return Strings::FormatString(L"%s: Unrecognized axis", std::wstring(paramParts.first).c_str()).Data();

                const EAxis kAxis = kAxisIter->second;

                // Second parameter is optional. It is a string that specifies the axis direction, with the default being both.
                EAxisDirection axisDirection = EAxisDirection::Both;

                paramParts = ExtractParameterListStringParts(paramParts.remaining).value_or(SParamStringParts());
                if (false == paramParts.first.empty())
                {
                    // It is an error for a second parameter to be present but invalid.
                    const auto kDirectionIter = kDirectionStrings.find(paramParts.first);
                    if (kDirectionStrings.cend() == kDirectionIter)
                        return Strings::FormatString(L"%s: Unrecognized axis direction", std::wstring(paramParts.first).c_str()).Data();

                    axisDirection = kDirectionIter->second;
                }

                // No further parameters allowed.
                if (false == paramParts.remaining.empty())
                    return Strings::FormatString(L"\"%s\" is extraneous", std::wstring(paramParts.remaining).c_str()).Data();

                return SAxisMapperParams({.axis = kAxis, .direction = axisDirection});
            }

            /// Parses a relatively small unsigned integer value from the supplied input string.
            /// A maximum of 8 characters are permitted, meaning any parsed values are guaranteed to fit into 32 bits.
            /// This function will fail if the input string is too long or if it does not entirely represent an unsigned integer value.
            /// @param [in] uintString String from which to parse.
            /// @param [in] base Representation base of the number, which defaults to auto-detecting the base using the prefix in the string.
            /// @return Parsed integer value if successful.
            static std::optional<unsigned int> ParseUnsignedInteger(std::wstring_view uintString, int base = 0)
            {
                static constexpr size_t kMaxChars = 8;
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
                        if (iswalnum(uintString[i]))
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

            /// Parses a string representation of a DirectInput keyboard scancode into an integer.
            /// This function will fail if the input string is too long or if it does not entirely represent an unsigned integer value.
            /// @param [in] kbString String from which to parse.
            /// @return Parsed integer value if successful.
            static std::optional<unsigned int> ParseKeyboardScancode(std::wstring_view kbString)
            {
                // Map of strings representing keyboard scancodes to the keyboard scancodes themselves.
                // One pair exists per DIK_* constant. Comparisons with the input string are case-insensitive because the input string is converted to uppercase to match the contents of this map.
                static const std::map<std::wstring_view, unsigned int> kKeyboardScanCodeStrings = {

                    // Convenience aliases
                    {L"ESC",                DIK_ESCAPE},
                    {L"ENTER",              DIK_RETURN},
                    {L"SCROLLLOCK",         DIK_SCROLL},

                    // DIK_ constants
                    {L"ESCAPE",             DIK_ESCAPE},
                    {L"1",                  DIK_1},
                    {L"2",                  DIK_2},
                    {L"3",                  DIK_3},
                    {L"4",                  DIK_4},
                    {L"5",                  DIK_5},
                    {L"6",                  DIK_6},
                    {L"7",                  DIK_7},
                    {L"8",                  DIK_8},
                    {L"9",                  DIK_9},
                    {L"0",                  DIK_0},
                    {L"MINUS",              DIK_MINUS},
                    {L"EQUALS",             DIK_EQUALS},
                    {L"BACK",               DIK_BACK},
                    {L"TAB",                DIK_TAB},
                    {L"Q",                  DIK_Q},
                    {L"W",                  DIK_W},
                    {L"E",                  DIK_E},
                    {L"R",                  DIK_R},
                    {L"T",                  DIK_T},
                    {L"Y",                  DIK_Y},
                    {L"U",                  DIK_U},
                    {L"I",                  DIK_I},
                    {L"O",                  DIK_O},
                    {L"P",                  DIK_P},
                    {L"LBRACKET",           DIK_LBRACKET},
                    {L"RBRACKET",           DIK_RBRACKET},
                    {L"RETURN",             DIK_RETURN},
                    {L"LCONTROL",           DIK_LCONTROL},
                    {L"A",                  DIK_A},
                    {L"S",                  DIK_S},
                    {L"D",                  DIK_D},
                    {L"F",                  DIK_F},
                    {L"G",                  DIK_G},
                    {L"H",                  DIK_H},
                    {L"J",                  DIK_J},
                    {L"K",                  DIK_K},
                    {L"L",                  DIK_L},
                    {L"SEMICOLON",          DIK_SEMICOLON},
                    {L"APOSTROPHE",         DIK_APOSTROPHE},
                    {L"GRAVE",              DIK_GRAVE},
                    {L"LSHIFT",             DIK_LSHIFT},
                    {L"BACKSLASH",          DIK_BACKSLASH},
                    {L"Z",                  DIK_Z},
                    {L"X",                  DIK_X},
                    {L"C",                  DIK_C},
                    {L"V",                  DIK_V},
                    {L"B",                  DIK_B},
                    {L"N",                  DIK_N},
                    {L"M",                  DIK_M},
                    {L"COMMA",              DIK_COMMA},
                    {L"PERIOD",             DIK_PERIOD},
                    {L"SLASH",              DIK_SLASH},
                    {L"RSHIFT",             DIK_RSHIFT},
                    {L"MULTIPLY",           DIK_MULTIPLY},
                    {L"LMENU",              DIK_LMENU},
                    {L"SPACE",              DIK_SPACE},
                    {L"CAPITAL",            DIK_CAPITAL},
                    {L"F1",                 DIK_F1},
                    {L"F2",                 DIK_F2},
                    {L"F3",                 DIK_F3},
                    {L"F4",                 DIK_F4},
                    {L"F5",                 DIK_F5},
                    {L"F6",                 DIK_F6},
                    {L"F7",                 DIK_F7},
                    {L"F8",                 DIK_F8},
                    {L"F9",                 DIK_F9},
                    {L"F10",                DIK_F10},
                    {L"NUMLOCK",            DIK_NUMLOCK},
                    {L"SCROLL",             DIK_SCROLL},
                    {L"NUMPAD7",            DIK_NUMPAD7},
                    {L"NUMPAD8",            DIK_NUMPAD8},
                    {L"NUMPAD9",            DIK_NUMPAD9},
                    {L"SUBTRACT",           DIK_SUBTRACT},
                    {L"NUMPAD4",            DIK_NUMPAD4},
                    {L"NUMPAD5",            DIK_NUMPAD5},
                    {L"NUMPAD6",            DIK_NUMPAD6},
                    {L"ADD",                DIK_ADD},
                    {L"NUMPAD1",            DIK_NUMPAD1},
                    {L"NUMPAD2",            DIK_NUMPAD2},
                    {L"NUMPAD3",            DIK_NUMPAD3},
                    {L"NUMPAD0",            DIK_NUMPAD0},
                    {L"DECIMAL",            DIK_DECIMAL},
                    {L"OEM_102",            DIK_OEM_102},
                    {L"F11",                DIK_F11},
                    {L"F12",                DIK_F12},
                    {L"F13",                DIK_F13},
                    {L"F14",                DIK_F14},
                    {L"F15",                DIK_F15},
                    {L"KANA",               DIK_KANA},
                    {L"ABNT_C1",            DIK_ABNT_C1},
                    {L"CONVERT",            DIK_CONVERT},
                    {L"NOCONVERT",          DIK_NOCONVERT},
                    {L"YEN",                DIK_YEN},
                    {L"ABNT_C2",            DIK_ABNT_C2},
                    {L"NUMPADEQUALS",       DIK_NUMPADEQUALS},
                    {L"PREVTRACK",          DIK_PREVTRACK},
                    {L"AT",                 DIK_AT},
                    {L"COLON",              DIK_COLON},
                    {L"UNDERLINE",          DIK_UNDERLINE},
                    {L"KANJI",              DIK_KANJI},
                    {L"STOP",               DIK_STOP},
                    {L"AX",                 DIK_AX},
                    {L"UNLABELED",          DIK_UNLABELED},
                    {L"NEXTTRACK",          DIK_NEXTTRACK},
                    {L"NUMPADENTER",        DIK_NUMPADENTER},
                    {L"RCONTROL",           DIK_RCONTROL},
                    {L"MUTE",               DIK_MUTE},
                    {L"CALCULATOR",         DIK_CALCULATOR},
                    {L"PLAYPAUSE",          DIK_PLAYPAUSE},
                    {L"MEDIASTOP",          DIK_MEDIASTOP},
                    {L"VOLUMEDOWN",         DIK_VOLUMEDOWN},
                    {L"VOLUMEUP",           DIK_VOLUMEUP},
                    {L"WEBHOME",            DIK_WEBHOME},
                    {L"NUMPADCOMMA",        DIK_NUMPADCOMMA},
                    {L"DIVIDE",             DIK_DIVIDE},
                    {L"SYSRQ",              DIK_SYSRQ},
                    {L"RMENU",              DIK_RMENU},
                    {L"PAUSE",              DIK_PAUSE},
                    {L"HOME",               DIK_HOME},
                    {L"UP",                 DIK_UP},
                    {L"PRIOR",              DIK_PRIOR},
                    {L"LEFT",               DIK_LEFT},
                    {L"RIGHT",              DIK_RIGHT},
                    {L"END",                DIK_END},
                    {L"DOWN",               DIK_DOWN},
                    {L"NEXT",               DIK_NEXT},
                    {L"INSERT",             DIK_INSERT},
                    {L"DELETE",             DIK_DELETE},
                    {L"LWIN",               DIK_LWIN},
                    {L"RWIN",               DIK_RWIN},
                    {L"APPS",               DIK_APPS},
                    {L"POWER",              DIK_POWER},
                    {L"SLEEP",              DIK_SLEEP},
                    {L"WAKE",               DIK_WAKE},
                    {L"WEBSEARCH",          DIK_WEBSEARCH},
                    {L"WEBFAVORITES",       DIK_WEBFAVORITES},
                    {L"WEBREFRESH",         DIK_WEBREFRESH},
                    {L"WEBSTOP",            DIK_WEBSTOP},
                    {L"WEBFORWARD",         DIK_WEBFORWARD},
                    {L"WEBBACK",            DIK_WEBBACK},
                    {L"MYCOMPUTER",         DIK_MYCOMPUTER},
                    {L"MAIL",               DIK_MAIL},
                    {L"MEDIASELECT",        DIK_MEDIASELECT},
                    {L"BACKSPACE",          DIK_BACKSPACE},
                    {L"NUMPADSTAR",         DIK_NUMPADSTAR},
                    {L"LALT",               DIK_LALT},
                    {L"CAPSLOCK",           DIK_CAPSLOCK},
                    {L"NUMPADMINUS",        DIK_NUMPADMINUS},
                    {L"NUMPADPLUS",         DIK_NUMPADPLUS},
                    {L"NUMPADPERIOD",       DIK_NUMPADPERIOD},
                    {L"NUMPADSLASH",        DIK_NUMPADSLASH},
                    {L"RALT",               DIK_RALT},
                    {L"UPARROW",            DIK_UPARROW},
                    {L"PGUP",               DIK_PGUP},
                    {L"LEFTARROW",          DIK_LEFTARROW},
                    {L"RIGHTARROW",         DIK_RIGHTARROW},
                    {L"DOWNARROW",          DIK_DOWNARROW},
                    {L"PGDN",               DIK_PGDN}
                };

                static constexpr size_t kMaxChars = 24;
                if (kbString.length() >= kMaxChars)
                    return std::nullopt;

                static constexpr std::wstring_view kOptionalPrefix = L"DIK_";
                if (true == kbString.starts_with(kOptionalPrefix))
                    kbString.remove_prefix(kOptionalPrefix.length());
                if (true == kbString.empty())
                    return std::nullopt;

                // Create an uppercase null-terminated version of the scancode string by copying the characters into a small buffer while also transforming them.
                wchar_t convertBuffer[1 + kMaxChars];
                convertBuffer[kMaxChars] = L'\0';
                for (size_t i = 0; i < kMaxChars; ++i)
                {
                    if (i < kbString.length())
                    {
                        convertBuffer[i] = std::towupper(kbString[i]);
                    }
                    else
                    {
                        convertBuffer[i] = L'\0';
                        break;
                    }
                }

                const auto keyboardScanCodeIter = kKeyboardScanCodeStrings.find(convertBuffer);
                if (kKeyboardScanCodeStrings.cend() == keyboardScanCodeIter)
                    return std::nullopt;
                else
                    return keyboardScanCodeIter->second;
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

            ElementMapperOrError ElementMapperFromString(std::wstring_view elementMapperString)
            {
                const std::optional<unsigned int>kMaybeRecursionDepth = ComputeRecursionDepth(elementMapperString);
                if (false == kMaybeRecursionDepth.has_value())
                    return Strings::FormatString(L"Syntax error: Unbalanced parentheses").Data();

                const unsigned int kRecursionDepth = ComputeRecursionDepth(elementMapperString).value();
                if (kRecursionDepth > kElementMapperMaxRecursionDepth)
                    return Strings::FormatString(L"Nesting depth %u exceeds limit of %u", kRecursionDepth, kElementMapperMaxRecursionDepth).Data();

                SElementMapperParseResult parseResult = ParseSingleElementMapper(elementMapperString);
                if (false == parseResult.maybeElementMapper.HasValue())
                    return std::move(parseResult.maybeElementMapper);
                else if (false == parseResult.remainingString.empty())
                    return Strings::FormatString(L"\"%s\" is extraneous", std::wstring(parseResult.remainingString).c_str()).Data();

                return std::move(parseResult.maybeElementMapper);
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

                    // Empty parameter lists are not allowed.
                    if (true == kParamString.empty())
                        return std::nullopt;

                    const std::wstring_view kRemainingString = possibleRemainingString;

                    return SElementMapperStringParts({.type = kTypeString, .params = kParamString, .remaining = kRemainingString});
                }
            }

            // --------

            std::optional<SParamStringParts> ExtractParameterListStringParts(std::wstring_view paramListString)
            {
                const size_t kFirstParamEndPosition = FindFirstParameterEndPosition(paramListString);

                if (std::wstring_view::npos == kFirstParamEndPosition)
                    return std::nullopt;
                
                const std::wstring_view kFirstParamString = TrimWhitespace(paramListString.substr(0, kFirstParamEndPosition));

                if (paramListString.length() == kFirstParamEndPosition)
                {
                    // Entire input string was consumed and no comma was located.
                    return SParamStringParts({.first = kFirstParamString});
                }
                
                const std::wstring_view kRemainingString = TrimWhitespace(paramListString.substr(1 + kFirstParamEndPosition));

                if (true == kRemainingString.empty())
                {
                    // A comma was located but nothing appears after it.
                    // This is an error because it indicates a dangling comma.
                    return std::nullopt;
                }

                return SParamStringParts({.first = kFirstParamString, .remaining = kRemainingString});
            }

            // --------

            ElementMapperOrError MakeAxisMapper(std::wstring_view params)
            {
                const AxisMapperParamsOrError kMaybeAxisMapperParams = ParseAxisMapperParams(params);
                if (true == kMaybeAxisMapperParams.HasError())
                    return Strings::FormatString(L"Axis: %s", kMaybeAxisMapperParams.Error().c_str()).Data();

                return std::make_unique<AxisMapper>(kMaybeAxisMapperParams.Value().axis, kMaybeAxisMapperParams.Value().direction);
            }

            // --------

            ElementMapperOrError MakeButtonMapper(std::wstring_view params)
            {
                const std::optional<unsigned int> kMaybeButtonNumber = ParseUnsignedInteger(params, 10);
                if (false == kMaybeButtonNumber.has_value())
                    return Strings::FormatString(L"Button: Parameter \"%s\" must be a number between 1 and %u", std::wstring(params).c_str(), (unsigned int)EButton::Count).Data();

                const unsigned int kButtonNumber = kMaybeButtonNumber.value() - 1;
                if (kButtonNumber >= (unsigned int)EButton::Count)
                    return Strings::FormatString(L"Button: Parameter \"%s\" must be a number between 1 and %u", std::wstring(params).c_str(), (unsigned int)EButton::Count).Data();

                return std::make_unique<ButtonMapper>((EButton)kButtonNumber);
            }

            // --------

            ElementMapperOrError MakeCompoundMapper(std::wstring_view params)
            {
                CompoundMapper::TElementMappers elementMappers;
                SElementMapperParseResult elementMapperResult = {.maybeElementMapper = nullptr, .remainingString = params};

                // Parse element mappers one at a time.
                // At least one underlying element mapper is required, with all the rest being optional.
                for (size_t i = 0; i < elementMappers.size(); ++i)
                {
                    elementMapperResult = ParseSingleElementMapper(elementMapperResult.remainingString);
                    if (false == elementMapperResult.maybeElementMapper.HasValue())
                        return Strings::FormatString(L"Compound: Parameter %u: %s", (unsigned int)(1 + i), elementMapperResult.maybeElementMapper.Error().c_str()).Data();

                    elementMappers[i] = std::move(elementMapperResult.maybeElementMapper.Value());

                    if (true == elementMapperResult.remainingString.empty())
                        break;
                }

                // No further parameters allowed.
                // Specifying too many underlying element mappers is an error.
                if (false == elementMapperResult.remainingString.empty())
                    return Strings::FormatString(L"Compound: Number of parameters exceeds limit of %u", (unsigned int)elementMappers.size()).Data();

                return std::make_unique<CompoundMapper>(std::move(elementMappers));
            }

            // --------

            ElementMapperOrError MakeDigitalAxisMapper(std::wstring_view params)
            {
                const AxisMapperParamsOrError kMaybeAxisMapperParams = ParseAxisMapperParams(params);
                if (true == kMaybeAxisMapperParams.HasError())
                    return Strings::FormatString(L"DigitalAxis: %s", kMaybeAxisMapperParams.Error().c_str()).Data();

                return std::make_unique<DigitalAxisMapper>(kMaybeAxisMapperParams.Value().axis, kMaybeAxisMapperParams.Value().direction);
            }

            // --------

            ElementMapperOrError MakeInvertMapper(std::wstring_view params)
            {
                SElementMapperParseResult elementMapperResult = ParseSingleElementMapper(params);

                if (false == elementMapperResult.maybeElementMapper.HasValue())
                    return Strings::FormatString(L"Invert: Parameter 1: %s", elementMapperResult.maybeElementMapper.Error().c_str()).Data();
                else if (false == elementMapperResult.remainingString.empty())
                    return Strings::FormatString(L"Invert: \"%s\" is extraneous", std::wstring(elementMapperResult.remainingString).c_str()).Data();

                return std::make_unique<InvertMapper>(std::move(elementMapperResult.maybeElementMapper.Value()));
            }

            // --------

            ElementMapperOrError MakeKeyboardMapper(std::wstring_view params)
            {
                // First try parsing a friendly string representation of the keyboard scan code (i.e. strings that look like "DIK_*" constants, the "DIK_" prefix being optional).
                // If that fails, try interpreting the scan code as an unsigned integer directly, with the possibility that it could be represented in decimal, octal, or hexadecimal.
                // If both attempts fail then the scancode cannot be parsed, which is an error.
                std::optional<unsigned int> maybeKeyScanCode = ParseKeyboardScancode(params);
                if (false == maybeKeyScanCode.has_value())
                    maybeKeyScanCode = ParseUnsignedInteger(params);
                if (false == maybeKeyScanCode.has_value())
                    return Strings::FormatString(L"Keyboard: \"%s\" must map to a scan code between 0 and %u", std::wstring(params).c_str(), (Keyboard::kVirtualKeyboardKeyCount - 1)).Data();

                const unsigned int kKeyScanCode = maybeKeyScanCode.value();
                if (kKeyScanCode >= Keyboard::kVirtualKeyboardKeyCount)
                    return Strings::FormatString(L"Keyboard: \"%s\" must map to a scan code between 0 and %u", std::wstring(params).c_str(), (Keyboard::kVirtualKeyboardKeyCount - 1)).Data();

                return std::make_unique<KeyboardMapper>((Keyboard::TKeyIdentifier)kKeyScanCode);
            }

            // --------

            ElementMapperOrError MakeNullMapper(std::wstring_view params)
            {
                if (false == params.empty())
                    return Strings::FormatString(L"Null: \"%s\" is extraneous", std::wstring(params).c_str()).Data();

                return nullptr;
            }

            // --------

            ElementMapperOrError MakePovMapper(std::wstring_view params)
            {
                // Map of strings representing axes to POV direction.
                static const std::map<std::wstring_view, EPovDirection> kPovDirectionStrings = {
                    {L"u",              EPovDirection::Up},
                    {L"U",              EPovDirection::Up},
                    {L"up",             EPovDirection::Up},
                    {L"Up",             EPovDirection::Up},
                    {L"UP",             EPovDirection::Up},

                    {L"d",              EPovDirection::Down},
                    {L"D",              EPovDirection::Down},
                    {L"dn",             EPovDirection::Down},
                    {L"Dn",             EPovDirection::Down},
                    {L"DN",             EPovDirection::Down},
                    {L"down",           EPovDirection::Down},
                    {L"Down",           EPovDirection::Down},
                    {L"DOWN",           EPovDirection::Down},

                    {L"l",              EPovDirection::Left},
                    {L"L",              EPovDirection::Left},
                    {L"lt",             EPovDirection::Left},
                    {L"Lt",             EPovDirection::Left},
                    {L"LT",             EPovDirection::Left},
                    {L"left",           EPovDirection::Left},
                    {L"Left",           EPovDirection::Left},
                    {L"LEFT",           EPovDirection::Left},

                    {L"r",              EPovDirection::Right},
                    {L"R",              EPovDirection::Right},
                    {L"rt",             EPovDirection::Right},
                    {L"Rt",             EPovDirection::Right},
                    {L"RT",             EPovDirection::Right},
                    {L"right",          EPovDirection::Right},
                    {L"Right",          EPovDirection::Right},
                    {L"RIGHT",          EPovDirection::Right},
                };

                const auto kPovDirectionIter = kPovDirectionStrings.find(params);
                if (kPovDirectionStrings.cend() == kPovDirectionIter)
                    return Strings::FormatString(L"Pov: %s: Unrecognized POV direction", std::wstring(params).c_str()).Data();

                return std::make_unique<PovMapper>(kPovDirectionIter->second);
            }

            // --------

            ElementMapperOrError MakeSplitMapper(std::wstring_view params)
            {
                // First parameter is required. It is a string that specifies the positive element mapper.
                SElementMapperParseResult positiveElementMapperResult = ParseSingleElementMapper(params);
                if (false == positiveElementMapperResult.maybeElementMapper.HasValue())
                    return Strings::FormatString(L"Split: Parameter 1: %s", positiveElementMapperResult.maybeElementMapper.Error().c_str()).Data();

                // Second parameter is required. It is a string that specifies the negative element mapper.
                SElementMapperParseResult negativeElementMapperResult = ParseSingleElementMapper(positiveElementMapperResult.remainingString);
                if (false == negativeElementMapperResult.maybeElementMapper.HasValue())
                    return Strings::FormatString(L"Split: Parameter 2: %s", negativeElementMapperResult.maybeElementMapper.Error().c_str()).Data();

                // No further parameters allowed.
                if (false == negativeElementMapperResult.remainingString.empty())
                    return Strings::FormatString(L"Split: \"%s\" is extraneous", std::wstring(negativeElementMapperResult.remainingString).c_str()).Data();

                return std::make_unique<SplitMapper>(std::move(positiveElementMapperResult.maybeElementMapper.Value()), std::move(negativeElementMapperResult.maybeElementMapper.Value()));
            }

            // --------

            SElementMapperParseResult ParseSingleElementMapper(std::wstring_view elementMapperString)
            {
                static const std::map<std::wstring_view, TMakeElementMapperFunc> kMakeElementMapperFunctions = {
                    {L"axis",               &MakeAxisMapper},
                    {L"Axis",               &MakeAxisMapper},

                    {L"button",             &MakeButtonMapper},
                    {L"Button",             &MakeButtonMapper},

                    {L"compound",           &MakeCompoundMapper},
                    {L"Compound",           &MakeCompoundMapper},

                    {L"digitalaxis",        &MakeDigitalAxisMapper},
                    {L"digitalAxis",        &MakeDigitalAxisMapper},
                    {L"Digitalaxis",        &MakeDigitalAxisMapper},
                    {L"DigitalAxis",        &MakeDigitalAxisMapper},

                    {L"invert",             &MakeInvertMapper},
                    {L"Invert",             &MakeInvertMapper},

                    {L"keyboard",           &MakeKeyboardMapper},
                    {L"Keyboard",           &MakeKeyboardMapper},
                    {L"keystroke",          &MakeKeyboardMapper},
                    {L"Keystroke",          &MakeKeyboardMapper},
                    {L"KeyStroke",          &MakeKeyboardMapper},

                    {L"pov",                &MakePovMapper},
                    {L"Pov",                &MakePovMapper},
                    {L"POV",                &MakePovMapper},
                    {L"povhat",             &MakePovMapper},
                    {L"povHat",             &MakePovMapper},
                    {L"Povhat",             &MakePovMapper},
                    {L"PovHat",             &MakePovMapper},

                    {L"null",               &MakeNullMapper},
                    {L"Null",               &MakeNullMapper},
                    {L"nothing",            &MakeNullMapper},
                    {L"Nothing",            &MakeNullMapper},
                    {L"none",               &MakeNullMapper},
                    {L"None",               &MakeNullMapper},
                    {L"nil",                &MakeNullMapper},
                    {L"Nil",                &MakeNullMapper},

                    {L"split",              &MakeSplitMapper},
                    {L"Split",              &MakeSplitMapper},
                };

                const std::optional<SElementMapperStringParts> kMaybeElementMapperStringParts = ExtractElementMapperStringParts(elementMapperString);
                if (false == kMaybeElementMapperStringParts.has_value())
                    return {.maybeElementMapper = Strings::FormatString(L"\"%s\" contains a syntax error", std::wstring(elementMapperString).c_str()).Data()};

                const SElementMapperStringParts& kElementMapperStringParts = kMaybeElementMapperStringParts.value();
                if (true == kElementMapperStringParts.type.empty())
                    return {.maybeElementMapper = L"Missing or unparseable element mapper type."};

                const auto kMakeElementMapperIter = kMakeElementMapperFunctions.find(kElementMapperStringParts.type);
                if (kMakeElementMapperFunctions.cend() == kMakeElementMapperIter)
                    return {.maybeElementMapper = Strings::FormatString(L"%s: Unrecognized element mapper type", std::wstring(kElementMapperStringParts.type).c_str()).Data()};

                return {.maybeElementMapper = kMakeElementMapperIter->second(kElementMapperStringParts.params), .remainingString = kElementMapperStringParts.remaining};
            }
        }
    }
}
