/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ControllerIdentification.h
 *      Declaration of helpers for identifying controller types.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


namespace XinputControllerDirectInput
{
    // Enumerates the known types of Xbox controllers.
    enum eControllerType
    {
        CONTROLLERTYPE_XBOX360,                 // Xbox 360 controller
        CONTROLLERTYPE_XBOXONE,                 // Xbox One controller
        
        CONTROLLERTYPE_UNKNOWN                  // Something unknown
    };
    
    // Encapsulates all constants and logic for identifying the controller type.
    // Methods are intended to be called directly rather than through an instance.
    class ControllerIdentification
    {
    private:
        // -------- CONSTANTS ------------------------------------------------------ //
        
        // GUID of the Xbox 360 controller.
        static const GUID kGuidXbox360Controller;
        
        // GUID of the Xbox One controller.
        static const GUID kGuidXboxOneController;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
        
        // Default constructor. Should never be invoked.
        ControllerIdentification();
        
        
    public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        // Returns TRUE if the specified controller is an Xbox 360 controller.
        // Pass in a reference to its product GUID.
        static BOOL IsXbox360Controller(REFGUID productGUID);
        
        // Returns TRUE if the specified controller is an Xbox One controller.
        // Pass in a reference to its product GUID.
        static BOOL IsXboxOneController(REFGUID productGUID);
        
        // Identifies the type of controller based on its product GUID.
        static eControllerType GetControllerType(REFGUID productGUID);
    };
}
