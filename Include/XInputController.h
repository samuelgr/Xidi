/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XInputController.h
 *      Declaration of a class that represents and interfaces with a single
 *      XInput-based controller and exposes a DirectInput-like interface.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"

#include <vector>
#include <Xinput.h>


namespace Xidi
{
    // Identifies each input component of an XInput-based controller.
    enum EXInputControllerElement
    {
        StickLeftHorizontal,
        StickLeftVertical,
        StickRightHorizontal,
        StickRightVertical,
        TriggerLT,
        TriggerRT,
        Dpad,
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

    // Represents a controller event.
    // Fields are based on DirectInput event format.
    struct SControllerEvent
    {
        EXInputControllerElement controllerElement; // Controller element identifier
        LONG value;                                 // Controller element value
        DWORD timestamp;                            // Event timestamp (system time at which the event occurred), in milliseconds
        DWORD sequenceNumber;                       // Monotonically increasing event sequence number, may wrap around
    };

    // Represents and interfaces with an XInput-based controller, providing a DirectInput-like interface.
    class XInputController
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

        // Mask for checking just the state of the dpad in an XINPUT_GAMEPAD structure.
        static const WORD kDpadStateMask = (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT);


    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        // Holds buffered events obtained from the controller.
        std::vector<SControllerEvent> bufferedEvents;
        
        // Specifies the next sequence number to use for reporting events in the buffer.
        DWORD bufferedEventsNextSequenceNumber;

        // Holds the current controller state, as of the last refresh operation.
        XINPUT_STATE controllerState;

        // Handle to an application-specified event to be notified when the device state changes.
        HANDLE controllerStateChangedEvent;

        // Enforces mutual exclusion on operations that update the internal event state.
        CRITICAL_SECTION eventChangeCriticalSection;
        
        // Specifies if the controller is "acquired" in DirectInput terms.
        // DirectInput requires controllers be acquired before applications can provide data from them.
        // Also, many property changes and other operations are unavailable once a controller is acquired.
        BOOL isAcquired;
        
        // User index of the controller with which this instance should interface.
        DWORD xinputUserIndex;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        XInputController();
        
    public:
        // Constructs a new controller object given the index (0 to 3) of the controller with which to interface.
        // If the value of the index is out of range, all method calls to the constructed object will fail.
        XInputController(DWORD xinputUserIndex);

        // Default destructor.
        ~XInputController();
        
        
        // -------- CLASS METHODS -------------------------------------------------- //

        // Given an XInput button reading, provides a DirectInput-style button reading (high bit of lowest byte is either set or not).
        static inline LONG DirectInputButtonStateFromXInputButtonReading(const WORD buttonState, const WORD buttonMask)
        {
            if (0 != (buttonState & buttonMask))
                return (LONG)0x0080;
            else
                return 0;
        }
        
        // Given an XInput button state, extracts the dpad state and converts to a DirectInput-style POV reading.
        static inline LONG DirectInputPovStateFromXInputButtonState(const WORD buttonState)
        {
            const WORD dpadState = buttonState & kDpadStateMask;
            LONG dpadValue = -1;

            // Report centered if either no positions pressed or all positions pressed.
            if (0 != dpadState && kDpadStateMask != dpadState)
            {
                LONG horizontalDpadComponent = 0;
                LONG verticalDpadComponent = 0;

                // Extract horizontal and vertical components (1 for up and right, -1 for down and left, 0 for center).
                // To detect a displacement, it is necessary that only one direction in each axis be pressed.
                if ((dpadState & XINPUT_GAMEPAD_DPAD_LEFT) && !(dpadState & XINPUT_GAMEPAD_DPAD_RIGHT))
                    horizontalDpadComponent = -1;
                else if ((dpadState & XINPUT_GAMEPAD_DPAD_RIGHT) && !(dpadState & XINPUT_GAMEPAD_DPAD_LEFT))
                    horizontalDpadComponent = 1;
                
                if ((dpadState & XINPUT_GAMEPAD_DPAD_DOWN) && !(dpadState & XINPUT_GAMEPAD_DPAD_UP))
                    verticalDpadComponent = -1;
                else if ((dpadState & XINPUT_GAMEPAD_DPAD_UP) && !(dpadState & XINPUT_GAMEPAD_DPAD_DOWN))
                    verticalDpadComponent = 1;

                // Convert to a reading in hundredths of degrees clockwise from north.
                switch (horizontalDpadComponent)
                {
                case -1:
                    if      (-1 == verticalDpadComponent) dpadValue = 22500;        // down and left
                    else if ( 0 == verticalDpadComponent) dpadValue = 27000;        // left
                    else if ( 1 == verticalDpadComponent) dpadValue = 31500;        // up and left
                    break;
                    
                case 0:
                    if      (-1 == verticalDpadComponent) dpadValue = 18000;        // down
                    else if ( 0 == verticalDpadComponent) dpadValue =    -1;        // centered
                    else if ( 1 == verticalDpadComponent) dpadValue =     0;        // up
                    break;
                    
                case 1:
                    if      (-1 == verticalDpadComponent) dpadValue = 13500;        // down and right
                    else if ( 0 == verticalDpadComponent) dpadValue =  9000;        // right
                    else if ( 1 == verticalDpadComponent) dpadValue =  4500;        // up and right
                    break;
                }
            }

            return dpadValue;
        }
        
        
    private:
        // -------- HELPERS -------------------------------------------------------- //

        // Clears the event buffer.
        void ClearBufferedEvents(void);
        
        // Submits a new event to the buffer. Simply enqueues it to the event buffer queue.
        void SubmitBufferedEvent(EXInputControllerElement controllerElement, LONG value, DWORD timestamp);


    public:
        // -------- INSTANCE METHODS ----------------------------------------------- //

        // Causes the device to enter an "acquired" state.
        // DirectInput requires that devices be acquired before data can be read.
        HRESULT AcquireController(void);

        // Fills in a DirectInput device capabilities structure with information about this controller's basic information.
        void FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps);
        
        // Retrieves buffered events from the controller and places them into the specified location.
        // May also remove the events from the buffer.
        // On input, count specifies the size of the events buffer.
        // On output, count specifies the number of events written to the buffer.
        HRESULT GetBufferedEvents(SControllerEvent* events, DWORD& count, BOOL removeFromBuffer);

        // Retrieves a DirectInput property on this controller.
        // Corresponds directly to IDirectInputDevice's GetProperty method.
        HRESULT GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);

        // Queries the controller for information on its current state (buttons, axes, etc.).
        // Places this information into the supplied device state structure.
        // Also clears out any buffered changes.
        HRESULT GetCurrentDeviceState(XINPUT_STATE* state);

        // Returns TRUE if this controller is currently acquired.
        BOOL IsAcquired(void);

        // Refreshes the controller state information.
        // Polls the controller for updated state.
        HRESULT RefreshControllerState(void);
        
        // Sets a DirectInput property on this controller.
        // Corresponds directly to IDirectInputDevice's SetProperty method.
        HRESULT SetControllerProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);

        // Sets the event to be notified if the controller's state changes.
        HRESULT SetControllerStateChangedEvent(HANDLE hEvent);

        // Causes the device to be removed from an "acquired" state.
        HRESULT UnacquireController(void);
    };
}
