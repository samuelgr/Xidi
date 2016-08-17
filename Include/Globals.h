/*****************************************************************************
* Xidi
*      DirectInput interface for XInput controllers.
*****************************************************************************
* Authored by Samuel Grossman
* Copyright (c) 2016
*****************************************************************************
* Globals.h
*      Declaration of a namespace for storing and retrieving global data.
*      Intended for miscellaneous data elements with no other suitable place.
*****************************************************************************/

#pragma once

#include "ApiWindows.h"


namespace Xidi
{
    // Encapsulates all miscellaneous global data elements with no other suitable location.
    // Not intended to be instantiated.
    class Globals
    {
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Handle of the instance that represents the running form of Xidi, be it the library or the test application.
        static HINSTANCE gInstanceHandle;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        Globals();


    public:
        // -------- CLASS METHODS (ACCESSORS) -------------------------------------- //

        // Retrieves the handle of the instance that represents the current running form of Xidi, be it the library or the test application.
        static HINSTANCE GetInstanceHandle(void);
        

        // -------- CLASS METHODS (MUTATORS) --------------------------------------- //
        
        // Sets the handle of the instance that represents the current running form of Xidi, be it the library or the test application.
        static void SetInstanceHandle(HINSTANCE newInstanceHandle);
    };
}
