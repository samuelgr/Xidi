/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ControllerIdentification.h
 *   Declaration of constants and functions for identifying and enumerating
 *   XInput-based game controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"

#include <string>


namespace Xidi
{
    namespace ControllerIdentification
    {
        // -------- CONSTANTS ---------------------------------------------- //

        /// Dummy product GUID for XInput controllers.
        inline constexpr GUID kXInputProductGUID = { 0xffffffff, 0x0000, 0x0000, { 0x00, 0x00, 'X', 'I', 'N', 'P', 'U', 'T' } };

        /// Dummy instance base GUID for XInput controllers, from which instance GUIDs per controller may be derived.
        inline constexpr GUID kXInputBaseInstGUID = { 0xffffffff, 0x0000, 0x0000, { 'X', 'I', 'N', 'P', 'U', 'T', 0x00, 0x00 } };


        // -------- FUNCTIONS ---------------------------------------------- //

        // Returns TRUE if the specified DirectInput controller supports XInput, FALSE if not or this information could not be determined.
        /// Determines if the specified DirectInput controller supports XInput.
        /// In so doing, an interface object is created to communicate with the controller.
        /// This interface object is released prior to returning from this method.
        /// @tparam EarliestIDirectInputType Either EarliestIDirectInputA or EarliestIDirectInputW depending on whether or not Unicode is being used.
        /// @tparam EarliestIDirectInputDeviceType Either EarliestIDirectInputDeviceA or EarliestIDirectInputDeviceW depending on whether or not Unicode is being used.
        /// @param [in] dicontext IDirectInput context from which the controller is to be obtained.
        /// @param [in] instanceGUID DirectInput instance GUID identifying the controller and obtained from the IDirectInput context.
        /// @param [out] devicePath If present, will be filled with the device identifying path, which was used to determine whether or not the controller supports XInput.
        /// @return `TRUE` if the controller supports XInput, `FALSE` otherwise.
        template <typename EarliestIDirectInputType, typename EarliestIDirectInputDeviceType> BOOL DoesDirectInputControllerSupportXInput(EarliestIDirectInputType* dicontext, REFGUID instanceGUID, std::wstring* devicePath = nullptr);

        /// Performs a DirectInput-style controller enumeration of connected XInput controllers.
        /// Callback parameter type was copied and pasted from DirectInput headers and modified to be amenable to being used in a template like this.
        /// @tparam DeviceInstanceType Either DIDEVICEINSTANCEA or DIDEVICEINSTANCEW depending on whether ASCII or Unicode is desired.
        /// @param [in] lpCallback Application-supplied callback function. Refer to DirectInput documentation for more.
        /// @param [in] pvRef Application-supplied parameter. Refer to DirectInput documentation for more.
        /// @return `DIENUM_CONTINUE` or `DIENUM_STOP` depending on what the application's callback returned.
        template <typename DeviceInstanceType> BOOL EnumerateXInputControllers(BOOL(FAR PASCAL* lpCallback)(const DeviceInstanceType*, LPVOID), LPVOID pvRef);

        /// Generates and places a string representing the XInput controller's product name for the controller at the specified index.
        /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
        /// @param [out] buf Buffer to fill.
        /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
        /// @param [in] controllerIndex XInput controller index, which is used to determine the actual text to produce.
        /// @return Number of characters written, or negative in the event of an error.
        template <typename StringType> int FillXInputControllerName(StringType buf, const size_t bufcount, const DWORD controllerIndex);

        /// Generates an instance GUID for an XInput controller of the specified index and places it into the supplied GUID.
        /// @param [out] xguid GUID to fill.
        /// @param [in] xindex XInput controller index.
        void MakeInstanceGUID(GUID& xguid, const WORD xindex);

        /// Retrieves the XInput controller index of the specified instance GUID.
        /// @param [in] instanceGUID XInput instance GUID.
        /// @return Either the XInput controller index of the specified GUID or -1 if the instance GUID provided does not correspond to a valid XInput controller.
        LONG XInputControllerIndexForInstanceGUID(REFGUID instanceGUID);
    }
}
