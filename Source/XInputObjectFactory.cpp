/*****************************************************************************
 * XInputControllerDirectInput
 *      Hook and helper for older DirectInput games.
 *      Fixes issues associated with certain XInput-based controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * XInputObjectFactory.cpp
 *      Implementation of methods used to construct objects that interface
 *      with XInput-based controllers.
 *****************************************************************************/

#include "ApiDirectInput8.h"
#include "WrapperIDirectInputDevice8.h"
#include "XInputControllerIdentification.h"
#include "XInputObjectFactory.h"
#include "Controller/DirectInputBase.h"
#include "Mapper/OldGamepad.h"

#include <unordered_map>


using namespace XInputControllerDirectInput;


// -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //
// See "XInputObjectFactory.h" for documentation.

XInputObjectFactory::XInputObjectFactory() : enumeratedControllers() {}

// ---------

XInputObjectFactory::~XInputObjectFactory()
{
    enumeratedControllers.clear();
}


// -------- INSTANCE METHODS ----------------------------------------------- //
// See "XInputObjectFactory.h" for documentation.

IDirectInputDevice8* XInputObjectFactory::CreateDirectInputDeviceForController(IDirectInputDevice8* underlyingController, REFGUID instanceGUID)
{
    auto it = enumeratedControllers.find(instanceGUID);

    if (enumeratedControllers.end() != it)
    {
        // Even if the controller is of a known type, its wrapper implementation may not yet be completed.
        // Filter based on the implementations that exist.
        switch (it->second)
        {
        case EControllerType::XboxOne:
            return new WrapperIDirectInputDevice8(underlyingController, new Controller::DirectInputBase(underlyingController),  new Mapper::OldGamepad());
        }
    }

    return underlyingController;
}

// ---------

void XInputObjectFactory::ResetEnumeratedControllers(void)
{
    enumeratedControllers.clear();
}

// ---------

void XInputObjectFactory::SubmitEnumeratedController(REFGUID productGUID, REFGUID instanceGUID)
{
    if (TRUE == XInputControllerIdentification::IsControllerTypeKnown(productGUID))
    {
        enumeratedControllers.insert({instanceGUID, XInputControllerIdentification::GetControllerType(productGUID)});
    }
}
