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

void XInputController::FillDeviceCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    lpDIDevCaps->dwFlags = (DIDC_ATTACHED | DIDC_EMULATED | DIDC_POLLEDDATAFORMAT);
    lpDIDevCaps->dwDevType = DI8DEVTYPE_GAMEPAD;
    lpDIDevCaps->dwFFSamplePeriod = 0;
    lpDIDevCaps->dwFFMinTimeResolution = 0;
    lpDIDevCaps->dwFirmwareRevision = 0;
    lpDIDevCaps->dwHardwareRevision = 0;
    lpDIDevCaps->dwFFDriverVersion = 0;
}

HRESULT XInputController::GetBufferedEvents(SControllerEvent* events, DWORD& count, BOOL removeFromBuffer)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;

    // Read events from the buffer and keep track of how many were read, stopping either at the application-requested number or the maximum number available.
    DWORD idx;
    for (idx = 0; idx < count && idx < bufferedEvents.size(); ++idx)
        events[idx] = bufferedEvents[idx];

    // Tell the application how many events were read.
    count = idx;

    // Optionally remove events from the buffer, based on the number of events read out of it.
    if (removeFromBuffer)
    {
        switch (count)
        {
        case 0:
            break;

        case 1:
            bufferedEvents.erase(bufferedEvents.begin());
            break;

        default:
            bufferedEvents.erase(bufferedEvents.begin(), bufferedEvents.begin() + idx);
            break;
        }
    }
    
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

HRESULT XInputController::RefreshControllerState(void)
{
    if (!IsAcquired()) return DIERR_NOTACQUIRED;
    
    // Get updated state information for the controller.
    XINPUT_STATE newControllerState;
    DWORD result = XInputGetState(xinputUserIndex, &newControllerState);

    // If the device was unplugged or otherwise has become unavailable, indicate this to the application.
    if (ERROR_SUCCESS != result)
    {
        UnacquireController();
        return DIERR_INPUTLOST;
    }

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
