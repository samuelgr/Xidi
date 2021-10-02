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
#include "PhysicalController.h"
#include "VirtualController.h"

#include <cstdint>
#include <stop_token>
#include <thread>


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


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //
        // See "VirtualController.h" for documentation.

        VirtualController::VirtualController(TControllerIdentifier controllerId, const Mapper& mapper) : kControllerIdentifier(controllerId), controllerMutex(), eventBuffer(), eventFilter(), mapper(mapper), properties(), stateRaw(), stateProcessed(), stateIdentifier(), stateChangeEventHandle(NULL), physicalControllerMonitor(), physicalControllerMonitorStop()
        {
            Message::OutputFormatted(Message::ESeverity::Info, L"Created virtual controller object with identifier %u.", (1 + kControllerIdentifier));

            const SPhysicalState initialState = GetCurrentPhysicalControllerState(kControllerIdentifier);
            
            RefreshState(initialState);
            physicalControllerMonitor = std::thread(MonitorPhysicalControllerState, this, initialState, physicalControllerMonitorStop.get_token());
        }

        // --------

        VirtualController::~VirtualController(void)
        {           
            physicalControllerMonitorStop.request_stop();
            physicalControllerMonitor.join();

            Message::OutputFormatted(Message::ESeverity::Info, L"Destroyed virtual controller object with identifier %u.", (1 + kControllerIdentifier));
        }


        // -------- CLASS METHODS ------------------------------------------ //
        // See "VirtualController.h" for documentation.

        void VirtualController::MonitorPhysicalControllerState(VirtualController* thisController, const SPhysicalState& initialState, std::stop_token stopMonitoringToken)
        {
            const TControllerIdentifier kControllerIdentifier = thisController->GetIdentifier();
            SPhysicalState state = initialState;

            while (false == stopMonitoringToken.stop_requested())
            {
                if (true == WaitForPhysicalControllerStateChange(kControllerIdentifier, state, stopMonitoringToken))
                {
                    if (true == thisController->RefreshState(state))
                        thisController->SignalStateChangeEvent();
                }
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
            return stateProcessed;
        }

        // --------

        void VirtualController::PopEventBufferOldestEvents(uint32_t numEventsToPop)
        {
            auto lock = Lock();
            eventBuffer.PopOldestEvents(numEventsToPop);
        }

        // --------
        
        void VirtualController::ReapplyProperties(void)
        {
            stateProcessed = stateRaw;
            ApplyProperties(stateProcessed);
        }

        // --------

        bool VirtualController::RefreshState(const SPhysicalState& newStateData)
        {
            XINPUT_STATE xinputState = newStateData.state;
            SStateIdentifier newStateIdentifier = {.packetNumber = 0, .errorCode = newStateData.errorCode};

            auto lock = Lock();

            // On success, the packet number is updated to the value received from XInput, otherwise it is left at 0.
            // On failure, the XInput state is zeroed out so that the controller appears to be in a completely neutral state.
            switch (newStateData.errorCode)
            {
            case ERROR_SUCCESS:
                newStateIdentifier.packetNumber = xinputState.dwPacketNumber;
                break;

            default:
                ZeroMemory(&xinputState, sizeof(xinputState));
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

            SState newStateRaw;
            mapper.MapXInputState(newStateRaw, xinputState.Gamepad);

            SState newStateProcessed = newStateRaw;
            ApplyProperties(newStateProcessed);

            // Based on the mapper and the applied properties, a change in XInput controller state might not necessarily mean a change in virtual controller state.
            // For example, deadzone might result in filtering out changes in analog stick position, or if a particular XInput controller element is ignored by the mapper then a change in that element does not influence the virtual controller state.
            if (newStateProcessed == stateProcessed)
                return false;

            SubmitStateChangeEvents(stateProcessed, newStateProcessed, eventFilter, eventBuffer);
            stateRaw = newStateRaw;
            stateProcessed = newStateProcessed;
            return true;
        }

        // --------

        bool VirtualController::SetAxisDeadzone(EAxis axis, uint32_t deadzone)
        {
            if ((deadzone >= kAxisDeadzoneMin) && (deadzone <= kAxisDeadzoneMax))
            {
                auto lock = Lock();

                properties.axis[(int)axis].SetDeadzone(deadzone);

                ReapplyProperties();
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

                ReapplyProperties();
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

                ReapplyProperties();
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

                ReapplyProperties();
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

                ReapplyProperties();
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

                ReapplyProperties();
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

        // --------

        void VirtualController::SetStateChangeEvent(HANDLE eventHandle)
        {
            auto lock = Lock();
            stateChangeEventHandle = eventHandle;
        }

        // --------

        void VirtualController::SignalStateChangeEvent(void)
        {
            const HANDLE eventHandleToSignal = stateChangeEventHandle;

            if ((NULL != eventHandleToSignal) && (INVALID_HANDLE_VALUE != eventHandleToSignal))
                SetEvent(eventHandleToSignal);
        }
    }
}
