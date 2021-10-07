/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file PhysicalController.cpp
 *   Implementation of all functionality for communicating with physical
 *   controllers.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "ImportApiWinMM.h"
#include "Message.h"
#include "PhysicalController.h"

#include <condition_variable>
#include <shared_mutex>
#include <stop_token>
#include <thread>
#include <xinput.h>


namespace Xidi
{
    namespace Controller
    {
        // -------- INTERNAL CONSTANTS ------------------------------------- //

        /// Number of milliseconds to wait between polling attempts.
        static constexpr DWORD kPollingPeriodMilliseconds = 5;


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// State data for each of the possible physical controllers.
        static SPhysicalState physicalControllerState[kPhysicalControllerCount];

        /// Condition variables used to wait for updates to the physical controller state information, one per possible physical controller.
        static std::condition_variable_any physicalControllerUpdateNotifier[_countof(physicalControllerState)];

        /// Mutex objects for protecting against concurrent accesses to the shared physical controller state data structure.
        static std::shared_mutex physicalControllerMutex[_countof(physicalControllerUpdateNotifier)];

        /// Thread handle for the internal polling thread.
        static std::thread pollingThread;

        /// Thread handle for the internal status monitoring threads.
        static std::thread monitoringStatusThread[_countof(physicalControllerState)];


        // -------- INTERNAL FUNCTIONS ------------------------------------- //
        
        /// Periodically polls for physical controller state.
        /// On detected state change, updates the internal data structure and notifies all waiting threads.
        static void PollForPhysicalControllerStateChanges(void)
        {
            SPhysicalState newPhysicalState;

            while (true)
            {
                Sleep(kPollingPeriodMilliseconds);
                
                for (auto controllerIdentifier = 0; controllerIdentifier < _countof(physicalControllerState); ++controllerIdentifier)
                {
                    newPhysicalState.errorCode = XInputGetState(controllerIdentifier, &newPhysicalState.state);

                    // Unguarded read is safe because all other threads only perform guarded reads.
                    // Update happens within the block and requires an exclusive lock.
                    if (newPhysicalState != physicalControllerState[controllerIdentifier])
                    {
                        do
                        {
                            std::unique_lock lock(physicalControllerMutex[controllerIdentifier]);
                            physicalControllerState[controllerIdentifier] = newPhysicalState;
                        } while (0);

                        physicalControllerUpdateNotifier[controllerIdentifier].notify_all();
                    }
                }
            }
        }

        /// Monitors physical controller status for events like hardware connection or disconnection and error conditions.
        /// Used exclusively for logging. Intended to be a thread entry point, one thread per monitored physical controller.
        /// @param [in] controllerIdentifier Identifier of the controller to monitor.
        static void MonitorPhysicalControllerStatus(TControllerIdentifier controllerIdentifier)
        {
            if (controllerIdentifier >= kPhysicalControllerCount)
            {
                Message::OutputFormatted(Message::ESeverity::Error, L"Attempted to monitor physical controller with invalid identifier %u.", controllerIdentifier);
                return;
            }

            SPhysicalState oldPhysicalState = GetCurrentPhysicalControllerState(controllerIdentifier);
            SPhysicalState newPhysicalState = oldPhysicalState;

            while (true)
            {
                WaitForPhysicalControllerStateChange(controllerIdentifier, newPhysicalState, std::stop_token());

                // Look for status changes and output to the log, as appropriate.
                switch (newPhysicalState.errorCode)
                {
                case ERROR_SUCCESS:
                    switch (oldPhysicalState.errorCode)
                    {
                    case ERROR_SUCCESS:
                        break;

                    case ERROR_DEVICE_NOT_CONNECTED:
                        Message::OutputFormatted(Message::ESeverity::Info, L"Physical controller %u: Hardware connected.", (1 + controllerIdentifier));
                        break;

                    default:
                        Message::OutputFormatted(Message::ESeverity::Warning, L"Physical controller %u: Cleared previous error condition with code 0x%08x.", (1 + controllerIdentifier), oldPhysicalState.errorCode);
                        break;
                    }
                    break;

                case ERROR_DEVICE_NOT_CONNECTED:
                    if (newPhysicalState.errorCode != oldPhysicalState.errorCode)
                        Message::OutputFormatted(Message::ESeverity::Info, L"Physical controller %u: Hardware disconnected.", (1 + controllerIdentifier));
                    break;

                default:
                    if (newPhysicalState.errorCode != oldPhysicalState.errorCode)
                        Message::OutputFormatted(Message::ESeverity::Warning, L"Physical controller %u: Encountered error condition with code 0x%08x.", (1 + controllerIdentifier), newPhysicalState.errorCode);
                    break;
                }

                oldPhysicalState = newPhysicalState;
            }
        }

