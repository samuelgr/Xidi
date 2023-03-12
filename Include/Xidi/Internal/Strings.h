/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file Strings.h
 *   Declaration of common strings and functions to manipulate them.
 *****************************************************************************/

#pragma once

#include "ControllerTypes.h"
#include "TemporaryBuffer.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <sal.h>
#include <string>
#include <string_view>


// -------- MACROS --------------------------------------------------------- //

// Strings that need to be available in multiple formats (ASCII and Unicode).
#define XIDI_AXIS_NAME_X                    "X Axis"
#define XIDI_AXIS_NAME_Y                    "Y Axis"
#define XIDI_AXIS_NAME_Z                    "Z Axis"
#define XIDI_AXIS_NAME_RX                   "RotX Axis"
#define XIDI_AXIS_NAME_RY                   "RotY Axis"
#define XIDI_AXIS_NAME_RZ                   "RotZ Axis"
#define XIDI_AXIS_NAME_UNKNOWN              "Unknown Axis"
#define XIDI_BUTTON_NAME_FORMAT             "Button %u"
#define XIDI_POV_NAME                       "POV"
#define XIDI_WHOLE_CONTROLLER_NAME          "Whole Controller"
#define XIDI_EFFECT_NAME_CONSTANT_FORCE     "Constant Force"
#define XIDI_EFFECT_NAME_RAMP_FORCE         "Ramp Force"
#define XIDI_EFFECT_NAME_SQUARE             "Square Wave"
#define XIDI_EFFECT_NAME_SINE               "Sine Wave"
#define XIDI_EFFECT_NAME_TRIANGLE           "Triangle Wave"
#define XIDI_EFFECT_NAME_SAWTOOTH_UP        "Sawtooth Up"
#define XIDI_EFFECT_NAME_SAWTOOTH_DOWN      "Sawtooth Down"
#define XIDI_EFFECT_NAME_CUSTOM_FORCE       "Custom Force"


namespace Xidi
{
    namespace Strings
    {
        // -------- COMPILE-TIME CONSTANTS --------------------------------- //
        // Can safely be used at any time, including to perform static initialization.
        // Views are guaranteed to be null-terminated.

        /// Base name of the DirectInput library to import.
        inline constexpr std::wstring_view kStrLibraryNameDirectInput = L"dinput.dll";

        /// Base name of the DirectInput8 library to import.
        inline constexpr std::wstring_view kStrLibraryNameDirectInput8 = L"dinput8.dll";

        /// Base name of the WinMM library to import.
        inline constexpr std::wstring_view kStrLibraryNameWinMM = L"winmm.dll";
        
        /// Configuration file section name for overriding import libraries.
        inline constexpr std::wstring_view kStrConfigurationSectionImport = L"Import";

        /// Configuration file setting for overriding import for DirectInput.
        inline constexpr std::wstring_view kStrConfigurationSettingImportDirectInput = kStrLibraryNameDirectInput;

        /// Configuration file setting for overriding import for DirectInput8.
        inline constexpr std::wstring_view kStrConfigurationSettingImportDirectInput8 = kStrLibraryNameDirectInput8;

        /// Configuration file setting for overriding import for WinMM.
        inline constexpr std::wstring_view kStrConfigurationSettingImportWinMM = kStrLibraryNameWinMM;


        /// Configuration file setting separator for generating per-controller setting strings.
        inline constexpr wchar_t kCharConfigurationSettingSeparator = L'.';


        /// Configuration file section name for log-related settings.
        inline constexpr std::wstring_view kStrConfigurationSectionLog = L"Log";

        /// Configuration file setting for specifying if the log is enabled.
        inline constexpr std::wstring_view kStrConfigurationSettingLogEnabled = L"Enabled";

        /// Configuration file setting for specifying the logging verbosity level.
        inline constexpr std::wstring_view kStrConfigurationSettingLogLevel = L"Level";


        /// Configuration file section name for mapper-related settings.
        inline constexpr std::wstring_view kStrConfigurationSectionMapper = L"Mapper";

        /// Configuration file setting for specifying the mapper type.
        inline constexpr std::wstring_view kStrConfigurationSettingMapperType = L"Type";


        /// Prefix for configuration file sections that define custom mappers.
        inline constexpr std::wstring_view kStrConfigurationSectionCustomMapperPrefix = L"CustomMapper";

        /// Character that separates a prefix from a custom mapper name within the section name for sections that define custom mappers.
        inline constexpr wchar_t kCharConfigurationSectionCustomMapperSeparator = L':';

        /// Configuration file setting for specifying a custom mapper template.
        inline constexpr std::wstring_view kStrConfigurationSettingCustomMapperTemplate = L"Template";


        /// Configuration file section name for customizing the various properties that govern behavior of virtual controllers.
        inline constexpr std::wstring_view kStrConfigurationSectionProperties = L"Properties";

        /// Configuration file setting for customizing the mouse speed. Expressed as a percentage that is used to scale the default mouse speed.
        inline constexpr std::wstring_view kStrConfigurationSettingPropertiesMouseSpeedScalingFactorPercent = L"MouseSpeedScalingFactorPercent";


        /// Configuration file section name for specifying behavioral tweaks to work around bugs in games.
        inline constexpr std::wstring_view kStrConfigurationSectionWorkarounds = L"Workarounds";

        /// Configuration file setting for a workaround that limits the specific virtual controllers Xidi will enumerate.
        inline constexpr std::wstring_view kStrConfigurationSettingWorkaroundsActiveVirtualControllerMask = L"ActiveVirtualControllerMask";

