/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file Strings.h
 *   Declaration of common strings and functions to manipulate them.
 **************************************************************************************************/

#pragma once

#include <guiddef.h>
#include <sal.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>

#include <Infra/Core/TemporaryBuffer.h>

#include "ControllerTypes.h"

// Strings that need to be available in multiple formats (ASCII and Unicode).
#define XIDI_AXIS_NAME_X                                       "X Axis"
#define XIDI_AXIS_NAME_Y                                       "Y Axis"
#define XIDI_AXIS_NAME_Z                                       "Z Axis"
#define XIDI_AXIS_NAME_RX                                      "RotX Axis"
#define XIDI_AXIS_NAME_RY                                      "RotY Axis"
#define XIDI_AXIS_NAME_RZ                                      "RotZ Axis"
#define XIDI_AXIS_NAME_UNKNOWN                                 "Unknown Axis"
#define XIDI_BUTTON_NAME_FORMAT                                "Button %u"
#define XIDI_POV_NAME                                          "POV"
#define XIDI_WHOLE_CONTROLLER_NAME                             "Whole Controller"
#define XIDI_EFFECT_NAME_CONSTANT_FORCE                        "Constant Force"
#define XIDI_EFFECT_NAME_RAMP_FORCE                            "Ramp Force"
#define XIDI_EFFECT_NAME_SQUARE                                "Square Wave"
#define XIDI_EFFECT_NAME_SINE                                  "Sine Wave"
#define XIDI_EFFECT_NAME_TRIANGLE                              "Triangle Wave"
#define XIDI_EFFECT_NAME_SAWTOOTH_UP                           "Sawtooth Up"
#define XIDI_EFFECT_NAME_SAWTOOTH_DOWN                         "Sawtooth Down"
#define XIDI_EFFECT_NAME_CUSTOM_FORCE                          "Custom Force"

// String prefixes and suffixes that need to be consumed as they are but also combined into longer
// literals. All exist as wide-character strings only.
#define XIDI_CONFIG_PROPERTIES_PREFIX_CIRCLE_TO_SQUARE_PERCENT L"CircleToSquarePercent"
#define XIDI_CONFIG_PROPERTIES_PREFIX_DEADZONE_PERCENT         L"DeadzonePercent"
#define XIDI_CONFIG_PROPERTIES_PREFIX_SATURATION_PERCENT       L"SaturationPercent"
#define XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_LEFT               L"StickLeft"
#define XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_RIGHT              L"StickRight"
#define XIDI_CONFIG_PROPERTIES_SUFFIX_TRIGGER_LT               L"TriggerLT"
#define XIDI_CONFIG_PROPERTIES_SUFFIX_TRIGGER_RT               L"TriggerRT"

namespace Xidi
{
  namespace Strings
  {
    // These strings can safely be used at any time, including to perform static initialization.
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
    inline constexpr std::wstring_view kStrConfigurationSettingImportDirectInput =
        kStrLibraryNameDirectInput;

    /// Configuration file setting for overriding import for DirectInput8.
    inline constexpr std::wstring_view kStrConfigurationSettingImportDirectInput8 =
        kStrLibraryNameDirectInput8;

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

    /// Character that separates a prefix from a custom mapper name within the section name for
    /// sections that define custom mappers.
    inline constexpr wchar_t kCharConfigurationSectionCustomMapperSeparator = L':';

    /// Configuration file setting for specifying a custom mapper template.
    inline constexpr std::wstring_view kStrConfigurationSettingCustomMapperTemplate = L"Template";

    /// Configuration file section name for customizing the various properties that govern behavior
    /// of virtual controllers.
    inline constexpr std::wstring_view kStrConfigurationSectionProperties = L"Properties";

    /// Configuration file setting for customizing the force feedback effect strength. Expressed as
    /// a percentage that is used to scale the final effect values sent to the controller hardware.
    /// This can be used to reduce, but not amplify, the strength of force feedback effects.
    inline constexpr std::wstring_view
        kStrConfigurationSettingPropertiesForceFeedbackEffectStrengthPercent =
            L"ForceFeedbackEffectStrengthPercent";

    /// Configuration file setting for customizing the mouse speed. Expressed as a percentage that
    /// is used to scale the default mouse speed.
    inline constexpr std::wstring_view
        kStrConfigurationSettingPropertiesMouseSpeedScalingFactorPercent =
            L"MouseSpeedScalingFactorPercent";

    /// Configuration file setting for enabling or disabling built-in properties like deadzone and
    /// saturation, which are used for interfaces that do not normally allow for customization.
    inline constexpr std::wstring_view kStrConfigurationSettingsPropertiesUseBuiltinProperties =
        L"UseBuiltInProperties";

    /// Configuration file setting for correcting the left analog stick's circular field of motion
    /// to a square field of motion, expressed as a percent of the maximum possible amount of
    /// correction (perfect circle to perfect square).
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesCircleToSquarePercentStickLeft =
            XIDI_CONFIG_PROPERTIES_PREFIX_CIRCLE_TO_SQUARE_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_LEFT;

    /// Configuration file setting for correcting the right analog stick's circular field of motion
    /// to a square field of motion, expressed as a percent of the maximum possible amount of
    /// correction (perfect circle to perfect square).
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesCircleToSquarePercentStickRight =
            XIDI_CONFIG_PROPERTIES_PREFIX_CIRCLE_TO_SQUARE_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_RIGHT;

    /// Configuration file setting for adding extra deadzone to the left analog stick, expressed as
    /// a percentage of the analog range.
    inline constexpr std::wstring_view kStrConfigurationSettingsPropertiesDeadzonePercentStickLeft =
        XIDI_CONFIG_PROPERTIES_PREFIX_DEADZONE_PERCENT XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_LEFT;

