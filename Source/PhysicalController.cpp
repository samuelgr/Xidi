/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
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

#include <SDL3/SDL.h>

#include <Infra/Core/Message.h>

#include "ApiWindows.h"
#include "ConcurrencyWrapper.h"
#include "ControllerTypes.h"
#include "ForceFeedbackDevice.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "ImportApiXInput.h"
#include "Mapper.h"
#include "Strings.h"
#include "VirtualController.h"

namespace Xidi
{
  namespace Controller
  {
    /// Underlying SDL joystick ID for controller.
    static SDL_Gamepad* physicalControllerSdlIdentifier[kPhysicalControllerCount];

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
      SDL_Gamepad* gamepad = physicalControllerSdlIdentifier[controllerIdentifier];
      if (gamepad == nullptr)
        return {.deviceStatus = EPhysicalDeviceStatus::Error}; 

      if (!SDL_GamepadConnected(gamepad))
      {
        SDL_CloseGamepad(gamepad);
        return {.deviceStatus = EPhysicalDeviceStatus::NotConnected};
      }
        
      return {
        .deviceStatus = EPhysicalDeviceStatus::Ok,
        .stick =
            {SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX),
            SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY),
            SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX),
            SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY)},
        .trigger =
            {SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER),
            SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)},
        .button = [&]() -> std::bitset<16>
          {
            std::bitset<16> button;
            for (int sdlButton = SDL_GAMEPAD_BUTTON_INVALID + 1;
                    sdlButton <= SDL_GAMEPAD_BUTTON_MISC1;
                    sdlButton++)
            {
              button.set(
                sdlButton,
                SDL_GetGamepadButton(gamepad, static_cast<SDL_GamepadButton>(sdlButton)));
            }
            return button;
          }()
      };
    }

    static_assert((unsigned int)EPhysicalButton::A == SDL_GAMEPAD_BUTTON_SOUTH);
    static_assert((unsigned int)EPhysicalButton::B == SDL_GAMEPAD_BUTTON_EAST);
    static_assert((unsigned int)EPhysicalButton::X == SDL_GAMEPAD_BUTTON_WEST);
    static_assert((unsigned int)EPhysicalButton::Y == SDL_GAMEPAD_BUTTON_NORTH);
    static_assert((unsigned int)EPhysicalButton::Back == SDL_GAMEPAD_BUTTON_BACK);
    static_assert((unsigned int)EPhysicalButton::Start == SDL_GAMEPAD_BUTTON_START);
    static_assert((unsigned int)EPhysicalButton::LS == SDL_GAMEPAD_BUTTON_LEFT_STICK);
    static_assert((unsigned int)EPhysicalButton::RS == SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    static_assert((unsigned int)EPhysicalButton::LB == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    static_assert((unsigned int)EPhysicalButton::RB == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    static_assert((unsigned int)EPhysicalButton::DpadUp == SDL_GAMEPAD_BUTTON_DPAD_UP);
    static_assert((unsigned int)EPhysicalButton::DpadDown == SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    static_assert((unsigned int)EPhysicalButton::DpadLeft == SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    static_assert((unsigned int)EPhysicalButton::DpadRight == SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

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
                  [Strings::kStrConfigurationSectionProperties]
                  [Strings::kStrConfigurationSettingPropertiesForceFeedbackEffectStrengthPercent]
                      .ValueOr(100)) /
          100.0;

      return SDL_RumbleGamepad(
          physicalControllerSdlIdentifier[controllerIdentifier],
          ScaledVibrationStrength(
              vibration.leftMotor, kForceFeedbackEffectStrengthScalingFactor),
          ScaledVibrationStrength(
              vibration.rightMotor, kForceFeedbackEffectStrengthScalingFactor),
          250);
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
       
        SDL_UpdateGamepads();
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
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
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
                Infra::Message::OutputFormatted(
                    Infra::Message::ESeverity::Info,
                    L"Physical controller %u: Hardware connected.",
                    (1 + controllerIdentifier));
                break;

              default:
                Infra::Message::OutputFormatted(
                    Infra::Message::ESeverity::Warning,
                    L"Physical controller %u: Cleared previous error condition.",
                    (1 + controllerIdentifier));
                break;
            }
            break;

          case EPhysicalDeviceStatus::NotConnected:
            if (newPhysicalState.deviceStatus != oldPhysicalState.deviceStatus)
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Physical controller %u: Hardware disconnected.",
                  (1 + controllerIdentifier));
            break;

          default:
            if (newPhysicalState.deviceStatus != oldPhysicalState.deviceStatus)
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Warning,
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
      // There is overhead to using call_once, even after the operation is completed, and physical
      // controller functions are called frequently. Using this additional flag avoids that overhead
      // in the common case.
      static bool isInitialized = false;
      if (true == isInitialized) return;

      static std::once_flag initFlag;
      std::call_once(
          initFlag,
          []() -> void
          {
            int gamepadCount;
            SDL_JoystickID* gamepads = SDL_GetGamepads(&gamepadCount);

            // Initialize controller state data structures.
            for (auto controllerIdentifier = 0;
                 controllerIdentifier < _countof(physicalControllerState);
                 ++controllerIdentifier)
            {
              if (controllerIdentifier <= gamepadCount)
              {
                physicalControllerSdlIdentifier[controllerIdentifier] =
                    SDL_OpenGamepad(gamepads[controllerIdentifier]);
              }
              else
                physicalControllerSdlIdentifier[controllerIdentifier] = nullptr;

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
                Infra::Message::OutputFormatted(
                    Infra::Message::ESeverity::Info,
                    L"Set the system timer resolution to %u ms.",
                    timeCaps.wPeriodMin);
              else
                Infra::Message::OutputFormatted(
                    Infra::Message::ESeverity::Warning,
                    L"Failed with code %u to set the system timer resolution.",
                    timeResult);
            }
            else
            {
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Warning,
                  L"Failed with code %u to obtain system timer resolution information.",
                  timeResult);
            }

            // Create and start the polling threads.
            for (auto controllerIdentifier = 0; controllerIdentifier < kPhysicalControllerCount;
                 ++controllerIdentifier)
            {
              std::thread(PollForPhysicalControllerStateChanges, controllerIdentifier).detach();
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
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
              Infra::Message::OutputFormatted(
                  Infra::Message::ESeverity::Info,
                  L"Initialized the physical controller force feedback actuation thread for controller %u. Desired actuation period is %u ms.",
                  (unsigned int)(1 + controllerIdentifier),
                  kPhysicalForceFeedbackPeriodMilliseconds);
            }

            // Create and start the physical controller hardware status monitoring threads, but only
            // if the messages generated by those threads will actually be delivered as output.
            if (Infra::Message::WillOutputMessageOfSeverity(Infra::Message::ESeverity::Warning))
            {
              for (auto controllerIdentifier = 0; controllerIdentifier < kPhysicalControllerCount;
                   ++controllerIdentifier)
              {
                std::thread(MonitorPhysicalControllerStatus, controllerIdentifier).detach();
                Infra::Message::OutputFormatted(
                    Infra::Message::ESeverity::Info,
                    L"Initialized the physical controller hardware status monitoring thread for controller %u.",
                    (unsigned int)(1 + controllerIdentifier));
              }
            }

            isInitialized = true;
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
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
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
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Error,
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
