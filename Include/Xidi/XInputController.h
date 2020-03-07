/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file XInputController.h
 *   Declaration of a class that represents and interfaces with a single
 *   XInput-based controller and exposes a DirectInput-like interface.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"

#include <deque>
#include <Xinput.h>


namespace Xidi
{
    /// Identifies each input component of an XInput-based controller.
    enum EXInputControllerElement
    {
        StickLeftHorizontal,                                                ///< Left stick, horizontal axis.
        StickLeftVertical,                                                  ///< Left stick, vertical axis.
        StickRightHorizontal,                                               ///< Right stick, horizontal axis.
        StickRightVertical,                                                 ///< Right stick, vertical axis.
        TriggerLT,                                                          ///< Left trigger (LT).
        TriggerRT,                                                          ///< Right trigger (RT).
        Dpad,                                                               ///< Directional pad, also known as a point-of-view controller.
        ButtonA,                                                            ///< A button.
        ButtonB,                                                            ///< B button.
        ButtonX,                                                            ///< X button.
        ButtonY,                                                            ///< Y button.
        ButtonLB,                                                           ///< Left shoulder button (LB).
        ButtonRB,                                                           ///< Right shoulder button (RB).
        ButtonBack,                                                         ///< Back button.
        ButtonStart,                                                        ///< Start button.
        ButtonLeftStick,                                                    ///< Left stick button (pressing directly into the controller on the left stick).
        ButtonRightStick                                                    ///< Right stick button (pressing directly into the controller on the right stick).
    };

    /// Represents a controller event.
    /// Fields are based on DirectInput event format.
    struct SControllerEvent
    {
        EXInputControllerElement controllerElement;                         ///< Controller element identifier.
        LONG value;                                                         ///< Controller element value.
        DWORD timestamp;                                                    ///< Event timestamp (system time at which the event occurred), in milliseconds.
        DWORD sequenceNumber;                                               ///< Monotonically increasing event sequence number.
    };

    /// Represents and interfaces with an XInput-based controller, providing a DirectInput-like interface.
    class XInputController
    {
    public:
        // -------- CONSTANTS ---------------------------------------------- //

        /// Maximum size of the event buffer, in number of elements.
        /// Relevant only for applications that read buffered data.
        /// Value is set to reserve 1MB at most per controller, which holds tens of thousands of events.
        static const DWORD kEventBufferCountMax = (1 * 1024 * 1024) / sizeof(SControllerEvent);
        
        /// Minimum value of readings from the left and right sticks, from the XInput documentation.
        static const LONG kStickRangeMin = -32768;

        /// Maximum value of readings from the left and right sticks, from the XInput documentation.
        static const LONG kStickRangeMax = 32767;

        /// Neutral position value for the left and right sticks, from the XInput documentation.
        static const LONG kStickNeutral = 0;

        /// Minimum value of readings from the LT and RT triggers, from the XInput documentation.
        static const LONG kTriggerRangeMin = 0;

        /// Maximum value of readings from the LT and RT triggers, from the XInput documentation.
        static const LONG kTriggerRangeMax = 255;

        /// Neutral position value for the LT and RT triggers, from the XInput documentation.
        static const LONG kTriggerNeutral = 0;

        /// Mask for checking just the state of the dpad in an XINPUT_GAMEPAD structure.
        static const WORD kDpadStateMask = (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT);

        /// Maximum number of XInput controllers.
        /// Valid user indices range from 0 to this number.
        static const WORD kMaxNumXInputControllers = XUSER_MAX_COUNT;


    private:
        // -------- INSTANCE VARIABLES ------------------------------------- //

        /// Holds buffered events obtained from the controller.
        std::deque<SControllerEvent> bufferedEvents;
        
        /// Specifies the next sequence number to use for reporting events in the buffer.
        DWORD bufferedEventsNextSequenceNumber;

        /// Holds the current controller state, as of the last refresh operation.
        XINPUT_STATE controllerState;

        /// Handle to an application-specified event to be notified when the device state changes.
        HANDLE controllerStateChangedEvent;

        /// Requested input buffer size, in bytes. Defaults to 0 (buffered events disabled).
        /// Kept separately from the actual input buffer size because the requested size is always returned whenever the buffer size property is queried, per DirectInput spec.
        DWORD eventBufferSizeRequested;

