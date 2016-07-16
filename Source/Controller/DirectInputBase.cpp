/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * DirectInputBase.cpp
 *      Abstract base class for all implementations that communicate with
 *      Xinput-based controllers via DirectInput.
 *      Provides common implementations of most core functionality.
 *****************************************************************************/

#include "ApiDirectInput8.h"
#include "Controller/Base.h"
#include "Controller/DirectInputBase.h"

using namespace XinputControllerDirectInput;
using namespace XinputControllerDirectInput::Controller;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "Controller/Base.h" for documentation.

DirectInputBase::DirectInputBase(IDirectInputDevice8* underlyingDIObject) : Base(), underlyingDIObject(underlyingDIObject) {}


// -------- CONCRETE INSTANCE METHODS -------------------------------------- //
// See "Controller/Base.h" for documentation.

HRESULT DirectInputBase::AcquireController(void)
{
    return underlyingDIObject->Acquire();
}

HRESULT DirectInputBase::GetBufferedEvents(SControllerEvent* events, DWORD count, BOOL removeFromBuffer)
{
    return S_OK;
}

HRESULT DirectInputBase::GetControllerProperty(REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    return underlyingDIObject->GetProperty(rguidProp, pdiph);
}

HRESULT DirectInputBase::GetCurrentDeviceState(SControllerState* state)
{
    return E_FAIL;
}

HRESULT DirectInputBase::SetControllerProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    return underlyingDIObject->SetProperty(rguidProp, pdiph);
}

HRESULT DirectInputBase::UnacquireController(void)
{
    return underlyingDIObject->Unacquire();
}
