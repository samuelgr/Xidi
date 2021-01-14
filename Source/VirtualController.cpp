/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file VirtualController.cpp
 *   Implementation of a complete virtual controller.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Mapper.h"
#include "Message.h"
#include "VirtualController.h"
#include "XInputInterface.h"

#include <cstdint>
#include <memory>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Maps a value in one range to its corresponding value in another range.
        /// Ranges are specified as origin values and displacements, essentially one-dimensional vectors with direction either positive (maximum displacement value is greater than origin value) or negative (maximum displacement value is less than origin value).
        /// It is not necessary that the direction of the vectors be the same for both old and new range.
        /// @param [in] oldRangeValue Raw value to transform in the old range.
        /// @param [in] oldRangeOrigin Origin value of the old range.
        /// @param [in] oldRangeDispMax Maximum displacement value in the old range.
        /// @param [in] newRangeOrigin Origin value of the new range.
        /// @param [in] newRangeDispMax Maximum displacement value in the new range.
        /// @return Result of mapping the input value from the old range to the new range.
        static inline int32_t MapValueInRangeToRange(int32_t oldRangeValue, int32_t oldRangeOrigin, int32_t oldRangeDispMax, int32_t newRangeOrigin, int32_t newRangeDispMax)
        {
            const int64_t oldRangeValueDisp = (int64_t)oldRangeValue - (int64_t)oldRangeOrigin;
            const int64_t newRangeMagnitudeMax = (int64_t)newRangeDispMax - (int64_t)newRangeOrigin;
            const int64_t oldRangeMagnitudeMax = (int64_t)oldRangeDispMax - (int64_t)oldRangeOrigin;

            return newRangeOrigin + (int32_t)((oldRangeValueDisp * newRangeMagnitudeMax) / oldRangeMagnitudeMax);
        }

        /// Looks for differences between two virtual controller state objects and submits them as events to the specified event buffer.
        /// Events are only submitted if the associated virtual controller element is included in the event filter.
        /// @param [in] oldState Old controller state, the baseline.
        /// @param [in] newState New controller state, which is compared with the old controller state. If different, controller element values submitted to the event buffer come from this object.
        /// @param [in] eventFilter Filter which specifies which virtual controller elements are allowed to generate events.
        /// @param [in,out] eventBuffer Event buffer object to which events are submitted.
        static inline void SubmitStateChangeEvents(const SState& oldState, const SState& newState, const VirtualController::EventFilter& eventFilter, StateChangeEventBuffer& eventBuffer)
        {
            if (true == eventBuffer.IsEnabled())
            {
                // DirectInput event buffer timestamps are allowed to overflow every ~50 days.
                const uint32_t kTimestamp = GetTickCount();

                for (unsigned int i = 0; i < _countof(oldState.axis); ++i)
                {
                    if (oldState.axis[i] != newState.axis[i])
                    {
                        const SElementIdentifier kAxisElement = {.type = EElementType::Axis, .axis = (EAxis)i};

                        if (eventFilter.Contains(kAxisElement))
                            eventBuffer.AppendEvent({.element = kAxisElement, .value = {.axis = newState.axis[i]}}, kTimestamp);
                    }
                }

                for (unsigned int i = 0; i < oldState.button.size(); ++i)
                {
                    if (oldState.button[i] != newState.button[i])
                    {
                        const SElementIdentifier kButtonElement = {.type = EElementType::Button, .button = (EButton)i};

                        if (eventFilter.Contains(kButtonElement))
                            eventBuffer.AppendEvent({.element = kButtonElement, .value = {.button = newState.button[i]}}, kTimestamp);
                    }
                }

                if (oldState.povDirection.all != newState.povDirection.all)
                {
                    const SElementIdentifier kPovElement = {.type = EElementType::Pov};

                    if (eventFilter.Contains(kPovElement))
                        eventBuffer.AppendEvent({.element = kPovElement, .value = {.povDirection = {.all = newState.povDirection.all}}}, kTimestamp);
                }
            }
        }

        /// Transforms a raw axis value using the supplied axis properties.
        /// @param [in] axisValueRaw Raw axis value as obtained from a mapper.
        /// @param [in] axisProperties Axis properties to apply.
        /// @return Axis value that results from applying the transformation.
        static int32_t TransformAxisValue(int32_t axisValueRaw, const VirtualController::SAxisProperties& axisProperties)
        {
            if (axisValueRaw > kAnalogValueNeutral)
            {
                if (axisValueRaw <= axisProperties.deadzoneRawCutoffPositive)
                    return axisProperties.rangeNeutral;
                else if (axisValueRaw >= axisProperties.saturationRawCutoffPositive)
                    return axisProperties.rangeMax;
                else
                    return MapValueInRangeToRange(axisValueRaw, axisProperties.deadzoneRawCutoffPositive, axisProperties.saturationRawCutoffPositive, axisProperties.rangeNeutral, axisProperties.rangeMax);
            }
            else
            {
                if (axisValueRaw >= axisProperties.deadzoneRawCutoffNegative)
                    return axisProperties.rangeNeutral;
                else if (axisValueRaw <= axisProperties.saturationRawCutoffNegative)
                    return axisProperties.rangeMin;
                else
                    return MapValueInRangeToRange(axisValueRaw, axisProperties.deadzoneRawCutoffNegative, axisProperties.saturationRawCutoffNegative, axisProperties.rangeNeutral, axisProperties.rangeMin);
            }
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "VirtualController.h" for documentation.

        void VirtualController::ApplyProperties(SState& controllerState) const
        {
            const SCapabilities controllerCapabilities = mapper.GetCapabilities();

            for (int i = 0; i < controllerCapabilities.numAxes; ++i)
            {
                const EAxis axis = controllerCapabilities.axisType[i];
                controllerState.axis[(int)axis] = TransformAxisValue(controllerState.axis[(int)axis], properties.axis[(int)axis]);
            }
        }

        // --------

        SState VirtualController::GetState(void)
        {
            auto lock = Lock();
            return GetStateRef();
        }

        // --------

        const SState& VirtualController::GetStateRef(void)
        {
            if (true == stateRefreshNeeded)
                RefreshState();

            stateRefreshNeeded = true;
            return state;
        }

        // --------

        void VirtualController::PopEventBufferOldestEvents(uint32_t numEventsToPop)
        {
            auto lock = Lock();
            eventBuffer.PopOldestEvents(numEventsToPop);
        }

        // --------

        bool VirtualController::RefreshState(void)
        {
            XINPUT_STATE xinputState;
            SStateIdentifier newStateIdentifier = {.packetNumber = 0, .errorCode = xinput->GetState(kControllerIdentifier, &xinputState)};

            auto lock = Lock();
            stateRefreshNeeded = false;

            // Most of the logic in this block is for debugging by outputting messages. The actual functionality is very simple.
            // On success, the packet number is updated to the value received from XInput, otherwise it is left at 0.
            // On failure, the XInput state is zeroed out so that the controller appears to be in a completely neutral state.
            switch (newStateIdentifier.errorCode)
            {
            case ERROR_SUCCESS:
                newStateIdentifier.packetNumber = xinputState.dwPacketNumber;
                switch (stateIdentifier.errorCode)
                {
                case ERROR_SUCCESS:
                    break;

                case ERROR_DEVICE_NOT_CONNECTED:
                    Message::OutputFormatted(Message::ESeverity::Info, L"Virtual controller %u: Hardware connected.", kControllerIdentifier);
                    break;

                default:
                    Message::OutputFormatted(Message::ESeverity::Warning, L"Virtual controller %u: Cleared previous error condition with code 0x%08x.", kControllerIdentifier, stateIdentifier.errorCode);
                    break;
                }
                break;

            case ERROR_DEVICE_NOT_CONNECTED:
                ZeroMemory(&xinputState, sizeof(xinputState));
                if (newStateIdentifier.errorCode != stateIdentifier.errorCode)
                    Message::OutputFormatted(Message::ESeverity::Info, L"Virtual controller %u: Hardware disconnected.", kControllerIdentifier);
                break;

            default:
                ZeroMemory(&xinputState, sizeof(xinputState));
                if (newStateIdentifier.errorCode != stateIdentifier.errorCode)
                    Message::OutputFormatted(Message::ESeverity::Warning, L"Virtual controller %u: Encountered error condition with code 0x%08x.", kControllerIdentifier, newStateIdentifier.errorCode);
                break;
            }

            // If the state identifier is effectively the same then there is nothing further to do.
            // If the packet numbers are the same and both previous and current attempt were successful, then there is no change.
            // Regardless of packet number, if an error condition is persisting then there is also no change.
            if ((newStateIdentifier.packetNumber == stateIdentifier.packetNumber) && (ERROR_SUCCESS == newStateIdentifier.errorCode) && (ERROR_SUCCESS == stateIdentifier.errorCode))
                return false;
            else if ((ERROR_SUCCESS != newStateIdentifier.errorCode) && (ERROR_SUCCESS != stateIdentifier.errorCode))
                return false;

            stateIdentifier = newStateIdentifier;

            SState newState;
            mapper.MapXInputState(newState, xinputState.Gamepad);
            ApplyProperties(newState);

            // Based on the mapper and the applied properties, a change in XInput controller state might not necessarily mean a change in virtual controller state.
            // For example, deadzone might result in filtering out changes in analog stick position, or if a particular XInput controller element is ignored by the mapper then a change in that element does not influence the virtual controller state.
            if (newState == state)
                return false;

            SubmitStateChangeEvents(state, newState, eventFilter, eventBuffer);
            state = newState;
            return true;
        }

        // --------

        bool VirtualController::SetAxisDeadzone(EAxis axis, uint32_t deadzone)
        {
            if ((deadzone >= kAxisDeadzoneMin) && (deadzone <= kAxisDeadzoneMax))
            {
                auto lock = Lock();
                properties.axis[(int)axis].SetDeadzone(deadzone);
                return true;
            }

            return false;
        }

        // --------

        bool VirtualController::SetAxisRange(EAxis axis, int32_t rangeMin, int32_t rangeMax)
        {
            if (rangeMax > rangeMin)
            {
                auto lock = Lock();
                properties.axis[(int)axis].SetRange(rangeMin, rangeMax);
                return true;
            }

            return false;
        }

        // --------

        bool VirtualController::SetAxisSaturation(EAxis axis, uint32_t saturation)
        {
            if ((saturation >= kAxisSaturationMin) && (saturation <= kAxisSaturationMax))
            {
                auto lock = Lock();
                properties.axis[(int)axis].SetSaturation(saturation);
                return true;
            }

            return false;
        }

        // --------

        bool VirtualController::SetAllAxisDeadzone(uint32_t deadzone)
        {
            if ((deadzone >= kAxisDeadzoneMin) && (deadzone <= kAxisDeadzoneMax))
            {
                auto lock = Lock();
                for (int i = 0; i < _countof(properties.axis); ++i)
                    properties.axis[(int)i].SetDeadzone(deadzone);
                return true;
            }

            return false;
        }

        // --------

        bool VirtualController::SetAllAxisRange(int32_t rangeMin, int32_t rangeMax)
        {
            if (rangeMax > rangeMin)
            {
                auto lock = Lock();
                for (int i = 0; i < _countof(properties.axis); ++i)
                    properties.axis[(int)i].SetRange(rangeMin, rangeMax);
                return true;
            }

            return false;
        }

        // --------

        bool VirtualController::SetAllAxisSaturation(uint32_t saturation)
        {
            if ((saturation >= kAxisSaturationMin) && (saturation <= kAxisSaturationMax))
            {
                auto lock = Lock();
                for (int i = 0; i < _countof(properties.axis); ++i)
                    properties.axis[(int)i].SetSaturation(saturation);
                return true;
            }

            return false;
        }

        // --------

        bool VirtualController::SetEventBufferCapacity(uint32_t capacity)
        {
            if (capacity != eventBuffer.GetCapacity())
            {
                auto lock = Lock();
                eventBuffer.SetCapacity(capacity);
            }

            return true;
        }

        // --------

        bool VirtualController::SetForceFeedbackGain(uint32_t ffGain)
        {
            if ((ffGain >= kFfGainMin) && (ffGain <= kFfGainMax))
            {
                auto lock = Lock();
                properties.device.SetFfGain(ffGain);
                return true;
            }

            return false;
        }
    }
}
