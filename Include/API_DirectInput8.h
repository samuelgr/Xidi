/*****************************************************************************
* XboxOneDirectInput
*      Hook and helper for older DirectInput games.
*      Fixes issues associated with Xbox One controllers.
*****************************************************************************
* Authored by Samuel Grossman
* Copyright (c) 2016
*****************************************************************************
* API_DirectInput8.h
*      Common header file for the DirectInput8 API.
*****************************************************************************/

#pragma once

#include "API_Windows.h"


// -------- DIRECTINPUT 8 API ---------------------------------------------- //

#define DIRECTINPUT_VERSION                     0x0800
#include <dinput.h>
