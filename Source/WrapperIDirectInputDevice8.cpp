/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * WrapperIDirectInputDevice8.cpp
 *      Implementation of the wrapper class for IDirectInputDevice8.
 *****************************************************************************/

#include "WrapperIDirectInputDevice8.h"
#include "Mapper/Base.h"

using namespace Xidi;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "WrapperIDirectInputDevice8.h" for documentation.

WrapperIDirectInputDevice8::WrapperIDirectInputDevice8(BOOL useUnicode, XInputController* controller, Mapper::Base* mapper) : controller(controller), mapper(mapper), refcount(0), useUnicode(useUnicode) {}

// ---------

WrapperIDirectInputDevice8::~WrapperIDirectInputDevice8()
{
    delete controller;
    delete mapper;
}


// -------- METHODS: IUnknown ---------------------------------------------- //
// See IUnknown documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT result = S_OK;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDirectInputDevice8))
    {
        AddRef();
        *ppvObj = this;
    }
    else
        result = E_NOINTERFACE;

    return result;
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInputDevice8::AddRef(void)
{
    refcount += 1;
    return refcount;
}

// ---------

ULONG STDMETHODCALLTYPE WrapperIDirectInputDevice8::Release(void)
{
    ULONG numRemainingRefs = refcount - 1;

    if (0 == numRemainingRefs)
        delete this;
    else
        refcount = numRemainingRefs;

    return numRemainingRefs;
}


// -------- METHODS: IDirectInputDevice8 ----------------------------------- //
// See DirectInput documentation for more information.

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::Acquire(void)
{
    return controller->AcquireController();
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::BuildActionMap(LPDIACTIONFORMAT lpdiaf, LPCTSTR lpszUserName, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::EnumEffects(LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::EnumEffectsInFile(LPCTSTR lptszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
    return mapper->EnumerateMappedObjects(useUnicode, lpCallback, pvRef, dwFlags);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::Escape(LPDIEFFESCAPE pesc)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetCapabilities(LPDIDEVCAPS lpDIDevCaps)
{
    if (sizeof(*lpDIDevCaps) != lpDIDevCaps->dwSize)
        return DIERR_INVALIDPARAM;
    
    controller->FillDeviceCapabilities(lpDIDevCaps);
    mapper->FillDeviceCapabilities(lpDIDevCaps);

    return DI_OK;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    // Not yet implemented.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetDeviceInfo(LPDIDEVICEINSTANCE pdidi)
{
    // Not yet implemented.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
    // Not yet implemented.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetEffectInfo(LPDIEFFECTINFO pdei, REFGUID rguid)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetForceFeedbackState(LPDWORD pdwOut)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetImageInfo(LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
    return mapper->GetMappedObjectInfo(useUnicode, pdidoi, dwObj, dwHow);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::GetProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    if (mapper->IsPropertyHandledByMapper(rguidProp))
        return mapper->GetMappedProperty(rguidProp, pdiph);
    else
        return controller->GetControllerProperty(rguidProp, pdiph);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::Initialize(HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
    // Operation not necessary.
    return S_FALSE;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::Poll(void)
{
    return controller->RefreshControllerState();
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SendForceFeedbackCommand(DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SetActionMap(LPDIACTIONFORMAT lpdiActionFormat, LPCTSTR lptszUserName, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    // Ineffective at present, but this may change.
    return DI_OK;
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
    return mapper->SetApplicationDataFormat(lpdf);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SetEventNotification(HANDLE hEvent)
{
    return controller->SetControllerStateChangedEvent(hEvent);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    if (mapper->IsPropertyHandledByMapper(rguidProp))
        return mapper->SetMappedProperty(rguidProp, pdiph);
    else
        return controller->SetControllerProperty(rguidProp, pdiph);
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::Unacquire(void)
{
    return controller->UnacquireController();
}

// ---------

HRESULT STDMETHODCALLTYPE WrapperIDirectInputDevice8::WriteEffectToFile(LPCTSTR lptszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
{
    // Operation not supported.
    return DIERR_UNSUPPORTED;
}
