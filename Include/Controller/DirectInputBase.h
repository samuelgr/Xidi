/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DirectInputBase.h
 *      Abstract base class for all implementations that communicate with
 *      Xinput-based controllers via DirectInput.
 *      Provides common implementations of most core functionality.
 *****************************************************************************/

#pragma once

#include "Controller/Base.h"


namespace XinputControllerDirectInput
{
    namespace Controller
    {
        // Provides a common implementation for communicating with Xinput-based controllers using DirectInput.
        // Subclasses are required to provide controller-specific functionality, each representing a specific supported controller.
        class DirectInputBase : public Base
        {
        private:
            // -------- INSTANCE VARIABLES --------------------------------------------- //

            // The IDirectInputDevice8 object that represents the underlying controller.
            IDirectInputDevice8* underlyingDIObject;


        public:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

            // Constructs a DirectInput-based controller given an underlying IDirectInputDevice8 interface pointer.
            DirectInputBase(IDirectInputDevice8* underlyingDIObject);


        private:
            // Default constructor. Should never be invoked.
            DirectInputBase();
            
            
        public:
            // -------- CONCRETE INSTANCE METHODS -------------------------------------- //
            // See "Controller/Base.h" for documentation.
            
            virtual HRESULT AcquireController(void);
            virtual HRESULT GetBufferedEvents(SControllerEvent* events, DWORD count, BOOL removeFromBuffer);
            virtual HRESULT GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);
            virtual HRESULT GetCurrentDeviceState(SControllerState* state);
            virtual HRESULT SetControllerProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);
            virtual HRESULT UnacquireController(void);
        };
    }
}
