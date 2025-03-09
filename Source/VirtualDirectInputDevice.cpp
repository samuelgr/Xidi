/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file VirtualDirectInputDevice.cpp
 *   Implementation of an IDirectInputDevice interface wrapper around virtual controllers.
 **************************************************************************************************/

#include "VirtualDirectInputDevice.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

#include <Infra/Core/Configuration.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiDirectInput.h"
#include "ApiGUID.h"
#include "ControllerIdentification.h"
#include "ControllerTypes.h"
#include "DataFormat.h"
#include "ForceFeedbackDevice.h"
#include "ForceFeedbackTypes.h"
#include "Globals.h"
#include "PhysicalController.h"
#include "Strings.h"
#include "VirtualController.h"
#include "VirtualDirectInputEffect.h"

/// Logs a DirectInput interface method invocation and returns.
#define LOG_INVOCATION_AND_RETURN(result, severity)                                                        \
  do                                                                                                       \
  {                                                                                                        \
    const HRESULT hresult = (result);                                                                      \
    Infra::Message::OutputFormatted(                                                                       \
        severity,                                                                                          \
        L"Invoked %s on interface object %u associated with Xidi virtual controller %u, result = 0x%08x.", \
        __FUNCTIONW__ L"()",                                                                               \
        this->kObjectId,                                                                                   \
        (1 + this->controller->GetIdentifier()),                                                           \
        hresult);                                                                                          \
    return hresult;                                                                                        \
  }                                                                                                        \
  while (false)

