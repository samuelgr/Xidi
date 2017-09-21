/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file XInputController.cpp
 *   Implementation of a class that represents and interfaces with a single
 *   XInput-based controller and exposes a DirectInput-like interface.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "XInputController.h"

#include <deque>
#include <Xinput.h>

using namespace Xidi;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "XInputController.h" for documentation.

XInputController::XInputController(DWORD xinputUserIndex) : bufferedEvents(), bufferedEventsNextSequenceNumber(0), controllerState(), controllerStateChangedEvent(NULL), eventBufferSizeRequested(0), eventBufferCountActual(0), eventBufferHasOverflowed(FALSE), isAcquired(FALSE), xinputUserIndex(xinputUserIndex)
{
    InitializeCriticalSectionEx(&eventChangeCriticalSection, 1000, CRITICAL_SECTION_NO_DEBUG_INFO);
}

// ---------

XInputController::~XInputController(void)
{
    ClearBufferedEvents();
    
    EnterCriticalSection(&eventChangeCriticalSection);
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

DWORD XInputController::AllowedEventCountForRequestedSize(DWORD requestedSize)
{
    const DWORD requestedNumberOfElements = requestedSize / sizeof(SControllerEvent);

    if ((0 == requestedNumberOfElements) && (0 != requestedSize))
    {
        // Requested a very small but non-zero buffer size, so small that it would not even hold a single element.
        // Round this type of request up and store a single element.
        return 1;
    }
    else if (requestedNumberOfElements > kEventBufferCountMax)
    {
        // Requested a buffer that is too large to be allowed.
        // Return the maximum.
        return kEventBufferCountMax;
    }
    else
    {
        // Requested a buffer size that is allowed.
        // Return the computed number of elements that fit.
        return requestedNumberOfElements;
    }
}

// ---------

void XInputController::ClearBufferedEvents(void)
{
    LockEventBuffer();
    
    bufferedEvents.clear();
    eventBufferHasOverflowed = FALSE;

    UnlockEventBuffer();
}

// ---------

void XInputController::SetEventBufferSize(DWORD requestedSize)
{
    const DWORD actualCount = AllowedEventCountForRequestedSize(requestedSize);
    
    if (actualCount == eventBufferCountActual)
    {
        // Requested change is ineffective.
        // Just pull in the requested size but otherwise there is nothing to do.
        eventBufferSizeRequested = requestedSize;
        return;
    }
    
    LockEventBuffer();

    if (0 == requestedSize)
    {
        // Disabling buffered events completely.
        eventBufferSizeRequested = 0;
        eventBufferCountActual = 0;
        eventBufferHasOverflowed = FALSE;

        // No point keeping buffered events around.
        ClearBufferedEvents();
    }
    else if (actualCount > eventBufferCountActual)
    {
        // Increasing the event buffer size.
        
        // Just update the stored values.
        // Because the buffer may have overflowed previously, do not modify that flag.
        eventBufferSizeRequested = requestedSize;
        eventBufferCountActual = actualCount;
    }
    else
    {
        // Decreasing the event buffer size.
        
        // First, drop any events that are in excess of the buffer size.
        // If this is required, the buffer has overflowed.
        if (bufferedEvents.size() > actualCount)
            eventBufferHasOverflowed = TRUE;
        
        while (bufferedEvents.size() > actualCount)
            bufferedEvents.pop_front();

        // Next, update the stored values.
        eventBufferSizeRequested = requestedSize;
        eventBufferCountActual = actualCount;
    }
    
    UnlockEventBuffer();
}

// ---------

void XInputController::SubmitBufferedEvent(EXInputControllerElement controllerElement, LONG value, DWORD timestamp)
{
    LockEventBuffer();
    
    // Create the event.
    SControllerEvent newEvent;
    newEvent.controllerElement = controllerElement;
    newEvent.value = value;
    newEvent.timestamp = timestamp;
    newEvent.sequenceNumber = bufferedEventsNextSequenceNumber;
    
    if (bufferedEvents.size() == eventBufferCountActual)
    {
        // Buffer is at capacity.
        // Discard an event and set the overflow flag.
        bufferedEvents.pop_front();
        eventBufferHasOverflowed = TRUE;
    }
    else
    {
        // Buffer has free space.
        // Exit any overflow state that might have existed beforehand.
        eventBufferHasOverflowed = FALSE;
    }
    
    // Enqueue the event.
    bufferedEvents.push_back(newEvent);
    
    // Increment the sequence number.
    bufferedEventsNextSequenceNumber += 1;
    
    UnlockEventBuffer();
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
    
    LockEventBuffer();
    numEvents = (DWORD)bufferedEvents.size();
    UnlockEventBuffer();
    
    return numEvents;
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
    if (&DIPROP_BUFFERSIZE == &rguidProp)
    {
        // Requesting the buffer size.

        // Reject the update if it is not targetting the whole device, per DirectInput spec.
        if (DIPH_DEVICE != pdiph->dwHow)
            return DIERR_INVALIDPARAM;

        // Retrieve the size, always the requested size per DirectInput spec, even if it exceeds the maximum allowable size.
        ((LPDIPROPDWORD)pdiph)->dwData = eventBufferSizeRequested;
    }
    else
    {
        // All other properties are unsupported.
        return DIERR_UNSUPPORTED;
    }
    
    return DI_OK;
}

// ---------

HRESULT XInputController::GetCurrentDeviceState(XINPUT_STATE* state)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;

    // Copy the most recent controller state into the specified buffer.
    *state = controllerState;
    
    return DI_OK;
}

