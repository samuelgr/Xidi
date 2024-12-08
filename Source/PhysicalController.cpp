/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file PhysicalController.cpp
 *   Implementation of all functionality for communicating with physical controllers.
 **************************************************************************************************/

#include "PhysicalController.h"

#include <cstdint>
#include <mutex>
#include <set>
#include <stop_token>
#include <thread>

#include "ApiWindows.h"
#include "ConcurrencyWrapper.h"
#include "ControllerTypes.h"
#include "ForceFeedbackDevice.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "ImportApiXInput.h"
#include "Mapper.h"
#include "Message.h"
#include "Strings.h"
#include "VirtualController.h"

namespace Xidi
{
  namespace Controller
  {
    /// Raw physical state data for each of the possible physical controllers.
    static ConcurrencyWrapper<SPhysicalState> physicalControllerState[kPhysicalControllerCount];

    /// State data for each of the possible physical controllers after it is passed through a mapper
    /// but without any further processing.
    static ConcurrencyWrapper<SState> rawVirtualControllerState[kPhysicalControllerCount];

    /// Per-controller force feedback device buffer objects.
    /// These objects are not safe for dynamic initialization, so they are initialized later by
    /// pointer.
    static ForceFeedback::Device* physicalControllerForceFeedbackBuffer;

    /// Pointers to the virtual controller objects registered for force feedback with each physical
    /// controller.
    static std::set<const VirtualController*>
        physicalControllerForceFeedbackRegistration[kPhysicalControllerCount];

    /// Mutex objects for protecting against concurrent accesses to the physical controller force
    /// feedback registration data.
    static std::mutex physicalControllerForceFeedbackMutex[kPhysicalControllerCount];

    /// Computes an opaque source identifier from a given controller identifier.
    /// @param [in] controllerIdentifier Identifier of the physical controller for which an
    /// identifier is needed.
    /// @return Opaque identifier that can be passed to mappers.
    static inline uint32_t OpaqueControllerSourceIdentifier(
        TControllerIdentifier controllerIdentifier)
    {
      return (uint32_t)controllerIdentifier;
    }

    /// Reads physical controller state.
    /// @param [in] controllerIdentifier Identifier of the controller on which to operate.
    /// @return Physical state of the identified controller.
    static SPhysicalState ReadPhysicalControllerState(TControllerIdentifier controllerIdentifier)
    {
      constexpr uint16_t kUnusedButtonMask =
          ~((uint16_t)((1u << (unsigned int)EPhysicalButton::UnusedGuide) |
                       (1u << (unsigned int)EPhysicalButton::UnusedShare)));

      XINPUT_STATE xinputState;
      DWORD xinputGetStateResult =
          ImportApiXInput::XInputGetState(controllerIdentifier, &xinputState);

      switch (xinputGetStateResult)
      {
        case ERROR_SUCCESS:
          // Directly using wButtons assumes that the bit layout is the same between the internal
          // bitset and the XInput data structure. The static assertions below this function verify
          // this assumption and will cause a compiler error if it is wrong.
          return {
              .deviceStatus = EPhysicalDeviceStatus::Ok,
              .stick =
                  {xinputState.Gamepad.sThumbLX,
                   xinputState.Gamepad.sThumbLY,
                   xinputState.Gamepad.sThumbRX,
                   xinputState.Gamepad.sThumbRY},
              .trigger = {xinputState.Gamepad.bLeftTrigger, xinputState.Gamepad.bRightTrigger},
              .button = (uint16_t)(xinputState.Gamepad.wButtons & kUnusedButtonMask)};

        case ERROR_DEVICE_NOT_CONNECTED:
          return {.deviceStatus = EPhysicalDeviceStatus::NotConnected};

        default:
          return {.deviceStatus = EPhysicalDeviceStatus::Error};
      }
    }

