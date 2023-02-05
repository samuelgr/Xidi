/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file VirtualController.cpp
 *   Implementation of a complete virtual controller.
 *****************************************************************************/

#include "ControllerTypes.h"
#include "ForceFeedbackTypes.h"
#include "ImportApiWinMM.h"
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

        /// Monitors for changes in an associated physical controller's state and, on state change, causes a virtual controller to refresh its state.
        /// Intended to be the entry point for per-virtual-controller background threads.
        /// @param [in] thisController Controller object for which state is to be monitored.
        /// @param [in] initialState Initial physical state of the controller. Used as the basis for looking for changes.
        /// @param [in] stopMonitoringToken Used to indicate that the monitoring should stop and the thread should exit.
        static void MonitorPhysicalControllerState(VirtualController* thisController, const SState& initialState, std::stop_token stopMonitoringToken)
        {
            const TControllerIdentifier kControllerIdentifier = thisController->GetIdentifier();
            SState state = initialState;

            while (false == stopMonitoringToken.stop_requested())
            {
                if (true == WaitForRawVirtualControllerStateChange(kControllerIdentifier, state, stopMonitoringToken))
                {
                    if (true == thisController->RefreshState(state))
                        thisController->SignalStateChangeEvent();
                }
            }
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
                const uint32_t kTimestamp = ImportApiWinMM::timeGetTime();

                for (unsigned int i = 0; i < oldState.axis.size(); ++i)
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

        VirtualController::VirtualController(TControllerIdentifier controllerId) : kControllerIdentifier(controllerId), controllerMutex(), eventBuffer(), eventFilter(), properties(), stateRaw(), stateProcessed(), stateChangeEventHandle(NULL), physicalControllerMonitor(), physicalControllerMonitorStop(), physicalControllerForceFeedbackBuffer()
        {
            const SState initialState = GetCurrentRawVirtualControllerState(kControllerIdentifier);

            RefreshState(initialState);
            ReapplyProperties();

            physicalControllerMonitor = std::thread(MonitorPhysicalControllerState, this, initialState, physicalControllerMonitorStop.get_token());

            Message::OutputFormatted(Message::ESeverity::Info, L"Created virtual controller object with identifier %u.", (1 + kControllerIdentifier));
        }

        // --------

        VirtualController::~VirtualController(void)
        {
            ForceFeedbackUnregister();

            physicalControllerMonitorStop.request_stop();
            physicalControllerMonitor.join();

            Message::OutputFormatted(Message::ESeverity::Info, L"Destroyed virtual controller object with identifier %u.", (1 + kControllerIdentifier));
        }


        // -------- INSTANCE METHODS --------------------------------------- //
        // See "VirtualController.h" for documentation.

        void VirtualController::ApplyProperties(SState& controllerState) const
        {
            const SCapabilities kCapabilities = GetCapabilities();

            for (int i = 0; i < kCapabilities.numAxes; ++i)
            {
                const EAxis axis = kCapabilities.axisCapabilities[i].type;
                controllerState[axis] = TransformAxisValue(controllerState[axis], properties[axis]);
            }
        }

        // --------

        bool VirtualController::ForceFeedbackRegister(void)
        {
            auto lock = Lock();

            if (nullptr == physicalControllerForceFeedbackBuffer)
                physicalControllerForceFeedbackBuffer = PhysicalControllerForceFeedbackRegister(kControllerIdentifier, this);

            return ForceFeedbackIsRegistered();
        }

        // --------

        void VirtualController::ForceFeedbackUnregister(void)
        {
            auto lock = Lock();

            PhysicalControllerForceFeedbackUnregister(kControllerIdentifier, this);
            physicalControllerForceFeedbackBuffer = nullptr;
        }

        // --------

        SCapabilities VirtualController::GetCapabilities(void) const
        {
            return GetControllerCapabilities(kControllerIdentifier);
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

        bool VirtualController::RefreshState(SState newStateRaw)
        {
            auto lock = Lock();
            stateRaw = newStateRaw;

            SState newStateProcessed = newStateRaw;
            ApplyProperties(newStateProcessed);

            // Based on the mapper and the applied properties, a change in raw virtual controller state might not necessarily mean a change in processed virtual controller state.
            // For example, deadzone might result in filtering out changes in analog stick position, or if a particular XInput controller element is ignored by the mapper then a change in that element does not influence the virtual controller state.
            if (newStateProcessed == stateProcessed)
                return false;

            SubmitStateChangeEvents(stateProcessed, newStateProcessed, eventFilter, eventBuffer);
            stateProcessed = newStateProcessed;
            return true;
        }

        // --------

        bool VirtualController::SetAxisDeadzone(EAxis axis, uint32_t deadzone)
        {
            if ((deadzone >= kAxisDeadzoneMin) && (deadzone <= kAxisDeadzoneMax))
            {
                auto lock = Lock();

                properties[axis].SetDeadzone(deadzone);

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

                properties[axis].SetRange(rangeMin, rangeMax);

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

                properties[axis].SetSaturation(saturation);

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

                for (auto& axis : properties.axis)
                    axis.SetDeadzone(deadzone);

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

                for (auto& axis : properties.axis)
                    axis.SetRange(rangeMin, rangeMax);

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

                for (auto& axis : properties.axis)
                    axis.SetSaturation(saturation);

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
            const ForceFeedback::TEffectValue newFfGain = (ForceFeedback::TEffectValue)ffGain;
            if ((newFfGain >= kFfGainMin) && (newFfGain <= kFfGainMax))
            {
                auto lock = Lock();
                properties.device.SetFfGain(newFfGain);
                return true;
            }

            return false;
        }

        // --------

        void VirtualController::SetStateChangeEvent(HANDLE eventHandle)
        {
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