        /// Actual input buffer size, in number of elements. Defaults to 0 (buffered events disabled).
        DWORD eventBufferCountActual;

        /// Flag to indicate if the event buffer has overflowed and events have been lost.
        BOOL eventBufferHasOverflowed;
        
        /// Enforces mutual exclusion on operations that update the internal event state.
        CRITICAL_SECTION eventChangeCriticalSection;
        
        /// Specifies if the controller is "acquired" in DirectInput terms.
        /// DirectInput requires controllers be acquired before applications can provide data from them.
        /// Also, many property changes and other operations are unavailable once a controller is acquired.
        BOOL isAcquired;
        
        /// User index of the controller with which this instance should interface.
        DWORD xinputUserIndex;
        
        
        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        /// Default constructor. Should never be invoked.
        XInputController(void);
        
    public:
        /// Constructs a new controller object given the index (0 to 3) of the controller with which to interface.
        /// If the value of the index is out of range, all method calls to the constructed object will fail.
        XInputController(DWORD xinputUserIndex);

        /// Default destructor.
        virtual ~XInputController(void);
        
        
        // -------- CLASS METHODS ------------------------------------------ //
        
        /// Translates an XInput-style button reading to a DirectInput-style button reading.
        /// XInput uses one bit per button, so both a state and a mask are required to identify the button of interest.
        /// DirectInput uses the uppermost bit of a byte to indicate the button state.
        /// @param [in] buttonState XInput button state bitmask.
        /// @param [in] buttonMask XInput button identifier (only 1 bit should be set).
        /// @return DirectInput button state indicator for the identified XInput button.
        static LONG DirectInputButtonStateFromXInputButtonReading(const WORD buttonState, const WORD buttonMask);
        
        /// Extracts XInput directional pad state information and converts the format to a DirectInput-style POV reading.
        /// @param [in] buttonState XInput state information, which contains directional pad state.
        /// @return Corresponding DirectInput POV reading.
        static LONG DirectInputPovStateFromXInputButtonState(const WORD buttonState);
        
        /// Specifies if the indicated controller is connected (i.e. physically present and can be queried).
        /// @param [in] xinputUserIndex User index of interest, in the range of 0 to (#kMaxNumXInputControllers - 1).
        /// @return `TRUE` if a controller exists at that index, `FALSE` otherwise.
        static BOOL IsControllerConnected(const DWORD xinputUserIndex);
        
        
    private:
        // -------- HELPERS -------------------------------------------------------- //

        /// Computes the allowed event count for sizing the event buffer based on the requested size.
        /// @param [in] requestedSize Requested event buffer size, in number of elements.
        /// @return Allowed actual buffer size, in number of elements.
        DWORD AllowedEventCountForRequestedSize(DWORD requestedSize);
        
        /// Clears the event buffer.
        void ClearBufferedEvents(void);
        
        /// Sets the event buffer size.
        /// @param [in] requestedSize Requested event buffer size, in bytes.
        void SetEventBufferSize(DWORD requestedSize);
        
        /// Submits a new event to the buffer.
        /// Simply enqueues it to the event buffer queue.
        /// @param [in] controllerElement Identifier of the XInput controller element that has been modified.
        /// @param [in] value Value of the XInput controller element.
        /// @param [in] timestamp Timestamp associated with the event.
        void SubmitBufferedEvent(EXInputControllerElement controllerElement, LONG value, DWORD timestamp);


    public:
        // -------- INSTANCE METHODS ----------------------------------------------- //

        /// Causes the device to enter an "acquired" state.
        /// DirectInput requires that devices be acquired before data can be read.
        /// This is a largely symbolic operation in Xidi; its use is enforced but it has no effect and does not fail.
        /// @return `DI_OK` on success, `DI_NOEFFECT` if the controller was already acquired.
        HRESULT AcquireController(void);

        /// Retrieves the number of buffered events present.
        /// @return Number of buffered events.
        DWORD BufferedEventsCount();
        
        /// Fills in a DirectInput device capabilities structure with information about this controller's basic information.
        /// @param [out] lpDIDevCaps DirectInput device capability structure that should receive information.
        void FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps);

