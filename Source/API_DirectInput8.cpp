/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * API_DirectInput8.h
 *      Pulls in all DirectInput GUID definitions to avoid linking with
 *      the "dinput8.lib" library.
 *****************************************************************************/

#define INITGUID
#include "API_DirectInput8.h"
