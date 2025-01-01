/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ControllerIdentification.h
 *   Declaration of constants and functions for identifying and enumerating Xidi virtual
*    controllers in the context of DirectInput.
 **************************************************************************************************/

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "ApiDirectInput.h"
#include "ControllerTypes.h"

namespace Xidi
{
  /// Vendor ID for Xidi virtual controllers.
  inline constexpr uint16_t kVirtualControllerVendorId = 0xffff;

  /// Base GUID for Xidi virtual controllers from which product and instance GUIDs per controller
  /// may be derived by using the controller identifier.
  inline constexpr GUID kVirtualControllerBaseGuid = {
      ((0x0000 << 16) | static_cast<unsigned long>(kVirtualControllerVendorId)),
      0x0000,
      0x0000,
      {'X', 'I', 'D', 'I', 'V', 'C', 0x00, 0x00}};

  /// Force feedback driver GUID for Xidi virtual controllers.
  inline constexpr GUID kVirtualControllerForceFeedbackDriverGuid = {
      ((0xffff << 16) | static_cast<unsigned long>(kVirtualControllerVendorId)),
      0x0000,
      0x0000,
      {'X', 'I', 'D', 'I', 'F', 'F', 'D', 0x00}};

  /// HID collection number for the top-level virtual controller object.
  inline constexpr uint16_t kVirtualControllerHidCollectionForEntireDevice = 0;

  /// HID collection number for all elements within a virtual controller.
  inline constexpr uint16_t kVirtualControllerHidCollectionForIndividualElements = 1;

  /// Holds HID usage data, which consists of both a usage page and a usage.
  /// See https://www.usb.org/document-library/hid-usage-tables-13 for more information.
  struct SHidUsageData
  {
    uint16_t usagePage;
    uint16_t usage;

    constexpr bool operator==(const SHidUsageData& other) const = default;
  };

  /// Extracts and approximately compares the vendor and product IDs contained within two controller
  /// hardware identification strings. All comparisons are without regard for case. Product IDs must
  /// be identically equal, but vendor IDs can be considered approximately equal if they are
  /// considered similar enough. Whichever string has the shorter vendor ID must have its vendor ID
  /// be identically equal to the last characters of the other string's vendor ID. For example, if
  /// the two vendor IDs are "5E" and "045E" then that is considered a match because the longer
  /// string ends with the entire contents of the shorter string. On the other hand, "5D" and "045E"
  /// do not match, and neither do "04" and "045E" because the criterion above is not satisfied.
  /// @param [in] controllerStringA First string to compare.
  /// @param [in] controllerStringB Second string to compare.
  /// @return `true` if the two controller identification strings are approximately equal, `false`
  /// if they are not, and no value if one or both of the strings are missing either a product ID or
  /// a vendor ID.
  std::optional<bool> ApproximatelyEqualVendorAndProductId(
      std::wstring_view controllerStringA, std::wstring_view controllerStringB);

  /// Obtains the virtual controller element, if it exists, for the specified HID usage data.
  /// @param [in] hidUsageData HID usage data for which a virtual controller element is requested.
  /// @return Associated virtual controller element, if it exists based on the usage data specified.
  std::optional<Controller::SElementIdentifier> ControllerElementFromHidUsageData(
      SHidUsageData hidUsageData);

  /// Returns TRUE if the specified DirectInput controller supports XInput, FALSE if not or this
  /// information could not be determined. Determines if the specified DirectInput controller
  /// supports XInput. In so doing, an interface object is created to communicate with the
  /// controller. This interface object is released prior to returning from this method.
  /// @tparam EarliestIDirectInputType Either EarliestIDirectInputA or EarliestIDirectInputW
  /// depending on whether or not Unicode is being used.
  /// @tparam EarliestIDirectInputDeviceType Either EarliestIDirectInputDeviceA or
  /// EarliestIDirectInputDeviceW depending on whether or not Unicode is being used.
  /// @param [in] dicontext IDirectInput context from which the controller is to be obtained.
  /// @param [in] instanceGUID DirectInput instance GUID identifying the controller and obtained
  /// from the IDirectInput context.
  /// @param [out] devicePath If present, will be filled with the device identifying path, which was
  /// used to determine whether or not the controller supports XInput.
  /// @return `true` if the controller supports XInput, `false` otherwise.
  template <typename EarliestIDirectInputType, typename EarliestIDirectInputDeviceType> bool
      DoesDirectInputControllerSupportXInput(
          EarliestIDirectInputType* dicontext,
          REFGUID instanceGUID,
          std::wstring* devicePath = nullptr);

