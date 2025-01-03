/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file MockMouse.h
 *   Declaration of a mock version of the mouse interface along with additional testing-specific
 *   functions.
 **************************************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <unordered_map>

#include "ApiBitSet.h"
#include "ControllerTypes.h"
#include "Mouse.h"

namespace XidiTest
{
  using namespace ::Xidi::Mouse;
  using ::Xidi::Controller::TControllerIdentifier;

  /// Object used to exert behavioral control over the mouse interface. The mouse interface itself
  /// is implemented as free functions. Test cases can use instances of this object to capture state
  /// changes to the virtual mouse. If any mouse interface functions are called while no mock mouse
  /// object is capturing state, the test case will fail.
  class MockMouse
  {
  public:

    ~MockMouse(void);

    inline bool operator==(const MockMouse& other) const
    {
      return (
          (virtualMouseButtonState == other.virtualMouseButtonState) &&
          (virtualMouseMovementContributionBySource ==
           other.virtualMouseMovementContributionBySource));
    }

    /// Installs this virtual mouse as the one to which mouse events generated by mouse interface
    /// function calls will be recorded.
    void BeginCapture(void);

    /// Removes this virtual mouse as the one to which mouse events generated by the mouse interface
    /// function calls will be recorded. Upon completion, no virtual mouse is the target of any
    /// captures.
    void EndCapture(void);

    /// Retrieves the most recent mouse movement contribution that was submitted from the specified
    /// source.
    /// @param [in] axis Mouse axis for which the movement contribution is being queried.
    /// @param [in] sourceIdentifier Opaque identifier for the source of the mouse movement event.
    /// @return Most recent mouse movement contribution from the specified source, if such a
    /// contribution exists.
    std::optional<int> GetMovementContributionFromSource(
        EMouseAxis axis, uint32_t sourceIdentifier) const;

    /// Submits a mouse movement.
    /// @param [in] axis Mouse axis that is affected.
    /// @param [in] mouseMovementUnits Number of internal mouse movement units along the target
    /// mouse axis.
    /// @param [in] sourceIdentifier Opaque identifier for the source of the mouse movement event.
    void SubmitMouseMovement(EMouseAxis axis, int mouseMovementUnits, uint32_t sourceIdentifier);

    /// Submits a mouse button state of pressed.
    /// @param [in] button Mouse button that is affected.
    void SubmitMouseButtonPressedState(EMouseButton button);

    /// Submits a mouse button state of released.
    /// @param [in] button Mouse button that is affected.
    void SubmitMouseButtonReleasedState(EMouseButton button);

  private:

    /// Holds the state of the virtual mouse that is represented by this object.
    Xidi::BitSetEnum<EMouseButton> virtualMouseButtonState;

    /// Holds the most recent virtual mouse movement contribution received from each opaque
    /// source identifier, one map per mouse axis.
    std::array<std::unordered_map<uint32_t, int>, (unsigned int)EMouseAxis::Count>
        virtualMouseMovementContributionBySource;
  };
} // namespace XidiTest