    static_assert(1u << (unsigned int)EPhysicalButton::DpadUp == XINPUT_GAMEPAD_DPAD_UP);
    static_assert(1u << (unsigned int)EPhysicalButton::DpadDown == XINPUT_GAMEPAD_DPAD_DOWN);
    static_assert(1u << (unsigned int)EPhysicalButton::DpadLeft == XINPUT_GAMEPAD_DPAD_LEFT);
    static_assert(1u << (unsigned int)EPhysicalButton::DpadRight == XINPUT_GAMEPAD_DPAD_RIGHT);
    static_assert(1u << (unsigned int)EPhysicalButton::Start == XINPUT_GAMEPAD_START);
    static_assert(1u << (unsigned int)EPhysicalButton::Back == XINPUT_GAMEPAD_BACK);
    static_assert(1u << (unsigned int)EPhysicalButton::LS == XINPUT_GAMEPAD_LEFT_THUMB);
    static_assert(1u << (unsigned int)EPhysicalButton::RS == XINPUT_GAMEPAD_RIGHT_THUMB);
    static_assert(1u << (unsigned int)EPhysicalButton::LB == XINPUT_GAMEPAD_LEFT_SHOULDER);
    static_assert(1u << (unsigned int)EPhysicalButton::RB == XINPUT_GAMEPAD_RIGHT_SHOULDER);
    static_assert(1u << (unsigned int)EPhysicalButton::A == XINPUT_GAMEPAD_A);
    static_assert(1u << (unsigned int)EPhysicalButton::B == XINPUT_GAMEPAD_B);
    static_assert(1u << (unsigned int)EPhysicalButton::X == XINPUT_GAMEPAD_X);
    static_assert(1u << (unsigned int)EPhysicalButton::Y == XINPUT_GAMEPAD_Y);

    /// Scales a vibration strength value by the specified scaling factor. If the resulting strength
    /// exceeds the maximum possible strength it is saturated at the maximum possible strength.
    /// @param [in] vibrationStrength Physical motor vibration strength value.
    /// @param [in] scalingFactor Scaling factor by which to scale up or down the physical motor
    /// vibration strength value.
    /// @return Scaled physical motor vibration strength value that can then be sent directly to the
    /// physical motor.
    static ForceFeedback::TPhysicalActuatorValue ScaledVibrationStrength(
        ForceFeedback::TPhysicalActuatorValue vibrationStrength, double scalingFactor)
    {
      if (0.0 == scalingFactor)
        return 0;
      else if (1.0 == scalingFactor)
        return static_cast<ForceFeedback::TPhysicalActuatorValue>(vibrationStrength);

      constexpr double kMaxVibrationStrength =
          static_cast<double>(std::numeric_limits<ForceFeedback::TPhysicalActuatorValue>::max());
      const double scaledVibrationStrength = static_cast<double>(vibrationStrength) * scalingFactor;

      return static_cast<ForceFeedback::TPhysicalActuatorValue>(
          std::min(scaledVibrationStrength, kMaxVibrationStrength));
    }

    /// Writes a vibration command to a physical controller.
    /// @param [in] controllerIdentifier Identifier of the controller on which to operate.
    /// @param [in] vibration Physical actuator vibration vector.
    /// @return `true` if successful, `false` otherwise.
    static bool WritePhysicalControllerVibration(
        TControllerIdentifier controllerIdentifier,
        ForceFeedback::SPhysicalActuatorComponents vibration)
    {
      static const double kForceFeedbackEffectStrengthScalingFactor =
          static_cast<double>(
              Globals::GetConfigurationData()
                  .GetFirstIntegerValue(
                      Strings::kStrConfigurationSectionProperties,
                      Strings::kStrConfigurationSettingPropertiesForceFeedbackEffectStrengthPercent)
                  .value_or(100)) /
          100.0;

      // Impulse triggers are ignored because the XInput API does not support them.
      XINPUT_VIBRATION xinputVibration = {
          .wLeftMotorSpeed = ScaledVibrationStrength(
              vibration.leftMotor, kForceFeedbackEffectStrengthScalingFactor),
          .wRightMotorSpeed = ScaledVibrationStrength(
              vibration.rightMotor, kForceFeedbackEffectStrengthScalingFactor)};
      return (
          ERROR_SUCCESS ==
          ImportApiXInput::XInputSetState((DWORD)controllerIdentifier, &xinputVibration));
    }

