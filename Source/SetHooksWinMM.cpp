/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file SetHooksWinMM.cpp
 *   Implementation of all functionality for setting WinMM hooks.
 **************************************************************************************************/

#include <map>
#include <string>
#include <string_view>

#include <Hookshot/Hookshot.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>
#include <Infra/Core/TemporaryBuffer.h>

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "SetHooks.h"
#include "Strings.h"

namespace Xidi
{
  /// Handle for the main Xidi library.
  static HMODULE xidiLibraryHandle = NULL;

  /// Handle for the system WinMM library.
  static HMODULE winmmLibraryHandle = NULL;

  /// Frees all loaded library handles. Used in case of an error when setting hooks.
  static void FreeLibraryHandles(void)
  {
    if (NULL != xidiLibraryHandle) FreeLibrary(xidiLibraryHandle);
    if (NULL != winmmLibraryHandle) FreeLibrary(winmmLibraryHandle);
  }

  /// Attempts to obtain the interface pointer from the main Xidi library for replacing import
  /// functions.
  /// @param [in] xidiMainLibraryFilename Filename for the main Xidi library. Used for logging.
  /// @param [in] xidiLibraryHandle Handle for the main Xidi library to be queried.
  /// @return Interface pointer on success, or `nullptr` on failure.
  static Xidi::Api::IImportFunctions* GetXidiImportFunctionsApi(
      std::wstring_view xidiMainLibraryFilename, HMODULE xidiLibraryHandle)
  {
    const Xidi::Api::TGetInterfaceFunc funcXidiApiGetInterface =
        reinterpret_cast<Xidi::Api::TGetInterfaceFunc>(
            GetProcAddress(xidiLibraryHandle, Xidi::Api::kGetInterfaceFuncName));
    if (nullptr == funcXidiApiGetInterface) return nullptr;
    return reinterpret_cast<Xidi::Api::IImportFunctions*>(
        funcXidiApiGetInterface(Xidi::Api::EClass::ImportFunctions));
  }

  void SetHooksWinMM(Hookshot::IHookshot* hookshot)
  {
    Infra::Message::Output(Infra::Message::ESeverity::Info, L"Beginning to set hooks for WinMM.");

    const std::wstring_view xidiMainLibraryFilename = Strings::GetXidiMainLibraryFilename();
    HMODULE xidiLibraryHandle = LoadLibraryW(xidiMainLibraryFilename.data());
    if (NULL == xidiLibraryHandle)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Failed to set hooks for WinMM: Library \"%.*s\" could not be loaded.",
          static_cast<int>(xidiMainLibraryFilename.length()),
          xidiMainLibraryFilename.data());
      FreeLibraryHandles();
      return;
    }

    Xidi::Api::IImportFunctions* xidiImportFunctions =
        GetXidiImportFunctionsApi(xidiMainLibraryFilename, xidiLibraryHandle);
    if (nullptr == xidiImportFunctions)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Failed to set hooks for WinMM: Library \"%.*s\" is missing one or more required Xidi API entry points.",
          static_cast<int>(xidiMainLibraryFilename.length()),
          xidiMainLibraryFilename.data());
      FreeLibraryHandles();
      return;
    }

    const std::wstring_view winmmLibraryFilename = Strings::GetSystemLibraryFilenameWinMM();
    HMODULE winmmLibraryHandle = LoadLibraryW(winmmLibraryFilename.data());
    if (NULL == winmmLibraryHandle)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Failed to set hooks for WinMM: Library \"%.*s\" could not be loaded.",
          static_cast<int>(winmmLibraryFilename.length()),
          winmmLibraryFilename.data());
      return;
    }

    const auto& replaceableImportFunctionNames = xidiImportFunctions->GetReplaceable();
    std::map<std::wstring_view, const void*> replacementImportFunctions;
    for (const auto& systemFunctionName : replaceableImportFunctionNames)
    {
      auto systemFunctionNameAscii = Infra::Strings::ConvertWideToNarrow(systemFunctionName.data());
      void* const systemFunc = GetProcAddress(winmmLibraryHandle, systemFunctionNameAscii.Data());
      if (nullptr == systemFunc)
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Warning,
            L"Entry point \"%.*s\" is missing from the system WinMM library.",
            static_cast<int>(systemFunctionName.length()),
            systemFunctionName.data());
        continue;
      }

      Infra::TemporaryString replacementFunctionName = L"winmm_";
      replacementFunctionName << systemFunctionName;
      auto replacementFunctionNameAscii =
          Infra::Strings::ConvertWideToNarrow(replacementFunctionName.AsCString());
      void* const replacementFunc =
          GetProcAddress(xidiLibraryHandle, replacementFunctionNameAscii.Data());
      if (nullptr == replacementFunc)
      {
        Infra::Message::OutputFormatted(
            Infra::Message::ESeverity::Warning,
            L"Entry point \"%s\" is missing from the main Xidi library.",
            replacementFunctionName.AsCString());
        continue;
      }

      const Hookshot::EResult hookResult = hookshot->CreateHook(systemFunc, replacementFunc);
      OutputSetHookResult(systemFunctionName.data(), hookResult);
      if (false == Hookshot::SuccessfulResult(hookResult)) continue;

      replacementImportFunctions[systemFunctionName.data()] =
          hookshot->GetOriginalFunction(systemFunc);
    }

    const size_t numUnsuccessfullyHooked =
        replaceableImportFunctionNames.size() - replacementImportFunctions.size();
    if (replaceableImportFunctionNames.size() == numUnsuccessfullyHooked)
    {
      // Not even a single function was successfully hooked.
      // There are no import functions to replace. The application is in a consistent state and can
      // run, but Xidi's WinMM form will not function.
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Failed to hook any of the %d function(s) attempted. The application can run in this state, but Xidi will likely not work.",
          static_cast<int>(numUnsuccessfullyHooked));
      return;
    }
    else if (0 != numUnsuccessfullyHooked)
    {
      // Some functions were successfully hooked, but others were not.
      // This is a serious error because some of the application's joystick API calls will be
      // redirected to Xidi while others will not, leading to inconsistent behavior.
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Failed to hook %d function(s) out of a total of %d attempted. The application will not function correctly in this state and is therefore being terminated.",
          static_cast<int>(numUnsuccessfullyHooked),
          static_cast<int>(replaceableImportFunctionNames.size()));
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }

    const size_t numSuccessfullyReplaced =
        xidiImportFunctions->SetReplaceable(replacementImportFunctions);
    if (replacementImportFunctions.size() == numSuccessfullyReplaced)
    {
      // Every hooked function has its original version successfully submitted to Xidi.
      // This is important because Xidi invokes the functions it invokes from the system, and the
      // addresses it uses need to provide the system functionality.
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Debug,
          L"Hooked and successfully replaced the import addresses for %d function(s).",
          (int)numSuccessfullyReplaced);
    }
    else
    {
      // It is a serious error to have hooked system functions but only replaced the import
      // addresses on a strict subset of them. Xidi invokes the functions it imports from the
      // system, and failure to replace the import addresses could lead to infinite accidental
      // recursion because the system functions are redirected to Xidi. Thus, the application is
      // practically guaranteed to freeze or crash.
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Hooked %d function(s) but only successfully replaced the import addresses for %d of them. The application will not function correctly in this state and is therefore being terminated.",
          (int)replacementImportFunctions.size(),
          (int)numSuccessfullyReplaced);
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }
  }
} // namespace Xidi
