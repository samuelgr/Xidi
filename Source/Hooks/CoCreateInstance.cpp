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


// -------- INTERNAL FUNCTIONS --------------------------------------------- //

namespace Xidi
{
    /// Outputs a message indicating that the CoCreateInstance invocation will be intercepted.
    /// @param [in] className Name of the class, in string form, to be passed verbatim to the output.
    /// @param [in] useUnicode Indicator of whether or not the target interface is Unicode or ASCII.
    static void OutputMessageIntercepting(const wchar_t* className, bool useUnicode)
    {
        Message::OutputFormatted(Message::ESeverity::Info, L"CoCreateInstance: Intercepting creation of object of type %s and compatible %s interface.", className, (true == useUnicode ? L"Unicode" : L"ASCII"));
    }

    /// Outputs a message indicating that the CoCreateInstance invocation will not be intercepted because the class is recognized but not currently supported.
    /// @param [in] className Name of the class, in string form, to be passed verbatim to the output.
    /// @param [in] useUnicode Indicator of whether or not the target interface is Unicode or ASCII.
    static void OutputMessageNotInterceptingClassUnsupported(const wchar_t* className, bool useUnicode)
    {
        Message::OutputFormatted(Message::ESeverity::Info, L"CoCreateInstance: Not intercepting creation of object of type %s and compatible %s interface (this class is not currently supported).", className, (true == useUnicode ? L"Unicode" : L"ASCII"));
    }

    /// Outputs a message indicating that the CoCreateInstance invocation will not be intercepted because the class is recognized but the interface is unknown.
    /// @param [in] className Name of the class, in string form, to be passed verbatim to the output.
    /// @param [in] riid Requested IID, to be converted to a string and output.
    static void OutputMessageNotInterceptingUnknownIID(const wchar_t* className, REFIID riid)
    {
        if (Xidi::Message::WillOutputMessageOfSeverity(Xidi::Message::ESeverity::Info))
        {
            LPOLESTR iidString;
            const HRESULT iidStringResult = StringFromIID(riid, &iidString);
            const wchar_t* const iidOutputString = (S_OK == iidStringResult ? iidString : L"(unknown)");

            Xidi::Message::OutputFormatted(Xidi::Message::ESeverity::Info, L"CoCreateInstance: Not intercepting creation of object of type %s but unrecognized IID %s.", className, iidString);

            if (S_OK == iidStringResult)
                CoTaskMemFree(iidString);
        }
    }
}


// -------- HOOK FUNCTION -------------------------------------------------- //
// See original function and Hookshot documentation for details.

HRESULT StaticHook_CoCreateInstance::Hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv)
{
    if (IsEqualCLSID(CLSID_DirectInput8, rclsid))
    {
        static constexpr wchar_t kClassIdString[] = L"CLSID_DirectInput8";

        if (IsEqualIID(riid, IID_IDirectInput8A))
        {
            Xidi::OutputMessageIntercepting(kClassIdString, false);
        }
        else if (IsEqualIID(riid, IID_IDirectInput8W))
        {
            Xidi::OutputMessageIntercepting(kClassIdString, true);
        }
        else
        {
            Xidi::OutputMessageNotInterceptingUnknownIID(kClassIdString, riid);
        }
    }
    else if (IsEqualCLSID(CLSID_DirectInput, rclsid))
    {
        static constexpr wchar_t kClassIdString[] = L"CLSID_DirectInput";

        if (IsEqualIID(riid, IID_IDirectInput7A) || IsEqualIID(riid, IID_IDirectInput2A) || IsEqualIID(riid, IID_IDirectInputA))
        {
            Xidi::OutputMessageIntercepting(kClassIdString, false);
        }
        else if (IsEqualIID(riid, IID_IDirectInput7W) || IsEqualIID(riid, IID_IDirectInput2W) || IsEqualIID(riid, IID_IDirectInputW))
        {
            Xidi::OutputMessageIntercepting(kClassIdString, true);
        }
        else
        {
            Xidi::OutputMessageNotInterceptingUnknownIID(kClassIdString, riid);
        }
    }
    else if (IsEqualCLSID(CLSID_DirectInputDevice8, rclsid))
    {
        static constexpr wchar_t kClassIdString[] = L"CLSID_DirectInputDevice8";

        if (IsEqualIID(riid, IID_IDirectInputDevice8A))
        {
            Xidi::OutputMessageNotInterceptingClassUnsupported(kClassIdString, false);
        }
        else if (IsEqualIID(riid, IID_IDirectInputDevice8W))
        {
            Xidi::OutputMessageNotInterceptingClassUnsupported(kClassIdString, true);
        }
        else
        {
            Xidi::OutputMessageNotInterceptingUnknownIID(kClassIdString, riid);
        }
    }
    else if (IsEqualCLSID(CLSID_DirectInputDevice, rclsid))
    {
        static constexpr wchar_t kClassIdString[] = L"CLSID_DirectInputDevice";

        if (IsEqualIID(riid, IID_IDirectInputDevice7A) || IsEqualIID(riid, IID_IDirectInputDevice2A) || IsEqualIID(riid, IID_IDirectInputDeviceA))
        {
            Xidi::OutputMessageNotInterceptingClassUnsupported(kClassIdString, false);
        }
        else if (IsEqualIID(riid, IID_IDirectInputDevice7W) || IsEqualIID(riid, IID_IDirectInputDevice2W) || IsEqualIID(riid, IID_IDirectInputDeviceW))
        {
            Xidi::OutputMessageNotInterceptingClassUnsupported(kClassIdString, true);
        }
        else
        {
            Xidi::OutputMessageNotInterceptingUnknownIID(kClassIdString, riid);
        }
    }
    
    return Original(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}
