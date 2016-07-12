/*****************************************************************************
 * XinputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain Xinput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XinputObjectFactory.cpp
 *      Implementation of methods used to construct objects that interface
 *      with Xinput-based controllers.
 *****************************************************************************/

#include "ApiDirectInput8.h"
#include "Hashers.h"
#include "WrapperIDirectInputDevice8.h"
#include "XinputControllerIdentification.h"
#include "XinputObjectFactory.h"

#include <unordered_map>


using namespace XinputControllerDirectInput;


// -------- CLASS METHODS -------------------------------------------------- //
// See "XinputObjectFactory.h" for documentation.

IDirectInputDevice8* XinputObjectFactory::CreateDirectInputDeviceForController(IDirectInputDevice8* underlyingController, REFGUID instanceGUID)
{
    auto it = enumeratedControllers.find(instanceGUID);

    if (enumeratedControllers.end() != it)
    {
        // Even if the controller is of a known type, its wrapper implementation may not yet be completed.
        // Filter based on the implementations that exist.
        switch (it->second)
        {
        case EControllerType::XboxOne:
            return new WrapperIDirectInputDevice8(underlyingController);
        }
    }

    return underlyingController;
}

// ---------

void XinputObjectFactory::ResetEnumeratedControllers(void)
{
    enumeratedControllers.clear();
}

// ---------

void XinputObjectFactory::SubmitEnumeratedController(REFGUID productGUID, REFGUID instanceGUID)
{
    if (TRUE == XinputControllerIdentification::IsControllerTypeKnown(productGUID))
    {
        enumeratedControllers.insert({instanceGUID, XinputControllerIdentification::GetControllerType(productGUID)});
    }
}
