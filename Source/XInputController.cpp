/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XInputController.cpp
 *      Implementation of a class that represents and interfaces with a single
 *      XInput-based controller and exposes a DirectInput-like interface.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "XInputController.h"

#include <deque>
#include <Xinput.h>

using namespace Xidi;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "XInputController.h" for documentation.

XInputController::XInputController(DWORD xinputUserIndex) : bufferedEvents(), bufferedEventsNextSequenceNumber(0), controllerState(), controllerStateChangedEvent(NULL), isAcquired(FALSE), xinputUserIndex(xinputUserIndex)
{
    InitializeCriticalSectionEx(&eventChangeCriticalSection, 1000, CRITICAL_SECTION_NO_DEBUG_INFO);
}

// ---------

XInputController::~XInputController()
{
    EnterCriticalSection(&eventChangeCriticalSection);

    ClearBufferedEvents();
    DeleteCriticalSection(&eventChangeCriticalSection);
}


// -------- CLASS METHODS -------------------------------------------------- //
// See "XInputController.h" for documentation.

LONG XInputController::DirectInputButtonStateFromXInputButtonReading(const WORD buttonState, const WORD buttonMask)
{
    if (0 != (buttonState & buttonMask))
        return (LONG)0x0080;
    else
        return 0;
}

// ---------

LONG XInputController::DirectInputPovStateFromXInputButtonState(const WORD buttonState)
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
            if (-1 == verticalDpadComponent) dpadValue = 22500;             // down and left
            else if (0 == verticalDpadComponent) dpadValue = 27000;         // left
            else if (1 == verticalDpadComponent) dpadValue = 31500;         // up and left
            break;

        case 0:
            if (-1 == verticalDpadComponent) dpadValue = 18000;             // down
            else if (0 == verticalDpadComponent) dpadValue = -1;            // centered
            else if (1 == verticalDpadComponent) dpadValue = 0;             // up
            break;

        case 1:
            if (-1 == verticalDpadComponent) dpadValue = 13500;             // down and right
            else if (0 == verticalDpadComponent) dpadValue = 9000;          // right
            else if (1 == verticalDpadComponent) dpadValue = 4500;          // up and right
            break;
        }
    }

    return dpadValue;
}

// ---------

BOOL XInputController::IsControllerConnected(const DWORD xinputUserIndex)
{
    XINPUT_CAPABILITIES dummyCapabilities;
    DWORD result = XInputGetCapabilities(xinputUserIndex, 0, &dummyCapabilities);

    return (result == ERROR_SUCCESS);
}


// -------- HELPERS -------------------------------------------------------- //
// See "XInputController.h" for documentation.

void XInputController::ClearBufferedEvents(void)
{
    EnterCriticalSection(&eventChangeCriticalSection);
    bufferedEvents.clear();
    LeaveCriticalSection(&eventChangeCriticalSection);
}

// ---------

void XInputController::SubmitBufferedEvent(EXInputControllerElement controllerElement, LONG value, DWORD timestamp)
{
    EnterCriticalSection(&eventChangeCriticalSection);
    
    // Create the event.
    SControllerEvent newEvent;
    newEvent.controllerElement = controllerElement;
    newEvent.value = value;
    newEvent.timestamp = timestamp;
    newEvent.sequenceNumber = bufferedEventsNextSequenceNumber;
    
    // Enqueue the event.
    bufferedEvents.push_back(newEvent);
    
    // Increment the sequence number.
    bufferedEventsNextSequenceNumber += 1;
    
    LeaveCriticalSection(&eventChangeCriticalSection);
}


// -------- INSTANCE METHODS ----------------------------------------------- //
// See "XInputController.h" for documentation.

HRESULT XInputController::AcquireController(void)
{
    BOOL wasAcquiredAlready = IsAcquired();

    isAcquired = TRUE;

    if (wasAcquiredAlready)
        return DI_NOEFFECT;
    else
        return DI_OK;
}

// ---------

DWORD XInputController::BufferedEventsCount()
{
    DWORD numEvents = 0;
    
    EnterCriticalSection(&eventChangeCriticalSection);
    numEvents = (DWORD)bufferedEvents.size();
    LeaveCriticalSection(&eventChangeCriticalSection);
    
    return numEvents;
}

// ---------