// ---------

DWORD XInputController::GetPlayerIndex(void)
{
    return xinputUserIndex;
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

BOOL XInputController::IsEventBufferEnabled(void)
{
    return (0 != eventBufferCountActual);
}

// ---------

BOOL XInputController::IsEventBufferOverflowed(void)
{
    return eventBufferHasOverflowed;
}

// ---------

void XInputController::LockEventBuffer(void)
{
    EnterCriticalSection(&eventChangeCriticalSection);
}

// ---------

HRESULT XInputController::PeekBufferedEvent(SControllerEvent* event, DWORD idx)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;

    LockEventBuffer();

    HRESULT result = DI_OK;

    if (idx >= BufferedEventsCount() || NULL == event)
        result = DIERR_INVALIDPARAM;
    else
        *event = bufferedEvents[idx];

    UnlockEventBuffer();

    return result;
}

// ---------

HRESULT XInputController::PopBufferedEvent(SControllerEvent* event)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;

    LockEventBuffer();

    if (NULL != event)
        *event = bufferedEvents[0];

    bufferedEvents.pop_front();

    UnlockEventBuffer();

    return DI_OK;
}

// ---------

HRESULT XInputController::RefreshControllerState(void)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;
    
    // Get updated state information for the controller.
    XINPUT_STATE newControllerState;
    DWORD result = XInputGetState(xinputUserIndex, &newControllerState);
    BOOL shouldNotifyApplicationEvent = FALSE;

    // If the device was unplugged or otherwise has become unavailable, reset its state to everything being neutral.
    if (ERROR_SUCCESS != result)
        ZeroMemory(&newControllerState.Gamepad, sizeof(newControllerState.Gamepad));
    
    // Add device events to the event buffer if buffered events are enabled.
    if (0 != eventBufferCountActual)
    {
        LockEventBuffer();

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
            SubmitBufferedEvent(EXInputControllerElement::StickRightVertical, (LONG)newControllerState.Gamepad.sThumbRY, eventTimestamp);

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

        UnlockEventBuffer();

        // Determine if the application state change event should be notified.
        shouldNotifyApplicationEvent = ((NULL != controllerStateChangedEvent) && (currentEventSequenceNumber != bufferedEventsNextSequenceNumber));
    }
    else
    {
        // Determine if the application state change event should be notified.
        shouldNotifyApplicationEvent = ((NULL != controllerStateChangedEvent) && (0 == memcmp((void*)&controllerState, (void*)&newControllerState, sizeof(controllerState))));
    }
    
    // Copy the new controller state to the current controller state.
    controllerState = newControllerState;

    // Notify the application if the controller state changed.
    if (shouldNotifyApplicationEvent)
        SetEvent(controllerStateChangedEvent);
    
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
    if (&DIPROP_BUFFERSIZE == &rguidProp)
    {
        // Setting the buffer size, potentially enabling or disabling buffered events.

        // Reject the update if it is not targetting the whole device, per DirectInput spec.
        if (DIPH_DEVICE != pdiph->dwHow)
            return DIERR_INVALIDPARAM;

        // Perform the update as requested.
        SetEventBufferSize(((LPDIPROPDWORD)pdiph)->dwData);
    }
    else
    {
        // All other properties are unsupported.
        return DIERR_UNSUPPORTED;
    }

    return DI_OK;
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

// ---------

void XInputController::UnlockEventBuffer(void)
{
    LeaveCriticalSection(&eventChangeCriticalSection);
}
