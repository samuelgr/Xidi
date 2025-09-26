/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ElementMapper.cpp
 *   Implementation of functionality used to implement mappings from individual XInput controller
 *   elements to virtual DirectInput controller elements.
 **************************************************************************************************/

#include "ElementMapper.h"

#include <cmath>
#include <cstdint>
#include <optional>

#include "ControllerMath.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Strings.h"

namespace Xidi
{
  namespace Controller
  {
    std::unique_ptr<IElementMapper> AxisMapper::Clone(void) const
    {
      return std::make_unique<AxisMapper>(*this);
    }

    void AxisMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      int32_t axisValueToContribute = (int32_t)analogValue;

      switch (direction)
      {
        case EAxisDirection::Both:
          break;

        case EAxisDirection::Positive:
          axisValueToContribute = (axisValueToContribute - kAnalogValueMin) >> 1;
          break;

        case EAxisDirection::Negative:
          axisValueToContribute = (axisValueToContribute - kAnalogValueMax) >> 1;
          break;
      }

      controllerState[axis] += axisValueToContribute;
    }

    void AxisMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      int32_t axisValueToContribute = 0;

      switch (direction)
      {
        case EAxisDirection::Both:
          axisValueToContribute = (buttonPressed ? kAnalogValueMax : kAnalogValueMin);
          break;

        case EAxisDirection::Positive:
          axisValueToContribute = (buttonPressed ? kAnalogValueMax : kAnalogValueNeutral);
          break;

        case EAxisDirection::Negative:
          axisValueToContribute = (buttonPressed ? kAnalogValueMin : kAnalogValueNeutral);
          break;
      }

