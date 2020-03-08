/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Mapper/ExtendedGamepad.h
 *   Declares a mapper that maps to the button layout of an older
 *   DirectInput-compatible gamepad, except with triggers as axes.
 *****************************************************************************/

#pragma once

#include "Mapper/XInputNative.h"


namespace Xidi
{
    namespace Mapper
    {
        /// Provides a mapping to the button layout of a standard DirectInput-compatible gamepad, except with triggers treated as axes.
        /// LT and RT triggers are mapped respectively to the X-Rot and Y-Rot axes, which are uncommonly used in older games.
        /// Right stick is mapped to the Z and Z-Rot axes.
        class ExtendedGamepad : public XInputNative
        {
        public:
            // -------- CONCRETE INSTANCE METHODS -------------------------- //
            // See "Mapper/Base.h" for documentation.
            
            virtual const TInstance MapXInputElementToDirectInputInstance(EXInputControllerElement element);
        };
    }
}
