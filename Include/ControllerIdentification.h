/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ControllerIdentification.h
 *      Declaration of helpers for identifying controller types.
 *****************************************************************************/

#pragma once

#include "API_Windows.h"


namespace XboxControllerDirectInput
{
    // Enumerates the known types of Xbox controllers.
    enum XboxControllerType
    {
        CONTROLLER_XBOX360,                     // Xbox 360 controller
        CONTROLLER_XBOXONE,                     // Xbox One controller
        
        CONTROLLER_NOTXBOX                      // Something else (non-Xbox)
    };
    
    // Encapsulates all constants and logic for identifying the controller type.
    // Methods are intended to be called directly rather than through an instance.
    class ControllerIdentification
    {
    private:
        // -------- CONSTANTS ------------------------------------------------------ //

            // GUID of the Xbox 360 controller.
        static const GUID guidXbox360Controller;

        // GUID of the Xbox One controller.
        static const GUID guidXboxOneController;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

            // Default constructor. Should never be invoked.
        ControllerIdentification();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Returns TRUE if the specified controller is an Xbox 360 controller.
        // Pass in a reference to its product GUID.
        static BOOL isXbox360Controller(const GUID& productGUID);

        // Returns TRUE if the specified controller is an Xbox One controller.
        // Pass in a reference to its product GUID.
        static BOOL isXboxOneController(const GUID& productGUID);

        // Identifies the type of controller based on its product GUID.
        static XboxControllerType xboxControllerType(const GUID& productGUID);
    };
}
