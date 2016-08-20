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

#include <deque>
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

        // Maximum number of XInput controllers that can be plugged into the system.
        // Valid user indices range from 0 to this number.
        static const WORD kMaxNumXInputControllers = 4;


    private:
        // -------- INSTANCE VARIABLES --------------------------------------------- //

        // Holds buffered events obtained from the controller.
        std::deque<SControllerEvent> bufferedEvents;
        
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
        static LONG DirectInputButtonStateFromXInputButtonReading(const WORD buttonState, const WORD buttonMask);
        
        // Given an XInput button state, extracts the dpad state and converts to a DirectInput-style POV reading.
        static LONG DirectInputPovStateFromXInputButtonState(const WORD buttonState);
        
        // Returns TRUE if the specified XInput controller is connected (i.e. there is a controller physically present for the specified device index).
        static BOOL IsControllerConnected(const DWORD xinputUserIndex);
        
        
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

        // Retrieves the number of buffered events present.
        DWORD BufferedEventsCount();
        
        // Fills in a DirectInput device capabilities structure with information about this controller's basic information.
        void FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps);

        // Retrieves a DirectInput property on this controller.
        // Corresponds directly to IDirectInputDevice's GetProperty method.
        HRESULT GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);

        // Queries the controller for information on its current state (buttons, axes, etc.).
        // Places this information into the supplied device state structure.
        // Also clears out any buffered changes.
        HRESULT GetCurrentDeviceState(XINPUT_STATE* state);

        // Returns TRUE if this controller is currently acquired.
        BOOL IsAcquired(void);

        // Returns TRUE if this controller is currently connected (i.e. there is a controller physically present for the assigned device index).
        BOOL IsConnected(void);

		// Locks the event buffer for multiple operations.
		// Idempotent; can be called multiple times from the same thread, so long as each call has an accompanying call to UnlockEventBuffer.
		void LockEventBuffer(void);
		
		// Retrieves the specified buffered event and places it into the specified location.
		// Does not remove the event from the buffer.
		HRESULT PeekBufferedEvent(SControllerEvent* event, DWORD idx);
		
		// Retrieves the first (oldest) buffered event from the controller and places it into the specified location.
		// Removes the event from the buffer.
		HRESULT PopBufferedEvent(SControllerEvent* event);
		
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

		// Unlocks the event buffer after multiple operations have completed.
		void UnlockEventBuffer(void);
    };
}