      controllerState[axis] += axisValueToContribute;
    }

    void AxisMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      constexpr double kBidirectionalStepSize = (double)(kAnalogValueMax - kAnalogValueMin) /
          (double)(kTriggerValueMax - kTriggerValueMin);
      constexpr double kPositiveStepSize =
          (double)kAnalogValueMax / (double)(kTriggerValueMax - kTriggerValueMin);
      constexpr double kNegativeStepSize =
          (double)kAnalogValueMin / (double)(kTriggerValueMax - kTriggerValueMin);

      int32_t axisValueToContribute = 0;

      switch (direction)
      {
        case EAxisDirection::Both:
          axisValueToContribute =
              (int32_t)((double)triggerValue * kBidirectionalStepSize) + kAnalogValueMin;
          break;

        case EAxisDirection::Positive:
          axisValueToContribute =
              (int32_t)((double)triggerValue * kPositiveStepSize) + kAnalogValueNeutral;
          break;

        case EAxisDirection::Negative:
          axisValueToContribute =
              (int32_t)((double)triggerValue * kNegativeStepSize) - kAnalogValueNeutral;
          break;
      }

      controllerState[axis] += axisValueToContribute;
    }

    int AxisMapper::GetTargetElementCount(void) const
    {
      return 1;
    }

    std::optional<SElementIdentifier> AxisMapper::GetTargetElementAt(int index) const
    {
      if (0 != index) return std::nullopt;

      return SElementIdentifier({.type = EElementType::Axis, .axis = axis});
    }

    std::unique_ptr<IElementMapper> ButtonMapper::Clone(void) const
    {
      return std::make_unique<ButtonMapper>(*this);
    }

    void ButtonMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      controllerState[button] = (controllerState[button] || Math::IsAnalogPressed(analogValue));
    }

    void ButtonMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      controllerState[button] = (controllerState[button] || buttonPressed);
    }

    void ButtonMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      controllerState[button] = (controllerState[button] || Math::IsTriggerPressed(triggerValue));
    }

    int ButtonMapper::GetTargetElementCount(void) const
    {
      return 1;
    }

    std::optional<SElementIdentifier> ButtonMapper::GetTargetElementAt(int index) const
    {
      if (0 != index) return std::nullopt;

      return SElementIdentifier({.type = EElementType::Button, .button = button});
    }

    std::unique_ptr<IElementMapper> CompoundMapper::Clone(void) const
    {
      return std::make_unique<CompoundMapper>(*this);
    }

    void CompoundMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      for (const auto& elementMapper : elementMappers)
      {
        if (nullptr != elementMapper)
          elementMapper->ContributeFromAnalogValue(controllerState, analogValue, sourceIdentifier);
      }
    }

    void CompoundMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      for (const auto& elementMapper : elementMappers)
      {
        if (nullptr != elementMapper)
          elementMapper->ContributeFromButtonValue(
              controllerState, buttonPressed, sourceIdentifier);
      }
    }

    void CompoundMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      for (const auto& elementMapper : elementMappers)
      {
        if (nullptr != elementMapper)
          elementMapper->ContributeFromTriggerValue(
              controllerState, triggerValue, sourceIdentifier);
      }
    }

    void CompoundMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
    {
      for (const auto& elementMapper : elementMappers)
      {
        if (nullptr != elementMapper)
          elementMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
    }

    int CompoundMapper::GetTargetElementCount(void) const
    {
      int numTargetElements = 0;

      for (const auto& elementMapper : elementMappers)
      {
        if (nullptr != elementMapper) numTargetElements += elementMapper->GetTargetElementCount();
      }

      return numTargetElements;
    }

    std::optional<SElementIdentifier> CompoundMapper::GetTargetElementAt(int index) const
    {
      int indexOffset = 0;

      for (const auto& elementMapper : elementMappers)
      {
        if (nullptr != elementMapper)
        {
          const int elementCount = elementMapper->GetTargetElementCount();

          if (index < (indexOffset + elementCount))
            return elementMapper->GetTargetElementAt(index - indexOffset);

          indexOffset += elementCount;
        }
      }

      return std::nullopt;
    }

    std::unique_ptr<IElementMapper> DigitalAxisMapper::Clone(void) const
    {
      return std::make_unique<DigitalAxisMapper>(*this);
    }

    void DigitalAxisMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      int32_t axisValueToContribute = 0;

      switch (GetAxisDirection())
      {
        case EAxisDirection::Both:
          if (Math::IsAnalogPressedNegative(analogValue))
            axisValueToContribute = kAnalogValueMin;
          else if (Math::IsAnalogPressedPositive(analogValue))
            axisValueToContribute = kAnalogValueMax;
          break;

        case EAxisDirection::Positive:
          if (Math::IsAnalogPressedPositive(analogValue)) axisValueToContribute = kAnalogValueMax;
          break;

        case EAxisDirection::Negative:
          if (Math::IsAnalogPressedNegative(analogValue)) axisValueToContribute = kAnalogValueMin;
          break;
      }

      controllerState[GetAxis()] += axisValueToContribute;
    }

    void DigitalAxisMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      ContributeFromButtonValue(
          controllerState, Math::IsTriggerPressed(triggerValue), sourceIdentifier);
    }

    std::unique_ptr<IElementMapper> InvertMapper::Clone(void) const
    {
      return std::make_unique<InvertMapper>(*this);
    }

    void InvertMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      if (nullptr != elementMapper)
      {
        const int32_t invertedAnalogValue =
            (kAnalogValueMax + kAnalogValueMin) - (int32_t)analogValue;
        elementMapper->ContributeFromAnalogValue(
            controllerState, (int16_t)invertedAnalogValue, sourceIdentifier);
      }
    }

    void InvertMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      if (nullptr != elementMapper)
      {
        const bool invertedButtonValue = !buttonPressed;
        elementMapper->ContributeFromButtonValue(
            controllerState, invertedButtonValue, sourceIdentifier);
      }
    }

    void InvertMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      if (nullptr != elementMapper)
      {
        const int32_t invertedTriggerValue =
            (kTriggerValueMax + kTriggerValueMin) - (int32_t)triggerValue;
        elementMapper->ContributeFromTriggerValue(
            controllerState, (uint8_t)invertedTriggerValue, sourceIdentifier);
      }
    }

    void InvertMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
    {
      if (nullptr != elementMapper)
        elementMapper->ContributeNeutral(controllerState, sourceIdentifier);
    }

    int InvertMapper::GetTargetElementCount(void) const
    {
      if (nullptr != elementMapper) return elementMapper->GetTargetElementCount();

      return 0;
    }

    std::optional<SElementIdentifier> InvertMapper::GetTargetElementAt(int index) const
    {
      if (nullptr != elementMapper) return elementMapper->GetTargetElementAt(index);

      return std::nullopt;
    }

    std::unique_ptr<IElementMapper> KeyboardMapper::Clone(void) const
    {
      return std::make_unique<KeyboardMapper>(*this);
    }

    void KeyboardMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsAnalogPressed(analogValue))
        Keyboard::SubmitKeyPressedState(key);
      else
        Keyboard::SubmitKeyReleasedState(key);
    }

    void KeyboardMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      if (true == buttonPressed)
        Keyboard::SubmitKeyPressedState(key);
      else
        Keyboard::SubmitKeyReleasedState(key);
    }

    void KeyboardMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsTriggerPressed(triggerValue))
        Keyboard::SubmitKeyPressedState(key);
      else
        Keyboard::SubmitKeyReleasedState(key);
    }

    void KeyboardMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
    {
      Keyboard::SubmitKeyReleasedState(key);
    }

    int KeyboardMapper::GetTargetElementCount(void) const
    {
      return 0;
    }

    std::optional<SElementIdentifier> KeyboardMapper::GetTargetElementAt(int index) const
    {
      return std::nullopt;
    }

    std::unique_ptr<IElementMapper> MouseAxisMapper::Clone(void) const
    {
      return std::make_unique<MouseAxisMapper>(*this);
    }

    void MouseAxisMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      static const bool kEnableMouseAxisProperites =
          Globals::GetConfigurationData()
              [Strings::kStrConfigurationSectionProperties]
              [Strings::kStrConfigurationSettingsPropertiesUseBuiltinProperties]
                  .ValueOr(true);

      constexpr double kAnalogToMouseScalingFactor =
          (double)(Mouse::kMouseMovementUnitsMax - Mouse::kMouseMovementUnitsMin) /
          (double)(kAnalogValueMax - kAnalogValueMin);

      constexpr unsigned int kAnalogMouseDeadzonePercent = 8;
      constexpr unsigned int kAnalogMouseSaturationPercent = 92;
      const int16_t analogValueForContribution =
          (kEnableMouseAxisProperites
               ? Math::ApplyRawAnalogTransform(
                     analogValue, kAnalogMouseDeadzonePercent, kAnalogMouseSaturationPercent)
               : analogValue);

      const double mouseAxisValueRaw =
          ((double)(analogValueForContribution - kAnalogValueNeutral) *
           kAnalogToMouseScalingFactor);
      const double mouseAxisValueTransformed = mouseAxisValueRaw;

      int mouseAxisValueToContribute = (int)mouseAxisValueTransformed;

      switch (direction)
      {
        case EAxisDirection::Both:
          break;

        case EAxisDirection::Positive:
          mouseAxisValueToContribute =
              (mouseAxisValueToContribute - Mouse::kMouseMovementUnitsMin) / 2;
          break;

        case EAxisDirection::Negative:
          mouseAxisValueToContribute =
              (mouseAxisValueToContribute - Mouse::kMouseMovementUnitsMax) / 2;
          break;
      }

      Mouse::SubmitMouseMovement(axis, mouseAxisValueToContribute, sourceIdentifier);
    }

    void MouseAxisMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      constexpr double kMouseButtonContributionScalingFactor = 0.5;

      int mouseAxisValueToContribute = Mouse::kMouseMovementUnitsNeutral;

      switch (direction)
      {
        case EAxisDirection::Both:
          mouseAxisValueToContribute +=
              (int)(kMouseButtonContributionScalingFactor *
                    (double)(buttonPressed ? (Mouse::kMouseMovementUnitsMax -
                                              Mouse::kMouseMovementUnitsNeutral)
                                           : (Mouse::kMouseMovementUnitsMin -
                                              Mouse::kMouseMovementUnitsNeutral)));
          break;

        case EAxisDirection::Positive:
          mouseAxisValueToContribute +=
              (int)(kMouseButtonContributionScalingFactor *
                    (double)(buttonPressed ? (Mouse::kMouseMovementUnitsMax -
                                              Mouse::kMouseMovementUnitsNeutral)
                                           : 0));
          break;

        case EAxisDirection::Negative:
          mouseAxisValueToContribute +=
              (int)(kMouseButtonContributionScalingFactor *
                    (double)(buttonPressed ? (Mouse::kMouseMovementUnitsMin -
                                              Mouse::kMouseMovementUnitsNeutral)
                                           : 0));
          break;
      }

      Mouse::SubmitMouseMovement(axis, mouseAxisValueToContribute, sourceIdentifier);
    }

    void MouseAxisMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      static const bool kEnableMouseAxisProperites =
          Globals::GetConfigurationData()
              [Strings::kStrConfigurationSectionProperties]
              [Strings::kStrConfigurationSettingsPropertiesUseBuiltinProperties]
                  .ValueOr(true);

      constexpr double kBidirectionalStepSize =
          (double)(Mouse::kMouseMovementUnitsMax - Mouse::kMouseMovementUnitsMin) /
          (double)(kTriggerValueMax - kTriggerValueMin);
      constexpr double kPositiveStepSize =
          (double)Mouse::kMouseMovementUnitsMax / (double)(kTriggerValueMax - kTriggerValueMin);
      constexpr double kNegativeStepSize =
          (double)Mouse::kMouseMovementUnitsMin / (double)(kTriggerValueMax - kTriggerValueMin);

      constexpr unsigned int kTriggerMouseDeadzonePercent = 8;
      constexpr unsigned int kTriggerMouseSaturationPercent = 92;
      const uint8_t triggerValueForContribution =
          (kEnableMouseAxisProperites
               ? Math::ApplyRawTriggerTransform(
                     triggerValue, kTriggerMouseDeadzonePercent, kTriggerMouseSaturationPercent)
               : triggerValue);

      int mouseAxisValueToContribute = 0;

      switch (direction)
      {
        case EAxisDirection::Both:
          mouseAxisValueToContribute =
              (int)((double)triggerValueForContribution * kBidirectionalStepSize) +
              Mouse::kMouseMovementUnitsMin;
          break;

        case EAxisDirection::Positive:
          mouseAxisValueToContribute =
              (int)((double)triggerValueForContribution * kPositiveStepSize) +
              Mouse::kMouseMovementUnitsNeutral;
          break;

        case EAxisDirection::Negative:
          mouseAxisValueToContribute =
              (int)((double)triggerValueForContribution * kNegativeStepSize) -
              Mouse::kMouseMovementUnitsNeutral;
          break;
      }

      Mouse::SubmitMouseMovement(axis, mouseAxisValueToContribute, sourceIdentifier);
    }

    void MouseAxisMapper::ContributeNeutral(
        SState& controllerState, uint32_t sourceIdentifier) const
    {
      Mouse::SubmitMouseMovement(axis, Mouse::kMouseMovementUnitsNeutral, sourceIdentifier);
    }

    int MouseAxisMapper::GetTargetElementCount(void) const
    {
      return 0;
    }

    std::optional<SElementIdentifier> MouseAxisMapper::GetTargetElementAt(int index) const
    {
      return std::nullopt;
    }

    std::unique_ptr<IElementMapper> MouseButtonMapper::Clone(void) const
    {
      return std::make_unique<MouseButtonMapper>(*this);
    }

    void MouseButtonMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsAnalogPressed(analogValue))
        Mouse::SubmitMouseButtonPressedState(mouseButton);
      else
        Mouse::SubmitMouseButtonReleasedState(mouseButton);
    }

    void MouseButtonMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      if (true == buttonPressed)
        Mouse::SubmitMouseButtonPressedState(mouseButton);
      else
        Mouse::SubmitMouseButtonReleasedState(mouseButton);
    }

    void MouseButtonMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsTriggerPressed(triggerValue))
        Mouse::SubmitMouseButtonPressedState(mouseButton);
      else
        Mouse::SubmitMouseButtonReleasedState(mouseButton);
    }

    void MouseButtonMapper::ContributeNeutral(
        SState& controllerState, uint32_t sourceIdentifier) const
    {
      Mouse::SubmitMouseButtonReleasedState(mouseButton);
    }

    int MouseButtonMapper::GetTargetElementCount(void) const
    {
      return 0;
    }

    std::optional<SElementIdentifier> MouseButtonMapper::GetTargetElementAt(int index) const
    {
      return std::nullopt;
    }

    std::unique_ptr<IElementMapper> MouseSpeedModifierMapper::Clone(void) const
    {
      return std::make_unique<MouseSpeedModifierMapper>(*this);
    }

    void MouseSpeedModifierMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsAnalogPressed(analogValue))
        Mouse::SubmitMouseSpeedOverride(mouseSpeedScalingFactor, sourceIdentifier);
      else
        Mouse::SubmitMouseSpeedOverride(std::nullopt, sourceIdentifier);
    }

    void MouseSpeedModifierMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      if (true == buttonPressed)
        Mouse::SubmitMouseSpeedOverride(mouseSpeedScalingFactor, sourceIdentifier);
      else
        Mouse::SubmitMouseSpeedOverride(std::nullopt, sourceIdentifier);
    }

    void MouseSpeedModifierMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsTriggerPressed(triggerValue))
        Mouse::SubmitMouseSpeedOverride(mouseSpeedScalingFactor, sourceIdentifier);
      else
        Mouse::SubmitMouseSpeedOverride(std::nullopt, sourceIdentifier);
    }

    void MouseSpeedModifierMapper::ContributeNeutral(
        SState& controllerState, uint32_t sourceIdentifier) const
    {
      Mouse::SubmitMouseSpeedOverride(std::nullopt, sourceIdentifier);
    }

    int MouseSpeedModifierMapper::GetTargetElementCount(void) const
    {
      return 0;
    }

    std::optional<SElementIdentifier> MouseSpeedModifierMapper::GetTargetElementAt(int index) const
    {
      return std::nullopt;
    }

    std::unique_ptr<IElementMapper> PovMapper::Clone(void) const
    {
      return std::make_unique<PovMapper>(*this);
    }

    void PovMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsAnalogPressed(analogValue))
        controllerState.povDirection.components[(int)povDirection] = true;
    }

    void PovMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      if (true == buttonPressed) controllerState.povDirection.components[(int)povDirection] = true;
    }

    void PovMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      if (true == Math::IsTriggerPressed(triggerValue))
        controllerState.povDirection.components[(int)povDirection] = true;
    }

    int PovMapper::GetTargetElementCount(void) const
    {
      return 1;
    }

    std::optional<SElementIdentifier> PovMapper::GetTargetElementAt(int index) const
    {
      if (0 != index) return std::nullopt;

      return SElementIdentifier({.type = EElementType::Pov});
    }

    std::unique_ptr<IElementMapper> SplitMapper::Clone(void) const
    {
      return std::make_unique<SplitMapper>(*this);
    }

    void SplitMapper::ContributeFromAnalogValue(
        SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
    {
      if ((int32_t)analogValue >= kAnalogValueNeutral)
      {
        if (nullptr != positiveMapper)
          positiveMapper->ContributeFromAnalogValue(controllerState, analogValue, sourceIdentifier);

        if (nullptr != negativeMapper)
          negativeMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
      else
      {
        if (nullptr != negativeMapper)
          negativeMapper->ContributeFromAnalogValue(controllerState, analogValue, sourceIdentifier);

        if (nullptr != positiveMapper)
          positiveMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
    }

    void SplitMapper::ContributeFromButtonValue(
        SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
    {
      if (true == buttonPressed)
      {
        if (nullptr != positiveMapper)
          positiveMapper->ContributeFromButtonValue(controllerState, true, sourceIdentifier);

        if (nullptr != negativeMapper)
          negativeMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
      else
      {
        if (nullptr != negativeMapper)
          negativeMapper->ContributeFromButtonValue(controllerState, true, sourceIdentifier);

        if (nullptr != positiveMapper)
          positiveMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
    }

    void SplitMapper::ContributeFromTriggerValue(
        SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
    {
      if ((int32_t)triggerValue >= kTriggerValueMid)
      {
        if (nullptr != positiveMapper)
          positiveMapper->ContributeFromTriggerValue(
              controllerState, triggerValue, sourceIdentifier);

        if (nullptr != negativeMapper)
          negativeMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
      else
      {
        if (nullptr != negativeMapper)
          negativeMapper->ContributeFromTriggerValue(
              controllerState, triggerValue, sourceIdentifier);

        if (nullptr != positiveMapper)
          positiveMapper->ContributeNeutral(controllerState, sourceIdentifier);
      }
    }

    void SplitMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
    {
      if (nullptr != positiveMapper)
        positiveMapper->ContributeNeutral(controllerState, sourceIdentifier);

      if (nullptr != negativeMapper)
        negativeMapper->ContributeNeutral(controllerState, sourceIdentifier);
    }

    int SplitMapper::GetTargetElementCount(void) const
    {
      const int positiveElementCount =
          ((nullptr != positiveMapper) ? positiveMapper->GetTargetElementCount() : 0);
      const int negativeElementCount =
          ((nullptr != negativeMapper) ? negativeMapper->GetTargetElementCount() : 0);

      return positiveElementCount + negativeElementCount;
    }

    std::optional<SElementIdentifier> SplitMapper::GetTargetElementAt(int index) const
    {
      const int positiveElementCount =
          ((nullptr != positiveMapper) ? positiveMapper->GetTargetElementCount() : 0);

      if (index >= positiveElementCount)
      {
        if (nullptr != negativeMapper)
          return negativeMapper->GetTargetElementAt(index - positiveElementCount);
      }
      else
      {
        if (nullptr != positiveMapper) return positiveMapper->GetTargetElementAt(index);
      }

      return std::nullopt;
    }
  } // namespace Controller
} // namespace Xidi
