/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file MockPhysicalController.h
 *   Declaration of a mock version of the physical controller interface along with additional
 *   testing-specific functions.
 **************************************************************************************************/

#pragma once

#include <cstddef>
#include <set>

#include "ForceFeedbackDevice.h"
#include "Mapper.h"
#include "PhysicalController.h"
#include "VirtualController.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  /// Object used to exert behavioral control over the physical controller interface. The physical
  /// controller interface itself is implemented as free functions. Test cases can use instances of
  /// this object to control the behavior of that interface. All required state is maintained in an
  /// object instance, and control is RAII-style. Within a test case MockPhysicalController objects
  /// should be created ahead of VirtualController objects because the initial state is the first
  /// state in the array. If multiple objects are created within the same scope for the same
  /// physical controller, the test will fail due to a test implementation error.
  class MockPhysicalController
  {
  public:

    MockPhysicalController(
        TControllerIdentifier controllerIdentifier,
        const Mapper& mapper,
        const SPhysicalState* mockPhysicalStates = nullptr,
        size_t mockPhysicalStateCount = 0);

    MockPhysicalController(const MockPhysicalController& other) = delete;

    ~MockPhysicalController(void);

    /// Advances to the next physical state.
    /// Intended to be invoked internally only.
    void AdvancePhysicalState(void);

    /// Unregisters a virtual controller for force feedback.
    /// @param [in] controllerToRegister Pointer to the virtual controller object that should be
    /// unregistered for force feedback.
    inline void EraseForceFeedbackRegistration(const VirtualController* controllerToUnregister)
    {
      forceFeedbackRegistration.erase(controllerToUnregister);
    }

    /// Retrieves and returns the capabilities implemented by the mapper associated with this mock
    /// physical controller.
    /// @return Mapper-derived capabilities data structure.
    SCapabilities GetControllerCapabilities(void) const;

    /// Retrieves and returns the current physical state.
    /// @return Current physical state being reported to the test cases that request it.
    SPhysicalState GetCurrentPhysicalState(void) const;

    /// Retrieves and returns the current raw virtual state, which is derived on-the-fly from the
    /// raw virtual state.
    /// @return Current raw virtual state being reported to the test cases that request it.
    SState GetCurrentRawVirtualState(void) const;

    /// Provides access to the force feedback device object.
    /// @return Reference to the force feedback device object.
    inline ForceFeedback::Device& GetForceFeedbackDevice(void)
    {
      return forceFeedbackDevice;
    }

    /// Registers a virtual controller for force feedback.
    /// @param [in] controllerToRegister Pointer to the virtual controller object that should be
    /// registered for force feedback.
    inline void InsertForceFeedbackRegistration(const VirtualController* controllerToRegister)
    {
      forceFeedbackRegistration.insert(controllerToRegister);
    }

    /// Checks if the specified virtual controller is registered for force feedback.
    /// @return `true` if so, `false` if not.
    inline bool IsVirtualControllerRegisteredForForceFeedback(
        const VirtualController* controllerToCheck)
    {
      return forceFeedbackRegistration.contains(controllerToCheck);
    }

    /// Retrieves and returns the controller identifier associated with this object.
    /// @return Associated controller identifier.
    inline TControllerIdentifier GetControllerIdentifier(void) const
    {
      return kControllerIdentifier;
    }

    /// Retrieves and returns whether or not an advancement to the next physical state has been
    /// requested.
    /// @return `true` if so, `false` if not.
    inline bool IsAdvanceStateRequested(void) const
    {
      return advanceRequested;
    }

    /// Requests an advancement to the next physical state.
    /// Test will fail due to a test implementation issue if attempting to advance past the end of
    /// the physical state array.
    void RequestAdvancePhysicalState(void);

  private:

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

    /// Flag which specifies whether or not the next wait-for-state-change operation should
    /// result in an advancement of the reported physical state to the next element in the
    /// physical state array.
    bool advanceRequested;

    /// Force feedback device associated with the physical controller.
    /// Initialized to use a base timestamp of 0.
    ForceFeedback::Device forceFeedbackDevice;

    /// Mapper to use with this mock physical controller object for mapping physical to raw
    /// virtual states.
    const Mapper& mapper;

    /// Virtual controllers registered for force feedback.
    std::set<const VirtualController*> forceFeedbackRegistration;
  };
} // namespace XidiTest
