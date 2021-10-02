/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file FakePhysicalController.cpp
 *   Fake implementation of physical controller functions.  Calling any of
 *   these fake functions immediately causes a test to fail.
 *****************************************************************************/

#include "PhysicalController.h"
#include "TestCase.h"

#include <stop_token>


namespace Xidi
{
    namespace Controller
    {
        // -------- FUNCTIONS ---------------------------------------------- //
        // See "PhysicalController.h" for documentation.

        SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier)
        {
            TEST_FAILED_BECAUSE(L"Unimplemented fake function called: %s", __FUNCTIONW__);
        }

        // --------

        bool WaitForPhysicalControllerStateChange(TControllerIdentifier controllerIdentifier, SPhysicalState& state, std::stop_token stopToken)
        {
            TEST_FAILED_BECAUSE(L"Unimplemented fake function called: %s", __FUNCTIONW__);
        }
    }
}