        /// Retrieves a DirectInput property of the controller.
        /// Corresponds directly to the IDirectInputDevice GetProperty method.
        /// @param [in] rguidProp DirectInput GUID of the property.
        /// @param [in] pdiph DirectInput property value information. Refer to DirectInput documentation for more.
        /// @return `DI_OK` on success or something else on error. Same return codes as IDirectInputDevice GetProperty method.
        HRESULT GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph);

        /// Queries the controller for information on its current state (buttons, axes, etc.).
        /// Places this information into the supplied device state structure.
        /// @param [out] state Structure that receives the controller state.
        /// @return `DI_OK` upon completion, or `DIERR_NOTACQUIRED` if the application did not first acquire the device.
        HRESULT GetCurrentDeviceState(XINPUT_STATE* state);

        /// Retrieves and returns the XInput player index associated with this controller object.
        /// @return Associated XInput player index.
        DWORD GetPlayerIndex(void);
        
        /// Specifies if the controller is currently acquired.
        /// @return `TRUE` if so, `FALSE` otherwise.
        BOOL IsAcquired(void);
        
        /// Specifies if the indicated controller is connected (i.e. physically present and can be queried).
        /// @return `TRUE` if a controller exists at the index configured for this object, `FALSE` otherwise.
        BOOL IsConnected(void);

        /// Specifies if event buffering is enabled.
        /// @return `TRUE` if so, `FALSE` if not.
        BOOL IsEventBufferEnabled(void);
        
        /// Specifies if the event buffer has overflowed.
        /// @return `TRUE` if so, `FALSE` if not.
        BOOL IsEventBufferOverflowed(void);

        /// Locks the event buffer for multiple operations.
        /// Idempotent; can be called multiple times from the same thread, so long as each call has an accompanying call to UnlockEventBuffer.
        void LockEventBuffer(void);
        
        /// Retrieves the specified buffered event and places it into the specified location.
        /// Does not remove the event from the buffer.
        /// @param [out] event Buffer that should receive the requested event.
        /// @param [in] idx Index within the event buffer of the event of interest.
        /// @return `DI_OK` on success, `DIERR_NOTACQUIRED` if the controller was not previously acquired, or `DIERR_INVALIDPARAM` if the buffered event does not exist.
        HRESULT PeekBufferedEvent(SControllerEvent* event, DWORD idx);
        
        /// Retrieves the first (oldest) buffered event from the controller and places it into the specified location.
        /// Removes the event from the buffer.
        /// @param [out] event Buffer that should receive the oldest buffered event.
        /// @return `DI_OK` on success, `DIERR_NOTACQUIRED` if the controller was not previously acquired, or `DIERR_INVALIDPARAM` if the event buffer is empty.
        HRESULT PopBufferedEvent(SControllerEvent* event);
        
        /// Refreshes the controller state information and adds changes to the event buffer.
        /// Essentially the same as the IDirectInputDevice Poll method.
        /// @return `DI_OK` upon completion, or `DIERR_NOTACQUIRED` if the application did not first acquire the device.
        HRESULT RefreshControllerState(void);
        
        /// Sets a DirectInput property of the controller.
        /// Corresponds directly to the IDirectInputDevice SetProperty method.
        /// @param [in] rguidProp DirectInput GUID of the property.
        /// @param [in] pdiph DirectInput property value information. Refer to DirectInput documentation for more.
        /// @return `DI_OK` on success or something else on error. Same return codes as IDirectInputDevice SetProperty method.
        HRESULT SetControllerProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph);

        /// Sets the event to be notified if the controller's state changes.
        /// @param [in] hEvent Windows event handle to trigger, or `NULL` to disable this functionality.
        /// @return `DI_POLLEDDEVICE` to indicate that events are only dispatched when the IDirectInputDevice Poll method is invoked.
        HRESULT SetControllerStateChangedEvent(HANDLE hEvent);

        /// Causes the device to be removed from an "acquired" state.
        /// `DI_OK` on success, or `DI_NOEFFECT` if the controller was not acquired before calling this method.
        HRESULT UnacquireController(void);

        /// Unlocks the event buffer after multiple operations have completed.
        void UnlockEventBuffer(void);
    };
}
