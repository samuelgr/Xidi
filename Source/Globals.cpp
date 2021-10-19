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
#include "Mapper.h"
#include "Message.h"
#include "Strings.h"
#include "XidiConfigReader.h"

#ifndef XIDI_SKIP_CONFIG
#include "Configuration.h"
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


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

#ifndef XIDI_SKIP_CONFIG
        /// Enables the log, if it is configured in the configuration file.
        static void EnableLogIfConfigured(void)
        {
            const Configuration::ConfigurationFile& config = GetConfiguration();

            bool logEnabled = false;
            int64_t logLevel = 0;

            if (true == config.IsDataValid())
            {
                if (true == config.GetData().SectionNamePairExists(Strings::kStrConfigurationSectionLog, Strings::kStrConfigurationSettingLogEnabled))
                    logEnabled = config.GetData()[Strings::kStrConfigurationSectionLog][Strings::kStrConfigurationSettingLogEnabled].FirstValue().GetBooleanValue();

                if (true == config.GetData().SectionNamePairExists(Strings::kStrConfigurationSectionLog, Strings::kStrConfigurationSettingLogLevel))
                    logLevel = config.GetData()[Strings::kStrConfigurationSectionLog][Strings::kStrConfigurationSettingLogLevel].FirstValue().GetIntegerValue();
            }

            if ((true == logEnabled) && (logLevel > 0))
            {
                // Offset the requested severity so that 0 = disabled, 1 = error, 2 = warning, etc.
                const Message::ESeverity configureSeverity = (Message::ESeverity)(logLevel + (int64_t)Message::ESeverity::LowerBoundConfigurableValue);

                Message::CreateAndEnableLogFile();
                Message::SetMinimumSeverityForOutput((Message::ESeverity)configureSeverity);
            }
        }
#endif


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Globals.h" for documentation.

#ifndef XIDI_SKIP_CONFIG
        const Configuration::ConfigurationFile& GetConfiguration(void)
        {
            static Configuration::ConfigurationFile configuration(std::make_unique<XidiConfigReader>());

            static std::once_flag readConfigFlag;
            std::call_once(readConfigFlag, []() -> void
                {
                    configuration.Read(Strings::kStrConfigurationFilename);

                    if (Configuration::EFileReadResult::Malformed == configuration.GetFileReadResult())
                        Message::Output(Message::ESeverity::ForcedInteractiveError, configuration.GetReadErrorMessage().data());
                }
            );

            return configuration;
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
            Controller::Mapper::DumpRegisteredMappers();
#endif
        }
    }
}
