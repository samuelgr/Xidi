/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ControllerElementMapper.cpp
 *   Implementation of functionality used to implement mappings from
 *   individual XInput controller elements to virtual DirectInput controller
 *   elements.
 *****************************************************************************/

#include "ControllerElementMapper.h"
#include "ControllerTypes.h"

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

        /// Filters (by saturation) analog values that might be slightly out of range due to differences between the implemented range and the XInput actual range.
        /// @param [in] analogValue Raw analog value.
        /// Filtered analog value, which will most likely be the same as the input.
        static inline int16_t AnalogValueFilter(int16_t analogValue)
        {
            if (analogValue > kAnalogValueMax)
                return kAnalogValueMax;
            else if (analogValue < kAnalogValueMin)
                return kAnalogValueMin;
            else
                return analogValue;
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
        // See "ControllerElementMapper.h" for documentation.

        void AxisMapper::ContributeFromAnalogValue(SState* controllerState, int16_t analogValue) const
        {
            int32_t axisValueToContribute = (int32_t)AnalogValueFilter(analogValue);

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

            controllerState->axis[(int)axis] += axisValueToContribute;
        }

        // --------

        void AxisMapper::ContributeFromButtonValue(SState* controllerState, bool buttonPressed) const
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

            controllerState->axis[(int)axis] += axisValueToContribute;
        }

        // --------

        void AxisMapper::ContributeFromTriggerValue(SState* controllerState, uint8_t triggerValue) const
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

            controllerState->axis[(int)axis] += axisValueToContribute;
        }

        // --------

        int AxisMapper::GetTargetElementIndex(void) const
        {
            return (int)axis;
        }

        // --------

        EElementType AxisMapper::GetTargetElementType(void) const
        {
            return EElementType::Axis;
        }
        
        // --------

        void ButtonMapper::ContributeFromAnalogValue(SState* controllerState, int16_t analogValue) const
        {
            controllerState->button[(int)button] = (controllerState->button[(int)button] || IsAnalogPressed(analogValue));
        }

        // --------

        void ButtonMapper::ContributeFromButtonValue(SState* controllerState, bool buttonPressed) const
        {
            controllerState->button[(int)button] = (controllerState->button[(int)button] || buttonPressed);
        }

        // --------

        void ButtonMapper::ContributeFromTriggerValue(SState* controllerState, uint8_t triggerValue) const
        {
            controllerState->button[(int)button] = (controllerState->button[(int)button] || IsTriggerPressed(triggerValue));
        }

        // --------

        int ButtonMapper::GetTargetElementIndex(void) const
        {
            return (int)button;
        }

        // --------

        EElementType ButtonMapper::GetTargetElementType(void) const
        {
            return EElementType::Button;
        }

        // --------

        void DigitalAxisMapper::ContributeFromAnalogValue(SState* controllerState, int16_t analogValue) const
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

            controllerState->axis[(int)axis] += axisValueToContribute;
        }

        // --------

        void DigitalAxisMapper::ContributeFromTriggerValue(SState* controllerState, uint8_t triggerValue) const
        {
            ContributeFromButtonValue(controllerState, IsTriggerPressed(triggerValue));
        }

        // --------

        void PovMapper::ContributeFromAnalogValue(SState* controllerState, int16_t analogValue) const
        {
            controllerState->povDirection[(int)povDirection] = (controllerState->povDirection[(int)povDirection] || IsAnalogPressed(analogValue));
        }

        // --------

        void PovMapper::ContributeFromButtonValue(SState* controllerState, bool buttonPressed) const
        {
            controllerState->povDirection[(int)povDirection] = (controllerState->povDirection[(int)povDirection] || buttonPressed);
        }

        // --------

        void PovMapper::ContributeFromTriggerValue(SState* controllerState, uint8_t triggerValue) const
        {
            controllerState->povDirection[(int)povDirection] = (controllerState->povDirection[(int)povDirection] || IsTriggerPressed(triggerValue));
        }

        // --------

        int PovMapper::GetTargetElementIndex(void) const
        {
            return 0;
        }

        // --------

        EElementType PovMapper::GetTargetElementType(void) const
        {
            return EElementType::Pov;
        }
    }
}
