/*****************************************************************************
 * XboxControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with Xbox 360 and Xbox One controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XboxDirectInput8.h
 *      Declaration of the wrapper class for IDirectInput8.
 *****************************************************************************/

#pragma once

#include "API_DirectInput8.h"


// Wraps the IDirectInput8 interface to hook into all calls to it.
// Holds an underlying instance of an IDirectInput8 object but wraps all method invocations.
struct XboxDirectInput8 : IDirectInput8
{
private:
// -------- INSTANCE VARIABLES --------------------------------------------- //
    
    // The underlying IDirectInput8 object that this instance wraps.
    IDirectInput8* underlyingDIObject;


public:
// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

    // Constructs an XboxDirectInput8 object, given an underlying IDirectInput8 object to wrap.
    XboxDirectInput8(IDirectInput8* underlyingDIObject);


// -------- METHODS: IUnknown ---------------------------------------------- //
    virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID* ppvObj);
    virtual ULONG __stdcall AddRef(void);
    virtual ULONG __stdcall Release(void);


// -------- METHODS: IDirectInput8 ----------------------------------------- //
    virtual HRESULT __stdcall CreateDevice(REFGUID rguid, LPDIRECTINPUTDEVICE8* lplpDirectInputDevice, LPUNKNOWN pUnkOuter);
    virtual HRESULT __stdcall ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData);
    virtual HRESULT __stdcall EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags);
    virtual HRESULT __stdcall EnumDevicesBySemantics(LPCTSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags);
    virtual HRESULT __stdcall FindDevice(REFGUID rguidClass, LPCTSTR ptszName, LPGUID pguidInstance);
    virtual HRESULT __stdcall GetDeviceStatus(REFGUID rguidInstance);
    virtual HRESULT __stdcall Initialize(HINSTANCE hinst, DWORD dwVersion);
    virtual HRESULT __stdcall RunControlPanel(HWND hwndOwner, DWORD dwFlags);
};
