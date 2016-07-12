/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * Controller.h
 *      Abstract base class for supported Xinput-based controllers.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput8.h"


namespace XinputControllerDirectInput
{
    // Represents the state of an Xinput-based controller.
    // State information from DirectInput will be filled into structures of this type for use by application mappers.
    // For non-binary values (sticks, triggers, and dpad) the value reported is as read from the controller.
    // For binary values, the value is 0x80 (upper bit set) if pressed or 0x00 (no bits set) if not.
    struct SControllerState
    {
        LONG stickLeftX;                        // horizontal position of the left stick
        LONG stickLeftY;                        // vertical position of the left stick
        LONG stickRightX;                       // horizontal position of the right stick
        LONG stickRightY;                       // vertical position of the right stick
        
        LONG dpad;                              // DPAD position reading
        
        LONG triggerLeft;                       // trigger LT
        LONG triggerRight;                      // trigger RT
        
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
    
    // Abstract base class representing a supported hardware controller.
    // Subclasses communicate with DirectInput, define an Xinput-specific data format, and provides state information.
    class Controller
    {
    public:
        // -------- TYPE DEFINITIONS ----------------------------------------------- //
        
        // Identifies each input component of an Xinput-based controller.
        enum EControllerInput : USHORT
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


        // -------- CONSTANTS ------------------------------------------------------ //
        
        // Minimum value of readings from the left and right sticks, from the Xinput documentation.
        static const LONG kStickRangeMin = -32768;
        
        // Maximum value of readings from the left and right sticks, from the Xinput documentation.
        static const LONG kStickRangeMax = 32767;

        // Neutral position value for the left and right sticks.
        static const LONG kStickNeutral = 0;
        
        // Minimum value of readings from the LT and RT triggers, from the Xinput documentation.
        static const LONG kTriggerRangeMin = 0;
        
        // Maximum value of readings from the LT and RT triggers, from the Xinput documentation.
        static const LONG kTriggerRangeMax = 255;
        
        
    protected:
        // -------- INSTANCE VARIABLES --------------------------------------------- //
        
        // The DirectInput controller, represented by its IDirectInputDevice8 interface.
        IDirectInputDevice8* underlyingDIController;
        
        
   public:
        // -------- CLASS METHODS -------------------------------------------------- //
        
        //
        
        
        // -------- INSTANCE METHODS ----------------------------------------------- //

        //


        // -------- ABSTRACT INSTANCE METHODS -------------------------------------- //

        // Maps the specified controller input to a DirectInput instance number for the corresponding class.
        // For example, if DirectInput considers the A button to be button 2, then passing CONTROLLERINPUT_BUTTON_A results in a return value of 2.
        // Must be implemented by subclasses.
        virtual WORD ControllerInputToDirectInputInstanceNumber(EControllerInput controllerInput) = 0;

        // Initializes the specified DirectInput device with the data format required to communicate with it properly.
        // Must be implemented by subclasses.
        virtual HRESULT InitializeDirectInputDeviceDataFormat(IDirectInputDevice8* device) = 0;
        
        // Retrieves data from a controller.
        // Uses the mapper to interpose between raw controller data and data presented to the application.
        // Directly corresponds to the IDirectInputDevice8 interface's "GetDeviceData" method.
        // Must be implemented by subclasses.
        virtual HRESULT GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) = 0;

        // Retrieves a snapshot of the controller's state.
        // Uses the mapper to interpose between raw controller data and data presented to the application.
        // Directly corresponds to the IDirectInputDevice8 interface's "GetDeviceState" method.
        // Must be implemented by subclasses.
        virtual HRESULT GetDeviceState(DWORD cbData, LPVOID lpvData) = 0;
    };
}