    /// Periodically plays force feedback effects on the physical controller actuators.
    /// @param [in] controllerIdentifier Identifier of the controller on which to operate.
    static void ForceFeedbackActuateEffects(TControllerIdentifier controllerIdentifier)
    {
      constexpr ForceFeedback::TOrderedMagnitudeComponents kVirtualMagnitudeVectorZero = {};

      ForceFeedback::SPhysicalActuatorComponents previousPhysicalActuatorValues;
      ForceFeedback::SPhysicalActuatorComponents currentPhysicalActuatorValues;

      const Mapper* mapper = Mapper::GetConfigured(controllerIdentifier);
      bool lastActuationResult = true;

      while (true)
      {
        if (true == lastActuationResult)
          Sleep(kPhysicalForceFeedbackPeriodMilliseconds);
        else
          Sleep(kPhysicalErrorBackoffPeriodMilliseconds);

        if (true == Globals::DoesCurrentProcessHaveInputFocus())
        {
          ForceFeedback::TEffectValue overallEffectGain = 10000;
          ForceFeedback::SPhysicalActuatorComponents physicalActuatorVector = {};
          ForceFeedback::TOrderedMagnitudeComponents virtualMagnitudeVector =
              physicalControllerForceFeedbackBuffer[controllerIdentifier].PlayEffects();

          if (kVirtualMagnitudeVectorZero != virtualMagnitudeVector)
          {
            std::unique_lock lock(physicalControllerForceFeedbackMutex[controllerIdentifier]);

            // Gain is modified downwards by each virtual controller object.
            // Typically there would only be one, in which case the properties of that object would
            // be effective. Otherwise this loop is essentially modeled as multiple volume knobs
            // connected in sequence, each lowering the volume of the effects by the value of its
            // own device-wide gain property.
            for (auto virtualController :
                 physicalControllerForceFeedbackRegistration[controllerIdentifier])
              overallEffectGain *=
                  ((ForceFeedback::TEffectValue)virtualController->GetForceFeedbackGain() /
                   ForceFeedback::kEffectModifierMaximum);

            physicalActuatorVector = mapper->MapForceFeedbackVirtualToPhysical(
                virtualMagnitudeVector, overallEffectGain);
          }

          currentPhysicalActuatorValues = physicalActuatorVector;
        }
        else
        {
          currentPhysicalActuatorValues = {};
        }

        if (previousPhysicalActuatorValues != currentPhysicalActuatorValues)
        {
          lastActuationResult =
              WritePhysicalControllerVibration(controllerIdentifier, currentPhysicalActuatorValues);
          previousPhysicalActuatorValues = currentPhysicalActuatorValues;
        }
        else
        {
          lastActuationResult = true;
        }
      }
    }

    /// Periodically polls for physical controller state.
    /// On detected state change, updates the internal data structure and notifies all waiting
    /// threads.
    /// @param [in] controllerIdentifier Identifier of the controller on which to operate.
    static void PollForPhysicalControllerStateChanges(TControllerIdentifier controllerIdentifier)
    {
      SPhysicalState newPhysicalState = physicalControllerState[controllerIdentifier].Get();

      while (true)
      {
        if (EPhysicalDeviceStatus::Ok == newPhysicalState.deviceStatus)
          Sleep(kPhysicalPollingPeriodMilliseconds);
        else
          Sleep(kPhysicalErrorBackoffPeriodMilliseconds);

        newPhysicalState = ReadPhysicalControllerState(controllerIdentifier);

        if (true == physicalControllerState[controllerIdentifier].Update(newPhysicalState))
        {
          const SState newRawVirtualState =
              ((EPhysicalDeviceStatus::Ok == newPhysicalState.deviceStatus)
                   ? Mapper::GetConfigured(controllerIdentifier)
                         ->MapStatePhysicalToVirtual(
                             newPhysicalState,
                             OpaqueControllerSourceIdentifier(controllerIdentifier))
                   : Mapper::GetConfigured(controllerIdentifier)
                         ->MapNeutralPhysicalToVirtual(
                             OpaqueControllerSourceIdentifier(controllerIdentifier)));

          rawVirtualControllerState[controllerIdentifier].Update(newRawVirtualState);
        }
      }
    }

