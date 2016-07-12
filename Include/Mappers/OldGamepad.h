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

#include "Mapper.h"


namespace XinputControllerDirectInput
{
    namespace Mappers
    {
        // Provides a mapping to the button layout of an older DirectInput-compatible gamepad.
        // LT and RT triggers are mapped to buttons.
        // Right stick is mapped to the Z and Z-Rot axes.
        class OldGamepad : public Mapper
        {
            // -------- CONCRETE INSTANCE METHODS -------------------------------------- //
            // See "Mapper.h" for documentation.

            virtual HRESULT ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf);
        };
    }
}
