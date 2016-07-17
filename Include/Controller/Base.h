/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Base.h
 *      Abstract base class for supported XInput-based controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"

#include <Xinput.h>


namespace Xidi
{
    namespace Controller {
        // Identifies each input component of an XInput-based controller.
        enum EControllerElement
        {
            StickLeft,
            StickRight,
            Dpad,
            TriggerLT,
            TriggerRT,
            ButtonA,
            ButtonB,
            ButtonX,
            ButtonY,
            ButtonLB,
            ButtonRB,
            ButtonBack,
            ButtonStart,
            ButtonLeftStick,
            ButtonRightStick
        };
        
        // Represents the state of an XInput-based controller.
        // State information will be filled into structures of this type for use by application mappers.
        // For non-binary values (sticks, triggers, and dpad) the value reported is as read from the controller.
        // For binary values, the value is 0x80 (upper bit set) if pressed or 0x00 (no bits set) if not.
        struct SControllerState
        {
            LONG stickLeftX;                        // Horizontal position of the left stick
            LONG stickLeftY;                        // Vertical position of the left stick
            LONG stickRightX;                       // Horizontal position of the right stick
            LONG stickRightY;                       // Vertical position of the right stick

            LONG dpad;                              // DPAD position reading

            LONG triggerLeft;                       // Trigger LT
            LONG triggerRight;                      // Trigger RT

            BYTE buttonA;                           // A button
            BYTE buttonB;                           // B button
            BYTE buttonX;                           // X button
            BYTE buttonY;                           // Y button
            BYTE buttonLB;                          // LB button
            BYTE buttonRB;                          // RB button
            BYTE buttonBack;                        // Back button
            BYTE buttonStart;                       // Start button
            BYTE buttonLS;                          // LS button (pushing down on the left stick)
            BYTE buttonRS;                          // RS button (pushing down on the right stick)
        };

        // Represents a controller event.
        // Fields are based on DirectInput event format.
        struct SControllerEvent
        {
            EControllerElement controllerElement;   // Controller element identifier
            DWORD value;                            // Controller element value
            DWORD timestamp;                        // Event timestamp (system time at which the event occurred), in milliseconds
            DWORD sequenceNumber;                   // Monotonically increasing event sequence number, may wrap around
        };

        // Abstract base class representing a supported hardware controller.
        // Subclasses implement much of the controller-specific functionality.
        class Base
        {
        public:
            // -------- CONSTANTS ------------------------------------------------------ //
            
            // Minimum value of readings from the left and right sticks, from the XInput documentation.
            static const LONG kStickRangeMin = -32768;
            
            // Maximum value of readings from the left and right sticks, from the XInput documentation.
            static const LONG kStickRangeMax = 32767;
            
            // Neutral position value for the left and right sticks, from the XInput documentation.
            static const LONG kStickNeutral = 0;
            
            // Minimum value of readings from the LT and RT triggers, from the XInput documentation.
            static const LONG kTriggerRangeMin = 0;
            
            // Maximum value of readings from the LT and RT triggers, from the XInput documentation.
            static const LONG kTriggerRangeMax = 255;
            
            // Neutral position value for the LT and RT triggers, from the XInput documentation.
            static const LONG kTriggerNeutral = 0;
            
            
        public:
            // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //
            
            // Causes the device to enter an "acquired" state.
            // DirectInput requires that devices be acquired before data can be read.
            // Must be implemented by subclasses.
            virtual HRESULT AcquireController(void) = 0;
            
            // Retrieves buffered events from the controller and places them into the specified location.
            // May also remove the events from the buffer.
            // Must be implemented by subclasses.
            virtual HRESULT GetBufferedEvents(SControllerEvent* events, DWORD count, BOOL removeFromBuffer) = 0;
            
            // Retrieves a DirectInput property on this controller.
            // Corresponds directly to IDirectInputDevice8's GetProperty method.
            // Must be implemented by subclasses.
            virtual HRESULT GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph) = 0;
            
            // Queries the controller for information on its current state (buttons, axes, etc.).
            // Places this information into the supplied device state structure.
            // Also clears out any buffered changes.
            // Must be implemented by subclasses.
            virtual HRESULT GetCurrentDeviceState(SControllerState* state) = 0;

            // Sets a DirectInput property on this controller.
            // Corresponds directly to IDirectInputDevice8's SetProperty method.
            // Must be implemented by subclasses.
            virtual HRESULT SetControllerProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph) = 0;

            // Causes the device to be removed from an "acquired" state.
            // Must be implemented by subclasses.
            virtual HRESULT UnacquireController(void) = 0;
        };
    }
}
