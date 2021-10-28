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

        std::unique_ptr<IElementMapper> AxisMapper::Clone(void) const
        {
            return std::make_unique<AxisMapper>(*this);
        }

        // --------

        void AxisMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
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

        void AxisMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
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

        void AxisMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
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

        std::unique_ptr<IElementMapper> ButtonMapper::Clone(void) const
        {
            return std::make_unique<ButtonMapper>(*this);
        }

        // --------

        void ButtonMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            controllerState.button[(int)button] = (controllerState.button[(int)button] || IsAnalogPressed(analogValue));
        }

        // --------

        void ButtonMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            controllerState.button[(int)button] = (controllerState.button[(int)button] || buttonPressed);
        }

        // --------

        void ButtonMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
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

        std::unique_ptr<IElementMapper> CompoundMapper::Clone(void) const
        {
            return std::make_unique<CompoundMapper>(*this);
        }

        // --------

        void CompoundMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeFromAnalogValue(controllerState, analogValue);
            }
        }

        // --------

        void CompoundMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeFromButtonValue(controllerState, buttonPressed);
            }
        }

        // --------

        void CompoundMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeFromTriggerValue(controllerState, triggerValue);
            }
        }

        // --------

        void CompoundMapper::ContributeNeutral(SState& controllerState) const
        {
            for (const auto& elementMapper : elementMappers)
            {
                if (nullptr != elementMapper)
                    elementMapper->ContributeNeutral(controllerState);
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
                    const int kElementCount = elementMapper->GetTargetElementCount();

                    if (index < (indexOffset + kElementCount))
                        return elementMapper->GetTargetElementAt(index - indexOffset);

                    indexOffset += kElementCount;
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

        void DigitalAxisMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
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

        void DigitalAxisMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
        {
            ContributeFromButtonValue(controllerState, IsTriggerPressed(triggerValue));
        }

        // --------

        std::unique_ptr<IElementMapper> InvertMapper::Clone(void) const
        {
            return std::make_unique<InvertMapper>(*this);
        }

        // --------

        void InvertMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            if (nullptr != elementMapper)
            {
                const int32_t kInvertedAnalogValue = (kAnalogValueMax + kAnalogValueMin) - (int32_t)analogValue;
                elementMapper->ContributeFromAnalogValue(controllerState, (int16_t)kInvertedAnalogValue);
            }
        }

        // --------

        void InvertMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            if (nullptr != elementMapper)
            {
                const bool kInvertedButtonValue = !buttonPressed;
                elementMapper->ContributeFromButtonValue(controllerState, kInvertedButtonValue);
            }
        }

        // --------

        void InvertMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
        {
            if (nullptr != elementMapper)
            {
                const int32_t kInvertedTriggerValue = (kTriggerValueMax + kTriggerValueMin) - (int32_t)triggerValue;
                elementMapper->ContributeFromTriggerValue(controllerState, (uint8_t)kInvertedTriggerValue);
            }
        }

        // --------

        void InvertMapper::ContributeNeutral(SState& controllerState) const
        {
            if (nullptr != elementMapper)
                elementMapper->ContributeNeutral(controllerState);
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

        void KeyboardMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            if (true == IsAnalogPressed(analogValue))
                Keyboard::SubmitKeyPressedState(key);
            else
                Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        void KeyboardMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
                Keyboard::SubmitKeyPressedState(key);
            else
                Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        void KeyboardMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
        {
            if (true == IsTriggerPressed(triggerValue))
                Keyboard::SubmitKeyPressedState(key);
            else
                Keyboard::SubmitKeyReleasedState(key);
        }

        // --------

        void KeyboardMapper::ContributeNeutral(SState& controllerState) const
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

        std::unique_ptr<IElementMapper> PovMapper::Clone(void) const
        {
            return std::make_unique<PovMapper>(*this);
        }

        // --------

        void PovMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            if (true == IsAnalogPressed(analogValue))
                controllerState.povDirection.components[(int)povDirection] = true;
        }

        // --------

        void PovMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
                controllerState.povDirection.components[(int)povDirection] = true;
        }

        // --------

        void PovMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
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

        void SplitMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            if ((int32_t)analogValue >= kAnalogValueNeutral)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromAnalogValue(controllerState, analogValue);

                if (nullptr != negativeMapper)
                    negativeMapper->ContributeNeutral(controllerState);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromAnalogValue(controllerState, analogValue);

                if (nullptr != positiveMapper)
                    positiveMapper->ContributeNeutral(controllerState);
            }
        }

        // --------

        void SplitMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromButtonValue(controllerState, true);

                if (nullptr != negativeMapper)
                    negativeMapper->ContributeNeutral(controllerState);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromButtonValue(controllerState, true);

                if (nullptr != positiveMapper)
                    positiveMapper->ContributeNeutral(controllerState);
            }
        }

        // --------

        void SplitMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
        {
            if ((int32_t)triggerValue >= kTriggerValueMid)
            {
                if (nullptr != positiveMapper)
                    positiveMapper->ContributeFromTriggerValue(controllerState, triggerValue);

                if (nullptr != negativeMapper)
                    negativeMapper->ContributeNeutral(controllerState);
            }
            else
            {
                if (nullptr != negativeMapper)
                    negativeMapper->ContributeFromTriggerValue(controllerState, triggerValue);

                if (nullptr != positiveMapper)
                    positiveMapper->ContributeNeutral(controllerState);
            }
        }

        // --------

        void SplitMapper::ContributeNeutral(SState& controllerState) const
        {
            if (nullptr != positiveMapper)
                positiveMapper->ContributeNeutral(controllerState);

            if (nullptr != negativeMapper)
                negativeMapper->ContributeNeutral(controllerState);
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