    /// Monitors physical controller status for events like hardware connection or disconnection and
    /// error conditions. Used exclusively for logging. Intended to be a thread entry point, one
    /// thread per monitored physical controller.
    /// @param [in] controllerIdentifier Identifier of the controller to monitor.
    static void MonitorPhysicalControllerStatus(TControllerIdentifier controllerIdentifier)
    {
      if (controllerIdentifier >= kPhysicalControllerCount)
      {
        Message::OutputFormatted(
            Message::ESeverity::Error,
            L"Attempted to monitor physical controller with invalid identifier %u.",
            controllerIdentifier);
        return;
      }

      SPhysicalState oldPhysicalState = GetCurrentPhysicalControllerState(controllerIdentifier);
      SPhysicalState newPhysicalState = oldPhysicalState;

      while (true)
      {
        WaitForPhysicalControllerStateChange(
            controllerIdentifier, newPhysicalState, std::stop_token());

        // Look for status changes and output to the log, as appropriate.
        switch (newPhysicalState.deviceStatus)
        {
          case EPhysicalDeviceStatus::Ok:
            switch (oldPhysicalState.deviceStatus)
            {
              case EPhysicalDeviceStatus::Ok:
                break;

              case EPhysicalDeviceStatus::NotConnected:
                Message::OutputFormatted(
                    Message::ESeverity::Info,
                    L"Physical controller %u: Hardware connected.",
                    (1 + controllerIdentifier));
                break;

              default:
                Message::OutputFormatted(
                    Message::ESeverity::Warning,
                    L"Physical controller %u: Cleared previous error condition.",
                    (1 + controllerIdentifier));
                break;
            }
            break;

          case EPhysicalDeviceStatus::NotConnected:
            if (newPhysicalState.deviceStatus != oldPhysicalState.deviceStatus)
              Message::OutputFormatted(
                  Message::ESeverity::Info,
                  L"Physical controller %u: Hardware disconnected.",
                  (1 + controllerIdentifier));
            break;

          default:
            if (newPhysicalState.deviceStatus != oldPhysicalState.deviceStatus)
              Message::OutputFormatted(
                  Message::ESeverity::Warning,
                  L"Physical controller %u: Encountered an error condition.",
                  (1 + controllerIdentifier));
            break;
        }

        oldPhysicalState = newPhysicalState;
      }
    }

    /// Initializes internal data structures and creates worker threads.
    /// Idempotent and concurrency-safe.
    static void Initialize(void)
    {
      static std::once_flag initFlag;
      std::call_once(
          initFlag,
          []() -> void
          {
            // Initialize controller state data structures.
            for (auto controllerIdentifier = 0;
                 controllerIdentifier < _countof(physicalControllerState);
                 ++controllerIdentifier)
            {
              const SPhysicalState initialPhysicalState =
                  ReadPhysicalControllerState(controllerIdentifier);
              const SState initialRawVirtualState =
                  Mapper::GetConfigured(controllerIdentifier)
                      ->MapStatePhysicalToVirtual(
                          initialPhysicalState,
                          OpaqueControllerSourceIdentifier(controllerIdentifier));

              physicalControllerState[controllerIdentifier].Set(initialPhysicalState);
              rawVirtualControllerState[controllerIdentifier].Set(initialRawVirtualState);
            }

            // Ensure the system timer resolution is suitable for the desired polling frequency.
            TIMECAPS timeCaps;
            MMRESULT timeResult = ImportApiWinMM::timeGetDevCaps(&timeCaps, sizeof(timeCaps));
            if (MMSYSERR_NOERROR == timeResult)
            {
              timeResult = ImportApiWinMM::timeBeginPeriod(timeCaps.wPeriodMin);

              if (MMSYSERR_NOERROR == timeResult)
                Message::OutputFormatted(
                    Message::ESeverity::Info,
                    L"Set the system timer resolution to %u ms.",
                    timeCaps.wPeriodMin);
              else
                Message::OutputFormatted(
                    Message::ESeverity::Warning,
                    L"Failed with code %u to set the system timer resolution.",
                    timeResult);
            }
            else
            {
              Message::OutputFormatted(
                  Message::ESeverity::Warning,
                  L"Failed with code %u to obtain system timer resolution information.",
                  timeResult);
            }

            // Create and start the polling threads.
            for (auto controllerIdentifier = 0; controllerIdentifier < kPhysicalControllerCount;
                 ++controllerIdentifier)
            {
              std::thread(PollForPhysicalControllerStateChanges, controllerIdentifier).detach();
              Message::OutputFormatted(
                  Message::ESeverity::Info,
                  L"Initialized the physical controller state polling thread for controller %u. Desired polling period is %u ms.",
                  (unsigned int)(1 + controllerIdentifier),
                  kPhysicalPollingPeriodMilliseconds);
            }

            // Allocate the force feedback device buffers, then create and start the force feedback
            // threads.
            physicalControllerForceFeedbackBuffer =
                new ForceFeedback::Device[kPhysicalControllerCount];
            for (auto controllerIdentifier = 0; controllerIdentifier < kPhysicalControllerCount;
                 ++controllerIdentifier)
            {
              std::thread(ForceFeedbackActuateEffects, controllerIdentifier).detach();
              Message::OutputFormatted(
                  Message::ESeverity::Info,
                  L"Initialized the physical controller force feedback actuation thread for controller %u. Desired actuation period is %u ms.",
                  (unsigned int)(1 + controllerIdentifier),
                  kPhysicalForceFeedbackPeriodMilliseconds);
            }

            // Create and start the physical controller hardware status monitoring threads, but only
            // if the messages generated by those threads will actually be delivered as output.
            if (Message::WillOutputMessageOfSeverity(Message::ESeverity::Warning))
            {
              for (auto controllerIdentifier = 0; controllerIdentifier < kPhysicalControllerCount;
                   ++controllerIdentifier)
              {
                std::thread(MonitorPhysicalControllerStatus, controllerIdentifier).detach();
                Message::OutputFormatted(
                    Message::ESeverity::Info,
                    L"Initialized the physical controller hardware status monitoring thread for controller %u.",
                    (unsigned int)(1 + controllerIdentifier));
              }
            }
          });
    }

