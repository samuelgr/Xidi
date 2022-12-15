/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file PhysicalController.h
 *   Declaration of all functionality for communicating with physical
 *   controllers.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ForceFeedbackDevice.h"
#include "VirtualController.h"

#include <stop_token>


namespace Xidi
{
    namespace Controller
    {
        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of milliseconds to wait between polling attempts.
        inline constexpr unsigned int kPhysicalPollingPeriodMilliseconds = 5;

        /// Number of milliseconds to wait between force feedback actuation passes.
        inline constexpr unsigned int kPhysicalForceFeedbackPeriodMilliseconds = 5;

        /// Number of milliseconds to wait between attempts to communicate with the physical hardware if the last attempt resulted in an error, such as the controller being disconnected.
        inline constexpr unsigned int kPhysicalErrorBackoffPeriodMilliseconds = 100;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Retrieves the instantaneous state of the specified controller.
        /// Concurrency-safe, but intended to be used for initialization and does not perform any bounds-checking on the controller identifier.
        /// @param [in] controllerIdentifier Identifier of the physical controller of interest.
        /// @return Physical controller state data.
        SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier);

        /// Attempts to register the specified virtual controller for force feedback with the specified physical controller.
        /// Concurrency-safe.
        /// @param [in] controllerIdentifier Identifier of the physical controller of interest.
        /// @param [in] virtualController Pointer to the virtual controller of interest.
        /// @return Pointer to the device buffer object if successful, `nullptr` otherwise. This function will fail if another object is already registered with the specified virtual controller or if the parameters are invalid.
        ForceFeedback::Device* PhysicalControllerForceFeedbackRegister(TControllerIdentifier controllerIdentifier, const VirtualController* virtualController);

        /// Unregisters the specified virtual controller for force feedback if it is currently registered with the specified physical controller.
        /// Concurrency-safe.
        /// @param [in] controllerIdentifier Identifier of the physical controller of interest.
        /// @param [in] virtualController Pointer to the virtual controller of interest.
        void PhysicalControllerForceFeedbackUnregister(TControllerIdentifier controllerIdentifier, const VirtualController* virtualController);

        /// Waits for the specified physical controller's state to change. When it does, retrieves and returns the new state.
        /// Intended to be invoked by background worker threads associated with virtual controller objects.
        /// This function is fully concurrency-safe. If needed, the caller can interrupt the wait using a stop token.
        /// @param [in] controllerIdentifier Identifier of the physical controller of interest.
        /// @param [in,out] state On input, used to identify the last-known physical controller state for the calling thread. On output, filled in with the updated state of the physical controller.
        /// @param [in] stopToken Token that allows the weight to be interrupted. Defaults to an empty token that does not allow interruption.
        /// @return `true` if the wait succeeded and the output structure was updated, `false` if no updates were made due to invalid parameter or interrupted wait.
        bool WaitForPhysicalControllerStateChange(TControllerIdentifier controllerIdentifier, SPhysicalState& state, std::stop_token stopToken = std::stop_token());
    }
}
