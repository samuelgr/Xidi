/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file XInputInterface.h
 *   Declaration of the interface through which all XInput functionality is
 *   accessed.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <xinput.h>


namespace Xidi
{
    /// XInput interface class.
    /// All XInput functions are invoked by calling methods on objects that implement this interface.
    /// The purpose of exposing XInput methods this way is to facilitate easier testing by mocking the XInput function calls.
    /// Methods are named after XInput API functions. See XInput documentation for more information.
    /// Presence or absence of a method in the interface is determined by whether or not Xidi needs to use it.
    class IXInput
    {
    public:
        // -------- ABSTRACT INSTANCE METHODS ------------------------------ //

        virtual DWORD GetState(DWORD dwUserIndex, XINPUT_STATE* pState) = 0;
    };

    /// Default implementation of the XInput interface.
    /// Methods are simply redirections to imported XInput functions.
    class XInput : public IXInput
    {
    public:
        // -------- CONCRETE INSTANCE METHODS ------------------------------ //

        DWORD GetState(DWORD dwUserIndex, XINPUT_STATE* pState) override;
    };
}
