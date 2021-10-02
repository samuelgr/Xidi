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

#include "ApiWindows.h"
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
            // Operation always succeeds and reports that the controller is completely neutral.
            return {.errorCode = ERROR_SUCCESS, .state = {}};
        }

        // --------

        bool WaitForPhysicalControllerStateChange(TControllerIdentifier controllerIdentifier, SPhysicalState& state, std::stop_token stopToken)
        {
            // No state changes are ever reported, so the only way this function ever returns is if a stop is requested.
            while (false == stopToken.stop_requested())
                Sleep(0);

            return false;
        }
    }
}
