/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * OldGamepad.cpp
 *      Implements a mapper that maps to the button layout of an older
 *      DirectInput-compatible gamepad.
 *****************************************************************************/

#include "Mappers/OldGamepad.h"

using namespace XinputControllerDirectInput;
using namespace XinputControllerDirectInput::Mappers;


 // -------- CONCRETE INSTANCE METHODS -------------------------------------- //
 // See "Mapper.h" for documentation.

HRESULT OldGamepad::ParseApplicationDataFormat(LPCDIDATAFORMAT lpdf)
{
    return S_OK;
}