/// Logs a DirectInput property-related method invocation and returns.
#define LOG_PROPERTY_INVOCATION_AND_RETURN(result, severity, rguidprop, propvalfmt, ...)                                                          \
  do                                                                                                                                              \
  {                                                                                                                                               \
    const HRESULT hresult = (result);                                                                                                             \
    Infra::Message::OutputFormatted(                                                                                                              \
        severity,                                                                                                                                 \
        L"Invoked function %s on interface object %u associated with Xidi virtual controller %u, result = 0x%08x, property = %s" propvalfmt L".", \
        __FUNCTIONW__ L"()",                                                                                                                      \
        this->kObjectId,                                                                                                                          \
        (1 + this->controller->GetIdentifier()),                                                                                                  \
        hresult,                                                                                                                                  \
        PropertyGuidString(rguidprop),                                                                                                            \
        ##__VA_ARGS__);                                                                                                                           \
    return hresult;                                                                                                                               \
  }                                                                                                                                               \
  while (false)

/// Logs a DirectInput property-related method without a value and returns.
#define LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(result, severity, rguidprop)                   \
  LOG_PROPERTY_INVOCATION_AND_RETURN(result, severity, rguidprop, L"")

/// Logs a DirectInput property-related method where the value is provided in a DIPROPDWORD
/// structure and returns.
#define LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(result, severity, rguidprop, ppropval)      \
  LOG_PROPERTY_INVOCATION_AND_RETURN(                                                              \
      result,                                                                                      \
      severity,                                                                                    \
      rguidprop,                                                                                   \
      L", value = { dwData = %u }",                                                                \
      ((LPDIPROPDWORD)ppropval)->dwData)

/// Logs a DirectInput property-related method where the value is provided in a DIPROPRANGE
/// structure and returns.
#define LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(result, severity, rguidprop, ppropval)      \
  LOG_PROPERTY_INVOCATION_AND_RETURN(                                                              \
      result,                                                                                      \
      severity,                                                                                    \
      rguidprop,                                                                                   \
      L", value = { lMin = %ld, lMax = %ld }",                                                     \
      ((LPDIPROPRANGE)ppropval)->lMin,                                                             \
      ((LPDIPROPRANGE)ppropval)->lMax)

/// Logs a DirectInput property-related method where the value is provided in a DIPROPSTRING
/// structure and returns.
#define LOG_PROPERTY_INVOCATION_DIPROPSTRING_AND_RETURN(result, severity, rguidprop, ppropval)     \
  LOG_PROPERTY_INVOCATION_AND_RETURN(                                                              \
      result, severity, rguidprop, L", value = { wsz = \"%s\" }", ((LPDIPROPSTRING)ppropval)->wsz)

namespace Xidi
{
  /// Alias for a pointer to a function that, when invoked, constructs a force feedback effect
  /// object using the specified GUID type and associated virtual DirectInput device.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  template <EDirectInputVersion diVersion> using TForceFeedbackEffectCreatorFunc =
      std::unique_ptr<VirtualDirectInputEffect<diVersion>> (*)(
          REFGUID, VirtualDirectInputDeviceBase<diVersion>&);

  /// Generator for unique internal object identifiers for each #VirtualDirectInputDeviceBase object
  /// that is created.
  static std::atomic<unsigned int> nextVirtualDirectInputDeviceBaseObjectId = 0;

  /// Converts from axis type enumerator to axis type GUID.
  /// @param [in] axis Axis type enumerator to convert.
  /// @return Read-only reference to the corresponding GUID object.
  static const GUID& AxisTypeGuid(Controller::EAxis axis)
  {
    switch (axis)
    {
      case Controller::EAxis::X:
        return GUID_XAxis;
      case Controller::EAxis::Y:
        return GUID_YAxis;
      case Controller::EAxis::Z:
        return GUID_ZAxis;
      case Controller::EAxis::RotX:
        return GUID_RxAxis;
      case Controller::EAxis::RotY:
        return GUID_RyAxis;
      case Controller::EAxis::RotZ:
        return GUID_RzAxis;
      default:
        return GUID_Unknown;
    }
  }

  /// Fills the specified buffer with a friendly string representation of the specified controller
  /// element. This override is for ANSI-format buffers.
  /// @param [in] element Controller element for which a string is desired.
  /// @param [out] buf Buffer to be filled with the string.
  /// @param [in] bufcount Buffer size in number of characters.
  static void ElementToStringInternal(
      Controller::SElementIdentifier element, LPSTR buf, int bufcount)
  {
    switch (element.type)
    {
      case Controller::EElementType::Axis:
        switch (element.axis)
        {
          case Controller::EAxis::X:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_X, _countof(XIDI_AXIS_NAME_X));
            break;
          case Controller::EAxis::Y:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_Y, _countof(XIDI_AXIS_NAME_Y));
            break;
          case Controller::EAxis::Z:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_Z, _countof(XIDI_AXIS_NAME_Z));
            break;
          case Controller::EAxis::RotX:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_RX, _countof(XIDI_AXIS_NAME_RX));
            break;
          case Controller::EAxis::RotY:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_RY, _countof(XIDI_AXIS_NAME_RY));
            break;
          case Controller::EAxis::RotZ:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_RZ, _countof(XIDI_AXIS_NAME_RZ));
            break;
          default:
            strncpy_s(buf, bufcount, XIDI_AXIS_NAME_UNKNOWN, _countof(XIDI_AXIS_NAME_UNKNOWN));
            break;
        }
        break;

      case Controller::EElementType::Button:
        sprintf_s(buf, bufcount, XIDI_BUTTON_NAME_FORMAT, (1 + (unsigned int)element.button));
        break;

      case Controller::EElementType::Pov:
        strncpy_s(buf, bufcount, XIDI_POV_NAME, _countof(XIDI_POV_NAME));
        break;

      case Controller::EElementType::WholeController:
        strncpy_s(buf, bufcount, XIDI_WHOLE_CONTROLLER_NAME, _countof(XIDI_WHOLE_CONTROLLER_NAME));
        break;
    }
  }

  /// Fills the specified buffer with a friendly string representation of the specified controller
  /// element. This override is for Unicode-format buffers.
  /// @param [in] element Controller element for which a string is desired.
  /// @param [out] buf Buffer to be filled with the string.
  /// @param [in] bufcount Buffer size in number of characters.
  static void ElementToStringInternal(
      Controller::SElementIdentifier element, LPWSTR buf, int bufcount)
  {
    switch (element.type)
    {
      case Controller::EElementType::Axis:
        switch (element.axis)
        {
          case Controller::EAxis::X:
            wcsncpy_s(
                buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_X), _countof(_CRT_WIDE(XIDI_AXIS_NAME_X)));
            break;
          case Controller::EAxis::Y:
            wcsncpy_s(
                buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_Y), _countof(_CRT_WIDE(XIDI_AXIS_NAME_Y)));
            break;
          case Controller::EAxis::Z:
            wcsncpy_s(
                buf, bufcount, _CRT_WIDE(XIDI_AXIS_NAME_Z), _countof(_CRT_WIDE(XIDI_AXIS_NAME_Z)));
            break;
          case Controller::EAxis::RotX:
            wcsncpy_s(
                buf,
                bufcount,
                _CRT_WIDE(XIDI_AXIS_NAME_RX),
                _countof(_CRT_WIDE(XIDI_AXIS_NAME_RX)));
            break;
          case Controller::EAxis::RotY:
            wcsncpy_s(
                buf,
                bufcount,
                _CRT_WIDE(XIDI_AXIS_NAME_RY),
                _countof(_CRT_WIDE(XIDI_AXIS_NAME_RY)));
            break;
          case Controller::EAxis::RotZ:
            wcsncpy_s(
                buf,
                bufcount,
                _CRT_WIDE(XIDI_AXIS_NAME_RZ),
                _countof(_CRT_WIDE(XIDI_AXIS_NAME_RZ)));
            break;
          default:
            wcsncpy_s(
                buf,
                bufcount,
                _CRT_WIDE(XIDI_AXIS_NAME_UNKNOWN),
                _countof(_CRT_WIDE(XIDI_AXIS_NAME_UNKNOWN)));
            break;
        }
        break;

      case Controller::EElementType::Button:
        swprintf_s(
            buf, bufcount, _CRT_WIDE(XIDI_BUTTON_NAME_FORMAT), (1 + (unsigned int)element.button));
        break;

      case Controller::EElementType::Pov:
        wcsncpy_s(buf, bufcount, _CRT_WIDE(XIDI_POV_NAME), _countof(_CRT_WIDE(XIDI_POV_NAME)));
        break;

      case Controller::EElementType::WholeController:
        wcsncpy_s(
            buf,
            bufcount,
            _CRT_WIDE(XIDI_WHOLE_CONTROLLER_NAME),
            _countof(_CRT_WIDE(XIDI_WHOLE_CONTROLLER_NAME)));
        break;
    }
  }

  /// Returns a human-readable string that represents the specified force feedback effect GUID.
  /// @param [in] rguidEffect GUID to check.
  /// @return String representation of the GUID's semantics.
  static const wchar_t* ForceFeedbackEffectGuidString(REFGUID rguidEffect)
  {
    if (rguidEffect == GUID_ConstantForce) return L"ConstantForce";
    if (rguidEffect == GUID_RampForce) return L"RampForce";
    if (rguidEffect == GUID_Square) return L"Square";
    if (rguidEffect == GUID_Sine) return L"Sine";
    if (rguidEffect == GUID_Triangle) return L"Triangle";
    if (rguidEffect == GUID_SawtoothUp) return L"SawtoothUp";
    if (rguidEffect == GUID_SawtoothDown) return L"SawtoothDown";
    if (rguidEffect == GUID_Spring) return L"Spring";
    if (rguidEffect == GUID_Damper) return L"Damper";
    if (rguidEffect == GUID_Inertia) return L"Inertia";
    if (rguidEffect == GUID_Friction) return L"Friction";
    if (rguidEffect == GUID_CustomForce) return L"CustomForce";

    return L"(unknown)";
  }

  /// Returns a string representation of the way in which a controller element is identified.
  /// @param [in] dwHow Value to check.
  /// @return String representation of the identification method.
  static const wchar_t* IdentificationMethodString(DWORD dwHow)
  {
    switch (dwHow)
    {
      case DIPH_DEVICE:
        return L"DIPH_DEVICE";
      case DIPH_BYOFFSET:
        return L"DIPH_BYOFFSET";
      case DIPH_BYUSAGE:
        return L"DIPH_BYUSAGE";
      case DIPH_BYID:
        return L"DIPH_BYID";
      default:
        return L"(unknown)";
    }
  }

  /// Returns a human-readable string that represents the specified property GUID.
  /// @param [in] pguid GUID to check.
  /// @return String representation of the GUID's semantics.
  static const wchar_t* PropertyGuidString(REFGUID rguidProp)
  {
    switch ((size_t)&rguidProp)
    {
      case ((size_t)&DIPROP_KEYNAME):
        return L"DIPROP_KEYNAME";
      case ((size_t)&DIPROP_CPOINTS):
        return L"DIPROP_CPOINTS";
      case ((size_t)&DIPROP_APPDATA):
        return L"DIPROP_APPDATA";
      case ((size_t)&DIPROP_SCANCODE):
        return L"DIPROP_SCANCODE";
      case ((size_t)&DIPROP_VIDPID):
        return L"DIPROP_VIDPID";
      case ((size_t)&DIPROP_USERNAME):
        return L"DIPROP_USERNAME";
      case ((size_t)&DIPROP_TYPENAME):
        return L"DIPROP_TYPENAME";
      case ((size_t)&DIPROP_BUFFERSIZE):
        return L"DIPROP_BUFFERSIZE";
      case ((size_t)&DIPROP_AXISMODE):
        return L"DIPROP_AXISMODE";
      case ((size_t)&DIPROP_GRANULARITY):
        return L"DIPROP_GRANULARITY";
      case ((size_t)&DIPROP_RANGE):
        return L"DIPROP_RANGE";
      case ((size_t)&DIPROP_DEADZONE):
        return L"DIPROP_DEADZONE";
      case ((size_t)&DIPROP_SATURATION):
        return L"DIPROP_SATURATION";
      case ((size_t)&DIPROP_FFGAIN):
        return L"DIPROP_FFGAIN";
      case ((size_t)&DIPROP_FFLOAD):
        return L"DIPROP_FFLOAD";
      case ((size_t)&DIPROP_AUTOCENTER):
        return L"DIPROP_AUTOCENTER";
      case ((size_t)&DIPROP_CALIBRATIONMODE):
        return L"DIPROP_CALIBRATIONMODE";
      case ((size_t)&DIPROP_CALIBRATION):
        return L"DIPROP_CALIBRATION";
      case ((size_t)&DIPROP_GUIDANDPATH):
        return L"DIPROP_GUIDANDPATH";
      case ((size_t)&DIPROP_INSTANCENAME):
        return L"DIPROP_INSTANCENAME";
      case ((size_t)&DIPROP_PRODUCTNAME):
        return L"DIPROP_PRODUCTNAME";
      case ((size_t)&DIPROP_JOYSTICKID):
        return L"DIPROP_JOYSTICKID";
      case ((size_t)&DIPROP_GETPORTDISPLAYNAME):
        return L"DIPROP_GETPORTDISPLAYNAME";
      case ((size_t)&DIPROP_PHYSICALRANGE):
        return L"DIPROP_PHYSICALRANGE";
      case ((size_t)&DIPROP_LOGICALRANGE):
        return L"DIPROP_LOGICALRANGE";
      default:
        return L"(unknown)";
    }
  }

  /// Performs property-specific validation of the supplied property header.
  /// Ensures the header exists and all sizes are correct.
  /// @param [in] rguidProp GUID of the property for which the header is being validated.
  /// @param [in] pdiph Pointer to the property header structure to validate.
  /// @return `true` if the header is valid, `false` otherwise.
  static bool IsPropertyHeaderValid(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
  {
    if (nullptr == pdiph)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Rejected null property header for %s.",
          PropertyGuidString(rguidProp));
      return false;
    }
    else if ((sizeof(DIPROPHEADER) != pdiph->dwHeaderSize))
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Rejected invalid property header for %s: Incorrect size for DIPROPHEADER (expected %u, got %u).",
          PropertyGuidString(rguidProp),
          (unsigned int)sizeof(DIPROPHEADER),
          (unsigned int)pdiph->dwHeaderSize);
      return false;
    }
    else if ((DIPH_DEVICE == pdiph->dwHow) && (0 != pdiph->dwObj))
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Rejected invalid property header for %s: Incorrect object identification value used with DIPH_DEVICE (expected %u, got %u).",
          PropertyGuidString(rguidProp),
          (unsigned int)0,
          (unsigned int)pdiph->dwObj);
      return false;
    }

    // Look for reasons why the property header might be invalid and reject it if any are found.
    switch ((size_t)&rguidProp)
    {
      case ((size_t)&DIPROP_CALIBRATIONMODE):
      case ((size_t)&DIPROP_DEADZONE):
      case ((size_t)&DIPROP_GRANULARITY):
      case ((size_t)&DIPROP_SATURATION):
        // These properties use DIPROPDWORD.
        if (sizeof(DIPROPDWORD) != pdiph->dwSize)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect size for DIPROPDWORD (expected %u, got %u).",
              PropertyGuidString(rguidProp),
              (unsigned int)sizeof(DIPROPDWORD),
              (unsigned int)pdiph->dwSize);
          return false;
        }
        break;

      case ((size_t)&DIPROP_AUTOCENTER):
      case ((size_t)&DIPROP_AXISMODE):
      case ((size_t)&DIPROP_BUFFERSIZE):
      case ((size_t)&DIPROP_FFGAIN):
      case ((size_t)&DIPROP_FFLOAD):
      case ((size_t)&DIPROP_JOYSTICKID):
      case ((size_t)&DIPROP_VIDPID):
        // These properties use DIPROPDWORD and are exclusively device-wide properties.
        if (DIPH_DEVICE != pdiph->dwHow)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect object identification method for this property (expected %s, got %s).",
              PropertyGuidString(rguidProp),
              IdentificationMethodString(DIPH_DEVICE),
              IdentificationMethodString(pdiph->dwHow));
          return false;
        }
        else if (sizeof(DIPROPDWORD) != pdiph->dwSize)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect size for DIPROPDWORD (expected %u, got %u).",
              PropertyGuidString(rguidProp),
              (unsigned int)sizeof(DIPROPDWORD),
              (unsigned int)pdiph->dwSize);
          return false;
        }
        break;

      case ((size_t)&DIPROP_RANGE):
      case ((size_t)&DIPROP_LOGICALRANGE):
      case ((size_t)&DIPROP_PHYSICALRANGE):
        // These properties use DIPROPRANGE.
        if (sizeof(DIPROPRANGE) != pdiph->dwSize)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect size for DIPROPRANGE (expected %u, got %u).",
              PropertyGuidString(rguidProp),
              (unsigned int)sizeof(DIPROPRANGE),
              (unsigned int)pdiph->dwSize);
          return false;
        }
        break;

      case ((size_t)&DIPROP_GETPORTDISPLAYNAME):
      case ((size_t)&DIPROP_INSTANCENAME):
      case ((size_t)&DIPROP_PRODUCTNAME):
      case ((size_t)&DIPROP_USERNAME):
        // These properties use DIPROPSTRING and are exclusively device-wide properties.
        if (DIPH_DEVICE != pdiph->dwHow)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect object identification method for this property (expected %s, got %s).",
              PropertyGuidString(rguidProp),
              IdentificationMethodString(DIPH_DEVICE),
              IdentificationMethodString(pdiph->dwHow));
          return false;
        }
        else if (sizeof(DIPROPSTRING) != pdiph->dwSize)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect size for DIPROPSTRING (expected %u, got %u).",
              PropertyGuidString(rguidProp),
              (unsigned int)sizeof(DIPROPSTRING),
              (unsigned int)pdiph->dwSize);
          return false;
        }
        break;

      case ((size_t)&DIPROP_GUIDANDPATH):
        // This property uses DIPROPGUIDANDPATH and is exclusively a device-wide property.
        if (DIPH_DEVICE != pdiph->dwHow)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect object identification method for this property (expected %s, got %s).",
              PropertyGuidString(rguidProp),
              IdentificationMethodString(DIPH_DEVICE),
              IdentificationMethodString(pdiph->dwHow));
          return false;
        }
        else if (sizeof(DIPROPGUIDANDPATH) != pdiph->dwSize)
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"Rejected invalid property header for %s: Incorrect size for DIPROPGUIDANDPATH (expected %u, got %u).",
              PropertyGuidString(rguidProp),
              (unsigned int)sizeof(DIPROPGUIDANDPATH),
              (unsigned int)pdiph->dwSize);
          return false;
        }
        break;

      default:
        // Any property not listed here is not supported by Xidi and therefore not validated by it.
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Warning,
            L"Skipped property header validation because the property %s is not supported.",
            PropertyGuidString(rguidProp));
        return true;
    }

    Infra::Message::OutputFormatted(
        Infra::Message::ESeverity::Info,
        L"Accepted valid property header for %s.",
        PropertyGuidString(rguidProp));
    return true;
  }

  /// Dumps the top-level components of a property request.
  /// @param [in] rguidProp GUID of the property.
  /// @param [in] pdiph Pointer to the property header.
  /// @param [in] requestTypeIsSet `true` if the request type is SetProperty, `false` if it is
  /// GetProperty.
  static void DumpPropertyRequest(REFGUID rguidProp, LPCDIPROPHEADER pdiph, bool requestTypeIsSet)
  {
    constexpr Infra::Message::ESeverity kDumpSeverity = Infra::Message::ESeverity::Debug;

    if (Infra::Message::WillOutputMessageOfSeverity(kDumpSeverity))
    {
      Infra::Message::Output(kDumpSeverity, L"Begin dump of property request.");

      Infra::Message::Output(kDumpSeverity, L"  Metadata:");
      Infra::Message::OutputFormatted(
          kDumpSeverity,
          L"    operation = %sProperty",
          ((true == requestTypeIsSet) ? L"Set" : L"Get"));
      Infra::Message::OutputFormatted(
          kDumpSeverity, L"    rguidProp = %s", PropertyGuidString(rguidProp));

      Infra::Message::Output(kDumpSeverity, L"  Header:");
      if (nullptr == pdiph)
      {
        Infra::Message::Output(kDumpSeverity, L"    (missing)");
      }
      else
      {
        Infra::Message::OutputFormatted(kDumpSeverity, L"    dwSize = %u", pdiph->dwSize);
        Infra::Message::OutputFormatted(
            kDumpSeverity, L"    dwHeaderSize = %u", pdiph->dwHeaderSize);
        Infra::Message::OutputFormatted(
            kDumpSeverity, L"    dwObj = %u (0x%08x)", pdiph->dwObj, pdiph->dwObj);
        Infra::Message::OutputFormatted(
            kDumpSeverity,
            L"    dwHow = %u (%s)",
            pdiph->dwHow,
            IdentificationMethodString(pdiph->dwHow));
      }

      Infra::Message::Output(kDumpSeverity, L"End dump of property request.");
    }
  }

  /// Holds a registry of functions that construct real force feedback effect objects by GUID and
  /// retrieves pointers to those functions, assuming the GUID identifies a force feedback effect
  /// object that can be constructed.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
  /// @return Pointer to the creation function for the specified GUID if it exists, `nullptr`
  /// otherwise.
  template <EDirectInputVersion diVersion> static TForceFeedbackEffectCreatorFunc<diVersion>
      ForceFeedbackEffectObjectCreator(REFGUID rguidEffect)
  {
    // This registry acts as the single knowledge center on which GUIDs can be constructed into
    // force feedback effect objects and how to do it. Presence or absence of a GUID in this
    // registry determines whether GUIDs are presented during enumeration or are recognized by calls
    // to device interface methods that use force feedback effect GUIDs.
    static const std::unordered_map<GUID, TForceFeedbackEffectCreatorFunc<diVersion>>
        kForceFeedbackEffectObjectCreators = {
            {GUID_ConstantForce,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<ConstantForceDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::ConstantForceEffect(), rguidEffect);
             }},
            {GUID_RampForce,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<RampForceDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::RampForceEffect(), rguidEffect);
             }},
            {GUID_Square,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<PeriodicDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::SquareWaveEffect(), rguidEffect);
             }},
            {GUID_Sine,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<PeriodicDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::SineWaveEffect(), rguidEffect);
             }},
            {GUID_Triangle,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<PeriodicDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::TriangleWaveEffect(), rguidEffect);
             }},
            {GUID_SawtoothUp,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<PeriodicDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::SawtoothUpEffect(), rguidEffect);
             }},
            {GUID_SawtoothDown,
             [](REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
                 -> std::unique_ptr<VirtualDirectInputEffect<diVersion>>
             {
               return std::make_unique<PeriodicDirectInputEffect<diVersion>>(
                   associatedDevice, Controller::ForceFeedback::SawtoothDownEffect(), rguidEffect);
             }},
        };

    auto forceFeedbackEffectObjectCreatorIt = kForceFeedbackEffectObjectCreators.find(rguidEffect);
    if (kForceFeedbackEffectObjectCreators.cend() == forceFeedbackEffectObjectCreatorIt)
      return nullptr;

    return forceFeedbackEffectObjectCreatorIt->second;
  }

  /// Allocates and constructs a new DirectInput force feedback effect object for the specified
  /// GUID.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
  /// @return Smart pointer to the newly-constructed object, or `nullptr` if the GUID is not
  /// supported.
  template <EDirectInputVersion diVersion>
  static std::unique_ptr<VirtualDirectInputEffect<diVersion>> ForceFeedbackEffectCreateObject(
      REFGUID rguidEffect, VirtualDirectInputDeviceBase<diVersion>& associatedDevice)
  {
    TForceFeedbackEffectCreatorFunc<diVersion> forceFeedbackObjectCreator =
        ForceFeedbackEffectObjectCreator<diVersion>(rguidEffect);
    if (nullptr == forceFeedbackObjectCreator) return nullptr;

    return forceFeedbackObjectCreator(rguidEffect, associatedDevice);
  }

  /// Fills the specified buffer with a friendly string representation of the specified force
  /// feedback effect. Default version does nothing.
  /// @tparam CharType String character type, either `char` or `wchar_t`.
  /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
  /// @param [out] buf Buffer to be filled with the string.
  /// @param [in] bufcount Buffer size in number of characters.
  template <typename CharType> static void ForceFeedbackEffectToString(
      REFGUID rguidEffect, CharType* buf, int bufcount)
  {}

  /// Fills the specified buffer with a friendly string representation of the specified force
  /// feedback effect, specialized for ASCII.
  /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
  /// @param [out] buf Buffer to be filled with the string.
  /// @param [in] bufcount Buffer size in number of characters.
  template <> static void ForceFeedbackEffectToString<char>(
      REFGUID rguidEffect, char* buf, int bufcount)
  {
    if (rguidEffect == GUID_ConstantForce)
      strncpy_s(
          buf,
          bufcount,
          XIDI_EFFECT_NAME_CONSTANT_FORCE,
          _countof(XIDI_EFFECT_NAME_CONSTANT_FORCE));
    if (rguidEffect == GUID_RampForce)
      strncpy_s(buf, bufcount, XIDI_EFFECT_NAME_RAMP_FORCE, _countof(XIDI_EFFECT_NAME_RAMP_FORCE));
    if (rguidEffect == GUID_Square)
      strncpy_s(buf, bufcount, XIDI_EFFECT_NAME_SQUARE, _countof(XIDI_EFFECT_NAME_SQUARE));
    if (rguidEffect == GUID_Sine)
      strncpy_s(buf, bufcount, XIDI_EFFECT_NAME_SINE, _countof(XIDI_EFFECT_NAME_SINE));
    if (rguidEffect == GUID_Triangle)
      strncpy_s(buf, bufcount, XIDI_EFFECT_NAME_TRIANGLE, _countof(XIDI_EFFECT_NAME_TRIANGLE));
    if (rguidEffect == GUID_SawtoothUp)
      strncpy_s(
          buf, bufcount, XIDI_EFFECT_NAME_SAWTOOTH_UP, _countof(XIDI_EFFECT_NAME_SAWTOOTH_UP));
    if (rguidEffect == GUID_SawtoothDown)
      strncpy_s(
          buf, bufcount, XIDI_EFFECT_NAME_SAWTOOTH_DOWN, _countof(XIDI_EFFECT_NAME_SAWTOOTH_DOWN));
    if (rguidEffect == GUID_CustomForce)
      strncpy_s(
          buf, bufcount, XIDI_EFFECT_NAME_CUSTOM_FORCE, _countof(XIDI_EFFECT_NAME_CUSTOM_FORCE));
  }

  /// Fills the specified buffer with a friendly string representation of the specified force
  /// feedback effect, specialized for Unicode.
  /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
  /// @param [out] buf Buffer to be filled with the string.
  /// @param [in] bufcount Buffer size in number of characters.
  template <> static void ForceFeedbackEffectToString<wchar_t>(
      REFGUID rguidEffect, wchar_t* buf, int bufcount)
  {
    if (rguidEffect == GUID_ConstantForce)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_CONSTANT_FORCE),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_CONSTANT_FORCE)));
    if (rguidEffect == GUID_RampForce)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_RAMP_FORCE),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_RAMP_FORCE)));
    if (rguidEffect == GUID_Square)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_SQUARE),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_SQUARE)));
    if (rguidEffect == GUID_Sine)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_SINE),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_SINE)));
    if (rguidEffect == GUID_Triangle)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_TRIANGLE),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_TRIANGLE)));
    if (rguidEffect == GUID_SawtoothUp)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_SAWTOOTH_UP),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_SAWTOOTH_UP)));
    if (rguidEffect == GUID_SawtoothDown)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_SAWTOOTH_DOWN),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_SAWTOOTH_DOWN)));
    if (rguidEffect == GUID_CustomForce)
      wcsncpy_s(
          buf,
          bufcount,
          _CRT_WIDE(XIDI_EFFECT_NAME_CUSTOM_FORCE),
          _countof(_CRT_WIDE(XIDI_EFFECT_NAME_CUSTOM_FORCE)));
  }

  /// Retrieves the force feedback effect type, given a force feedback effect GUID.
  /// @param [in] rguidEffect Reference to the GUID that identifies the force feedback effect.
  /// @return DirectInput effect type constant that corresponds to the GUID, if it is recognized.
  static std::optional<DWORD> ForceFeedbackEffectType(REFGUID rguidEffect)
  {
    if (rguidEffect == GUID_ConstantForce) return DIEFT_CONSTANTFORCE;
    if (rguidEffect == GUID_RampForce) return DIEFT_RAMPFORCE;
    if (rguidEffect == GUID_Square) return DIEFT_PERIODIC;
    if (rguidEffect == GUID_Sine) return DIEFT_PERIODIC;
    if (rguidEffect == GUID_Triangle) return DIEFT_PERIODIC;
    if (rguidEffect == GUID_SawtoothUp) return DIEFT_PERIODIC;
    if (rguidEffect == GUID_SawtoothDown) return DIEFT_PERIODIC;
    if (rguidEffect == GUID_CustomForce) return DIEFT_CUSTOMFORCE;

    return std::nullopt;
  }

  /// Computes the offset in a virtual controller's "native" data packet.
  /// For application information only. Cannot be used to identify objects.
  /// Application is presented with the image of a native data packet that stores axes first, then
  /// buttons (one byte per button), then POV.
  /// @param [in] controllerElement Virtual controller element for which a native offset is desired.
  /// @return Native offset value for providing to applications.
  static TOffset NativeOffsetForElement(Controller::SElementIdentifier controllerElement)
  {
    switch (controllerElement.type)
    {
      case Controller::EElementType::Axis:
        return offsetof(Controller::SState, axis) +
            (sizeof(Controller::SState::axis[0]) * (int)controllerElement.axis);

      case Controller::EElementType::Button:
        return offsetof(Controller::SState, button) + (TOffset)controllerElement.button;

      case Controller::EElementType::Pov:
        return offsetof(Controller::SState, button) + (TOffset)Controller::EButton::Count;

      default: // This should never happen.
        return DataFormat::kInvalidOffsetValue;
    }
  }

  /// Generates an object identifier given a controller element and its associated controller
  /// capabilities.
  /// @param [in] controllerCapabilities Capabilities that describe the layout of the virtual
  /// controller.
  /// @param [in] controllerElement Virtual controller element for which an identifier is desired.
  /// @return Resulting object identifier, where a value of `0` indicates an error.
  static DWORD GetObjectId(
      Controller::SCapabilities controllerCapabilities,
      Controller::SElementIdentifier controllerElement)
  {
    switch (controllerElement.type)
    {
      case Controller::EElementType::Axis:
        return (
            DIDFT_ABSAXIS |
            DIDFT_MAKEINSTANCE((int)controllerCapabilities.FindAxis(controllerElement.axis)));

      case Controller::EElementType::Button:
        return (DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE((int)controllerElement.button));

      case Controller::EElementType::Pov:
        return (DIDFT_POV | DIDFT_MAKEINSTANCE(0));

      default:
        return 0;
    }
  }

  /// Fills the specified force feedback effect information structure with information about the
  /// specified force feedback effect. Size member is not touched, so the caller is responsible for
  /// ensuring it is filled correctly. GUID member must already be initialized to identify the
  /// desired effect. Type member must already be initialized to the correct constant value that
  /// corresponds to the pre-filled GUID member.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  /// @param effectInfo [in, out] Structure to be filled with force feedback effect information,
  /// pre-filled with GUID and type.
  template <EDirectInputVersion diVersion> static void FillForceFeedbackEffectInfo(
      typename DirectInputTypes<diVersion>::EffectInfoType* effectInfo)
  {
    // All effects support envelope parameters, both attack and fade.
    constexpr DWORD kEffectTypeExtraFlags = (DIEFT_FFATTACK | DIEFT_FFFADE);
    effectInfo->dwEffType |= kEffectTypeExtraFlags;

    // All effects support these parameters, and they can be changed on-the-fly while effects are
    // playing.
    constexpr DWORD kEffectSupportedParameters =
        (DIEP_AXES | DIEP_DIRECTION | DIEP_DURATION | DIEP_ENVELOPE | DIEP_GAIN |
         DIEP_SAMPLEPERIOD | DIEP_STARTDELAY | DIEP_TYPESPECIFICPARAMS);
    effectInfo->dwStaticParams = kEffectSupportedParameters;
    effectInfo->dwDynamicParams = kEffectSupportedParameters;

    // Last step is to fill in the friendly name.
    ForceFeedbackEffectToString(
        effectInfo->guid, effectInfo->tszName, _countof(effectInfo->tszName));
  }

  /// Fills the specified object instance information structure with information about the specified
  /// HID collection. Size member must already be initialized because multiple versions of the
  /// structure exist, so it is used to determine which members to fill in.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  /// @param hidCollectionNumber HID collection number for which information should be filled.
  /// @param objectInfo [out] Structure to be filled with instance information.
  template <EDirectInputVersion diVersion> static void FillHidCollectionInstanceInfo(
      uint16_t hidCollectionNumber,
      typename DirectInputTypes<diVersion>::DeviceObjectInstanceType* objectInfo)
  {
    // DirectInput versions 5 and higher include extra members in this structure, and this is
    // indicated on input using the size member of the structure.
    if (objectInfo->dwSize > sizeof(DirectInputTypes<diVersion>::DeviceObjectInstanceCompatType))
    {
      const SHidUsageData virtualControllerHidUsageData =
          HidUsageDataForControllerElement({.type = Controller::EElementType::WholeController});

      objectInfo->dwFFMaxForce = 0;
      objectInfo->dwFFForceResolution = 0;
      objectInfo->wCollectionNumber = 0;
      objectInfo->wDesignatorIndex = 0;
      objectInfo->wUsagePage = virtualControllerHidUsageData.usagePage;
      objectInfo->wUsage =
          ((kVirtualControllerHidCollectionForEntireDevice == hidCollectionNumber)
               ? virtualControllerHidUsageData.usage
               : 0);
      objectInfo->dwDimension = 0;
      objectInfo->wExponent = 0;
      objectInfo->wReportId = 0;
    }

    objectInfo->guidType = GUID_Unknown;
    objectInfo->dwOfs = 0;
    objectInfo->dwType = DIDFT_COLLECTION | DIDFT_NODATA | DIDFT_MAKEINSTANCE(hidCollectionNumber);
    objectInfo->dwFlags = 0;

    FillHidCollectionName(objectInfo->tszName, _countof(objectInfo->tszName), hidCollectionNumber);
  }

  /// Fills the specified object instance information structure with information about the specified
  /// controller element. Size member must already be initialized because multiple versions of the
  /// structure exist, so it is used to determine which members to fill in.
  /// @tparam charMode Selects between ASCII ("A" suffix) and Unicode ("W") suffix versions of types
  /// and interfaces.
  /// @param [in] controllerCapabilities Capabilities that describe the layout of the virtual
  /// controller.
  /// @param [in] controllerElement Virtual controller element about which to fill information.
  /// @param [in] offset Offset to place into the object instance information structure.
  /// @param [out] objectInfo Structure to be filled with instance information.
  template <EDirectInputVersion diVersion> static void FillObjectInstanceInfo(
      Controller::SCapabilities controllerCapabilities,
      Controller::SElementIdentifier controllerElement,
      TOffset offset,
      typename DirectInputTypes<diVersion>::DeviceObjectInstanceType* objectInfo)
  {
    // DirectInput versions 5 and higher include extra members in this structure, and this is
    // indicated on input using the size member of the structure.
    if (objectInfo->dwSize > sizeof(DirectInputTypes<diVersion>::DeviceObjectInstanceCompatType))
    {
      const SHidUsageData elementHidUsageData = HidUsageDataForControllerElement(controllerElement);

      objectInfo->dwFFMaxForce = 0;
      objectInfo->dwFFForceResolution = 0;
      objectInfo->wCollectionNumber = kVirtualControllerHidCollectionForIndividualElements;
      objectInfo->wDesignatorIndex = 0;
      objectInfo->wUsagePage = elementHidUsageData.usagePage;
      objectInfo->wUsage = elementHidUsageData.usage;
      objectInfo->dwDimension = 0;
      objectInfo->wExponent = 0;
      objectInfo->wReportId = 0;
    }

    objectInfo->dwOfs = offset;
    objectInfo->dwType = GetObjectId(controllerCapabilities, controllerElement);
    VirtualDirectInputDeviceBase<diVersion>::ElementToString(
        controllerElement, objectInfo->tszName, _countof(objectInfo->tszName));

    switch (controllerElement.type)
    {
      case Controller::EElementType::Axis:
        objectInfo->guidType = AxisTypeGuid(controllerElement.axis);
        objectInfo->dwFlags = DIDOI_ASPECTPOSITION;

        if (controllerCapabilities.ForceFeedbackIsSupportedForAxis(controllerElement.axis))
        {
          objectInfo->dwType |= DIDFT_FFACTUATOR;
          objectInfo->dwFlags |= DIDOI_FFACTUATOR;

          if (objectInfo->dwSize >
              sizeof(DirectInputTypes<diVersion>::DeviceObjectInstanceCompatType))
          {
            // Maximum force is supposedly measured in Newtons. This value is taken from a Logitech
            // RumblePad 2.
            objectInfo->dwFFMaxForce = 10;

            // Supported force range follows the DirectInput allowed range.
            // A difference of 1 probably will not be noticeable to a user, but nevertheless that
            // resolution is supported.
            objectInfo->dwFFForceResolution =
                (DWORD)Controller::ForceFeedback::kEffectForceMagnitudeMaximum;
          }
        }
        break;

      case Controller::EElementType::Button:
        objectInfo->guidType = GUID_Button;
        objectInfo->dwFlags = 0;
        break;

      case Controller::EElementType::Pov:
        objectInfo->guidType = GUID_POV;
        objectInfo->dwFlags = 0;
        break;
    }
  }

  template <EDirectInputVersion diVersion> VirtualDirectInputDeviceBase<diVersion>::
      VirtualDirectInputDeviceBase(std::unique_ptr<Controller::VirtualController>&& controller)
      : kObjectId(nextVirtualDirectInputDeviceBaseObjectId++),
        controller(std::move(controller)),
        cooperativeLevel(ECooperativeLevel::Shared),
        dataFormat(),
        effectRegistry(),
        refCount(1),
        unusedProperties()
  {}

  template <EDirectInputVersion diVersion> VirtualDirectInputDeviceBase<
      diVersion>::~VirtualDirectInputDeviceBase(void)
  {
    controller->ForceFeedbackUnregister();
  }

  template <EDirectInputVersion diVersion> void
      VirtualDirectInputDeviceBase<diVersion>::ElementToString(
          Controller::SElementIdentifier element,
          DirectInputTypes<diVersion>::StringType buf,
          int bufcount)
  {
    return ElementToStringInternal(element, buf, bufcount);
  }

  template <EDirectInputVersion diVersion> bool
      VirtualDirectInputDeviceBase<diVersion>::ForceFeedbackEffectCanCreateObject(
          REFGUID rguidEffect)
  {
    return (nullptr != ForceFeedbackEffectObjectCreator<diVersion>(rguidEffect));
  }

  template <EDirectInputVersion diVersion> Controller::ForceFeedback::Device*
      VirtualDirectInputDeviceBase<diVersion>::AutoAcquireAndGetForceFeedbackDevice(void)
  {
    Controller::ForceFeedback::Device* forceFeedbackDevice = controller->ForceFeedbackGetDevice();

    if (nullptr == forceFeedbackDevice)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Info,
          L"Attempting to acquire Xidi virtual controller %u automatically because the application did not do so explicitly.",
          (1 + controller->GetIdentifier()));

      Acquire();
      forceFeedbackDevice = controller->ForceFeedbackGetDevice();
    }

    return forceFeedbackDevice;
  }

  template <EDirectInputVersion diVersion> std::optional<Controller::SElementIdentifier>
      VirtualDirectInputDeviceBase<diVersion>::IdentifyElement(DWORD dwObj, DWORD dwHow) const
  {
    switch (dwHow)
    {
      case DIPH_DEVICE:
        // Whole device is referenced.
        // Per DirectInput documentation, the object identifier must be 0.
        if (0 == dwObj)
          return Controller::SElementIdentifier(
              {.type = Controller::EElementType::WholeController});
        break;

      case DIPH_BYOFFSET:
        // Controller element is being identified by offset.
        // Object identifier is an offset into the application's data format.
        if (true == IsApplicationDataFormatSet()) return dataFormat->GetElementForOffset(dwObj);
        break;

      case DIPH_BYID:
        // Controller element is being identified by instance identifier.
        // Object identifier contains type and index, and the latter refers to the controller's
        // reported capabilities.
        if ((unsigned int)DIDFT_GETINSTANCE(dwObj) >= 0)
        {
          const unsigned int kType = (unsigned int)DIDFT_GETTYPE(dwObj);
          const unsigned int kIndex = (unsigned int)DIDFT_GETINSTANCE(dwObj);

          switch (kType)
          {
            case DIDFT_ABSAXIS:
              if ((kIndex < (unsigned int)Controller::EAxis::Count) &&
                  (kIndex < (unsigned int)controller->GetCapabilities().numAxes))
                return Controller::SElementIdentifier(
                    {.type = Controller::EElementType::Axis,
                     .axis = controller->GetCapabilities().axisCapabilities[kIndex].type});
              break;

            case DIDFT_PSHBUTTON:
              if ((kIndex < (unsigned int)Controller::EButton::Count) &&
                  (kIndex < (unsigned int)controller->GetCapabilities().numButtons))
                return Controller::SElementIdentifier(
                    {.type = Controller::EElementType::Button,
                     .button = (Controller::EButton)kIndex});
              break;

            case DIDFT_POV:
              if (kIndex == 0)
                return Controller::SElementIdentifier({.type = Controller::EElementType::Pov});
              break;
          }
        }
        break;

      case DIPH_BYUSAGE:
        // Controller element is being identified by HID usage data.
        // The HID specification defines a fixed mapping between this information and controller
        // elements. If the identified controller element exists on this virtual controller then the
        // usage-based identification succeeds.
        {
          const uint16_t hidUsage = (uint16_t)((dwObj >> 0) & 0x0000ffff);
          const uint16_t hidUsagePage = (uint16_t)((dwObj >> 16) & 0x0000ffff);

          const std::optional<Controller::SElementIdentifier> maybeControllerElement =
              ControllerElementFromHidUsageData({.usagePage = hidUsagePage, .usage = hidUsage});

          if (true == maybeControllerElement.has_value())
          {
            const Controller::SElementIdentifier controllerElement = maybeControllerElement.value();
            if (controller->GetCapabilities().HasElement(controllerElement))
              return controllerElement;
          }
          break;
        }
    }

    return std::nullopt;
  }

  template <EDirectInputVersion diVersion> std::optional<DWORD>
      VirtualDirectInputDeviceBase<diVersion>::IdentifyObjectById(
          Controller::SElementIdentifier element) const
  {
    const DWORD objectId = GetObjectId(controller->GetCapabilities(), element);
    if (0 != objectId) return objectId;

    return std::nullopt;
  }

  template <EDirectInputVersion diVersion> std::optional<TOffset>
      VirtualDirectInputDeviceBase<diVersion>::IdentifyObjectByOffset(
          Controller::SElementIdentifier element) const
  {
    if (true == IsApplicationDataFormatSet()) return dataFormat->GetOffsetForElement(element);

    return std::nullopt;
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    if (nullptr == ppvObj) return E_POINTER;

    if (true == DirectInputTypes<diVersion>::IsCompatibleDirectInputDeviceIID(riid))
    {
      AddRef();
      *ppvObj = this;
      return S_OK;
    }

    return E_NOINTERFACE;
  }

  template <EDirectInputVersion diVersion> ULONG VirtualDirectInputDeviceBase<diVersion>::AddRef(
      void)
  {
    return ++refCount;
  }

  template <EDirectInputVersion diVersion> ULONG VirtualDirectInputDeviceBase<diVersion>::Release(
      void)
  {
    const unsigned long numRemainingRefs = --refCount;

    if (0 == numRemainingRefs) delete this;

    return (ULONG)numRemainingRefs;
  }

  template <EDirectInputVersion diVersion> HRESULT VirtualDirectInputDeviceBase<diVersion>::Acquire(
      void)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    // DirectInput documentation requires that the application data format already be set before a
    // device can be acquired.
    if (false == IsApplicationDataFormatSet())
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, Infra::Message::ESeverity::Warning);

    switch (cooperativeLevel)
    {
      case ECooperativeLevel::Exclusive:
        // In exclusive mode, the virtual controller gets access to the physical controller's force
        // feedback buffer. Normally DirectInput would enforce mutual exclusion between
        // applications, hence the "exclusive" terminology. Xidi does not enforce mutual exclusion
        // at all. However, in the interest of compabitility with DirectInput, Xidi does require
        // exclusive acquisition in order for force feedback to be available.

        Infra::Message::OutputFormatted(
            kMethodSeverity,
            L"Acquiring Xidi virtual controller %u in exclusive mode.",
            (1 + controller->GetIdentifier()));

        if (true == controller->ForceFeedbackIsRegistered())
          LOG_INVOCATION_AND_RETURN(S_FALSE, kMethodSeverity);

        if (true == controller->ForceFeedbackRegister())
          LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);

        // Getting to this point means force feedback registration failed. This should not normally
        // occur.
        LOG_INVOCATION_AND_RETURN(DIERR_OTHERAPPHASPRIO, Infra::Message::ESeverity::Error);

      default:
        // No other cooperative level requires any action on Xidi's part for the acquisition to
        // succeed.
        LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::CreateEffect(
          REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (false == controller->GetCapabilities().ForceFeedbackIsSupported())
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Application is attempting to create an effect on Xidi virtual controller %u which does not support force feedback.",
          (1 + controller->GetIdentifier()));
      LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    if (nullptr != punkOuter)
      Infra::Message::Output(
          Infra::Message::ESeverity::Warning,
          L"Application requested COM aggregation, which is not implemented, while creating a force feedback effect.");

    Infra::Message::OutputFormatted(
        Infra::Message::ESeverity::Debug,
        L"Creating effect with GUID %s.",
        ForceFeedbackEffectGuidString(rguid));

    std::unique_ptr<VirtualDirectInputEffect<diVersion>> newEffect =
        ForceFeedbackEffectCreateObject<diVersion>(rguid, *this);
    if (nullptr == newEffect) LOG_INVOCATION_AND_RETURN(DIERR_DEVICENOTREG, kMethodSeverity);

    if (nullptr != lpeff)
    {
      // If parameters are provided they need to be complete.
      // This method does not provide any way of specifying flags to restrict the parameters that
      // are set. However, for compatibility with older versions of DirectInput 5 it is necessary to
      // check the structure size here to avoid having the method read past the end of the valid
      // parameter buffer. Success of this method does not depend on whether or not a download
      // completed successfully. If it failed because the device is full, then the effect can still
      // exist even if it is not physically on the device. Likewise, if the device is not
      // exclusively acquired, then the device just needs to be acquired before the effect can be
      // downloaded.
      const DWORD parameterFlags =
          ((sizeof(DIEFFECT_DX5) == lpeff->dwSize) ? DIEP_ALLPARAMS_DX5 : DIEP_ALLPARAMS);
      switch (newEffect->SetParametersInternal(lpeff, parameterFlags))
      {
        case DI_OK:
        case DI_DOWNLOADSKIPPED:
        case DIERR_NOTEXCLUSIVEACQUIRED:
          break;

        default:
          LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
      }
    }

    Infra::Message::OutputFormatted(
        kMethodSeverity,
        L"Created a force feedback effect and assigned it an identifier of %llu.",
        (unsigned long long)newEffect->UnderlyingEffect().Identifier());

    *ppdeff = newEffect.release();
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::EnumCreatedEffectObjects(
          LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if ((nullptr == lpCallback) || (0 != fl))
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    if (false == controller->GetCapabilities().ForceFeedbackIsSupported())
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Application is attempting to enumerate created effect objects on Xidi virtual controller %u which does not support force feedback.",
          (1 + controller->GetIdentifier()));
      LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    // The loop needs to be structured this way because applications are allowed to destroy the
    // specific effect that is passed in to the callback function during the callback function
    // invocation. Destroying a container element invalidates its iterator, so at all times it is
    // necessary to maintain a valid iterator for the effect after the one being passed into the
    // callback function.
    auto nextEffectIter = effectRegistry.begin();
    while (nextEffectIter != effectRegistry.end())
    {
      auto currentEffectIter = nextEffectIter++;
      switch (lpCallback((LPDIRECTINPUTEFFECT)(*currentEffectIter), pvRef))
      {
        case DIENUM_CONTINUE:
          break;
        case DIENUM_STOP:
          LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
        default:
          LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
      }
    }

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::EnumEffects(
          DirectInputTypes<diVersion>::EnumEffectsCallbackType lpCallback,
          LPVOID pvRef,
          DWORD dwEffType)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (nullptr == lpCallback) LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    if (false == controller->GetCapabilities().ForceFeedbackIsSupported())
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Application is attempting to enumerate effects on Xidi virtual controller %u which does not support force feedback.",
          (1 + controller->GetIdentifier()));
      LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
    }

    const bool willEnumerateConstantForce =
        ((DIEFT_ALL == dwEffType) || (DIEFT_CONSTANTFORCE == DIEFT_GETTYPE(dwEffType)));
    const bool willEnumerateRampForce =
        ((DIEFT_ALL == dwEffType) || (DIEFT_RAMPFORCE == DIEFT_GETTYPE(dwEffType)));
    const bool willEnumeratePeriodic =
        ((DIEFT_ALL == dwEffType) || (DIEFT_PERIODIC == DIEFT_GETTYPE(dwEffType)));
    const bool willEnumerateCustomForce =
        ((DIEFT_ALL == dwEffType) || (DIEFT_CUSTOMFORCE == DIEFT_GETTYPE(dwEffType)));

    if ((true == willEnumerateConstantForce) || (true == willEnumerateCustomForce) ||
        (true == willEnumeratePeriodic) || (true == willEnumerateRampForce))
    {
      std::unique_ptr<DirectInputTypes<diVersion>::EffectInfoType> effectDescriptor =
          std::make_unique<DirectInputTypes<diVersion>::EffectInfoType>();

      if (true == willEnumerateConstantForce)
      {
        const GUID* kEffectGuids[] = {&GUID_ConstantForce};
        for (const auto effectGuid : kEffectGuids)
        {
          if (true == ForceFeedbackEffectCanCreateObject(*effectGuid))
          {
            *effectDescriptor = {
                .dwSize = sizeof(*effectDescriptor),
                .guid = *effectGuid,
                .dwEffType = ForceFeedbackEffectType(*effectGuid).value()};
            FillForceFeedbackEffectInfo<diVersion>(effectDescriptor.get());
            switch (lpCallback(effectDescriptor.get(), pvRef))
            {
              case DIENUM_CONTINUE:
                break;
              case DIENUM_STOP:
                LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
              default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
            }
          }
        }
      }

      if (true == willEnumerateRampForce)
      {
        const GUID* kEffectGuids[] = {&GUID_RampForce};
        for (const auto effectGuid : kEffectGuids)
        {
          if (true == ForceFeedbackEffectCanCreateObject(*effectGuid))
          {
            *effectDescriptor = {
                .dwSize = sizeof(*effectDescriptor),
                .guid = *effectGuid,
                .dwEffType = ForceFeedbackEffectType(*effectGuid).value()};
            FillForceFeedbackEffectInfo<diVersion>(effectDescriptor.get());
            switch (lpCallback(effectDescriptor.get(), pvRef))
            {
              case DIENUM_CONTINUE:
                break;
              case DIENUM_STOP:
                LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
              default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
            }
          }
        }
      }

      if (true == willEnumeratePeriodic)
      {
        const GUID* kEffectGuids[] = {
            &GUID_Square, &GUID_Sine, &GUID_Triangle, &GUID_SawtoothUp, &GUID_SawtoothDown};
        for (const auto effectGuid : kEffectGuids)
        {
          if (true == ForceFeedbackEffectCanCreateObject(*effectGuid))
          {
            *effectDescriptor = {
                .dwSize = sizeof(*effectDescriptor),
                .guid = *effectGuid,
                .dwEffType = ForceFeedbackEffectType(*effectGuid).value()};
            FillForceFeedbackEffectInfo<diVersion>(effectDescriptor.get());
            switch (lpCallback(effectDescriptor.get(), pvRef))
            {
              case DIENUM_CONTINUE:
                break;
              case DIENUM_STOP:
                LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
              default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
            }
          }
        }
      }

      if (true == willEnumerateCustomForce)
      {
        const GUID* kEffectGuids[] = {&GUID_CustomForce};
        for (const auto effectGuid : kEffectGuids)
        {
          if (true == ForceFeedbackEffectCanCreateObject(*effectGuid))
          {
            *effectDescriptor = {
                .dwSize = sizeof(*effectDescriptor),
                .guid = *effectGuid,
                .dwEffType = ForceFeedbackEffectType(*effectGuid).value()};
            FillForceFeedbackEffectInfo<diVersion>(effectDescriptor.get());
            switch (lpCallback(effectDescriptor.get(), pvRef))
            {
              case DIENUM_CONTINUE:
                break;
              case DIENUM_STOP:
                LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
              default:
                LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
            }
          }
        }
      }
    }

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::EnumEffectsInFile(
          DirectInputTypes<diVersion>::ConstStringType lptszFileName,
          LPDIENUMEFFECTSINFILECALLBACK pec,
          LPVOID pvRef,
          DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::EnumObjects(
          DirectInputTypes<diVersion>::EnumObjectsCallbackType lpCallback,
          LPVOID pvRef,
          DWORD dwFlags)
  {
    static const bool kAlwaysContinueEnumerating =
        Globals::GetConfigurationData()
            [Strings::kStrConfigurationSectionWorkarounds]
            [Strings::kStrConfigurationSettingsWorkaroundsIgnoreEnumObjectsCallbackReturnCode]
                .ValueOr(false);
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (nullptr == lpCallback) LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    // Force feedback effect triggers are not supported, so no objects will match.
    const bool forceFeedbackEffectTriggersOnly = (0 != (dwFlags & DIDFT_FFEFFECTTRIGGER));
    if (true == forceFeedbackEffectTriggersOnly) LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);

    const bool forceFeedbackActuatorsOnly = (0 != (dwFlags & DIDFT_FFACTUATOR));
    const bool outsideHidCollectionOnly = (0 != (dwFlags & DIDFT_NOCOLLECTION));

    const bool willEnumerateAxes = ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_ABSAXIS)));
    const bool willEnumerateButtons =
        ((false == forceFeedbackActuatorsOnly) && (false == outsideHidCollectionOnly) &&
         ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_PSHBUTTON))));
    const bool willEnumeratePov =
        ((false == forceFeedbackActuatorsOnly) && (false == outsideHidCollectionOnly) &&
         ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_POV))));
    const bool willEnumerateHidCollections =
        ((false == forceFeedbackActuatorsOnly) && (false == outsideHidCollectionOnly) &&
         ((DIDFT_ALL == dwFlags) || (0 != (dwFlags & DIDFT_COLLECTION))));

    if ((true == willEnumerateAxes) || (true == willEnumerateButtons) ||
        (true == willEnumeratePov) || (true == willEnumerateHidCollections))
    {
      std::unique_ptr<DirectInputTypes<diVersion>::DeviceObjectInstanceType> objectDescriptor =
          std::make_unique<DirectInputTypes<diVersion>::DeviceObjectInstanceType>();
      const Controller::SCapabilities controllerCapabilities = controller->GetCapabilities();

      if (true == willEnumerateAxes)
      {
        for (int i = 0; i < controllerCapabilities.numAxes; ++i)
        {
          if ((true == forceFeedbackActuatorsOnly) &&
              (false == controllerCapabilities.axisCapabilities[i].supportsForceFeedback))
            continue;

          const Controller::EAxis axis = controllerCapabilities.axisCapabilities[i].type;
          const Controller::SElementIdentifier axisIdentifier = {
              .type = Controller::EElementType::Axis, .axis = axis};
          const TOffset axisOffset =
              ((true == IsApplicationDataFormatSet())
                   ? dataFormat->GetOffsetForElement(axisIdentifier)
                         .value_or(DataFormat::kInvalidOffsetValue)
                   : NativeOffsetForElement(axisIdentifier));

          *objectDescriptor = {.dwSize = sizeof(*objectDescriptor)};
          FillObjectInstanceInfo<diVersion>(
              controllerCapabilities, axisIdentifier, axisOffset, objectDescriptor.get());

          const bool continueEnumerating =
              (DIENUM_STOP != lpCallback(objectDescriptor.get(), pvRef));
          if (!kAlwaysContinueEnumerating && !continueEnumerating)
            LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
        }
      }

      if (true == willEnumerateButtons)
      {
        for (int i = 0; i < controllerCapabilities.numButtons; ++i)
        {
          const Controller::EButton button = (Controller::EButton)i;
          const Controller::SElementIdentifier buttonIdentifier = {
              .type = Controller::EElementType::Button, .button = button};
          const TOffset buttonOffset =
              ((true == IsApplicationDataFormatSet())
                   ? dataFormat->GetOffsetForElement(buttonIdentifier)
                         .value_or(DataFormat::kInvalidOffsetValue)
                   : NativeOffsetForElement(buttonIdentifier));

          *objectDescriptor = {.dwSize = sizeof(*objectDescriptor)};
          FillObjectInstanceInfo<diVersion>(
              controllerCapabilities, buttonIdentifier, buttonOffset, objectDescriptor.get());

          const bool continueEnumerating =
              (DIENUM_STOP != lpCallback(objectDescriptor.get(), pvRef));
          if (!kAlwaysContinueEnumerating && !continueEnumerating)
            LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
        }
      }

      if (true == willEnumeratePov)
      {
        if (true == controllerCapabilities.HasPov())
        {
          const Controller::SElementIdentifier povIdentifier = {
              .type = Controller::EElementType::Pov};
          const TOffset povOffset =
              ((true == IsApplicationDataFormatSet())
                   ? dataFormat->GetOffsetForElement(povIdentifier)
                         .value_or(DataFormat::kInvalidOffsetValue)
                   : NativeOffsetForElement(povIdentifier));

          *objectDescriptor = {.dwSize = sizeof(*objectDescriptor)};
          FillObjectInstanceInfo<diVersion>(
              controllerCapabilities, povIdentifier, povOffset, objectDescriptor.get());

          const bool continueEnumerating =
              (DIENUM_STOP != lpCallback(objectDescriptor.get(), pvRef));
          if (!kAlwaysContinueEnumerating && !continueEnumerating)
            LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
        }
      }

      if (true == willEnumerateHidCollections)
      {
        constexpr uint16_t kHidCollectionsToEnumerate[] = {
            kVirtualControllerHidCollectionForEntireDevice,
            kVirtualControllerHidCollectionForIndividualElements};

        for (const auto hidCollectionNumber : kHidCollectionsToEnumerate)
        {
          *objectDescriptor = {.dwSize = sizeof(*objectDescriptor)};
          FillHidCollectionInstanceInfo<diVersion>(hidCollectionNumber, objectDescriptor.get());

          const bool continueEnumerating =
              (DIENUM_STOP != lpCallback(objectDescriptor.get(), pvRef));
          if (!kAlwaysContinueEnumerating && !continueEnumerating)
            LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
        }
      }
    }

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT VirtualDirectInputDeviceBase<diVersion>::Escape(
      LPDIEFFESCAPE pesc)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (nullptr == lpDIDevCaps) LOG_INVOCATION_AND_RETURN(E_POINTER, kMethodSeverity);

    const bool kForceFeedbackIsSupported =
        GetVirtualController().GetCapabilities().ForceFeedbackIsSupported();

    switch (lpDIDevCaps->dwSize)
    {
      case (sizeof(DIDEVCAPS)):
        // Hardware information, only present in the latest version of the structure.
        lpDIDevCaps->dwFirmwareRevision = 1;
        lpDIDevCaps->dwHardwareRevision = 1;

        // Force feedback information, only present in the latest version of the structure.
        if (true == kForceFeedbackIsSupported)
        {
          lpDIDevCaps->dwFFSamplePeriod =
              VirtualDirectInputEffect<diVersion>::ConvertTimeToDirectInput(
                  Controller::kPhysicalForceFeedbackPeriodMilliseconds);
          lpDIDevCaps->dwFFMinTimeResolution =
              VirtualDirectInputEffect<diVersion>::ConvertTimeToDirectInput(1);
          lpDIDevCaps->dwFFDriverVersion = 1;
        }
        else
        {
          lpDIDevCaps->dwFFSamplePeriod = 0;
          lpDIDevCaps->dwFFMinTimeResolution = 0;
          lpDIDevCaps->dwFFDriverVersion = 0;
        }
        [[fallthrough]];

      case (sizeof(DIDEVCAPS_DX3)):
        // Top-level controller information is common to all virtual controllers.
        lpDIDevCaps->dwFlags = DIDC_ATTACHED | DIDC_EMULATED;
        lpDIDevCaps->dwDevType = DirectInputTypes<diVersion>::XinputGamepadDeviceType();

        // Additional flags must be specified for force feedback axes.
        if (true == kForceFeedbackIsSupported)
          lpDIDevCaps->dwFlags |=
              (DIDC_FORCEFEEDBACK | DIDC_FFFADE | DIDC_FFATTACK | DIDC_STARTDELAY);

        // Information about controller layout comes from controller capabilities.
        lpDIDevCaps->dwAxes = controller->GetCapabilities().numAxes;
        lpDIDevCaps->dwButtons = controller->GetCapabilities().numButtons;
        lpDIDevCaps->dwPOVs = ((true == controller->GetCapabilities().HasPov()) ? 1 : 0);
        break;

      default:
        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
    }

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetDeviceData(
          DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::SuperDebug;
    constexpr Infra::Message::ESeverity kMethodSeverityForError = Infra::Message::ESeverity::Info;

    // DIDEVICEOBJECTDATA and DIDEVICEOBJECTDATA_DX3 are defined identically for all DirectInput
    // versions below 8. There is therefore no need to differentiate, as the distinction between
    // "dinput" and "dinput8" takes care of it.

    if ((false == IsApplicationDataFormatSet()) || (nullptr == pdwInOut) ||
        (sizeof(DIDEVICEOBJECTDATA) != cbObjectData))
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverityForError);

    switch (dwFlags)
    {
      case 0:
      case DIGDD_PEEK:
        break;

      default:
        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverityForError);
    }

    if (false == controller->IsEventBufferEnabled())
      LOG_INVOCATION_AND_RETURN(DIERR_NOTBUFFERED, kMethodSeverityForError);

    auto lock = controller->Lock();
    const DWORD numEventsAffected = std::min(*pdwInOut, (DWORD)controller->GetEventBufferCount());
    const bool eventBufferOverflowed = controller->IsEventBufferOverflowed();
    const bool shouldPopEvents = (0 == (dwFlags & DIGDD_PEEK));

    if (nullptr != rgdod)
    {
      for (DWORD i = 0; i < numEventsAffected; ++i)
      {
        const Controller::StateChangeEventBuffer::SEvent& event =
            controller->GetEventBufferEvent(i);
        ZeroMemory(&rgdod[i], sizeof(rgdod[i]));
        rgdod[i].dwOfs = dataFormat->GetOffsetForElement(event.data.element)
                             .value(); // A value should always be present.
        rgdod[i].dwTimeStamp = event.timestamp;
        rgdod[i].dwSequence = event.sequence;

        switch (event.data.element.type)
        {
          case Controller::EElementType::Axis:
            rgdod[i].dwData = (DWORD)DataFormat::DirectInputAxisValue(event.data.value.axis);
            break;

          case Controller::EElementType::Button:
            rgdod[i].dwData = (DWORD)DataFormat::DirectInputButtonValue(event.data.value.button);
            break;

          case Controller::EElementType::Pov:
            rgdod[i].dwData = (DWORD)DataFormat::DirectInputPovValue(event.data.value.povDirection);
            break;

          default:
            LOG_INVOCATION_AND_RETURN(
                DIERR_GENERIC, kMethodSeverityForError); // This should never happen.
            break;
        }
      }
    }

    if (true == shouldPopEvents) controller->PopEventBufferOldestEvents(numEventsAffected);

    *pdwInOut = numEventsAffected;
    LOG_INVOCATION_AND_RETURN(
        ((true == eventBufferOverflowed) ? DI_BUFFEROVERFLOW : DI_OK), kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetDeviceInfo(
          DirectInputTypes<diVersion>::DeviceInstanceType* pdidi)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (nullptr == pdidi) LOG_INVOCATION_AND_RETURN(E_POINTER, kMethodSeverity);

    switch (pdidi->dwSize)
    {
      case (sizeof(DirectInputTypes<diVersion>::DeviceInstanceType)):
      case (sizeof(DirectInputTypes<diVersion>::DeviceInstanceCompatType)):
        break;

      default:
        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
    }

    FillVirtualControllerInfo<diVersion>(*pdidi, controller->GetIdentifier());
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetDeviceState(DWORD cbData, LPVOID lpvData)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::SuperDebug;
    constexpr Infra::Message::ESeverity kMethodSeverityForError = Infra::Message::ESeverity::Info;

    if ((nullptr == lpvData) || (false == IsApplicationDataFormatSet()) ||
        (cbData < dataFormat->GetPacketSizeBytes()))
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverityForError);

    bool writeDataPacketResult = false;
    {
      auto lock = controller->Lock();
      writeDataPacketResult = dataFormat->WriteDataPacket(lpvData, cbData, controller->GetState());
    }
    LOG_INVOCATION_AND_RETURN(
        ((true == writeDataPacketResult) ? DI_OK : DIERR_INVALIDPARAM), kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetEffectInfo(
          DirectInputTypes<diVersion>::EffectInfoType* pdei, REFGUID rguid)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (false == controller->GetCapabilities().ForceFeedbackIsSupported())
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Application is attempting to get force feedback effect information on Xidi virtual controller %u which does not support force feedback.",
          (1 + controller->GetIdentifier()));
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
    }

    if (nullptr == pdei) LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    if (sizeof(*pdei) != pdei->dwSize)
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    const std::optional<DWORD> maybeEffectType = ForceFeedbackEffectType(rguid);
    if (false == maybeEffectType.has_value())
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    const DWORD effectType = maybeEffectType.value();
    *pdei = {.dwSize = sizeof(*pdei), .guid = rguid, .dwEffType = effectType};
    FillForceFeedbackEffectInfo<diVersion>(pdei);

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetForceFeedbackState(LPDWORD pdwOut)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (false == controller->GetCapabilities().ForceFeedbackIsSupported())
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Application is attempting to get force feedback state on Xidi virtual controller %u which does not support force feedback.",
          (1 + controller->GetIdentifier()));
      LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    if (nullptr == pdwOut) LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    Controller::ForceFeedback::Device* forceFeedbackDevice = AutoAcquireAndGetForceFeedbackDevice();
    if (nullptr == forceFeedbackDevice)
      LOG_INVOCATION_AND_RETURN(DIERR_NOTEXCLUSIVEACQUIRED, kMethodSeverity);

    DWORD forceFeedbackDeviceState = DIGFFS_POWERON;

    if (true == forceFeedbackDevice->IsDeviceOutputMuted())
      forceFeedbackDeviceState |= DIGFFS_ACTUATORSOFF;
    else
      forceFeedbackDeviceState |= DIGFFS_ACTUATORSON;

    const bool deviceIsEmpty = forceFeedbackDevice->IsDeviceEmpty();
    const bool deviceIsPaused = forceFeedbackDevice->IsDeviceOutputPaused();

    if (true == deviceIsEmpty)
    {
      // If the device is empty it could also be paused.

      forceFeedbackDeviceState |= DIGFFS_EMPTY;

      if (true == deviceIsPaused) forceFeedbackDeviceState |= DIGFFS_PAUSED;
    }
    else
    {
      // If the device is not empty, then it could either be playing effects, stopped (playing no
      // effects), or paused (whether or not effects are playing is irrelevant). DirectInput
      // documentation defines "stopped" state as being mutually exclusive with "paused" state, with
      // the latter taking priority.

      if (true == deviceIsPaused)
        forceFeedbackDeviceState |= DIGFFS_PAUSED;
      else if (false == forceFeedbackDevice->IsDevicePlayingAnyEffects())
        forceFeedbackDeviceState |= DIGFFS_STOPPED;
    }

    *pdwOut = forceFeedbackDeviceState;
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetObjectInfo(
          DirectInputTypes<diVersion>::DeviceObjectInstanceType* pdidoi, DWORD dwObj, DWORD dwHow)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (nullptr == pdidoi) LOG_INVOCATION_AND_RETURN(E_POINTER, kMethodSeverity);

    switch (pdidoi->dwSize)
    {
      case (sizeof(DirectInputTypes<diVersion>::DeviceObjectInstanceType)):
      case (sizeof(DirectInputTypes<diVersion>::DeviceObjectInstanceCompatType)):
        break;

      default:
        LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);
    }

    const std::optional<Controller::SElementIdentifier> maybeElement =
        IdentifyElement(dwObj, dwHow);
    if (false == maybeElement.has_value())
      LOG_INVOCATION_AND_RETURN(DIERR_OBJECTNOTFOUND, kMethodSeverity);

    const Controller::SElementIdentifier element = maybeElement.value();
    if (Controller::EElementType::WholeController == element.type)
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    FillObjectInstanceInfo<diVersion>(
        controller->GetCapabilities(),
        element,
        ((true == IsApplicationDataFormatSet())
             ? dataFormat->GetOffsetForElement(element).value_or(DataFormat::kInvalidOffsetValue)
             : NativeOffsetForElement(element)),
        pdidoi);
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    DumpPropertyRequest(rguidProp, pdiph, false);

    if (false == IsPropertyHeaderValid(rguidProp, pdiph))
      LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);

    const std::optional<Controller::SElementIdentifier> maybeElement =
        IdentifyElement(pdiph->dwObj, pdiph->dwHow);
    if (false == maybeElement.has_value())
      LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_OBJECTNOTFOUND, kMethodSeverity, rguidProp);

    const Controller::SElementIdentifier element = maybeElement.value();

    switch ((size_t)&rguidProp)
    {
      case ((size_t)&DIPROP_AXISMODE):
        ((LPDIPROPDWORD)pdiph)->dwData = DIPROPAXISMODE_ABS;
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_AUTOCENTER):
        ((LPDIPROPDWORD)pdiph)->dwData = unusedProperties.autocenter;
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_BUFFERSIZE):
        ((LPDIPROPDWORD)pdiph)->dwData = controller->GetEventBufferCapacity();
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_CALIBRATIONMODE):
        if (Controller::EElementType::Axis != element.type)
          LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(
              DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
        ((LPDIPROPDWORD)pdiph)->dwData =
            ((true == controller->GetAxisTransformationsEnabled(element.axis))
                 ? DIPROPCALIBRATIONMODE_COOKED
                 : DIPROPCALIBRATIONMODE_RAW);
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_DEADZONE):
        if (Controller::EElementType::Axis != element.type)
          LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(
              DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
        ((LPDIPROPDWORD)pdiph)->dwData = controller->GetAxisDeadzone(element.axis);
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_FFGAIN):
        ((LPDIPROPDWORD)pdiph)->dwData = controller->GetForceFeedbackGain();
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_FFLOAD):
      {
        Controller::ForceFeedback::Device* forceFeedbackDevice =
            AutoAcquireAndGetForceFeedbackDevice();
        if (nullptr == forceFeedbackDevice)
          LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
              DIERR_NOTEXCLUSIVEACQUIRED, kMethodSeverity, rguidProp, pdiph);

        // There is no practical limit on the number of force feedback effects that be loaded to a
        // virtual force feedback device. If the device has no effects then it is zero percent
        // loaded, otherwise it is ceiling(a small quantity greater than 0) = 1 percent loaded.
        ((LPDIPROPDWORD)pdiph)->dwData = ((true == forceFeedbackDevice->IsDeviceEmpty()) ? 0 : 1);

        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);
      }

      case ((size_t)&DIPROP_GETPORTDISPLAYNAME):
        // Port display name is not particularly important but it is the same for all virtual
        // controllers. Xidi just reports its own product name as the port display name. Per
        // DirectInput documentation the return code for this one particular property is always
        // `S_FALSE`.
        wcsncpy_s(
            ((LPDIPROPSTRING)pdiph)->wsz,
            _countof(((LPDIPROPSTRING)pdiph)->wsz),
            Infra::ProcessInfo::GetProductName().data(),
            Infra::ProcessInfo::GetProductName().length());
        LOG_PROPERTY_INVOCATION_DIPROPSTRING_AND_RETURN(S_FALSE, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_GRANULARITY):
        switch (element.type)
        {
          case Controller::EElementType::Axis:
          case Controller::EElementType::WholeController:
            break;

          default:
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
        }
        ((LPDIPROPDWORD)pdiph)->dwData = 1;
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_GUIDANDPATH):
        ((LPDIPROPGUIDANDPATH)pdiph)->guidClass = VirtualControllerClassGuid();
        FillVirtualControllerPath(
            ((LPDIPROPGUIDANDPATH)pdiph)->wszPath,
            _countof(((LPDIPROPGUIDANDPATH)pdiph)->wszPath),
            controller->GetIdentifier());
        LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DI_OK, kMethodSeverity, rguidProp);

      case ((size_t)&DIPROP_INSTANCENAME):
      case ((size_t)&DIPROP_PRODUCTNAME):
        FillVirtualControllerName(
            ((LPDIPROPSTRING)pdiph)->wsz,
            _countof(((LPDIPROPSTRING)pdiph)->wsz),
            controller->GetIdentifier());
        LOG_PROPERTY_INVOCATION_DIPROPSTRING_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_JOYSTICKID):
        ((LPDIPROPDWORD)pdiph)->dwData = (DWORD)controller->GetIdentifier();
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_LOGICALRANGE):
      case ((size_t)&DIPROP_PHYSICALRANGE):
        switch (element.type)
        {
          case Controller::EElementType::Axis:
          case Controller::EElementType::WholeController:
            break;

          default:
            LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
        }
        ((LPDIPROPRANGE)pdiph)->lMin = Controller::kAnalogValueMin;
        ((LPDIPROPRANGE)pdiph)->lMax = Controller::kAnalogValueMax;
        LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_RANGE):
      {
        if (Controller::EElementType::Axis != element.type)
          LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(
              DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);

        const std::pair axisRange = controller->GetAxisRange(element.axis);
        ((LPDIPROPRANGE)pdiph)->lMin = axisRange.first;
        ((LPDIPROPRANGE)pdiph)->lMax = axisRange.second;

        LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);
      }

      case ((size_t)&DIPROP_SATURATION):
        if (Controller::EElementType::Axis != element.type)
          LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(
              DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);
        ((LPDIPROPDWORD)pdiph)->dwData = controller->GetAxisSaturation(element.axis);
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_USERNAME):
        // This property is available starting in DirectInput version 8.
        if constexpr (
            (diVersion == EDirectInputVersion::k8A) || (diVersion == EDirectInputVersion::k8W))
        {
          // Xidi does not support action maps, so the user name property cannot be set on a virtual
          // controller. Per DirectInput documentation the return code is `S_FALSE` when a user name
          // is not assigned to a DirectInput device.
          ((LPDIPROPSTRING)pdiph)->wsz[0] = L'\0';
          LOG_PROPERTY_INVOCATION_DIPROPSTRING_AND_RETURN(
              S_FALSE, kMethodSeverity, rguidProp, pdiph);
        }
        else
        {
          LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
              DIERR_UNSUPPORTED, kMethodSeverity, rguidProp, pdiph);
        }

      case ((size_t)&DIPROP_VIDPID):
        // This property is available starting in DirectInput version 8.
        if constexpr (
            (diVersion == EDirectInputVersion::k8A) || (diVersion == EDirectInputVersion::k8W))
        {
          ((LPDIPROPDWORD)pdiph)->dwData =
              ((DWORD)VirtualControllerProductId(controller->GetIdentifier()) << 16) |
              ((DWORD)kVirtualControllerVendorId);
          LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);
        }
        else
        {
          LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
              DIERR_UNSUPPORTED, kMethodSeverity, rguidProp, pdiph);
        }

      default:
        LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity, rguidProp);
    }
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::Initialize(
          HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
  {
    // Not required for Xidi virtual controllers as they are implemented now.
    // However, this method is needed for creating IDirectInputDevice objects via COM.

    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT VirtualDirectInputDeviceBase<diVersion>::Poll(
      void)
  {
    // Not required for Xidi virtual controllers as they are implemented now.
    // However, some applications explicitly check for return codes like `DI_OK`, which is why a
    // workaround is allowed to change the return code.
    static const DWORD kPollReturnCode = static_cast<DWORD>(
        Globals::GetConfigurationData()[Strings::kStrConfigurationSectionWorkarounds]
                                       [Strings::kStrConfigurationSettingWorkaroundsPollReturnCode]
                                           .ValueOr(DI_NOEFFECT));

    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::SuperDebug;
    LOG_INVOCATION_AND_RETURN(kPollReturnCode, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::SendDeviceData(
          DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::SendForceFeedbackCommand(DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (false == controller->GetCapabilities().ForceFeedbackIsSupported())
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Application is attempting to send a force feedback command on Xidi virtual controller %u which does not support force feedback.",
          (1 + controller->GetIdentifier()));
      LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    Controller::ForceFeedback::Device* forceFeedbackDevice = AutoAcquireAndGetForceFeedbackDevice();
    if (nullptr == forceFeedbackDevice)
      LOG_INVOCATION_AND_RETURN(DIERR_NOTEXCLUSIVEACQUIRED, kMethodSeverity);

    switch (dwFlags)
    {
      case DISFFC_CONTINUE:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug, L"Sending force feedback command DISFFC_CONTINUE.");
        forceFeedbackDevice->SetPauseState(false);
        break;

      case DISFFC_PAUSE:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug, L"Sending force feedback command DISFFC_PAUSE.");
        forceFeedbackDevice->SetPauseState(true);
        break;

      case DISFFC_RESET:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug, L"Sending force feedback command DISFFC_RESET.");
        forceFeedbackDevice->Clear();
        break;

      case DISFFC_SETACTUATORSOFF:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug,
            L"Sending force feedback command DISFFC_SETACTUATORSOFF.");
        forceFeedbackDevice->SetMutedState(true);
        break;

      case DISFFC_SETACTUATORSON:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug,
            L"Sending force feedback command DISFFC_SETACTUATORSON.");
        forceFeedbackDevice->SetMutedState(false);
        break;

      case DISFFC_STOPALL:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug, L"Sending force feedback command DISFFC_STOPALL.");
        forceFeedbackDevice->StopAllEffects();
        break;

      default:
        Infra::Message::Output(
            Infra::Message::ESeverity::Debug, L"Sending force feedback command (unknown).");
        LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
    }

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    // The only piece of information Xidi needs from the cooperative level is whether shared or
    // exclusive mode is desired.
    if (0 != (dwFlags & DISCL_EXCLUSIVE))
      cooperativeLevel = ECooperativeLevel::Exclusive;
    else
      cooperativeLevel = ECooperativeLevel::Shared;

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::SetDataFormat(LPCDIDATAFORMAT lpdf)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (nullptr == lpdf) LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    // If this operation fails, then the current data format and event filter remain unaltered.
    std::unique_ptr<DataFormat> newDataFormat =
        DataFormat::CreateFromApplicationFormatSpec(*lpdf, controller->GetCapabilities());
    if (nullptr == newDataFormat) LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    // Use the event filter to prevent the controller from buffering any events that correspond to
    // elements with no offsets.
    auto lock = controller->Lock();
    controller->EventFilterAddAllElements();

    for (int i = 0; i < (int)Controller::EAxis::Count; ++i)
    {
      const Controller::SElementIdentifier element = {
          .type = Controller::EElementType::Axis, .axis = (Controller::EAxis)i};
      if (false == newDataFormat->HasElement(element))
        controller->EventFilterRemoveElement(element);
    }

    for (int i = 0; i < (int)Controller::EButton::Count; ++i)
    {
      const Controller::SElementIdentifier element = {
          .type = Controller::EElementType::Button, .button = (Controller::EButton)i};
      if (false == newDataFormat->HasElement(element))
        controller->EventFilterRemoveElement(element);
    }

    {
      const Controller::SElementIdentifier element = {.type = Controller::EElementType::Pov};
      if (false == newDataFormat->HasElement(element))
        controller->EventFilterRemoveElement(element);
    }

    dataFormat = std::move(newDataFormat);
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::SetEventNotification(HANDLE hEvent)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    if (INVALID_HANDLE_VALUE == hEvent)
      LOG_INVOCATION_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity);

    if ((NULL != hEvent) && (controller->HasStateChangeEventHandle()))
      LOG_INVOCATION_AND_RETURN(DIERR_HANDLEEXISTS, kMethodSeverity);

    controller->SetStateChangeEvent(hEvent);
    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    DumpPropertyRequest(rguidProp, pdiph, true);

    if (false == IsPropertyHeaderValid(rguidProp, pdiph))
      LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_INVALIDPARAM, kMethodSeverity, rguidProp);

    const std::optional<Controller::SElementIdentifier> maybeElement =
        IdentifyElement(pdiph->dwObj, pdiph->dwHow);
    if (false == maybeElement.has_value())
      LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_OBJECTNOTFOUND, kMethodSeverity, rguidProp);

    const Controller::SElementIdentifier element = maybeElement.value();

    switch ((size_t)&rguidProp)
    {
      case ((size_t)&DIPROP_AXISMODE):
        if (DIPROPAXISMODE_ABS == ((LPDIPROPDWORD)pdiph)->dwData)
          LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);
        else
          LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
              DIERR_UNSUPPORTED, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_AUTOCENTER):
        switch (((LPDIPROPDWORD)pdiph)->dwData)
        {
          case DIPROPAUTOCENTER_OFF:
          case DIPROPAUTOCENTER_ON:
            unusedProperties.autocenter = ((LPDIPROPDWORD)pdiph)->dwData;
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DI_OK, kMethodSeverity, rguidProp, pdiph);
          default:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
        }

      case ((size_t)&DIPROP_BUFFERSIZE):
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
            ((true == controller->SetEventBufferCapacity(((LPDIPROPDWORD)pdiph)->dwData))
                 ? DI_OK
                 : DIERR_INVALIDPARAM),
            kMethodSeverity,
            rguidProp,
            pdiph);

      case ((size_t)&DIPROP_CALIBRATIONMODE):
      {
        bool transformationsEnabled = true;

        switch (((LPDIPROPDWORD)pdiph)->dwData)
        {
          case DIPROPCALIBRATIONMODE_COOKED:
            transformationsEnabled = true;
            break;
          case DIPROPCALIBRATIONMODE_RAW:
            transformationsEnabled = false;
            break;
          default:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
        }

        switch (element.type)
        {
          case Controller::EElementType::Axis:
            controller->SetAxisTransformationsEnabled(element.axis, transformationsEnabled);
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DI_OK, kMethodSeverity, rguidProp, pdiph);
          case Controller::EElementType::WholeController:
            controller->SetAllAxisTransformationsEnabled(transformationsEnabled);
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DI_OK, kMethodSeverity, rguidProp, pdiph);
          default:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
        }
      }

      case ((size_t)&DIPROP_DEADZONE):
        switch (element.type)
        {
          case Controller::EElementType::Axis:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                ((true == controller->SetAxisDeadzone(element.axis, ((LPDIPROPDWORD)pdiph)->dwData))
                     ? DI_OK
                     : DIERR_INVALIDPARAM),
                kMethodSeverity,
                rguidProp,
                pdiph);
          case Controller::EElementType::WholeController:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                ((true == controller->SetAllAxisDeadzone(((LPDIPROPDWORD)pdiph)->dwData))
                     ? DI_OK
                     : DIERR_INVALIDPARAM),
                kMethodSeverity,
                rguidProp,
                pdiph);
          default:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
        }

      case ((size_t)&DIPROP_FFGAIN):
        LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
            ((true == controller->SetForceFeedbackGain(((LPDIPROPDWORD)pdiph)->dwData))
                 ? DI_OK
                 : DIERR_INVALIDPARAM),
            kMethodSeverity,
            rguidProp,
            pdiph);

      case ((size_t)&DIPROP_INSTANCENAME):
      case ((size_t)&DIPROP_PRODUCTNAME):
        // DirectInput API documentation for SetProperty says that these properties can be set even
        // if the values are not stored in a place retrievable by GetProperty. Xidi therefore
        // accepts them but does nothing with the value provided.
        LOG_PROPERTY_INVOCATION_DIPROPSTRING_AND_RETURN(DI_OK, kMethodSeverity, rguidProp, pdiph);

      case ((size_t)&DIPROP_RANGE):
        switch (element.type)
        {
          case Controller::EElementType::Axis:
            LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(
                ((true ==
                  controller->SetAxisRange(
                      element.axis, ((LPDIPROPRANGE)pdiph)->lMin, ((LPDIPROPRANGE)pdiph)->lMax))
                     ? DI_OK
                     : DIERR_INVALIDPARAM),
                kMethodSeverity,
                rguidProp,
                pdiph);
          case Controller::EElementType::WholeController:
            LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(
                ((true ==
                  controller->SetAllAxisRange(
                      ((LPDIPROPRANGE)pdiph)->lMin, ((LPDIPROPRANGE)pdiph)->lMax))
                     ? DI_OK
                     : DIERR_INVALIDPARAM),
                kMethodSeverity,
                rguidProp,
                pdiph);
          default:
            LOG_PROPERTY_INVOCATION_DIPROPRANGE_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
        }

      case ((size_t)&DIPROP_SATURATION):
        switch (element.type)
        {
          case Controller::EElementType::Axis:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                ((true ==
                  controller->SetAxisSaturation(element.axis, ((LPDIPROPDWORD)pdiph)->dwData))
                     ? DI_OK
                     : DIERR_INVALIDPARAM),
                kMethodSeverity,
                rguidProp,
                pdiph);
          case Controller::EElementType::WholeController:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                ((true == controller->SetAllAxisSaturation(((LPDIPROPDWORD)pdiph)->dwData))
                     ? DI_OK
                     : DIERR_INVALIDPARAM),
                kMethodSeverity,
                rguidProp,
                pdiph);
          default:
            LOG_PROPERTY_INVOCATION_DIPROPDWORD_AND_RETURN(
                DIERR_INVALIDPARAM, kMethodSeverity, rguidProp, pdiph);
        }

      default:
        LOG_PROPERTY_INVOCATION_NO_VALUE_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity, rguidProp);
    }
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::Unacquire(void)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;

    // The only possible state that would need to be undone when unacquiring a device is
    // relinquishing control over the physical device's force feedback buffer.
    controller->ForceFeedbackUnregister();

    LOG_INVOCATION_AND_RETURN(DI_OK, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion> HRESULT
      VirtualDirectInputDeviceBase<diVersion>::WriteEffectToFile(
          DirectInputTypes<diVersion>::ConstStringType lptszFileName,
          DWORD dwEntries,
          LPDIFILEEFFECT rgDiFileEft,
          DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIs8<diVersion>)
  HRESULT VirtualDirectInputDeviceVersion8Only<diVersion>::BuildActionMap(
      DirectInputTypes<diVersion>::ActionFormatType* lpdiaf,
      DirectInputTypes<diVersion>::ConstStringType lpszUserName,
      DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIs8<diVersion>)
  HRESULT VirtualDirectInputDeviceVersion8Only<diVersion>::GetImageInfo(
      DirectInputTypes<diVersion>::DeviceImageInfoHeaderType* lpdiDevImageInfoHeader)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template <EDirectInputVersion diVersion>
    requires (DirectInputVersionIs8<diVersion>)
  HRESULT VirtualDirectInputDeviceVersion8Only<diVersion>::SetActionMap(
      DirectInputTypes<diVersion>::ActionFormatType* lpdiActionFormat,
      DirectInputTypes<diVersion>::ConstStringType lptszUserName,
      DWORD dwFlags)
  {
    constexpr Infra::Message::ESeverity kMethodSeverity = Infra::Message::ESeverity::Info;
    LOG_INVOCATION_AND_RETURN(DIERR_UNSUPPORTED, kMethodSeverity);
  }

  template class VirtualDirectInputDeviceBase<EDirectInputVersion::k8A>;
  template class VirtualDirectInputDeviceBase<EDirectInputVersion::k8W>;
  template class VirtualDirectInputDeviceBase<EDirectInputVersion::kLegacyA>;
  template class VirtualDirectInputDeviceBase<EDirectInputVersion::kLegacyW>;
  template class VirtualDirectInputDeviceVersion8Only<EDirectInputVersion::k8A>;
  template class VirtualDirectInputDeviceVersion8Only<EDirectInputVersion::k8W>;
  template class VirtualDirectInputDeviceVersionLegacyOnly<EDirectInputVersion::kLegacyA>;
  template class VirtualDirectInputDeviceVersionLegacyOnly<EDirectInputVersion::kLegacyW>;
  template class VirtualDirectInputDevice<EDirectInputVersion::k8A>;
  template class VirtualDirectInputDevice<EDirectInputVersion::k8W>;
  template class VirtualDirectInputDevice<EDirectInputVersion::kLegacyA>;
  template class VirtualDirectInputDevice<EDirectInputVersion::kLegacyW>;
} // namespace Xidi
