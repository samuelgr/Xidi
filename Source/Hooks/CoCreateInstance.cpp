/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file CoCreateInstance.cpp
 *   Implementation of hook for CoCreateInstance.
 *****************************************************************************/

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "Message.h"
#include "Hooks.h"
#include "TemporaryBuffer.h"



// -------- HOOK FUNCTION -------------------------------------------------- //
// See original function and Hookshot documentation for details.

HRESULT StaticHook_CoCreateInstance::Hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
    return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
