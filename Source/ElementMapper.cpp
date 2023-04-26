/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ElementMapper.cpp
 *   Implementation of functionality used to implement mappings from
 *   individual XInput controller elements to virtual DirectInput controller
 *   elements.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Globals.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Strings.h"

#include <cmath>
#include <cstdint>
#include <optional>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL CONSTANTS ------------------------------------- //

        /// Threshold value used to determine if a trigger is considered "pressed" or not as a digital button.
        static constexpr uint8_t kTriggerPressedThreshold = 40;

        /// Threshold negative direction value used to determine if an analog stick is considered "pressed" or not as a digital button.
        static constexpr int16_t kAnalogPressedThresholdNegative = -14750;

        /// Threshold positive direction value used to determine if an analog stick is considered "pressed" or not as a digital button.
        static constexpr int16_t kAnalogPressedThresholdPositive = 14750;


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Applies deadzone and saturation transformations to a raw analog value.
        /// @param [in] analogValue Analog value for which a deadzone should be applied.
        /// @param [in] deadzoneHudnredthsOfPercent Hundredths of a percent of the analog range for which the deadzone should be applied.
        static int16_t ApplyRawAnalogTransform(int16_t analogValue, unsigned int deadzonePercent, unsigned int saturationPercent)
        {
            if ((0 == deadzonePercent) && (100 == saturationPercent))
                return analogValue;

            const int16_t deadzoneCutoff = ((kAnalogValueMax - kAnalogValueNeutral) * deadzonePercent) / 100;
            if (std::abs(analogValue) <= deadzoneCutoff)
                return kAnalogValueNeutral;

            const int16_t saturationCutoff = ((kAnalogValueMax - kAnalogValueNeutral) * saturationPercent) / 100;;
            if (std::abs(analogValue) >= saturationCutoff)
                return ((analogValue >= 0) ? kAnalogValueMax : kAnalogValueMin);

            const double transformedAnalogBase = ((analogValue >= 0) ? ((double)analogValue - (double)deadzoneCutoff) : ((double)analogValue + (double)deadzoneCutoff));
            const double transformationScaleFactor = ((double)(kAnalogValueMax - kAnalogValueNeutral)) / ((double)(saturationCutoff - deadzoneCutoff));

            return kAnalogValueNeutral + (int16_t)(transformedAnalogBase * transformationScaleFactor);
        }

        /// Applies deadzone and saturation transformations to a raw trigger value.
        /// @param [in] analogValue Analog value for which a deadzone should be applied.
        /// @param [in] deadzoneHudnredthsOfPercent Hundredths of a percent of the analog range for which the deadzone should be applied.
        static uint8_t ApplyRawTriggerTransform(uint8_t triggerValue, unsigned int deadzonePercent, unsigned int saturationPercent)
        {
            if ((0 == deadzonePercent) && (100 == saturationPercent))
                return triggerValue;

            const uint8_t deadzoneCutoff = (uint8_t)((((unsigned int)kTriggerValueMax - (unsigned int)kTriggerValueMin) * deadzonePercent) / 100);
            if (triggerValue <= deadzoneCutoff)
                return kTriggerValueMin;

            const uint8_t saturationCutoff = (uint8_t)((((unsigned int)kTriggerValueMax - (unsigned int)kTriggerValueMin) * saturationPercent) / 100);
            if (triggerValue >= saturationCutoff)
                return kTriggerValueMax;

            const float transformedTriggerBase = (float)triggerValue - (float)deadzoneCutoff;
            const float transformationScaleFactor = ((float)(kTriggerValueMax - kTriggerValueMin)) / ((float)(saturationCutoff - deadzoneCutoff));

            return kTriggerValueMin + (uint8_t)(transformedTriggerBase * transformationScaleFactor);
        }

        /// Determines if an analog reading is considered "pressed" as a digital button in the negative direction.
        /// @param [in] analogValue Analog reading from the XInput controller.
        /// @return `true` if the virtual button is considered pressed, `false` otherwise.
        static inline bool IsAnalogPressedNegative(int16_t analogValue)
        {
            return (analogValue <= kAnalogPressedThresholdNegative);
        }

        /// Determines if an analog reading is considered "pressed" as a digital button in the positive direction.
        /// @param [in] analogValue Analog reading from the XInput controller.
        /// @return `true` if the virtual button is considered pressed, `false` otherwise.
        static inline bool IsAnalogPressedPositive(int16_t analogValue)
        {
            return (analogValue >= kAnalogPressedThresholdPositive);
        }

        /// Determines if an analog reading is considered "pressed" as a digital button.
        /// @param [in] analogValue Analog reading from the XInput controller.
        /// @return `true` if the virtual button is considered pressed, `false` otherwise.
        static inline bool IsAnalogPressed(int16_t analogValue)
        {
            return (IsAnalogPressedNegative(analogValue) || IsAnalogPressedPositive(analogValue));
        }

        /// Determines if a trigger reading is considered "pressed" as a digital button.
        /// @param [in] triggerValue Trigger reading from the XInput controller.
        /// @return `true` if the virtual button is considered pressed, `false` otherwise.
        static inline bool IsTriggerPressed(uint8_t triggerValue)
        {
            return (triggerValue >= kTriggerPressedThreshold);
        }


        // -------- CONCRETE INSTANCE METHODS -------------------------- //
        // See "ElementMapper.h" for documentation.

        std::unique_ptr<IElementMapper> AxisMapper::Clone(void) const
        {
            return std::make_unique<AxisMapper>(*this);
        }

        // --------

        void AxisMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
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

        // --------

        void AxisMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
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

        // --------

        void AxisMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            constexpr double kBidirectionalStepSize = (double)(kAnalogValueMax - kAnalogValueMin) / (double)(kTriggerValueMax - kTriggerValueMin);
            constexpr double kPositiveStepSize = (double)kAnalogValueMax / (double)(kTriggerValueMax - kTriggerValueMin);
            constexpr double kNegativeStepSize = (double)kAnalogValueMin / (double)(kTriggerValueMax - kTriggerValueMin);

            int32_t axisValueToContribute = 0;

            switch (direction)
            {
            case EAxisDirection::Both:
                axisValueToContribute = (int32_t)((double)triggerValue * kBidirectionalStepSize) + kAnalogValueMin;
                break;

            case EAxisDirection::Positive:
                axisValueToContribute = (int32_t)((double)triggerValue * kPositiveStepSize) + kAnalogValueNeutral;
                break;

            case EAxisDirection::Negative:
                axisValueToContribute = (int32_t)((double)triggerValue * kNegativeStepSize) - kAnalogValueNeutral;
                break;
            }

            controllerState[axis] += axisValueToContribute;
        }

        // --------

        int AxisMapper::GetTargetElementCount(void) const
        {
            return 1;
        }

        // --------

        std::optional<SElementIdentifier> AxisMapper::GetTargetElementAt(int index) const
        {
            if (0 != index)
                return std::nullopt;

            return SElementIdentifier({.type = EElementType::Axis, .axis = axis});
        }

        // --------

        std::unique_ptr<IElementMapper> ButtonMapper::Clone(void) const
        {
            return std::make_unique<ButtonMapper>(*this);
        }

        // --------

        void ButtonMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            controllerState[button] = (controllerState[button] || IsAnalogPressed(analogValue));
        }

        // --------

        void ButtonMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            controllerState[button] = (controllerState[button] || buttonPressed);
        }

        // --------

        void ButtonMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            controllerState[button] = (controllerState[button] || IsTriggerPressed(triggerValue));
        }

        // --------

        int ButtonMapper::GetTargetElementCount(void) const
        {
            return 1;
        }

        // --------

        std::optional<SElementIdentifier> ButtonMapper::GetTargetElementAt(int index) const
        {
            if (0 != index)
                return std::nullopt;

            return SElementIdentifier({.type = EElementType::Button, .button = button});
        }

        // --------

        std::unique_ptr<IElementMapper> CompoundMapper::Clone(void) const
        {
            return std::make_unique<CompoundMapper>(*this);
        }

        // --------

        void CompoundMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeFromAnalogValue(controllerState, analogValue, sourceIdentifier);
            }
        }

        // --------

        void CompoundMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeFromButtonValue(controllerState, buttonPressed, sourceIdentifier);
            }
        }

        // --------

        void CompoundMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeFromTriggerValue(controllerState, triggerValue, sourceIdentifier);
            }
        }

        // --------

        void CompoundMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeNeutral(controllerState, sourceIdentifier);
            }
        }

        // --------

        int CompoundMapper::GetTargetElementCount(void) const
        {
            int numTargetElements = 0;

            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    numTargetElements += elementMapper->GetTargetElementCount();
            }

            return numTargetElements;
        }

        // --------

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

        // --------

        std::unique_ptr<IElementMapper> DigitalAxisMapper::Clone(void) const
        {
            return std::make_unique<DigitalAxisMapper>(*this);
        }

        // --------

        void DigitalAxisMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            int32_t axisValueToContribute = 0;

            switch (direction)
            {
            case EAxisDirection::Both:
                if (IsAnalogPressedNegative(analogValue))
                    axisValueToContribute = kAnalogValueMin;
                else if (IsAnalogPressedPositive(analogValue))
                    axisValueToContribute = kAnalogValueMax;
                break;

            case EAxisDirection::Positive:
                if (IsAnalogPressedPositive(analogValue))
                    axisValueToContribute = kAnalogValueMax;
                break;

            case EAxisDirection::Negative:
                if (IsAnalogPressedNegative(analogValue))
                    axisValueToContribute = kAnalogValueMin;
                break;
            }

            controllerState[axis] += axisValueToContribute;
        }

        // --------

        void DigitalAxisMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            ContributeFromButtonValue(controllerState, IsTriggerPressed(triggerValue), sourceIdentifier);
        }

        // --------

        std::unique_ptr<IElementMapper> InvertMapper::Clone(void) const
        {
            return std::make_unique<InvertMapper>(*this);
        }

        // --------

        void InvertMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            if (nullptr != elementMapper)
            {
                const int32_t invertedAnalogValue = (kAnalogValueMax + kAnalogValueMin) - (int32_t)analogValue;
                elementMapper->ContributeFromAnalogValue(controllerState, (int16_t)invertedAnalogValue, sourceIdentifier);
            }
        }

        // --------

        void InvertMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            if (nullptr != elementMapper)
            {
                const bool invertedButtonValue = !buttonPressed;
                elementMapper->ContributeFromButtonValue(controllerState, invertedButtonValue, sourceIdentifier);
            }
        }

        // --------

        void InvertMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            if (nullptr != elementMapper)
            {
                const int32_t invertedTriggerValue = (kTriggerValueMax + kTriggerValueMin) - (int32_t)triggerValue;
                elementMapper->ContributeFromTriggerValue(controllerState, (uint8_t)invertedTriggerValue, sourceIdentifier);
            }
        }

        // --------

        void InvertMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
        {
            if (nullptr != elementMapper)
                elementMapper->ContributeNeutral(controllerState, sourceIdentifier);
        }

        // --------

        int InvertMapper::GetTargetElementCount(void) const
        {
            if (nullptr != elementMapper)
                return elementMapper->GetTargetElementCount();

            return 0;
        }

        // --------

        std::optional<SElementIdentifier> InvertMapper::GetTargetElementAt(int index) const
        {
            if (nullptr != elementMapper)
                return elementMapper->GetTargetElementAt(index);

            return std::nullopt;
        }

        // --------

        std::unique_ptr<IElementMapper> KeyboardMapper::Clone(void) const
        {
            return std::make_unique<KeyboardMapper>(*this);
        }

        // --------

        void KeyboardMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            if (true == IsAnalogPressed(analogValue))
                Keyboard::SubmitKeyPressedState(key);
            else
                Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        void KeyboardMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            if (true == buttonPressed)
                Keyboard::SubmitKeyPressedState(key);
            else
                Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        void KeyboardMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            if (true == IsTriggerPressed(triggerValue))
                Keyboard::SubmitKeyPressedState(key);
            else
                Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        void KeyboardMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
        {
            Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        int KeyboardMapper::GetTargetElementCount(void) const
        {
            return 0;
        }

        // --------

        std::optional<SElementIdentifier> KeyboardMapper::GetTargetElementAt(int index) const
        {
            return std::nullopt;
        }

        // --------

        std::unique_ptr<IElementMapper> MouseAxisMapper::Clone(void) const
        {
            return std::make_unique<MouseAxisMapper>(*this);
        }

        // --------

        void MouseAxisMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            static const bool kEnableMouseAxisProperites = Globals::GetConfigurationData().GetFirstBooleanValue(Strings::kStrConfigurationSectionProperties, Strings::kStrConfigurationSettingsPropertiesUseBuiltinProperties).value_or(true);

            constexpr double kAnalogToMouseScalingFactor = (double)(Mouse::kMouseMovementUnitsMax - Mouse::kMouseMovementUnitsMin) / (double)(kAnalogValueMax - kAnalogValueMin);
            
            constexpr unsigned int kAnalogMouseDeadzonePercent = 8;
            constexpr unsigned int kAnalogMouseSaturationPercent = 92;
            const int16_t analogValueForContribution = (kEnableMouseAxisProperites ? ApplyRawAnalogTransform(analogValue, kAnalogMouseDeadzonePercent, kAnalogMouseSaturationPercent) : analogValue);

            const double mouseAxisValueRaw = ((double)(analogValueForContribution - kAnalogValueNeutral) * kAnalogToMouseScalingFactor);
            const double mouseAxisValueTransformed = mouseAxisValueRaw;

            int mouseAxisValueToContribute = (int)mouseAxisValueTransformed;

            switch (direction)
            {
            case EAxisDirection::Both:
                break;

            case EAxisDirection::Positive:
                mouseAxisValueToContribute = (mouseAxisValueToContribute - Mouse::kMouseMovementUnitsMin) / 2;
                break;

            case EAxisDirection::Negative:
                mouseAxisValueToContribute = (mouseAxisValueToContribute - Mouse::kMouseMovementUnitsMax) / 2;
                break;
            }

            Mouse::SubmitMouseMovement(axis, mouseAxisValueToContribute, sourceIdentifier);
        }

        // --------

        void MouseAxisMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            constexpr double kMouseButtonContributionScalingFactor = 0.5;

            int mouseAxisValueToContribute = Mouse::kMouseMovementUnitsNeutral;

            switch (direction)
            {
            case EAxisDirection::Both:
                mouseAxisValueToContribute += (int)(kMouseButtonContributionScalingFactor * (double)(buttonPressed ? (Mouse::kMouseMovementUnitsMax - Mouse::kMouseMovementUnitsNeutral) : (Mouse::kMouseMovementUnitsMin - Mouse::kMouseMovementUnitsNeutral)));
                break;

            case EAxisDirection::Positive:
                mouseAxisValueToContribute += (int)(kMouseButtonContributionScalingFactor * (double)(buttonPressed ? (Mouse::kMouseMovementUnitsMax - Mouse::kMouseMovementUnitsNeutral) : 0));
                break;

            case EAxisDirection::Negative:
                mouseAxisValueToContribute += (int)(kMouseButtonContributionScalingFactor * (double)(buttonPressed ? (Mouse::kMouseMovementUnitsMin - Mouse::kMouseMovementUnitsNeutral) : 0));
                break;
            }

            Mouse::SubmitMouseMovement(axis, mouseAxisValueToContribute, sourceIdentifier);
        }

        // --------

        void MouseAxisMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            static const bool kEnableMouseAxisProperites = Globals::GetConfigurationData().GetFirstBooleanValue(Strings::kStrConfigurationSectionProperties, Strings::kStrConfigurationSettingsPropertiesUseBuiltinProperties).value_or(true);

            constexpr double kBidirectionalStepSize = (double)(Mouse::kMouseMovementUnitsMax - Mouse::kMouseMovementUnitsMin) / (double)(kTriggerValueMax - kTriggerValueMin);
            constexpr double kPositiveStepSize = (double)Mouse::kMouseMovementUnitsMax / (double)(kTriggerValueMax - kTriggerValueMin);
            constexpr double kNegativeStepSize = (double)Mouse::kMouseMovementUnitsMin / (double)(kTriggerValueMax - kTriggerValueMin);
            
            constexpr unsigned int kTriggerMouseDeadzonePercent = 8;
            constexpr unsigned int kTriggerMouseSaturationPercent = 92;
            const uint8_t triggerValueForContribution = (kEnableMouseAxisProperites ? ApplyRawTriggerTransform(triggerValue, kTriggerMouseDeadzonePercent, kTriggerMouseSaturationPercent) : triggerValue);

            int mouseAxisValueToContribute = 0;

            switch (direction)
            {
            case EAxisDirection::Both:
                mouseAxisValueToContribute = (int)((double)triggerValueForContribution * kBidirectionalStepSize) + Mouse::kMouseMovementUnitsMin;
                break;

            case EAxisDirection::Positive:
                mouseAxisValueToContribute = (int)((double)triggerValueForContribution * kPositiveStepSize) + Mouse::kMouseMovementUnitsNeutral;
                break;

            case EAxisDirection::Negative:
                mouseAxisValueToContribute = (int)((double)triggerValueForContribution * kNegativeStepSize) - Mouse::kMouseMovementUnitsNeutral;
                break;
            }

            Mouse::SubmitMouseMovement(axis, mouseAxisValueToContribute, sourceIdentifier);
        }

        // --------

        void MouseAxisMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
        {
            Mouse::SubmitMouseMovement(axis, Mouse::kMouseMovementUnitsNeutral, sourceIdentifier);
        }

        // --------

        int MouseAxisMapper::GetTargetElementCount(void) const
        {
            return 0;
        }

        // --------

        std::optional<SElementIdentifier> MouseAxisMapper::GetTargetElementAt(int index) const
        {
            return std::nullopt;
        }

        // --------
        
        std::unique_ptr<IElementMapper> MouseButtonMapper::Clone(void) const
        {
            return std::make_unique<MouseButtonMapper>(*this);
        }

        // --------

        void MouseButtonMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            if (true == IsAnalogPressed(analogValue))
                Mouse::SubmitMouseButtonPressedState(mouseButton);
            else
                Mouse::SubmitMouseButtonReleasedState(mouseButton);
        }

        // --------

        void MouseButtonMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            if (true == buttonPressed)
                Mouse::SubmitMouseButtonPressedState(mouseButton);
            else
                Mouse::SubmitMouseButtonReleasedState(mouseButton);
        }

        // --------

        void MouseButtonMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            if (true == IsTriggerPressed(triggerValue))
                Mouse::SubmitMouseButtonPressedState(mouseButton);
            else
                Mouse::SubmitMouseButtonReleasedState(mouseButton);
        }

        // --------

        void MouseButtonMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
        {
            Mouse::SubmitMouseButtonReleasedState(mouseButton);
        }

        // --------

        int MouseButtonMapper::GetTargetElementCount(void) const
        {
            return 0;
        }

        // --------

        std::optional<SElementIdentifier> MouseButtonMapper::GetTargetElementAt(int index) const
        {
            return std::nullopt;
        }

        // --------

        std::unique_ptr<IElementMapper> PovMapper::Clone(void) const
        {
            return std::make_unique<PovMapper>(*this);
        }

        // --------

        void PovMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
        {
            if (true == IsAnalogPressed(analogValue))
                controllerState.povDirection.components[(int)povDirection] = true;
        }

        // --------

        void PovMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
        {
            if (true == buttonPressed)
                controllerState.povDirection.components[(int)povDirection] = true;
        }

        // --------

        void PovMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            if (true == IsTriggerPressed(triggerValue))
                controllerState.povDirection.components[(int)povDirection] = true;
        }

        // --------

        int PovMapper::GetTargetElementCount(void) const
        {
            return 1;
        }

        // --------

        std::optional<SElementIdentifier> PovMapper::GetTargetElementAt(int index) const
        {
            if (0 != index)
                return std::nullopt;

            return SElementIdentifier({.type = EElementType::Pov});
        }

        // --------

        std::unique_ptr<IElementMapper> SplitMapper::Clone(void) const
        {
            return std::make_unique<SplitMapper>(*this);
        }

        // --------

        void SplitMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue, uint32_t sourceIdentifier) const
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

        // --------

        void SplitMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed, uint32_t sourceIdentifier) const
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

        // --------

        void SplitMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue, uint32_t sourceIdentifier) const
        {
            if ((int32_t)triggerValue >= kTriggerValueMid)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromTriggerValue(controllerState, triggerValue, sourceIdentifier);

                if (nullptr != negativeMapper)
                    negativeMapper->ContributeNeutral(controllerState, sourceIdentifier);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromTriggerValue(controllerState, triggerValue, sourceIdentifier);

                if (nullptr != positiveMapper)
                    positiveMapper->ContributeNeutral(controllerState, sourceIdentifier);
            }
        }

        // --------

        void SplitMapper::ContributeNeutral(SState& controllerState, uint32_t sourceIdentifier) const
        {
            if (nullptr != positiveMapper)
                positiveMapper->ContributeNeutral(controllerState, sourceIdentifier);

            if (nullptr != negativeMapper)
                negativeMapper->ContributeNeutral(controllerState, sourceIdentifier);
        }

        // --------

        int SplitMapper::GetTargetElementCount(void) const
        {
            const int positiveElementCount = ((nullptr != positiveMapper) ? positiveMapper->GetTargetElementCount() : 0);
            const int negativeElementCount = ((nullptr != negativeMapper) ? negativeMapper->GetTargetElementCount() : 0);

            return positiveElementCount + negativeElementCount;
        }

        // --------

        std::optional<SElementIdentifier> SplitMapper::GetTargetElementAt(int index) const
        {
            const int positiveElementCount = ((nullptr != positiveMapper) ? positiveMapper->GetTargetElementCount() : 0);

            if (index >= positiveElementCount)
            {
                if (nullptr != negativeMapper)
                    return negativeMapper->GetTargetElementAt(index - positiveElementCount);
            }
            else
            {
                if (nullptr != positiveMapper)
                    return positiveMapper->GetTargetElementAt(index);
            }

            return std::nullopt;
        }
    }
}