    /// Configuration file setting for adding extra deadzone to the right analog stick, expressed as
    /// a percentage of the analog range.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesDeadzonePercentStickRight =
            XIDI_CONFIG_PROPERTIES_PREFIX_DEADZONE_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_RIGHT;

    /// Configuration file setting for adding extra deadzone to the left analog trigger, expressed
    /// as a percentage of the analog range.
    inline constexpr std::wstring_view kStrConfigurationSettingsPropertiesDeadzonePercentTriggerLT =
        XIDI_CONFIG_PROPERTIES_PREFIX_DEADZONE_PERCENT XIDI_CONFIG_PROPERTIES_SUFFIX_TRIGGER_LT;

    /// Configuration file setting for adding extra deadzone to the right analog trigger, expressed
    /// as a percentage of the analog range.
    inline constexpr std::wstring_view kStrConfigurationSettingsPropertiesDeadzonePercentTriggerRT =
        XIDI_CONFIG_PROPERTIES_PREFIX_DEADZONE_PERCENT XIDI_CONFIG_PROPERTIES_SUFFIX_TRIGGER_RT;

    /// Configuration file setting for adding extra saturation to the left analog stick, expressed
    /// as a percentage of the analog range.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesSaturationPercentStickLeft =
            XIDI_CONFIG_PROPERTIES_PREFIX_SATURATION_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_LEFT;

    /// Configuration file setting for adding extra saturation to the right analog stick, expressed
    /// as a percentage of the analog range.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesSaturationPercentStickRight =
            XIDI_CONFIG_PROPERTIES_PREFIX_SATURATION_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_STICK_RIGHT;

    /// Configuration file setting for adding extra saturation to the left analog trigger, expressed
    /// as a percentage of the analog range.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesSaturationPercentTriggerLT =
            XIDI_CONFIG_PROPERTIES_PREFIX_SATURATION_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_TRIGGER_LT;

    /// Configuration file setting for adding extra saturation to the right analog trigger,
    /// expressed as a percentage of the analog range.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsPropertiesSaturationPercentTriggerRT =
            XIDI_CONFIG_PROPERTIES_PREFIX_SATURATION_PERCENT
                XIDI_CONFIG_PROPERTIES_SUFFIX_TRIGGER_RT;

    /// Configuration file section name for specifying behavioral tweaks to work around bugs in
    /// games.
    inline constexpr std::wstring_view kStrConfigurationSectionWorkarounds = L"Workarounds";

    /// Configuration file setting for a workaround that limits the specific virtual controllers
    /// Xidi will enumerate.
    inline constexpr std::wstring_view
        kStrConfigurationSettingWorkaroundsActiveVirtualControllerMask =
            L"ActiveVirtualControllerMask";

    /// Configuration file setting for a workaround that overrides the return code of the
    /// `IDirectInputDevice::Poll` method, which is unnecessary with Xidi. Usually this returns
    /// `DI_NOEFFECT` but some applications explicitly check for a different return code.
    inline constexpr std::wstring_view kStrConfigurationSettingWorkaroundsPollReturnCode =
        L"PollReturnCode";

    /// Configuration file setting for a workaround that overrides the return code that Xidi
    /// receives from a callback it makes during `IDirectInputDevice::EnumObjects`. If ignored, the
    /// application's callback is always assumed to return `DIENUM_CONTINUE`.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsWorkaroundsIgnoreEnumObjectsCallbackReturnCode =
            L"IgnoreEnumObjectsCallbackReturnCode";

    /// Configuration file setting for a workaround that causes Xidi to use the short-form names for
    /// virtual controllers when providing the "friendly" name to the application. Useful for
    /// applications that truncate the "friendly" name after a small number of characters.
    inline constexpr std::wstring_view
        kStrConfigurationSettingsWorkaroundsUseShortVirtualControllerNames =
            L"UseShortVirtualControllerNames";

    // These strings are not safe to access before run-time, and should not be used to perform
    // dynamic initialization. Views are guaranteed to be null-terminated.

    /// Form name.
    /// Use this to identify Xidi's form (dinput, dinput8, winmm) in areas of user interaction.
    std::wstring_view GetFormName(void);

    /// Directory name in which system-supplied libraries are found.
    std::wstring_view GetSystemDirectoryName(void);

    /// Complete path and filename of the system-supplied DirectInput library.
    std::wstring_view GetSystemLibraryFilenameDirectInput(void);

    /// Complete path and filename of the system-supplied DirectInput8 library.
    std::wstring_view GetSystemLibraryFilenameDirectInput8(void);

    /// Complete path and filename of the system-supplied WinMM library.
    std::wstring_view GetSystemLibraryFilenameWinMM(void);

    /// Returns a string representing the specified axis type.
    /// @param [in] axis Axis type for which a string is requested.
    /// @return String representation of the axis type.
    const wchar_t* AxisTypeString(Controller::EAxis axis);

    /// Generates a string representation of a GUID, in the format
    /// "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}" where X is a hexadecimal digit.
    /// @param guid GUID for which a string is desired.
    /// @return Resulting string representation for the specified GUID.
    Infra::TemporaryString GuidToString(const GUID& guid);

    /// Retrieves a string used to represent a per-controller mapper type configuration setting.
    /// These are initialized on first invocation and returned subsequently as read-only views.
    /// An empty view is returned if an invalid controller identifier is specified.
    /// @param [in] controllerIdentifier Controller identifier for which a string is desired.
    /// @return Corresponding configuration setting string, or an empty view if the controller
    /// identifier is out of range.
    std::wstring_view MapperTypeConfigurationNameString(
        Controller::TControllerIdentifier controllerIdentifier);
  } // namespace Strings
} // namespace Xidi