        /// Configuration file setting for a workaround that overrides the return code of the `IDirectInputDevice::Poll` method, which is unnecessary with Xidi.
        /// Usually this returns `DI_NOEFFECT` but some applications explicitly check for a different return code.
        inline constexpr std::wstring_view kStrConfigurationSettingWorkaroundsPollReturnCode = L"PollReturnCode";

        /// Configuration file setting for a workaround that overrides the return code that Xidi receives from a callback it makes during `IDirectInputDevice::EnumObjects`.
        /// If ignored, the application's callback is always assumed to return `DIENUM_CONTINUE`.
        inline constexpr std::wstring_view kStrConfigurationSettingsWorkaroundsIgnoreEnumObjectsCallbackReturnCode = L"IgnoreEnumObjectsCallbackReturnCode";


        // -------- RUN-TIME CONSTANTS ------------------------------------- //
        // Not safe to access before run-time, and should not be used to perform dynamic initialization.
        // Views are guaranteed to be null-terminated.

        /// Product name.
        /// Use this to identify Xidi in areas of user interaction.
        extern const std::wstring_view kStrProductName;

        /// Form name.
        /// Use this to identify Xidi's form (dinput, dinput8, winmm) in areas of user interaction.
        extern const std::wstring_view kStrFormName;

        /// Complete path and filename of the currently-running executable.
        extern const std::wstring_view kStrExecutableCompleteFilename;

        /// Base name of the currently-running executable.
        extern const std::wstring_view kStrExecutableBaseName;

        /// Directory name of the currently-running executable, including trailing backslash if available.
        extern const std::wstring_view kStrExecutableDirectoryName;

        /// Complete path and filename of the currently-running form of Xidi.
        extern const std::wstring_view kStrXidiCompleteFilename;

        /// Base name of the currently-running form of Xidi.
        extern const std::wstring_view kStrXidiBaseName;

        /// Directory name of the currently-running form of Xidi, including trailing backslash if available.
        extern const std::wstring_view kStrXidiDirectoryName;

        /// Directory name in which system-supplied libraries are found.
        extern const std::wstring_view kStrSystemDirectoryName;

        /// Complete path and filename of the system-supplied DirectInput library.
        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput;

        /// Complete path and filename of the system-supplied DirectInput8 library.
        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput8;

        /// Complete path and filename of the system-supplied WinMM library.
        extern const std::wstring_view kStrSystemLibraryFilenameWinMM;

        /// Expected filename of a configuration file.
        /// Xidi configuration filename = (Xidi directory)\Xidi.ini
        extern const std::wstring_view kStrConfigurationFilename;

        /// Expected filename for the log file.
        /// Xidi log filename = (current user's desktop)\Xidi_(Xidi Form)_(base name of the running executable)_(process ID).log
        extern const std::wstring_view kStrLogFilename;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Returns a string representing the specified axis type.
        /// @param [in] axis Axis type for which a string is requested.
        /// @return String representation of the axis type.
        const wchar_t* AxisTypeString(Controller::EAxis axis);

        /// Compares two strings without regard for the case of each individual character.
        /// @tparam CharType Type of character in each string, either narrow or wide.
        /// @param [in] strA First string in the comparison.
        /// @param [in] strB Second string in the comparison.
        /// @return `true` if the strings compare equal, `false` otherwise.
        template <typename CharType> bool EqualsCaseInsensitive(std::basic_string_view<CharType> strA, std::basic_string_view<CharType> strB);

        /// Formats a string and returns the result in a newly-allocated null-terminated temporary buffer.
        /// @param [in] format Format string, possibly with format specifiers which must be matched with the arguments that follow.
        /// @return Resulting string after all formatting is applied.
        TemporaryString FormatString(_Printf_format_string_ const wchar_t* format, ...);

        /// Retrieves a string used to represent a per-controller mapper type configuration setting.
        /// These are initialized on first invocation and returned subsequently as read-only views.
        /// An empty view is returned if an invalid controller identifier is specified.
        /// @param [in] controllerIdentifier Controller identifier for which a string is desired.
        /// @return Corresponding configuration setting string, or an empty view if the controller identifier is out of range.
        std::wstring_view MapperTypeConfigurationNameString(Controller::TControllerIdentifier controllerIdentifier);

        /// Splits a string using the specified delimiter string and returns a list of views each corresponding to a part of the input string.
        /// If there are too many delimiters present such that not all of the pieces can fit into the returned container type then the returned container will be empty.
        /// Otherwise the returned container will contain at least one element.
        /// @param [in] stringToSplit Input string to be split.
        /// @param [in] delimiter Delimiter character sequence that identifies boundaries between pieces of the input string.
        /// @return Container that holds views referring to pieces of the input string split using the specified delimiter.
        TemporaryVector<std::wstring_view> SplitString(std::wstring_view stringToSplit, std::wstring_view delimiter);

        /// Splits a string using the specified delimiter strings and returns a list of views each corresponding to a part of the input string.
        /// If there are too many delimiters present such that not all of the pieces can fit into the returned container type then the returned container will be empty.
        /// Otherwise the returned container will contain at least one element.
        /// @param [in] stringToSplit Input string to be split.
        /// @param [in] delimiters Delimiter character sequences each of which identifies a boundary between pieces of the input string.
        /// @return Container that holds views referring to pieces of the input string split using the specified delimiter.
        TemporaryVector<std::wstring_view> SplitString(std::wstring_view stringToSplit, std::initializer_list<std::wstring_view> delimiters);

        /// Generates a string representation of a system error code.
        /// @param [in] systemErrorCode System error code for which to generate a string.
        /// @return String representation of the system error code.
        TemporaryString SystemErrorCodeString(const unsigned long systemErrorCode);
    }
}