void XInputController::DiscardBufferedEvents(DWORD numEvents)
{
    EnterCriticalSection(&eventChangeCriticalSection);
    
    if (numEvents >= bufferedEvents.size())
        bufferedEvents.clear();
    else
    {
        switch (numEvents)
        {
        case 0:
            break;
            
        case 1:
            bufferedEvents.erase(bufferedEvents.begin());
            break;
            
        default:
            bufferedEvents.erase(bufferedEvents.begin(), bufferedEvents.begin() + numEvents);
            break;
        }
    }

    LeaveCriticalSection(&eventChangeCriticalSection);
}

// ---------

void XInputController::FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    lpDIDevCaps->dwFlags = (DIDC_ATTACHED | DIDC_EMULATED | DIDC_POLLEDDATAFORMAT);
    lpDIDevCaps->dwDevType = DINPUT_DEVTYPE_XINPUT_GAMEPAD;
    lpDIDevCaps->dwFFSamplePeriod = 0;
    lpDIDevCaps->dwFFMinTimeResolution = 0;
    lpDIDevCaps->dwFirmwareRevision = 0;
    lpDIDevCaps->dwHardwareRevision = 0;
    lpDIDevCaps->dwFFDriverVersion = 0;
}

HRESULT XInputController::GetBufferedEvents(SControllerEvent* events, DWORD& count, BOOL removeFromBuffer)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;

    EnterCriticalSection(&eventChangeCriticalSection);
    
    // Read events from the buffer and keep track of how many were read, stopping either at the application-requested number or the maximum number available.
    DWORD idx;
    for (idx = 0; idx < count && idx < bufferedEvents.size(); ++idx)
        events[idx] = bufferedEvents[idx];

    // Tell the application how many events were read.
    count = idx;

    // Make note if this was all of them.
    BOOL allEventsRead = (count == bufferedEvents.size());

    // Optionally remove events from the buffer, based on the number of events read out of it.
    DiscardBufferedEvents(count);

    LeaveCriticalSection(&eventChangeCriticalSection);
    
    // If all events were read, no overflow, otherwise there was a buffer overflow.
    if (TRUE != allEventsRead)
        return DI_BUFFEROVERFLOW;
    
    return DI_OK;
}

// ---------

HRESULT XInputController::GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    // Verify the correct header size.
    if (pdiph->dwHeaderSize != sizeof(DIPROPHEADER))
        return DIERR_INVALIDPARAM;

    // Verify whole-device properties have the correct value for object identification.
    if (DIPH_DEVICE == pdiph->dwHow && 0 != pdiph->dwObj)
        return DIERR_INVALIDPARAM;

    // Branch based on the property in question.

    // All other properties are unsupported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT XInputController::GetCurrentDeviceState(XINPUT_STATE* state)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;

    EnterCriticalSection(&eventChangeCriticalSection);

    // Copy the most recent controller state into the specified buffer.
    *state = controllerState;

    // Getting the full controller state also gets all buffered events in aggregate, so clear the buffer.
    ClearBufferedEvents();

    LeaveCriticalSection(&eventChangeCriticalSection);
    
    return DI_OK;
}

// ---------

BOOL XInputController::IsAcquired(void)
{
    return isAcquired;
}

// ---------

BOOL XInputController::IsConnected(void)
{
    return IsControllerConnected(xinputUserIndex);
}

// ---------

