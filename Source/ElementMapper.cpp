/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ElementMapper.cpp
 *   Implementation of functionality used to implement mappings from
 *   individual XInput controller elements to virtual DirectInput controller
 *   elements.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "ElementMapper.h"
#include "Keyboard.h"
#include "KeyboardTypes.h"

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

        void AxisMapper::ContributeFromAnalogValue(TControllerIdentifier controllerIdentifier, SState& controllerState, int16_t analogValue) const
        {
            int32_t axisValueToContribute = (int32_t)analogValue;

            switch (direction)
            {
            case EDirection::Both:
                break;

            case EDirection::Positive:
                axisValueToContribute = (axisValueToContribute - kAnalogValueMin) >> 1;
                break;

            case EDirection::Negative:
                axisValueToContribute = (axisValueToContribute - kAnalogValueMax) >> 1;
                break;
            }

            controllerState.axis[(int)axis] += axisValueToContribute;
        }

        // --------

        void AxisMapper::ContributeFromButtonValue(TControllerIdentifier controllerIdentifier, SState& controllerState, bool buttonPressed) const
        {
            int32_t axisValueToContribute = 0;
            
            switch (direction)
            {
            case EDirection::Both:
                axisValueToContribute = (buttonPressed ? kAnalogValueMax : kAnalogValueMin);
                break;

            case EDirection::Positive:
                axisValueToContribute = (buttonPressed ? kAnalogValueMax : kAnalogValueNeutral);
                break;

            case EDirection::Negative:
                axisValueToContribute = (buttonPressed ? kAnalogValueMin : kAnalogValueNeutral);
                break;
            }

            controllerState.axis[(int)axis] += axisValueToContribute;
        }

        // --------

        void AxisMapper::ContributeFromTriggerValue(TControllerIdentifier controllerIdentifier, SState& controllerState, uint8_t triggerValue) const
        {
            constexpr double kBidirectionalStepSize = (double)(kAnalogValueMax - kAnalogValueMin) / (double)(kTriggerValueMax - kTriggerValueMin);
            constexpr double kPositiveStepSize = (double)kAnalogValueMax / (double)(kTriggerValueMax - kTriggerValueMin);
            constexpr double kNegativeStepSize = (double)kAnalogValueMin / (double)(kTriggerValueMax - kTriggerValueMin);

            int32_t axisValueToContribute = 0;

            switch (direction)
            {
            case EDirection::Both:
                axisValueToContribute = (int32_t)((double)triggerValue * kBidirectionalStepSize) + kAnalogValueMin;
                break;

            case EDirection::Positive:
                axisValueToContribute = (int32_t)((double)triggerValue * kPositiveStepSize) + kAnalogValueNeutral;
                break;

            case EDirection::Negative:
                axisValueToContribute = (int32_t)((double)triggerValue * kNegativeStepSize) - kAnalogValueNeutral;
                break;
            }

            controllerState.axis[(int)axis] += axisValueToContribute;
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

        void ButtonMapper::ContributeFromAnalogValue(TControllerIdentifier controllerIdentifier, SState& controllerState, int16_t analogValue) const
        {
            controllerState.button[(int)button] = (controllerState.button[(int)button] || IsAnalogPressed(analogValue));
        }

        // --------

        void ButtonMapper::ContributeFromButtonValue(TControllerIdentifier controllerIdentifier, SState& controllerState, bool buttonPressed) const
        {
            controllerState.button[(int)button] = (controllerState.button[(int)button] || buttonPressed);
        }

        // --------

        void ButtonMapper::ContributeFromTriggerValue(TControllerIdentifier controllerIdentifier, SState& controllerState, uint8_t triggerValue) const
        {
            controllerState.button[(int)button] = (controllerState.button[(int)button] || IsTriggerPressed(triggerValue));
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

        void DigitalAxisMapper::ContributeFromAnalogValue(TControllerIdentifier controllerIdentifier, SState& controllerState, int16_t analogValue) const
        {
            int32_t axisValueToContribute = 0;

            switch (direction)
            {
            case EDirection::Both:
                if (IsAnalogPressedNegative(analogValue))
                    axisValueToContribute = kAnalogValueMin;
                else if (IsAnalogPressedPositive(analogValue))
                    axisValueToContribute = kAnalogValueMax;
                break;

            case EDirection::Positive:
                if (IsAnalogPressedPositive(analogValue))
                    axisValueToContribute = kAnalogValueMax;
                break;

            case EDirection::Negative:
                if (IsAnalogPressedNegative(analogValue))
                    axisValueToContribute = kAnalogValueMin;
                break;
            }

            controllerState.axis[(int)axis] += axisValueToContribute;
        }

        // --------

        void DigitalAxisMapper::ContributeFromTriggerValue(TControllerIdentifier controllerIdentifier, SState& controllerState, uint8_t triggerValue) const
        {
            ContributeFromButtonValue(controllerIdentifier, controllerState, IsTriggerPressed(triggerValue));
        }

        // --------

        void KeyboardMapper::ContributeFromAnalogValue(TControllerIdentifier controllerIdentifier, SState& controllerState, int16_t analogValue) const
        {
            if (true == IsAnalogPressed(analogValue))
                Keyboard::SubmitKeyPressedState(controllerIdentifier, key);
            else
                Keyboard::SubmitKeyReleasedState(controllerIdentifier, key);
        }

        // --------

        void KeyboardMapper::ContributeFromButtonValue(TControllerIdentifier controllerIdentifier, SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
                Keyboard::SubmitKeyPressedState(controllerIdentifier, key);
            else
                Keyboard::SubmitKeyReleasedState(controllerIdentifier, key);
        }

        // --------

        void KeyboardMapper::ContributeFromTriggerValue(TControllerIdentifier controllerIdentifier, SState& controllerState, uint8_t triggerValue) const
        {
            if (true == IsTriggerPressed(triggerValue))
                Keyboard::SubmitKeyPressedState(controllerIdentifier, key);
            else
                Keyboard::SubmitKeyReleasedState(controllerIdentifier, key);
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

        void PovMapper::ContributeFromAnalogValue(TControllerIdentifier controllerIdentifier, SState& controllerState, int16_t analogValue) const
        {
            if (true == IsAnalogPressedPositive(analogValue))
                controllerState.povDirection.components[(int)povDirectionPositive] = true;
            else if ((maybePovDirectionNegative.has_value()) && (true == IsAnalogPressedNegative(analogValue)))
                controllerState.povDirection.components[(int)maybePovDirectionNegative.value()] = true;
        }

        // --------

        void PovMapper::ContributeFromButtonValue(TControllerIdentifier controllerIdentifier, SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
                controllerState.povDirection.components[(int)povDirectionPositive] = true;
            else if (maybePovDirectionNegative.has_value())
                controllerState.povDirection.components[(int)maybePovDirectionNegative.value()] = true;
        }

        // --------

        void PovMapper::ContributeFromTriggerValue(TControllerIdentifier controllerIdentifier, SState& controllerState, uint8_t triggerValue) const
        {
            if (true == IsTriggerPressed(triggerValue))
                controllerState.povDirection.components[(int)povDirectionPositive] = true;
            else if (maybePovDirectionNegative.has_value())
                controllerState.povDirection.components[(int)maybePovDirectionNegative.value()] = true;
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

        void SplitMapper::ContributeFromAnalogValue(TControllerIdentifier controllerIdentifier, SState& controllerState, int16_t analogValue) const
        {
            if ((int32_t)analogValue >= kAnalogValueNeutral)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromAnalogValue(controllerIdentifier, controllerState, analogValue);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromAnalogValue(controllerIdentifier, controllerState, analogValue);
            }
        }

        // --------

        void SplitMapper::ContributeFromButtonValue(TControllerIdentifier controllerIdentifier, SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromButtonValue(controllerIdentifier, controllerState, buttonPressed);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromButtonValue(controllerIdentifier, controllerState, buttonPressed);
            }
        }

        // --------

        void SplitMapper::ContributeFromTriggerValue(TControllerIdentifier controllerIdentifier, SState& controllerState, uint8_t triggerValue) const
        {
            if ((int32_t)triggerValue >= kTriggerValueMid)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromTriggerValue(controllerIdentifier, controllerState, triggerValue);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromTriggerValue(controllerIdentifier, controllerState, triggerValue);
            }
        }

        // --------

        int SplitMapper::GetTargetElementCount(void) const
        {
            const int kPositiveElementCount = ((nullptr != positiveMapper) ? positiveMapper->GetTargetElementCount() : 0);
            const int kNegativeElementCount = ((nullptr != negativeMapper) ? negativeMapper->GetTargetElementCount() : 0);

            return kPositiveElementCount + kNegativeElementCount;
        }

        // --------

        std::optional<SElementIdentifier> SplitMapper::GetTargetElementAt(int index) const
        {
            const int kPositiveElementCount = ((nullptr != positiveMapper) ? positiveMapper->GetTargetElementCount() : 0);

            if (index >= kPositiveElementCount)
            {
                if (nullptr != negativeMapper)
                    return negativeMapper->GetTargetElementAt(index - kPositiveElementCount);
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
