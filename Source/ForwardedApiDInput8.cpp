/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ForwardedApiDInput8.cpp
 *   Partial implementation of exported function entry points for DInput8.
 **************************************************************************************************/

#include "DllFunctions.h"
#include "Globals.h"
#include "Strings.h"

DLL_EXPORT_FORWARD_DEFINE_DLL_WITH_CUSTOM_PATH(Xidi)
{
  return Xidi::Strings::GetXidiMainLibraryFilename();
}

DLL_EXPORT_FORWARD(Xidi, dinput8_DirectInput8Create);
DLL_EXPORT_FORWARD(Xidi, dinput8_DllRegisterServer);
DLL_EXPORT_FORWARD(Xidi, dinput8_DllUnregisterServer);
DLL_EXPORT_FORWARD(Xidi, dinput8_DllCanUnloadNow);
DLL_EXPORT_FORWARD(Xidi, dinput8_DllGetClassObject);
