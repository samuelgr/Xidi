/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MockPhysicalController.cpp
 *   Implementation of a mock version of the physical controller interface
 *   along with additional testing-specific functions.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ForceFeedbackDevice.h"
#include "MockPhysicalController.h"
#include "PhysicalController.h"
#include "TestCase.h"
#include "VirtualController.h"

#include <vector>
#include <shared_mutex>
#include <stop_token>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    // -------- INTERNAL CONSTANTS ----------------------------------------- //

    /// Default physical state to return when no other physical states are specified.
    static constexpr SPhysicalState kDefaultPhysicalState = {.errorCode = ERROR_SUCCESS, .state = {}};


    // -------- INTERNAL VARIABLES ----------------------------------------- //

    /// Guards all mock physical state data structures, one per physical controller.
    /// Even in tests, additional threads may exist to wait for state changes.
    static std::shared_mutex mockPhysicalStateGuard[kPhysicalControllerCount];

    /// Holds pointers to all mock physical controller objects, one per physical controller.
    /// Each such object governs the behavior of the physical controller interface for a given physical controller.
    static MockPhysicalController* mockPhysicalController[kPhysicalControllerCount];


    // -------- CONSTRUCTION AND DESTRUCTION ------------------------------- //
    // See "MockPhysicalController.h" for documentation.

    MockPhysicalController::MockPhysicalController(TControllerIdentifier controllerIdentifier, const SPhysicalState* mockPhysicalStates, size_t mockPhysicalStateCount) : kControllerIdentifier(controllerIdentifier), kMockPhysicalStates(mockPhysicalStates), kMockPhysicalStateCount(mockPhysicalStateCount), currentPhysicalStateIndex(0), advanceRequested(false), forceFeedbackDevice(0), forceFeedbackRegistration()
    {
        if (controllerIdentifier >= kPhysicalControllerCount)
            TEST_FAILED_BECAUSE(L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

        if (1 > mockPhysicalStateCount)
            TEST_FAILED_BECAUSE(L"%s: Attempting to create a mock physical controller with identifier %u having 0 valid states.", __FUNCTIONW__, controllerIdentifier);

        std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

        if (nullptr != mockPhysicalController[kControllerIdentifier])
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to multiple attempts to control physical controller with identifier %u.", __FUNCTIONW__, controllerIdentifier);

        mockPhysicalController[kControllerIdentifier] = this;
    }

    // --------

    MockPhysicalController::~MockPhysicalController(void)
    {
        std::unique_lock lock(mockPhysicalStateGuard[kControllerIdentifier]);
        mockPhysicalController[kControllerIdentifier] = nullptr;
    }


    // -------- INSTANCE METHODS ------------------------------------------- //
    // See "MockPhysicalController.h" for documentation.

    void MockPhysicalController::AdvancePhysicalState(void)
    {
        currentPhysicalStateIndex += 1;
        advanceRequested = false;
    }

    // --------

    SPhysicalState MockPhysicalController::GetCurrentPhysicalState(void) const
    {
        return kMockPhysicalStates[currentPhysicalStateIndex];
    }

    // --------

    void MockPhysicalController::RequestAdvancePhysicalState(void)
    {
        std::unique_lock lock(mockPhysicalStateGuard[kControllerIdentifier]);

        if (currentPhysicalStateIndex >= (kMockPhysicalStateCount - 1))
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to out-of-bounds physical state advancement for physical controller with identifier %u.", __FUNCTIONW__, kControllerIdentifier);

        if (true == advanceRequested)
            TEST_FAILED_BECAUSE(L"%s: Test implementation error due to unconsumed advance request for physical controller with identifier %u.", __FUNCTIONW__, kControllerIdentifier);

        advanceRequested = true;
    }
}


namespace Xidi
{
    namespace Controller
    {
        using namespace ::XidiTest;


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "PhysicalController.h" for documentation.

        SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier)
        {
            if (controllerIdentifier >= kPhysicalControllerCount)
                TEST_FAILED_BECAUSE(L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

            std::shared_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

            if (nullptr != mockPhysicalController[controllerIdentifier])
                return mockPhysicalController[controllerIdentifier]->GetCurrentPhysicalState();
            else
                return kDefaultPhysicalState;
        }

        // --------

        ForceFeedback::Device* PhysicalControllerForceFeedbackRegister(TControllerIdentifier controllerIdentifier, const VirtualController* virtualController)
        {
            if (controllerIdentifier >= kPhysicalControllerCount)
                TEST_FAILED_BECAUSE(L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

            std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

            if (nullptr != mockPhysicalController[controllerIdentifier])
            {
                mockPhysicalController[controllerIdentifier]->InsertForceFeedbackRegistration(virtualController);
                return &mockPhysicalController[controllerIdentifier]->GetForceFeedbackDevice();
            }

            return nullptr;
        }

        // --------

        void PhysicalControllerForceFeedbackUnregister(TControllerIdentifier controllerIdentifier, const VirtualController* virtualController)
        {
            if (controllerIdentifier >= kPhysicalControllerCount)
                TEST_FAILED_BECAUSE(L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

            std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

            if (nullptr != mockPhysicalController[controllerIdentifier])
            {
                mockPhysicalController[controllerIdentifier]->EraseForceFeedbackRegistration(virtualController);
            }
        }

        // --------

        bool WaitForPhysicalControllerStateChange(TControllerIdentifier controllerIdentifier, SPhysicalState& state, std::stop_token stopToken)
        {
            if (controllerIdentifier >= kPhysicalControllerCount)
                TEST_FAILED_BECAUSE(L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

            while (false == stopToken.stop_requested())
            {
                Sleep(1);

                if (nullptr != mockPhysicalController[controllerIdentifier])
                {
                    std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

                    if (nullptr != mockPhysicalController[controllerIdentifier])
                    {
                        if (mockPhysicalController[controllerIdentifier]->IsAdvanceStateRequested())
                        {
                            mockPhysicalController[controllerIdentifier]->AdvancePhysicalState();
                            state = mockPhysicalController[controllerIdentifier]->GetCurrentPhysicalState();
                            return true;
                        }
                    }
                }
            }

            return false;
        }
    }
}
