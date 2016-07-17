/*****************************************************************************
 * XInputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XInputObjectFactory.h
 *      Declaration of methods used to construct objects that interface
 *      with XInput-based controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"
#include "XInputControllerIdentification.h"

#include <unordered_map>


namespace XInputControllerDirectInput
{
    // Encapsulates all constants and logic for creating objects that interface with XInput-based controllers.
    // Objects created are presented through the IDirectInput8 interface with which an application can communicate.
    // As a result, each instance is associated with a particular IDirectInput8 interface.
    class XInputObjectFactory
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //
        std::unordered_map<const GUID, EControllerType> enumeratedControllers;
        
        
    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor.
        XInputObjectFactory();

        // Default destructor.
        ~XInputObjectFactory();

        
        // -------- INSTANCE METHODS ----------------------------------------------- //

        // Supplies a pointer to an object that complies with the IDirectInputDevice8 interface.
        // Requires an instance of IDirectInputDevice8 and the instance GUID of the controller with which to interface.
        // If the controller is of a known type and interception is supported, produces a wrapped interface of the correct type.
        // Otherwise does not encapsulate and simply creates and returns the underlyingController parameter.
        IDirectInputDevice8* CreateDirectInputDeviceForController(IDirectInputDevice8* underlyingController, REFGUID instanceGUID);
        
        // Initializes the internal data structures that track enumerated controllers.
        // Intended to be invoked when a DirectInput controller enumeration operation commences.
        void ResetEnumeratedControllers(void);

        // Submits an enumerated controller.
        // Intended to be invoked for each controller that is enumerated during a DirectInput controller enumeration.
        void SubmitEnumeratedController(REFGUID productGUID, REFGUID instanceGUID);
    };
}
