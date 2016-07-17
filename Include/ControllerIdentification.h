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

#include "ApiDirectInput8.h"
#include "Hashers.h"

#include <unordered_map>


namespace Xidi
{
    // Encapsulates all constants and logic for identifying the controller type.
    // Methods are intended to be called directly rather than through an instance.
    class ControllerIdentification
    {
    public:
        // -------- CONSTANTS ------------------------------------------------------ //
        
        // Dummy product GUID for XInput controllers.
        static const GUID kXInputProductGUID;

        // Dummy instance GUID for XInput controllers, indexed by controller port.
        static const GUID kXInputInstGUID[4];
        
        
    private:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        ControllerIdentification();
        
        
    public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        // Returns TRUE if the specified DirectInput controller supports XInput, FALSE if not or this information could not be determined.
        static BOOL DoesDirectInputControllerSupportXInput(IDirectInput8* dicontext, REFGUID instanceGUID);

        // Performs a DirectInput8-style controller enumeration of connected XInput controllers.
        // Returns DIENUM_CONTINUE or DIENUM_STOP depending on what the application requested.
        static BOOL EnumerateXInputControllers(LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef);
    };
}
