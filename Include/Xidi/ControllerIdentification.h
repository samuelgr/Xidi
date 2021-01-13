/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ControllerIdentification.h
 *   Declaration of constants and functions for identifying and enumerating
 *   Xidi virtual controllers in the context of DirectInput.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"

#include <optional>
#include <string>


namespace Xidi
{
    // -------- CONSTANTS -------------------------------------------------- //

    /// Product GUID for Xidi virtual controllers.
    inline constexpr GUID kVirtualControllerProductGuid = { 0xffffffff, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 'X', 'I', 'D', 'I' } };

    /// Instance base GUID for Xidi virtual controllers from which instance GUIDs per controller may be derived by using the controller identifier..
    inline constexpr GUID kVirtualControllerInstanceBaseGuid = { 0xffffffff, 0x0000, 0x0000, { 'X', 'I', 'D', 'I', 0x00, 0x00, 0x00, 0x00 } };


    // -------- FUNCTIONS -------------------------------------------------- //

    // Returns TRUE if the specified DirectInput controller supports XInput, FALSE if not or this information could not be determined.
    /// Determines if the specified DirectInput controller supports XInput.
    /// In so doing, an interface object is created to communicate with the controller.
    /// This interface object is released prior to returning from this method.
    /// @tparam EarliestIDirectInputType Either EarliestIDirectInputA or EarliestIDirectInputW depending on whether or not Unicode is being used.
    /// @tparam EarliestIDirectInputDeviceType Either EarliestIDirectInputDeviceA or EarliestIDirectInputDeviceW depending on whether or not Unicode is being used.
    /// @param [in] dicontext IDirectInput context from which the controller is to be obtained.
    /// @param [in] instanceGUID DirectInput instance GUID identifying the controller and obtained from the IDirectInput context.
    /// @param [out] devicePath If present, will be filled with the device identifying path, which was used to determine whether or not the controller supports XInput.
    /// @return `true` if the controller supports XInput, `false` otherwise.
    template <typename EarliestIDirectInputType, typename EarliestIDirectInputDeviceType> bool DoesDirectInputControllerSupportXInput(EarliestIDirectInputType* dicontext, REFGUID instanceGUID, std::wstring* devicePath = nullptr);

    /// Performs a DirectInput-style controller enumeration of Xidi virtual controllers.
    /// Callback parameter type was copied and pasted from DirectInput headers and modified to be amenable to being used in a template like this.
    /// @tparam DeviceInstanceType Either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether ASCII or Unicode is desired.
    /// @param [in] lpCallback Application-supplied callback function. Refer to DirectInput documentation for more.
    /// @param [in] pvRef Application-supplied parameter. Refer to DirectInput documentation for more.
    /// @return `DIENUM_CONTINUE` or `DIENUM_STOP` depending on what the application's callback returned.
    template <typename DeviceInstanceType> BOOL EnumerateVirtualControllers(BOOL(FAR PASCAL* lpCallback)(const DeviceInstanceType*, LPVOID), LPVOID pvRef);

    /// Fills a DirectInput device information structure with information about the virtual controller at the specified index.
    /// On input, the size field is expected to be initialized. Since multiple structure versions exist, it is used to determine which members to fill in.
    /// @tparam DeviceInstanceType Either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether ASCII or Unicode is desired.
    /// @param [out] deviceInstanceInfo Structure to be filled with information.
    /// @param [in] controllerId Identifier of the controller for which information is to be filled in.
    template <typename DeviceInstanceType> void FillVirtualControllerInfo(DeviceInstanceType& instanceInfo, DWORD controllerId);
    
    /// Generates and places a string representing the Xidi virtual controller's product name for the controller at the specified index.
    /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
    /// @param [out] buf Buffer to fill.
    /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
    /// @param [in] controllerId Xidi virtual controller identifier, which is used to determine the actual text to produce.
    /// @return Number of characters written, or negative in the event of an error.
    template <typename StringType> int FillVirtualControllerName(StringType buf, size_t bufcount, DWORD controllerId);

    /// Generates an instance GUID for an Xidi virtual controller of the specified index and places it into the supplied GUID.
    /// @param [out] xguid GUID to fill.
    /// @param [in] controllerId Xidi virtual controller identifier.
    void MakeVirtualControllerInstanceGuid(GUID& xguid, DWORD controllerId);

    /// Retrieves the Xidi virtual controller index of the specified instance GUID.
    /// @param [in] instanceGUID Xidi virtual controller instance GUID.
    /// @return Xidi virtual controller identifier from the specified GUID, assuming said GUID is actually a Xidi virtual controller instance GUID.
    std::optional<DWORD> VirtualControllerIdFromInstanceGuid(REFGUID instanceGUID);
}
