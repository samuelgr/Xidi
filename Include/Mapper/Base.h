/*****************************************************************************
* XinputControllerDirectInput
*      Hook and helper for older DirectInput games.
*      Fixes issues associated with certain Xinput-based controllers.
*****************************************************************************
* Authored by Samuel Grossman
* Copyright (c) 2016
*****************************************************************************
* Mapper.h
*      Abstract base class for supported control mapping schemes.
*****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"


namespace XinputControllerDirectInput
{
    namespace Mapper
    {
        // Abstract base class representing a mapped controller to the application.
        // Subclasses define the button layout to present to the application and convert data received from a Controller to the format requested by the application.
        class Base
        {
        public:
            // -------- CLASS METHODS -------------------------------------------------- //

            //


            // -------- INSTANCE METHODS ----------------------------------------------- //

            //


            // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //

            // Parses an application-supplied DirectInput data format.
            // Return code will either be DI_OK (succeeded) or DIERR_INVALIDPARAM (failed due to an issue with the proposed data format).
            // Must be implemented by subclasses.
            virtual HRESULT ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf) = 0;
        };
    }
}
