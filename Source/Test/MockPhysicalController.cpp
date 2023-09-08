/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 ***********************************************************************************************//**
 * @file MockPhysicalController.cpp
 *   Implementation of a mock version of the physical controller interface along with additional
 *   testing-specific functions.
 **************************************************************************************************/

#include "TestCase.h"

#include "MockPhysicalController.h"

#include <shared_mutex>
#include <stop_token>
#include <vector>

#include "ApiWindows.h"
#include "ForceFeedbackDevice.h"
#include "Mapper.h"
#include "PhysicalController.h"
#include "VirtualController.h"

namespace XidiTest
{
  using namespace ::Xidi::Controller;

  /// Default physical state to return when no other physical states are specified.
  static constexpr SPhysicalState kDefaultPhysicalState = {
      .deviceStatus = EPhysicalDeviceStatus::Ok};

  /// Guards all mock physical state data structures, one per physical controller.
  /// Even in tests, additional threads may exist to wait for state changes.
  static std::shared_mutex mockPhysicalStateGuard[kPhysicalControllerCount];

  /// Holds pointers to all mock physical controller objects, one per physical controller.
  /// Each such object governs the behavior of the physical controller interface for a given
  /// physical controller.
  static MockPhysicalController* mockPhysicalController[kPhysicalControllerCount];

  MockPhysicalController::MockPhysicalController(
      TControllerIdentifier controllerIdentifier,
      const Mapper& mapper,
      const SPhysicalState* mockPhysicalStates,
      size_t mockPhysicalStateCount)
      : kControllerIdentifier(controllerIdentifier),
        kMockPhysicalStates(mockPhysicalStates),
        kMockPhysicalStateCount(mockPhysicalStateCount),
        currentPhysicalStateIndex(0),
        advanceRequested(false),
        forceFeedbackDevice(0),
        mapper(mapper),
        forceFeedbackRegistration()
  {
    if (controllerIdentifier >= kPhysicalControllerCount)
      TEST_FAILED_BECAUSE(
          L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

    std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

    if (nullptr != mockPhysicalController[kControllerIdentifier])
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to multiple attempts to control physical controller with identifier %u.",
          __FUNCTIONW__,
          controllerIdentifier);

    mockPhysicalController[kControllerIdentifier] = this;
  }

  MockPhysicalController::~MockPhysicalController(void)
  {
    std::unique_lock lock(mockPhysicalStateGuard[kControllerIdentifier]);
    mockPhysicalController[kControllerIdentifier] = nullptr;
  }

  void MockPhysicalController::AdvancePhysicalState(void)
  {
    if (kMockPhysicalStateCount == currentPhysicalStateIndex)
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to out-of-bounds physical state advancement for physical controller with identifier %u.",
          __FUNCTIONW__,
          kControllerIdentifier);

    currentPhysicalStateIndex += 1;
    advanceRequested = false;
  }

  SCapabilities MockPhysicalController::GetControllerCapabilities(void) const
  {
    return mapper.GetCapabilities();
  }

  SPhysicalState MockPhysicalController::GetCurrentPhysicalState(void) const
  {
    if (nullptr != kMockPhysicalStates) return kMockPhysicalStates[currentPhysicalStateIndex];

    return kDefaultPhysicalState;
  }

  SState MockPhysicalController::GetCurrentRawVirtualState(void) const
  {
    return mapper.MapStatePhysicalToVirtual(GetCurrentPhysicalState(), kControllerIdentifier);
  }

  void MockPhysicalController::RequestAdvancePhysicalState(void)
  {
    std::unique_lock lock(mockPhysicalStateGuard[kControllerIdentifier]);

    if (currentPhysicalStateIndex >= (kMockPhysicalStateCount - 1))
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to out-of-bounds physical state advancement for physical controller with identifier %u.",
          __FUNCTIONW__,
          kControllerIdentifier);

    if (true == advanceRequested)
      TEST_FAILED_BECAUSE(
          L"%s: Test implementation error due to unconsumed advance request for physical controller with identifier %u.",
          __FUNCTIONW__,
          kControllerIdentifier);

    advanceRequested = true;
  }
} // namespace XidiTest

namespace Xidi
{
  namespace Controller
  {
    using namespace ::XidiTest;

    SCapabilities GetControllerCapabilities(TControllerIdentifier controllerIdentifier)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

      std::shared_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

      if (nullptr != mockPhysicalController[controllerIdentifier])
        return mockPhysicalController[controllerIdentifier]->GetControllerCapabilities();
      else
        TEST_FAILED_BECAUSE(
            L"%s: No mock physical controller associated with identifier %u.",
            __FUNCTIONW__,
            controllerIdentifier);
    }

    SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

      std::shared_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

      if (nullptr != mockPhysicalController[controllerIdentifier])
        return mockPhysicalController[controllerIdentifier]->GetCurrentPhysicalState();
      else
        TEST_FAILED_BECAUSE(
            L"%s: No mock physical controller associated with identifier %u.",
            __FUNCTIONW__,
            controllerIdentifier);
    }

    SState GetCurrentRawVirtualControllerState(TControllerIdentifier controllerIdentifier)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

      std::shared_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

      if (nullptr != mockPhysicalController[controllerIdentifier])
        return mockPhysicalController[controllerIdentifier]->GetCurrentRawVirtualState();
      else
        TEST_FAILED_BECAUSE(
            L"%s: No mock physical controller associated with identifier %u.",
            __FUNCTIONW__,
            controllerIdentifier);
    }

    ForceFeedback::Device* PhysicalControllerForceFeedbackRegister(
        TControllerIdentifier controllerIdentifier, const VirtualController* virtualController)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

      std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

      if (nullptr != mockPhysicalController[controllerIdentifier])
      {
        mockPhysicalController[controllerIdentifier]->InsertForceFeedbackRegistration(
            virtualController);
        return &mockPhysicalController[controllerIdentifier]->GetForceFeedbackDevice();
      }

      return nullptr;
    }

    void PhysicalControllerForceFeedbackUnregister(
        TControllerIdentifier controllerIdentifier, const VirtualController* virtualController)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

      std::unique_lock lock(mockPhysicalStateGuard[controllerIdentifier]);

      if (nullptr != mockPhysicalController[controllerIdentifier])
      {
        mockPhysicalController[controllerIdentifier]->EraseForceFeedbackRegistration(
            virtualController);
      }
    }

    bool WaitForPhysicalControllerStateChange(
        TControllerIdentifier controllerIdentifier,
        SPhysicalState& state,
        std::stop_token stopToken)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

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

              SPhysicalState newState =
                  mockPhysicalController[controllerIdentifier]->GetCurrentPhysicalState();
              if (newState != state)
              {
                state = newState;
                return true;
              }
            }
          }
        }
      }

      return false;
    }

    bool WaitForRawVirtualControllerStateChange(
        TControllerIdentifier controllerIdentifier, SState& state, std::stop_token stopToken)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
        TEST_FAILED_BECAUSE(
            L"%s: Invalid controller identifier (%u).", __FUNCTIONW__, controllerIdentifier);

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

              SState newState =
                  mockPhysicalController[controllerIdentifier]->GetCurrentRawVirtualState();
              if (newState != state)
              {
                state = newState;
                return true;
              }
            }
          }
        }
      }

      return false;
    }
  } // namespace Controller
} // namespace Xidi
