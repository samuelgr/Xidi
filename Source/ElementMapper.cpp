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

#include <cstdint>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL CONSTANTS ------------------------------------- //

        /// Threshold value used to determine if a trigger is considered "pressed" or not as a digital button.
        static constexpr uint8_t kTriggerPressedThreshold = 32;

        /// Threshold negative direction value used to determine if an analog stick is considered "pressed" or not as a digital button.
        static constexpr int16_t kAnalogPressedThresholdNegative = -8192;

        /// Threshold positive direction value used to determine if an analog stick is considered "pressed" or not as a digital button.
        static constexpr int16_t kAnalogPressedThresholdPositive = 8192;


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
        // See "ControllerElementMapper.h" for documentation.

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

        SElementIdentifier AxisMapper::GetTargetElement(void) const
        {
            return {.type = EElementType::Axis, .axis = axis};
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

        SElementIdentifier ButtonMapper::GetTargetElement(void) const
        {
            return {.type = EElementType::Button, .button = button};
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

        void PovMapper::ContributeFromAnalogValue(SState& controllerState, int16_t analogValue) const
        {
            if (true == IsAnalogPressedPositive(analogValue))
                controllerState.povDirection.components[(int)povDirectionPositive] = true;
            else if ((maybePovDirectionNegative.has_value()) && (true == IsAnalogPressedNegative(analogValue)))
                controllerState.povDirection.components[(int)maybePovDirectionNegative.value()] = true;
        }

        // --------

        void PovMapper::ContributeFromButtonValue(SState& controllerState, bool buttonPressed) const
        {
            if (true == buttonPressed)
                controllerState.povDirection.components[(int)povDirectionPositive] = true;
            else if (maybePovDirectionNegative.has_value())
                controllerState.povDirection.components[(int)maybePovDirectionNegative.value()] = true;
        }

        // --------

        void PovMapper::ContributeFromTriggerValue(SState& controllerState, uint8_t triggerValue) const
        {
            if (true == IsTriggerPressed(triggerValue))
                controllerState.povDirection.components[(int)povDirectionPositive] = true;
            else if (maybePovDirectionNegative.has_value())
                controllerState.povDirection.components[(int)maybePovDirectionNegative.value()] = true;
        }

        // --------

        SElementIdentifier PovMapper::GetTargetElement(void) const
        {
            return {.type = EElementType::Pov};
        }
    }
}
