/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ControllerIdentification.h
 *   Declaration of helpers for identifying and enumerating
 *   XInput-based game controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "XInputController.h"

#include <string>


namespace Xidi
{
    /// Encapsulates all constants and logic for identifying the controller type.
    /// Methods are intended to be called directly rather than through an instance.
    class ControllerIdentification
    {
    private:
        // -------- CONSTANTS ---------------------------------------------- //
        
        /// Dummy product GUID for XInput controllers.
        static const GUID kXInputProductGUID;
        
        /// Dummy instance base GUID for XInput controllers, from which instance GUIDs per controller may be derived.
        static const GUID kXInputBaseInstGUID;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //
        
        /// Default constructor. Should never be invoked.
        ControllerIdentification(void);
        
        
        // -------- HELPERS ------------------------------------------------ //

        /// Extracts and returns the instance index from an XInput controller's GUID.
        /// Does not verify that the supplied GUID actually represents an XInput instance GUID.
        /// @param [in] xguid XInput controller's instance GUID.
        /// @return Instance index (a.k.a. XInput player number).
        static WORD ExtractInstanceFromXInputInstanceGUID(REFGUID xguid);
        
        /// Turns the provided base instance GUID for an XInput controller into an instance GUID for a controller of the specified index.
        /// Does not verify that the supplied GUID actually represents an XInput instance GUID.
        /// @param [in,out] xguid GUID whose XInput instance field should be set.
        /// @param [in] xindex Instance index (a.k.a. XInput player number).
        static void SetInstanceInXInputInstanceGUID(GUID& xguid, const WORD xindex);
        
        
    public:
        // -------- CLASS METHODS ------------------------------------------ //
        
        // Returns TRUE if the specified DirectInput controller supports XInput, FALSE if not or this information could not be determined.
        /// Determines if the specified DirectInput controller supports XInput.
        /// In so doing, an interface object is created to communicate with the controller.
        /// This interface object is released prior to returning from this method.
        /// @param [in] dicontext IDirectInput context from which the controller is to be obtained.
        /// @param [in] instanceGUID DirectInput instance GUID identifying the controller and obtained from the IDirectInput context.
        /// @param [out] devicePath If present, will be filled with the device identifying path, which was used to determine whether or not the controller supports XInput.
        /// @return `TRUE` if the controller supports XInput, `FALSE` otherwise.
        static BOOL DoesDirectInputControllerSupportXInput(EarliestIDirectInput* dicontext, REFGUID instanceGUID, std::wstring* devicePath = NULL);

        /// Performs a DirectInput-style controller enumeration of connected XInput controllers.
        /// This is the non-Unicode version.
        /// @param [in] lpCallback Application-supplied callback function. Refer to DirectInput documentation for more.
        /// @param [in] pvRef Application-supplied parameter. Refer to DirectInput documentation for more.
        /// @return `DIENUM_CONTINUE` or `DIENUM_STOP` depending on what the application's callback returned.
        static BOOL EnumerateXInputControllersA(LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef);
        
        /// Performs a DirectInput-style controller enumeration of connected XInput controllers.
        /// This is the Unicode version.
        /// @param [in] lpCallback Application-supplied callback function. Refer to DirectInput documentation for more.
        /// @param [in] pvRef Application-supplied parameter. Refer to DirectInput documentation for more.
        /// @return `DIENUM_CONTINUE` or `DIENUM_STOP` depending on what the application's callback returned.
        static BOOL EnumerateXInputControllersW(LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef);
        
        /// Generates and places a string representing the XInput controller's product name for the controller at the specified index.
        /// This is the non-Unicode version.
        /// @param [out] buf Buffer to fill.
        /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
        /// @param [in] controllerIndex XInput controller index, which is used to determine the actual text to produce.
        /// @return Number of characters written, or negative in the event of an error.
        static int FillXInputControllerNameA(LPSTR buf, const size_t bufcount, const DWORD controllerIndex);
        
        /// Generates and places a string representing the XInput controller's product name for the controller at the specified index.
        /// This is the Unicode version.
        /// @param [out] buf Buffer to fill.
        /// @param [in] bufcount Buffer size, expressed in terms of number of characters.
        /// @param [in] controllerIndex XInput controller index, which is used to determine the actual text to produce.
        /// @return Number of characters written, or negative in the event of an error.
        static int FillXInputControllerNameW(LPWSTR buf, const size_t bufcount, const DWORD controllerIndex);

        /// Writes the XInput product GUID to the specified GUID.
        /// @param [out] xguid GUID to fill.
        static void GetProductGUID(GUID& xguid);
        
        /// Generates an instance GUID for an XInput controller of the specified index and places it into the supplied GUID.
        /// @param [out] xguid GUID to fill.
        /// @param [in] xindex XInput controller index.
        static void MakeInstanceGUID(GUID& xguid, const WORD xindex);
        
        /// Retrieves the XInput controller index of the specified instance GUID.
        /// @param [in] instanceGUID XInput instance GUID.
        /// @return Either the XInput controller index of the specified GUID or -1 if the instance GUID provided does not correspond to a valid XInput controller.
        static LONG XInputControllerIndexForInstanceGUID(REFGUID instanceGUID);
    };
}
