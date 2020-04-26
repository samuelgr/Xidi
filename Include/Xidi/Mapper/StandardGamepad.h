/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper/StandardGamepad.h
 *   Declares a mapper that maps to the button layout of an older
 *   DirectInput-compatible gamepad.
 *****************************************************************************/

#pragma once

#include "Mapper.h"


namespace Xidi
{
    /// Provides a mapping to the button layout of a standard DirectInput-compatible gamepad.
    /// LT and RT triggers are mapped to their own buttons.
    /// Right stick is mapped to the Z and Z-Rot axes.
    class StandardGamepadMapper : public Mapper
    {
    public:
        // -------- TYPE DEFINITIONS ----------------------------------- //

        /// Identifies each button modelled by this mapper.
        /// Values specify DirectInput instance number.
        enum class EButton : TInstanceIdx
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

        /// Identifies each axis modelled by this mapper.
        /// Values specify DirectInput instance number.
        enum class EAxis : TInstanceIdx
        {
            AxisX                           = 0,
            AxisY                           = 1,
            AxisZ                           = 2,
            AxisRZ                          = 3,

            AxisCount                       = 4
        };

        /// Identifies each point-of-view controller modelled by this mapper.
        /// Values specify DirectInput instance number.
        enum class EPov : TInstanceIdx
        {
            PovDpad                         = 0,

            PovCount                        = 1
        };


        // -------- CONCRETE INSTANCE METHODS -------------------------- //
        // See "Mapper.h" for documentation.

        const TInstanceIdx AxisInstanceIndex(REFGUID axisGUID, const TInstanceIdx instanceNumber) override;
        const TInstanceCount AxisTypeCount(REFGUID axisGUID) override;
        const GUID AxisTypeFromInstanceNumber(const TInstanceIdx instanceNumber) override;
        const TInstance MapXInputElementToDirectInputInstance(EXInputControllerElement element) override;
        const TInstanceCount NumInstancesOfType(const EInstanceType type) override;
    };
}