  /// Performs a DirectInput-style controller enumeration of Xidi virtual controllers.
  /// Callback parameter type was copied and pasted from DirectInput headers and modified to be
  /// amenable to being used in a template like this.
  /// @tparam DeviceInstanceType Either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether
  /// ASCII or Unicode is desired.
  /// @param [in] lpCallback Application-supplied callback function. Refer to DirectInput
  /// documentation for more.
  /// @param [in] pvRef Application-supplied parameter. Refer to DirectInput documentation for more.
  /// @param [in] forceFeedbackRequired If `true` then the enumeration is limited to virtual
  /// controllers that have mappers that support force feedback.
  /// @return `DIENUM_CONTINUE` or `DIENUM_STOP` depending on what the application's callback
  /// returned.
  template <typename DeviceInstanceType> BOOL EnumerateVirtualControllers(
      BOOL(FAR PASCAL* lpCallback)(const DeviceInstanceType*, LPVOID),
      LPVOID pvRef,
      bool forceFeedbackRequired);

  /// Generates and places a string representing the name of the specified HID collection.
  /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
  /// @param [out] buf Buffer to fill.
  /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
  /// @param [in] HID collection number.
  /// @return Number of characters written, or negative in the event of an error.
  template <typename StringType> int FillHidCollectionName(
      StringType buf, size_t bufcount, uint16_t hidCollectionNumber);

  /// Fills a DirectInput device information structure with information about the virtual controller
  /// at the specified index. On input, the size field is expected to be initialized. Since multiple
  /// structure versions exist, it is used to determine which members to fill in.
  /// @tparam DeviceInstanceType Either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether
  /// ASCII or Unicode is desired.
  /// @param [out] deviceInstanceInfo Structure to be filled with information.
  /// @param [in] controllerId Identifier of the controller for which information is to be filled
  /// in.
  template <typename DeviceInstanceType> void FillVirtualControllerInfo(
      DeviceInstanceType& instanceInfo, Controller::TControllerIdentifier controllerId);

  /// Generates and places a string representing the Xidi virtual controller's product or instance
  /// name for the controller at the specified index.
  /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
  /// @param [out] buf Buffer to fill.
  /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
  /// @param [in] controllerId Xidi virtual controller identifier, which is used to determine the
  /// actual text to produce.
  /// @return Number of characters written, or negative in the event of an error.
  template <typename StringType> int FillVirtualControllerName(
      StringType buf, size_t bufcount, Controller::TControllerIdentifier controllerId);

  /// Generates and places a string representing the Xidi virtual controller's underlying device
  /// path.
  /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
  /// @param [out] buf Buffer to fill.
  /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
  /// @param [in] controllerId Xidi virtual controller identifier, which is used to determine the
  /// actual text to produce.
  /// @return Number of characters written, or negative in the event of an error.
  template <typename StringType> int FillVirtualControllerPath(
      StringType buf, size_t bufcount, Controller::TControllerIdentifier controllerId);

  /// Obtains HID usage data for the specified controller element or, if the entire controller is
  /// requested, then obtains HID usage data for the virtual controller itself.
  /// @param [in] element Identifier of the virtual controller element of interest.
  /// @return HID data for the specified virtual controller element.
  SHidUsageData HidUsageDataForControllerElement(Controller::SElementIdentifier element);

  /// Retrieves the Xidi virtual controller index of the specified instance GUID.
  /// @param [in] instanceGUID Xidi virtual controller instance GUID.
  /// @return Xidi virtual controller identifier from the specified GUID, assuming said GUID is
  /// actually a Xidi virtual controller instance GUID.
  std::optional<Controller::TControllerIdentifier> VirtualControllerIdFromInstanceGuid(
      REFGUID instanceGUID);

  /// Retrieves the 16-bit product identifier for a Xidi virtual controller.
  /// @param [in] controllerId Xidi virtual controller identifier.
  /// @return Product identifier for all virtual controllers.
  constexpr uint16_t VirtualControllerProductId(Controller::TControllerIdentifier controllerId)
  {
    return ((uint16_t)1 + controllerId);
  }

  /// Generates a class GUID for a Xidi virtual controller.
  /// All Xidi virtual controllers use the same class GUID.
  /// @return GUID for identifying the device class of Xidi virtual controllers.
  constexpr GUID VirtualControllerClassGuid(void)
  {
    constexpr GUID kHidClassGuid = {
        0x745a17a0, 0x74d3, 0x11d0, {0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda}};
    return kHidClassGuid;
  }

  /// Generates a product or instance GUID for a Xidi virtual controller of the specified index.
  /// @param [in] controllerId Xidi virtual controller identifier.
  /// @return Instance GUID for a Xidi virtual controller of the specified index.
  constexpr GUID VirtualControllerGuid(Controller::TControllerIdentifier controllerId)
  {
    GUID xguid = kVirtualControllerBaseGuid;
    xguid.Data1 |= ((LONG)VirtualControllerProductId(controllerId)) << 16l;
    return xguid;
  }
} // namespace Xidi