HRESULT XInputController::RefreshControllerState(void)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;
    
    // Get updated state information for the controller.
    XINPUT_STATE newControllerState;
    DWORD result = XInputGetState(xinputUserIndex, &newControllerState);

    // If the device was unplugged or otherwise has become unavailable, pretend that no change has taken place.
    if (ERROR_SUCCESS != result)
        return DI_OK;

    // If there has been no state change, there is nothing to do.
    if (newControllerState.dwPacketNumber != controllerState.dwPacketNumber)
    {
        EnterCriticalSection(&eventChangeCriticalSection);
        
        // Capture the current event sequence number, which will be used to see if the application should be notified of a controller state change.
        const DWORD currentEventSequenceNumber = bufferedEventsNextSequenceNumber;
        
        // All events get a timestamp, which in this case will be the current system time in milliseconds.
        DWORD eventTimestamp = GetTickCount();

        // For each controller component, check if there has been a change and, if so, add an event to the queue.
        if (newControllerState.Gamepad.sThumbLX != controllerState.Gamepad.sThumbLX)
            SubmitBufferedEvent(EXInputControllerElement::StickLeftHorizontal, (LONG)newControllerState.Gamepad.sThumbLX, eventTimestamp);
        
        if (newControllerState.Gamepad.sThumbLY != controllerState.Gamepad.sThumbLY)
            SubmitBufferedEvent(EXInputControllerElement::StickLeftVertical, (LONG)newControllerState.Gamepad.sThumbLY, eventTimestamp);

        if (newControllerState.Gamepad.sThumbRX != controllerState.Gamepad.sThumbRX)
            SubmitBufferedEvent(EXInputControllerElement::StickRightHorizontal, (LONG)newControllerState.Gamepad.sThumbRX, eventTimestamp);

        if (newControllerState.Gamepad.sThumbRY != controllerState.Gamepad.sThumbRY)
            SubmitBufferedEvent(EXInputControllerElement::StickRightHorizontal, (LONG)newControllerState.Gamepad.sThumbRY, eventTimestamp);

        if (newControllerState.Gamepad.bLeftTrigger != controllerState.Gamepad.bLeftTrigger)
            SubmitBufferedEvent(EXInputControllerElement::TriggerLT, (LONG)newControllerState.Gamepad.bLeftTrigger, eventTimestamp);

        if (newControllerState.Gamepad.bRightTrigger != controllerState.Gamepad.bRightTrigger)
            SubmitBufferedEvent(EXInputControllerElement::TriggerRT, (LONG)newControllerState.Gamepad.bRightTrigger, eventTimestamp);

        // For comparing buttons and dpad, will need to look at individual bits.
        if (newControllerState.Gamepad.wButtons != controllerState.Gamepad.wButtons)
        {
            // Dpad.
            if ((newControllerState.Gamepad.wButtons & kDpadStateMask) != (controllerState.Gamepad.wButtons & kDpadStateMask))
                SubmitBufferedEvent(EXInputControllerElement::Dpad, DirectInputPovStateFromXInputButtonState(newControllerState.Gamepad.wButtons), eventTimestamp);

            // Each button in sequence.
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A))
                SubmitBufferedEvent(EXInputControllerElement::ButtonA, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_A), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B))
                SubmitBufferedEvent(EXInputControllerElement::ButtonB, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_B), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X))
                SubmitBufferedEvent(EXInputControllerElement::ButtonX, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_X), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y))
                SubmitBufferedEvent(EXInputControllerElement::ButtonY, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_Y), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER))
                SubmitBufferedEvent(EXInputControllerElement::ButtonLB, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER))
                SubmitBufferedEvent(EXInputControllerElement::ButtonRB, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
                SubmitBufferedEvent(EXInputControllerElement::ButtonBack, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_BACK), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START))
                SubmitBufferedEvent(EXInputControllerElement::ButtonStart, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_START), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB))
                SubmitBufferedEvent(EXInputControllerElement::ButtonLeftStick, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB), eventTimestamp);
            if ((newControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != (controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB))
                SubmitBufferedEvent(EXInputControllerElement::ButtonRightStick, DirectInputButtonStateFromXInputButtonReading(newControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB), eventTimestamp);
        }

        // Copy the new controller state to the current controller state.
        controllerState = newControllerState;

        LeaveCriticalSection(&eventChangeCriticalSection);

        // Notify the application if the controller state changed.
        if (currentEventSequenceNumber != bufferedEventsNextSequenceNumber && NULL != controllerStateChangedEvent)
            SetEvent(controllerStateChangedEvent);
    }

    return DI_OK;
}

// ---------

HRESULT XInputController::SetControllerProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    // Verify the correct header size.
    if (pdiph->dwHeaderSize != sizeof(DIPROPHEADER))
        return DIERR_INVALIDPARAM;
    
    // Verify whole-device properties have the correct value for object identification.
    if (DIPH_DEVICE == pdiph->dwHow && 0 != pdiph->dwObj)
        return DIERR_INVALIDPARAM;
    
    // Branch based on the property in question.
    
    // All other properties are unsupported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT XInputController::SetControllerStateChangedEvent(HANDLE hEvent)
{
    controllerStateChangedEvent = hEvent;
    return DI_POLLEDDEVICE;
}

// ---------

HRESULT XInputController::UnacquireController(void)
{
    BOOL wasAcquiredAlready = IsAcquired();

    isAcquired = FALSE;

    if (wasAcquiredAlready)
        return DI_OK;
    else
        return DI_NOEFFECT;
}
