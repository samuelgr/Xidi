/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file Globals.cpp
 *   Implementation of accessors and mutators for global data items. Intended for miscellaneous
 *   data elements with no other suitable place.
 **************************************************************************************************/

#include "Globals.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include <Infra/Core/Configuration.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiWindows.h"
#include "Strings.h"

#include "GitVersionInfo.generated.h"

#ifndef XIDI_SKIP_CONFIG
#include "XidiConfigReader.h"
#ifndef XIDI_SKIP_MAPPERS
#include "Mapper.h"
#include "MapperBuilder.h"
#endif
#endif

INFRA_DEFINE_PRODUCT_NAME_FROM_RESOURCE(
    Infra::ProcessInfo::GetThisModuleInstanceHandle(), IDS_XIDI_PRODUCT_NAME);
INFRA_DEFINE_PRODUCT_VERSION_FROM_GIT_VERSION_INFO();

namespace Xidi
{
  namespace Globals
  {
#ifndef XIDI_SKIP_CONFIG
#ifndef XIDI_SKIP_MAPPERS
    /// Holds custom mapper blueprints produced while reading from a configuration file.
    static Controller::MapperBuilder customMapperBuilder;

    /// Attempts to build all custom mappers held by the custom mapper builder object.
    /// Upon completion, regardless of outcome, clears out all of the stored blueprint objects.
    static inline void BuildCustomMappers(void)
    {
      if (false == customMapperBuilder.Build())
      {
        if (true == Infra::Message::IsLogFileEnabled())
          Infra::Message::Output(
              Infra::Message::ESeverity::ForcedInteractiveWarning,
              L"Errors were encountered during custom mapper construction. See log file for more information.");
        else
          Infra::Message::Output(
              Infra::Message::ESeverity::ForcedInteractiveWarning,
              L"Errors were encountered during custom mapper construction. Enable logging and see log file for more information.");
      }

      customMapperBuilder.Clear();
    }
#endif

    /// Enables the log if it is not already enabled.
    /// Regardless, the minimum severity for output is set based on the parameter.
    /// @param [in] logLevel Logging level to configure as the minimum severity for output.
    static void EnableLog(Infra::Message::ESeverity logLevel)
    {
      static std::once_flag enableLogFlag;
      std::call_once(
          enableLogFlag,
          [logLevel]() -> void
          {
            Infra::Message::CreateAndEnableLogFile();
          });

      Infra::Message::SetMinimumSeverityForOutput(logLevel);
    }

    /// Enables the log, if it is configured in the configuration file.
    static void EnableLogIfConfigured(void)
    {
      const bool logEnabled = GetConfigurationData()[Strings::kStrConfigurationSectionLog]
                                                    [Strings::kStrConfigurationSettingLogEnabled]
                                                        .ValueOr(false);
      const int64_t logLevel = GetConfigurationData()[Strings::kStrConfigurationSectionLog]
                                                     [Strings::kStrConfigurationSettingLogLevel]
                                                         .ValueOr(0);

      if ((true == logEnabled) && (logLevel > 0))
      {
        // Offset the requested severity so that 0 = disabled, 1 = error, 2 = warning, etc.
        const Infra::Message::ESeverity configuredSeverity = (Infra::Message::ESeverity)(
            logLevel + (int64_t)Infra::Message::ESeverity::LowerBoundConfigurableValue);
        EnableLog(configuredSeverity);
      }
    }
#endif

    bool DoesCurrentProcessHaveInputFocus(void)
    {
      DWORD foregroundProcess = 0;
      GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);

      return (GetCurrentProcessId() == foregroundProcess);
    }

    const Infra::Configuration::ConfigurationData& GetConfigurationData(void)
    {
      static Infra::Configuration::ConfigurationData configData;

#ifndef XIDI_SKIP_CONFIG
      static std::once_flag readConfigFlag;
      std::call_once(
          readConfigFlag,
          []() -> void
          {
            XidiConfigReader configReader;

#ifndef XIDI_SKIP_MAPPERS
            configReader.SetMapperBuilder(&customMapperBuilder);
#endif

            configData = configReader.ReadConfigurationFile();

            if (false == configReader.HasErrorMessages())
            {
#ifndef XIDI_SKIP_MAPPERS
              BuildCustomMappers();
#endif
            }
            else
            {
              EnableLog(Infra::Message::ESeverity::Error);

              Infra::Message::Output(
                  Infra::Message::ESeverity::Error,
                  L"Errors were encountered during configuration file reading.");
              configReader.LogAllErrorMessages();
              Infra::Message::Output(
                  Infra::Message::ESeverity::Error,
                  L"None of the settings in the configuration file were applied. Fix the errors and restart the application.");

              Infra::Message::Output(
                  Infra::Message::ESeverity::ForcedInteractiveWarning,
                  L"Errors were encountered during configuration file reading. See log file on the Desktop for more information.");

              configData.Clear();
            }
          });
#endif

      return configData;
    }

    void Initialize(void)
    {
#ifndef XIDI_SKIP_CONFIG
      EnableLogIfConfigured();

#ifndef XIDI_SKIP_MAPPERS
      Controller::Mapper::DumpRegisteredMappers();
#endif
#endif
    }
  } // namespace Globals
} // namespace Xidi
