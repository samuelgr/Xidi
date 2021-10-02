/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file PhysicalController.h
 *   Declaration of all functionality for communicating with physical
 *   controllers.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <stop_token>
#include <xinput.h>


namespace Xidi
{
    namespace Controller
    {
        // -------- TYPE DEFINITIONS --------------------------------------- //
        
        /// Integer type used to identify physical controllers to the underlying system interfaces.
        /// XInput controllers are identified by user index, also known as player number.
        typedef DWORD TControllerIdentifier;

        /// Structure used for holding physical controller state data.
        struct SPhysicalState
        {
            DWORD errorCode;                                                ///< Error code resulting from the last attempt to poll the physical controller.
            XINPUT_STATE state;                                             ///< State data from the last attempt to poll the physical controller.

            /// Simple equality check to detect physical state changes.
            /// @param [in] other Object with which to compare.
            /// @return `true` if this object is equal to the other object, `false` otherwise.
            inline bool operator==(const SPhysicalState& other) const
            {
                if (errorCode != other.errorCode)
                    return false;

                if ((errorCode == 0) && (state.dwPacketNumber != other.state.dwPacketNumber))
                    return false;

                return true;
            }
        };


        // -------- CONSTANTS ---------------------------------------------- //

        /// Number of physical controllers that the underlying system supports.
        /// Not all will necessarily be physically present at any given time, but all of them will be polled periodically.
        /// Maximum allowable controller identifier is one less than this value.
        inline constexpr TControllerIdentifier kPhysicalControllerCount = XUSER_MAX_COUNT;


        // -------- FUNCTIONS ---------------------------------------------- //

        /// Retrieves the instantaneous state of the specified controller.
        /// Concurrency-safe, but intended to be used for initialization and does not perform any bounds-checking on the controller identifier.
        /// @param [in] controllerIdentifier Identifier of the physical controller of interest.
        /// @return Physical controller state data.
        SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier);

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
