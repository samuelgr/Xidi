/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ControllerIdentification.h
 *      Declaration of helpers for identifying and enumerating
 *      XInput-based game controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "XInputController.h"

#include <unordered_map>


namespace Xidi
{
    // Encapsulates all constants and logic for identifying the controller type.
    // Methods are intended to be called directly rather than through an instance.
    class ControllerIdentification
    {
    private:
        // -------- CONSTANTS ------------------------------------------------------ //
        
        // Dummy product GUID for XInput controllers.
        static const GUID kXInputProductGUID;
        
        // Dummy instance base GUID for XInput controllers, from which instance GUIDs per controller may be derived.
        static const GUID kXInputBaseInstGUID;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        ControllerIdentification();
        
        
        // -------- HELPERS -------------------------------------------------------- //

        // Extracts and returns the instance index from an XInput controller's GUID.
        // Does not verify that the supplied GUID actually represents an XInput instance GUID.
        static WORD ExtractInstanceFromXInputInstanceGUID(REFGUID xguid);
        
        // Turns the provided base instance GUID for an XInput controller into an instance GUID for a controller of the specified index.
        // Does not verify that the supplied GUID actually represents an XInput instance GUID.
        static void SetInstanceInXInputInstanceGUID(GUID& xguid, const WORD xindex);
        
        
    public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        // Returns TRUE if the specified DirectInput controller supports XInput, FALSE if not or this information could not be determined.
        static BOOL DoesDirectInputControllerSupportXInput(EarliestIDirectInput* dicontext, REFGUID instanceGUID);

        // Performs a DirectInput-style controller enumeration of connected XInput controllers.
        // Returns DIENUM_CONTINUE or DIENUM_STOP depending on what the application requested.
        // This is the non-Unicode version.
        static BOOL EnumerateXInputControllersA(LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef);
        
        // Performs a DirectInput-style controller enumeration of connected XInput controllers.
        // Returns DIENUM_CONTINUE or DIENUM_STOP depending on what the application requested.
        // This is the Unicode version.
        static BOOL EnumerateXInputControllersW(LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef);
        
        // Generates and places a string representing the XInput controller's product name for the controller at the specified index.
        // Returns the number of characters written, or negative in the event of an error.
        // This is the non-Unicode version.
        static int FillXInputControllerNameA(LPSTR buf, const size_t bufcount, const DWORD controllerIndex);
        
        // Generates and places a string representing the XInput controller's product name for the controller at the specified index.
        // Returns the number of characters written, or negative in the event of an error.
        // This is the Unicode version.
        static int FillXInputControllerNameW(LPWSTR buf, const size_t bufcount, const DWORD controllerIndex);

        // Writes the XInput product GUID to the specified GUID.
        static void GetProductGUID(GUID& xguid);
        
        // Generates an instance GUID for an XInput controller of the specified index and places it into the supplied GUID.
        static void MakeInstanceGUID(GUID& xguid, const WORD xindex);
        
        // Retrieves the XInput controller index of the specified instance GUID.
        // If the instance GUID does not correspond to a valid XInput controller, returns -1.
        static LONG XInputControllerIndexForInstanceGUID(REFGUID instanceGUID);
    };
}
