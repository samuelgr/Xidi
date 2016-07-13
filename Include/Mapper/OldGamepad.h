/*****************************************************************************
* XinputControllerDirectInput
*      Hook and helper for older DirectInput games.
*      Fixes issues associated with certain Xinput-based controllers.
*****************************************************************************
* Authored by Samuel Grossman
* Copyright (c) 2016
*****************************************************************************
* OldGamepad.h
*      Declares a mapper that maps to the button layout of an older
*      DirectInput-compatible gamepad.
*****************************************************************************/

#pragma once

#include "Mapper/Base.h"

#include <unordered_map>


namespace XinputControllerDirectInput
{
    namespace Mapper
    {
        // Provides a mapping to the button layout of an older DirectInput-compatible gamepad.
        // LT and RT triggers are mapped to buttons.
        // Right stick is mapped to the Z and Z-Rot axes.
        class OldGamepad : public Base
        {
        public:
            // -------- TYPE DEFINITIONS ----------------------------------------------- //
            
            // Identifies each button modelled by this mapper.
            // Values specify DirectInput instance number.
            enum EButton : TInstanceIdx
            {
                ButtonA                         = 0,
                ButtonB                         = 1,
                ButtonX                         = 2,
                ButtonY                         = 3,
                ButtonL1                        = 4,
                ButtonR1                        = 5,
                ButtonL2                        = 6,
                ButtonR2                        = 7,
                ButtonBack                      = 8,
                ButtonStart                     = 9,
                ButtonLeftStick                 = 10,
                ButtonRightStick                = 11,
                
                ButtonCount                     = 12
            };

            // Identifies each axis modelled by this mapper.
            // Values specify DirectInput instance number.
            enum EAxis : TInstanceIdx
            {
                AxisX                           = 0,
                AxisY                           = 1,
                AxisZ                           = 2,
                AxisRZ                          = 3,
                
                AxisCount                       = 4
            };

            // Identifies each point-of-view controller modelled by this mapper.
            // Values specify DirectInput instance number.
            enum EPov : TInstanceIdx
            {
                PovDpad                         = 0,
                
                PovCount                        = 1
            };
            

            // -------- INSTANCE VARIABLES --------------------------------------------- //

            

            
            // -------- INSTANCE METHODS ----------------------------------------------- //
            
            virtual TInstanceIdx AxisInstanceIndex(REFGUID axisGUID, DWORD instanceNumber);

            virtual BOOL AxisInstanceExists(REFGUID axisGUID, DWORD instanceNumber);

            virtual TInstanceCount AxisTypeCount(REFGUID axisGUID);

            
            // -------- CONCRETE INSTANCE METHODS -------------------------------------- //
            // See "Mapper.h" for documentation.

            virtual TInstanceCount NumInstancesOfType(EInstanceType type);
        };
    }
}

#include <dinput.h>