    SCapabilities GetControllerCapabilities(TControllerIdentifier controllerIdentifier)
    {
      Initialize();
      return Mapper::GetConfigured(controllerIdentifier)->GetCapabilities();
    }

    SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier)
    {
      Initialize();
      return physicalControllerState[controllerIdentifier].Get();
    }

    SState GetCurrentRawVirtualControllerState(TControllerIdentifier controllerIdentifier)
    {
      Initialize();
      return rawVirtualControllerState[controllerIdentifier].Get();
    }

    ForceFeedback::Device* PhysicalControllerForceFeedbackRegister(
        TControllerIdentifier controllerIdentifier, const VirtualController* virtualController)
    {
      Initialize();

      if (controllerIdentifier >= kPhysicalControllerCount)
      {
        Message::OutputFormatted(
            Message::ESeverity::Error,
            L"Attempted to register with a physical controller for force feedback with invalid identifier %u.",
            controllerIdentifier);
        return nullptr;
      }

      std::unique_lock lock(physicalControllerForceFeedbackMutex[controllerIdentifier]);
      physicalControllerForceFeedbackRegistration[controllerIdentifier].insert(virtualController);

      return &physicalControllerForceFeedbackBuffer[controllerIdentifier];
    }

    void PhysicalControllerForceFeedbackUnregister(
        TControllerIdentifier controllerIdentifier, const VirtualController* virtualController)
    {
      Initialize();

      if (controllerIdentifier >= kPhysicalControllerCount)
      {
        Message::OutputFormatted(
            Message::ESeverity::Error,
            L"Attempted to unregister with a physical controller for force feedback with invalid identifier %u.",
            controllerIdentifier);
        return;
      }

      std::unique_lock lock(physicalControllerForceFeedbackMutex[controllerIdentifier]);
      physicalControllerForceFeedbackRegistration[controllerIdentifier].erase(virtualController);
    }

    bool WaitForPhysicalControllerStateChange(
        TControllerIdentifier controllerIdentifier,
        SPhysicalState& state,
        std::stop_token stopToken)
    {
      Initialize();

      if (controllerIdentifier >= kPhysicalControllerCount) return false;

      return physicalControllerState[controllerIdentifier].WaitForUpdate(state, stopToken);
    }

    bool WaitForRawVirtualControllerStateChange(
        TControllerIdentifier controllerIdentifier, SState& state, std::stop_token stopToken)
    {
      Initialize();

      if (controllerIdentifier >= kPhysicalControllerCount) return false;

      return rawVirtualControllerState[controllerIdentifier].WaitForUpdate(state, stopToken);
    }
  } // namespace Controller
} // namespace Xidi
