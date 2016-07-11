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
    struct sControllerState
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

    // Identifies each input component of an Xinput-based controller.
    enum eControllerInput
    {
        CONTROLLERINPUT_STICK_LEFT,
        CONTROLLERINPUT_STICK_RIGHT,
        CONTROLLERINPUT_DPAD,
        CONTROLLERINPUT_TRIGGER_LEFT,
        CONTROLLERINPUT_TRIGGER_RIGHT,
        CONTROLLERINPUT_BUTTON_A,
        CONTROLLERINPUT_BUTTON_B,
        CONTROLLERINPUT_BUTTON_X,
        CONTROLLERINPUT_BUTTON_Y,
        CONTROLLERINPUT_BUTTON_LB,
        CONTROLLERINPUT_BUTTON_RB,
        CONTROLLERINPUT_BUTTON_BACK,
        CONTROLLERINPUT_BUTTON_START,
        CONTROLLERINPUT_BUTTON_LS,
        CONTROLLERINPUT_BUTTON_RS
    };
    
    // Abstract base class representing a supported hardware controller.
    // Subclasses communicate with DirectInput, define an Xinput-specific data format, and provides state information.
    class Controller
    {
    public:
        // -------- CONSTANTS ------------------------------------------------------ //
        
        // Minimum value of readings from the left and right sticks, based on Xinput documentation.
        static const LONG kStickRangeMin = -32768;
        
        // Maximum value of readings from the left and right sticks, based on Xinput documentation.
        static const LONG kStickRangeMax = 32768;
        
        // Minimum value of readings from the LT and RT triggers, based on Xinput documentation.
        static const LONG kTriggerRangeMin = 0;
        
        // Maximum value of readings from the LT and RT triggers, based on Xinput documentation.
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
        virtual WORD ControllerInputToDirectInputInstanceNumber(eControllerInput controllerInput) = 0;

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
