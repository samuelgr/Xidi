/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ForwardedApiDInput.cpp
 *   Partial implementation of exported function entry points for DInput.
 **************************************************************************************************/

#include "DllFunctions.h"
#include "Globals.h"
#include "Strings.h"

DLL_EXPORT_FORWARD_DEFINE_DLL_WITH_CUSTOM_PATH(Xidi)
{
  return Xidi::Strings::GetXidiMainLibraryFilename();
}

DLL_EXPORT_FORWARD(Xidi, dinput_DirectInputCreateA);
DLL_EXPORT_FORWARD(Xidi, dinput_DirectInputCreateW);
DLL_EXPORT_FORWARD(Xidi, dinput_DirectInputCreateEx);
DLL_EXPORT_FORWARD(Xidi, dinput_DllRegisterServer);
DLL_EXPORT_FORWARD(Xidi, dinput_DllUnregisterServer);
DLL_EXPORT_FORWARD(Xidi, dinput_DllCanUnloadNow);
DLL_EXPORT_FORWARD(Xidi, dinput_DllGetClassObject);
