/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Globals.cpp
 *   Implementation of accessors and mutators for global data items.
 *   Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#include "ApiWindows.h"
#include "GitVersionInfo.h"
#include "Globals.h"
#include "Message.h"
#include "Strings.h"
#include "XidiConfigReader.h"

#ifndef XIDI_SKIP_CONFIG
#include "Configuration.h"
#endif

#ifndef XIDI_SKIP_MAPPERS
#include "Mapper.h"
#include "MapperBuilder.h"
#endif

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>


namespace Xidi
{
    namespace Globals
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Holds all static data that falls under the global category.
        /// Used to make sure that globals are initialized as early as possible so that values are available during dynamic initialization.
        /// Implemented as a singleton object.
        class GlobalData
        {
        public:
            // -------- INSTANCE VARIABLES --------------------------------- //

            /// Pseudohandle of the current process.
            HANDLE gCurrentProcessHandle;

            /// PID of the current process.
            DWORD gCurrentProcessId;

            /// Holds information about the current system, as retrieved from Windows.
            SYSTEM_INFO gSystemInformation;

            /// Handle of the instance that represents the running form of this code.
            HINSTANCE gInstanceHandle;


        private:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default constructor. Objects cannot be constructed externally.
            GlobalData(void) : gCurrentProcessHandle(GetCurrentProcess()), gCurrentProcessId(GetProcessId(GetCurrentProcess())), gSystemInformation(), gInstanceHandle(nullptr)
            {
                GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&GlobalData::GetInstance, &gInstanceHandle);
                GetNativeSystemInfo(&gSystemInformation);
            }

            /// Copy constructor. Should never be invoked.
            GlobalData(const GlobalData& other) = delete;


        public:
            // -------- CLASS METHODS -------------------------------------- //

            /// Returns a reference to the singleton instance of this class.
            /// @return Reference to the singleton instance.
            static GlobalData& GetInstance(void)
            {
                static GlobalData globalData;
                return globalData;
            }
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //

#ifndef XIDI_SKIP_MAPPERS
        /// Holds custom mapper blueprints produced while reading from a configuration file.
        static Controller::MapperBuilder customMapperBuilder;
#endif


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

#ifndef XIDI_SKIP_MAPPERS
        /// Attempts to build all custom mappers held by the custom mapper builder object.
        /// Upon completion, regardless of outcome, clears out all of the stored blueprint objects.
        static inline void BuildCustomMappers(void)
        {
            if ((false == customMapperBuilder.Build()) && (false == GetConfigurationData().HasErrors()))
            {
                if (true == Message::IsLogFileEnabled())
                    Message::Output(Message::ESeverity::ForcedInteractiveWarning, L"Errors were encountered during custom mapper construction. See log file for more information.");
                else
                    Message::Output(Message::ESeverity::ForcedInteractiveWarning, L"Errors were encountered during custom mapper construction. Enable logging and see log file for more information.");
            }

            customMapperBuilder.Clear();
        }
#endif

#ifndef XIDI_SKIP_CONFIG
        /// Enables the log if it is not already enabled.
        /// Regardless, the minimum severity for output is set based on the parameter.
        /// @param [in] logLevel Logging level to configure as the minimum severity for output.
        static void EnableLog(Message::ESeverity logLevel)
        {
            static std::once_flag enableLogFlag;
            std::call_once(enableLogFlag, [logLevel]() -> void
                {
                    Message::CreateAndEnableLogFile();
                }
            );

            Message::SetMinimumSeverityForOutput(logLevel);
        }

        /// Enables the log, if it is configured in the configuration file.
        static void EnableLogIfConfigured(void)
        {
            const bool kLogEnabled = GetConfigurationData().GetFirstBooleanValue(Strings::kStrConfigurationSectionLog, Strings::kStrConfigurationSettingLogEnabled).value_or(false);
            const int64_t kLogLevel = GetConfigurationData().GetFirstIntegerValue(Strings::kStrConfigurationSectionLog, Strings::kStrConfigurationSettingLogLevel).value_or(0);

            if ((true == kLogEnabled) && (kLogLevel > 0))
            {
                // Offset the requested severity so that 0 = disabled, 1 = error, 2 = warning, etc.
                const Message::ESeverity configuredSeverity = (Message::ESeverity)(kLogLevel + (int64_t)Message::ESeverity::LowerBoundConfigurableValue);
                EnableLog(configuredSeverity);
            }
        }
#endif


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Globals.h" for documentation.

#ifndef XIDI_SKIP_CONFIG
        const Configuration::ConfigurationData& GetConfigurationData(void)
        {
            static Configuration::ConfigurationData configData;

            static std::once_flag readConfigFlag;
            std::call_once(readConfigFlag, []() -> void
                {
                    XidiConfigReader configReader;

#ifndef XIDI_SKIP_MAPPERS
                    configReader.SetMapperBuilder(&customMapperBuilder);
#endif

                    configData = configReader.ReadConfigurationFile(Strings::kStrConfigurationFilename);

                    if (true == configReader.HasReadErrors())
                    {
                        EnableLog(Message::ESeverity::Error);

                        Message::Output(Message::ESeverity::Error, L"Errors were encountered during configuration file reading.");
                        for (const auto& readError : configReader.GetReadErrors())
                            Message::OutputFormatted(Message::ESeverity::Error, L"    %s", readError.c_str());

                        Message::Output(Message::ESeverity::ForcedInteractiveWarning, L"Errors were encountered during configuration file reading. See log file on the Desktop for more information.");
                    }
                }
            );

            return configData;
        }
#endif

        // --------

        HANDLE GetCurrentProcessHandle(void)
        {
            return GlobalData::GetInstance().gCurrentProcessHandle;
        }

        // --------

        DWORD GetCurrentProcessId(void)
        {
            return GlobalData::GetInstance().gCurrentProcessId;
        }

        // --------

        HINSTANCE GetInstanceHandle(void)
        {
            return GlobalData::GetInstance().gInstanceHandle;
        }

        // --------

        const SYSTEM_INFO& GetSystemInformation(void)
        {
            return GlobalData::GetInstance().gSystemInformation;
        }

        // --------

        SVersionInfo GetVersion(void)
        {
            constexpr uint16_t kVersionStructured[] = {GIT_VERSION_STRUCT};
            static_assert(4 == _countof(kVersionStructured), "Invalid structured version information.");

            return {.major = kVersionStructured[0], .minor = kVersionStructured[1], .patch = kVersionStructured[2], .flags = kVersionStructured[3], .string = _CRT_WIDE(GIT_VERSION_STRING)};
        }

        // --------

        void Initialize(void)
        {
#ifndef XIDI_SKIP_CONFIG
            EnableLogIfConfigured();
#endif

#ifndef XIDI_SKIP_MAPPERS
            BuildCustomMappers();
            Controller::Mapper::DumpRegisteredMappers();
#endif
        }
    }
}