        /// Initializes internal data structures, creates polling and monitoring threads, and starts the polling and monitoring of physical controllers.
        /// Idempotent and concurrency-safe.
        static void InitializeAndBeginPolling(void)
        {
            static std::once_flag initFlag;
            std::call_once(initFlag, []() -> void
                {
                    // Initialize controller state data structures.
                    for (auto controllerIdentifier = 0; controllerIdentifier < _countof(physicalControllerState); ++controllerIdentifier)
                        physicalControllerState[controllerIdentifier].errorCode = XInputGetState(controllerIdentifier, &physicalControllerState[controllerIdentifier].state);

                    // Ensure the system timer resolution is suitable for the desired polling frequency.
                    TIMECAPS timeCaps;
                    MMRESULT timeResult = ImportApiWinMM::timeGetDevCaps(&timeCaps, sizeof(timeCaps));
                    if (MMSYSERR_NOERROR == timeResult)
                    {
                        timeResult = ImportApiWinMM::timeBeginPeriod(timeCaps.wPeriodMin);

                        if (MMSYSERR_NOERROR == timeResult)
                            Message::OutputFormatted(Message::ESeverity::Info, L"Set the system timer resolution to %u ms. Desired polling period is %u ms.", timeCaps.wPeriodMin, kPollingPeriodMilliseconds);
                        else
                            Message::OutputFormatted(Message::ESeverity::Warning, L"Failed with code %u to set the system timer resolution.", timeResult);
                    }
                    else
                    {
                        Message::OutputFormatted(Message::ESeverity::Warning, L"Failed with code %u to obtain system timer resolution information.", timeResult);
                    }

                    // Create and start the polling thread.
                    pollingThread = std::thread(PollForPhysicalControllerStateChanges);

                    // No point monitoring physical controllers for hardware status changes if none of the messages will actually be delivered as output.
                    if (Message::WillOutputMessageOfSeverity(Message::ESeverity::Warning))
                    {
                        for (auto controllerIdentifier = 0; controllerIdentifier < _countof(physicalControllerState); ++controllerIdentifier)
                            monitoringStatusThread[controllerIdentifier] = std::thread(MonitorPhysicalControllerStatus, controllerIdentifier);
                    }
                }
            );
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "PhysicalController.h" for documentation.

        SPhysicalState GetCurrentPhysicalControllerState(TControllerIdentifier controllerIdentifier)
        {
            InitializeAndBeginPolling();
            
            std::shared_lock lock(physicalControllerMutex[controllerIdentifier]);
            return physicalControllerState[controllerIdentifier];
        }

        // --------

        bool WaitForPhysicalControllerStateChange(TControllerIdentifier controllerIdentifier, SPhysicalState& state, std::stop_token stopToken)
        {
            InitializeAndBeginPolling();

            if (controllerIdentifier >= kPhysicalControllerCount)
                return false;

            std::shared_lock lock(physicalControllerMutex[controllerIdentifier]);
            physicalControllerUpdateNotifier[controllerIdentifier].wait(lock, stopToken, [controllerIdentifier, &state]() -> bool
                {
                    return (physicalControllerState[controllerIdentifier] != state);
                }
            );

            if (stopToken.stop_requested())
                return false;

            state = physicalControllerState[controllerIdentifier];
            return true;
        }
    }
}
