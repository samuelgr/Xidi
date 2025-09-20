/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file HookModuleMain.cpp
 *   Entry point when injecting Xidi as a hook module.
 **************************************************************************************************/

#include <Hookshot/Hookshot.h>
#include <Infra/Core/Message.h>

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "SetHooks.h"
#include "Strings.h"

namespace Xidi
{
  static HMODULE GetMainLibraryHandle(void)
  {
    static HMODULE xidiLibraryHandle = LoadLibraryW(Strings::GetXidiMainLibraryFilename().data());
    if (NULL == xidiLibraryHandle)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Main Xidi library \"%.*s\" could not be loaded.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.",
          static_cast<int>(Strings::GetXidiMainLibraryFilename().length()),
          Strings::GetXidiMainLibraryFilename().data());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
      return nullptr;
    }

    return xidiLibraryHandle;
  }

  static Api::IImportFunctions* GetImportFunctionsApi(void)
  {
    HMODULE xidiLibraryHandle = GetMainLibraryHandle();
    if (NULL == xidiLibraryHandle)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Main Xidi library \"%.*s\" could not be loaded.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.",
          static_cast<int>(Strings::GetXidiMainLibraryFilename().length()),
          Strings::GetXidiMainLibraryFilename().data());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
      return nullptr;
    }

    const Api::TGetInterfaceFunc funcXidiApiGetInterface = reinterpret_cast<Api::TGetInterfaceFunc>(
        GetProcAddress(xidiLibraryHandle, Api::kGetInterfaceFuncName));
    if (nullptr == funcXidiApiGetInterface)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Main Xidi library \"%.*s\" is missing a required entry point.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.",
          static_cast<int>(Strings::GetXidiMainLibraryFilename().length()),
          Strings::GetXidiMainLibraryFilename().data());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
      return nullptr;
    }

    Api::IImportFunctions* xidiImportFunctions = reinterpret_cast<Xidi::Api::IImportFunctions*>(
        funcXidiApiGetInterface(Xidi::Api::EClass::ImportFunctions));
    if (nullptr == xidiImportFunctions)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Main Xidi library \"%.*s\" is missing a required interface for replacing imported system functions.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.",
          static_cast<int>(Strings::GetXidiMainLibraryFilename().length()),
          Strings::GetXidiMainLibraryFilename().data());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
      return nullptr;
    }

    return xidiImportFunctions;
  }
} // namespace Xidi

HOOKSHOT_HOOK_MODULE_ENTRY(hookshot)
{
  // CoCreateInstance
  {
    Xidi::SetHookCoCreateInstance(hookshot);
  }

  // DInput
  {
    Hookshot::EResult notifyOnLibraryLoadResult = hookshot->NotifyOnLibraryLoad(
        Xidi::Strings::GetSystemLibraryFilenameDirectInput().data(),
        [](Hookshot::IHookshot* hookshot, const wchar_t* modulePath) -> void
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Debug,
              L"Notification received from Hookshot: OnLibraryLoad: \"%s\" for DInput.",
              modulePath);
        });
    if (false == Hookshot::SuccessfulResult(notifyOnLibraryLoadResult))
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Failed to initialize hooks for DInput: Hookshot::EResult = %u.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.");
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }
  }

  // DInput8
  {
    Hookshot::EResult notifyOnLibraryLoadResult = hookshot->NotifyOnLibraryLoad(
        Xidi::Strings::GetSystemLibraryFilenameDirectInput8().data(),
        [](Hookshot::IHookshot* hookshot, const wchar_t* modulePath) -> void
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Debug,
              L"Notification received from Hookshot: OnLibraryLoad: \"%s\" for DInput8.",
              modulePath);
        });
    if (false == Hookshot::SuccessfulResult(notifyOnLibraryLoadResult))
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Failed to initialize hooks for DInput8: Hookshot::EResult = %u.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.");
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }
  }

  // WinMM
  {
    Hookshot::EResult notifyOnLibraryLoadResult = hookshot->NotifyOnLibraryLoad(
        Xidi::Strings::GetSystemLibraryFilenameWinMM().data(),
        [](Hookshot::IHookshot* hookshot, const wchar_t* modulePath) -> void
        {
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Debug,
              L"Notification received from Hookshot: OnLibraryLoad: \"%s\" for WinMM.",
              modulePath);
          Xidi::SetHooksWinMM(
              hookshot, Xidi::GetImportFunctionsApi(), Xidi::GetMainLibraryHandle(), modulePath);
        });
    if (false == Hookshot::SuccessfulResult(notifyOnLibraryLoadResult))
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::ForcedInteractiveError,
          L"Failed to initialize hooks for WinMM: Hookshot::EResult = %u.\n\nXidi failed to initialize and is therefore terminating the application to avoid unexpected behavior.");
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }
  }
}
