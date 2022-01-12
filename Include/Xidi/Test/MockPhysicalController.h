/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2022
 *************************************************************************//**
 * @file MockPhysicalController.h
 *   Declaration of a mock version of the physical controller interface along
 *   with additional testing-specific functions.
 *****************************************************************************/

#pragma once

#include "ForceFeedbackDevice.h"
#include "PhysicalController.h"
#include "VirtualController.h"

#include <cstddef>


namespace XidiTest
{
    using namespace ::Xidi::Controller;


    /// Object used to exert behavioral control over the physical controller interface.
    /// The physical controller interface itself is implemented as free functions.
    /// Test cases can use instances of this object to control the behavior of that interface.
    /// All required state is maintained in an object instance, and control is RAII-style.
    /// Within a test case MockPhysicalController objects should be created ahead of VirtualController objects because the initial state is the first state in the array.
    /// If multiple objects are created within the same scope for the same physical controller, the test will fail due to a test implementation error.
    class MockPhysicalController
    {
    private:
        // -------- INSTANCE VARIABLES --------------------------------- //

        /// Physical controller identifier for which this object is asserting control.
        const TControllerIdentifier kControllerIdentifier;

        /// Array of physical states through which a test should iterate.
        /// Owned by the test case and must remain valid throughout this object's lifetime.
        const SPhysicalState* const kMockPhysicalStates;

        /// Number of physical states in the array.
        const size_t kMockPhysicalStateCount;

        /// Holds the index of the current physical state.
        /// Begins at 0 and increases whenever a test case advances to the next physical state.
        size_t currentPhysicalStateIndex;

        /// Flag which specifies whether or not the next wait-for-state-change operation should result in an advancement of the reported physical state to the next element in the physical state array.
        bool advanceRequested;

        /// Force feedback device associated with the physical controller.
        /// Initialized to use a base timestamp of 0.
        ForceFeedback::Device forceFeedbackDevice;

        /// Virtual controller registered for force feedback.
        const VirtualController* forceFeedbackRegistration;


    public:
        // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

        /// Initialization constructor.
        /// Requires specification of controller identifier and all applicable physical states.
        MockPhysicalController(TControllerIdentifier controllerIdentifier, const SPhysicalState* mockPhysicalStates, size_t mockPhysicalStateCount);

        /// Default destructor.
        ~MockPhysicalController(void);


        // -------- INSTANCE METHODS ----------------------------------- //
        
        /// Advances to the next physical state.
        /// Intended to be invoked internally only.
        void AdvancePhysicalState(void);

        /// Retrieves and returns the current physical state.
        /// @return Current physical state being reported to the test cases that request it.
        SPhysicalState GetCurrentPhysicalState(void) const;

        /// Provides access to the force feedback device object.
        /// @return Reference to the force feedback device object.
        inline ForceFeedback::Device& GetForceFeedbackDevice(void)
        {
            return forceFeedbackDevice;
        }

        /// Provides access to the virtual controller object that is registered for force feedback.
        /// @return Pointer to the registered virtual controller, or `nullptr` if no virtual controller is registered.
        inline const VirtualController* GetForceFeedbackRegistration(void) const
        {
            return forceFeedbackRegistration;
        }

        /// Retrieves and returns whether or not an advancement to the next physical state has been requested.
        /// @return `true` if so, `false` if not.
        inline bool IsAdvanceStateRequested(void) const
        {
            return advanceRequested;
        }

        /// Requests an advancement to the next physical state.
        /// Test will fail due to a test implementation issue if attempting to advance past the end of the physical state array.
        void RequestAdvancePhysicalState(void);

        /// Sets the virtual controller that is registered for force feedback.
        /// Passing `nullptr` clears the registration entirely.
        /// @param [in] controllerToRegister Pointer to the virtual controller object that should be registered for force feedback.
        inline void SetForceFeedbackRegistration(const VirtualController* controllerToRegister)
        {
            forceFeedbackRegistration = controllerToRegister;
        }
    };
}
