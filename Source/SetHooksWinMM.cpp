/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file SetHooksWinMM.cpp
 *   Implementation of all functionality for setting WinMM hooks.
 **************************************************************************************************/

#include <shlwapi.h>

#include <map>
#include <string>
#include <string_view>

#include <Hookshot/Hookshot.h>
#include <Infra/Core/ProcessInfo.h>
#include <Infra/Core/TemporaryBuffer.h>

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "Message.h"
#include "SetHooks.h"
#include "Strings.h"

namespace Xidi
{
  void SetHooksWinMM(Hookshot::IHookshot* hookshot)
  {
    Message::Output(Message::ESeverity::Info, L"Beginning to set hooks for WinMM.");

    // First precondition.
    // System joystick functions are only hooked if there exists a WinMM DLL in the same directory
    // as this hook module and it is not already loaded.
    static const std::wstring kImportLibraryFilename(
        std::wstring(Strings::kStrXidiDirectoryName) + std::wstring(Strings::kStrLibraryNameWinMM));
    if (TRUE == PathFileExists(kImportLibraryFilename.c_str()))
    {
      const HMODULE importLibraryHandle = GetModuleHandle(kImportLibraryFilename.c_str());
      if ((nullptr != importLibraryHandle) && (INVALID_HANDLE_VALUE != importLibraryHandle))
      {
        Message::OutputFormatted(
            Message::ESeverity::Debug,
            L"%s exists and is already loaded. Not attempting to hook WinMM joystick functions.",
            kImportLibraryFilename.c_str());
        return;
      }
    }
    else
    {
      Message::OutputFormatted(
          Message::ESeverity::Debug,
          L"%s does not exist. Not attempting to hook WinMM joystick functions.",
          kImportLibraryFilename.c_str());
      return;
    }

    // Second precondition.
    // System joystick functions are only hooked if the system API set DLL is already loaded.
    static const std::wstring_view kApiSetJoystickName = L"api-ms-win-mm-joystick-l1-1-0";
    const HMODULE systemLibraryHandle = GetModuleHandle(kApiSetJoystickName.data());
    if ((nullptr != systemLibraryHandle) && (INVALID_HANDLE_VALUE != systemLibraryHandle))
    {
      Infra::TemporaryBuffer<wchar_t> systemModuleName;
      GetModuleFileName(systemLibraryHandle, systemModuleName.Data(), systemModuleName.Capacity());
      Message::OutputFormatted(
          Message::ESeverity::Debug,
          L"System API set '%s' is already loaded as %s.",
          kApiSetJoystickName.data(),
          &systemModuleName[0]);
    }
    else
    {
      Message::OutputFormatted(
          Message::ESeverity::Debug,
          L"System API set '%s' is not loaded. Not attempting to hook WinMM joystick functions.",
          kApiSetJoystickName.data());
      return;
    }

    // Once all preconditions are satisfied, attempt to load the WinMM DLL in the same directory as
    // this hook module. Then use Hookshot to redirect all WinMM joystick API functions provided by
    // the system library to the Xidi library.
    const HMODULE importLibraryHandle = LoadLibrary(kImportLibraryFilename.c_str());
    if ((nullptr == importLibraryHandle) || (INVALID_HANDLE_VALUE == importLibraryHandle))
    {
      Message::OutputFormatted(
          Message::ESeverity::Error,
          L"Failed to load %s. Unable to hook WinMM joystick functions.",
          kImportLibraryFilename.c_str());
      return;
    }
    else
    {
      Message::OutputFormatted(
          Message::ESeverity::Debug, L"Successfully loaded %s.", kImportLibraryFilename.c_str());
    }

    const Xidi::Api::TGetInterfaceFunc funcXidiApiGetInterface =
        (Xidi::Api::TGetInterfaceFunc)GetProcAddress(importLibraryHandle, "XidiApiGetInterface");
    if (nullptr == funcXidiApiGetInterface)
    {
      Message::OutputFormatted(
          Message::ESeverity::Warning,
          L"Unloading %s because it is missing one or more required Xidi API entry points.",
          kImportLibraryFilename.c_str());
      FreeLibrary(importLibraryHandle);
      return;
    }

    Xidi::Api::IImportFunctions* const importFunctions =
        (Xidi::Api::IImportFunctions*)funcXidiApiGetInterface(Xidi::Api::EClass::ImportFunctions);
    if (nullptr == importFunctions)
    {
      Message::OutputFormatted(
          Message::ESeverity::Warning,
          L"Unloading %s because it does not support the required Xidi API interface.",
          kImportLibraryFilename.c_str());
      FreeLibrary(importLibraryHandle);
      return;
    }

    const auto& replaceableImportFunctionNames = importFunctions->GetReplaceable();
    std::map<std::wstring_view, const void*> replacementImportFunctions;
    for (auto importFunctionName : replaceableImportFunctionNames)
    {
      Infra::TemporaryBuffer<char> importFunctionNameAscii;
      wcstombs_s(
          nullptr,
          importFunctionNameAscii.Data(),
          importFunctionNameAscii.Capacity(),
          &importFunctionName[0],
          importFunctionNameAscii.CapacityBytes());

      void* const systemFunc = GetProcAddress(systemLibraryHandle, importFunctionNameAscii.Data());
      if (nullptr == systemFunc)
      {
        Message::OutputFormatted(
            Message::ESeverity::Warning,
            L"Function %s is missing from the system API set module.",
            &importFunctionName[0]);
        continue;
      }

      void* const importFunc = GetProcAddress(importLibraryHandle, importFunctionNameAscii.Data());
      if (nullptr == importFunc)
      {
        Message::OutputFormatted(
            Message::ESeverity::Warning,
            L"Function %s is missing from %s.",
            &importFunctionName[0],
            kImportLibraryFilename.c_str());
        continue;
      }

      const Hookshot::EResult hookResult = hookshot->CreateHook(systemFunc, importFunc);
      OutputSetHookResult(&importFunctionName[0], hookResult);
      if (false == Hookshot::SuccessfulResult(hookResult)) continue;

      replacementImportFunctions[importFunctionName.data()] =
          hookshot->GetOriginalFunction(systemFunc);
    }

    const size_t numUnsuccessfullyHooked =
        replaceableImportFunctionNames.size() - replacementImportFunctions.size();
    if (replaceableImportFunctionNames.size() == numUnsuccessfullyHooked)
    {
      // Not even a single function was successfully hooked.
      // There are no import functions to replace. The application is in a consistent state and can
      // run, but Xidi's WinMM form will not function.
      Message::OutputFormatted(
          Message::ESeverity::Error,
          L"Failed to hook any of the %d function(s) attempted. The application can run in this state, but Xidi will likely not work.",
          (int)numUnsuccessfullyHooked);
      return;
    }
    else if (0 != numUnsuccessfullyHooked)
    {
      // Some functions were successfully hooked, but others were not.
      // This is a serious error because some of the application's joystick API calls will be
      // redirected to Xidi while others will not, leading to inconsistent behavior.
      Message::OutputFormatted(
          Message::ESeverity::ForcedInteractiveError,
          L"Failed to hook %d function(s) out of a total of %d attempted. The application will likely not function correctly in this state.",
          (int)numUnsuccessfullyHooked,
          (int)replaceableImportFunctionNames.size());
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }

    const size_t numSuccessfullyReplaced =
        importFunctions->SetReplaceable(replacementImportFunctions);
    if (replacementImportFunctions.size() == numSuccessfullyReplaced)
    {
      // Every hooked function has its original version successfully submitted to Xidi.
      // This is important because Xidi invokes the functions it invokes from the system, and the
      // addresses it uses need to provide the system functionality.
      Message::OutputFormatted(
          Message::ESeverity::Debug,
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
      Message::OutputFormatted(
          Message::ESeverity::ForcedInteractiveError,
          L"Hooked %d function(s) but only successfully replaced the import addresses for %d of them. The application will likely not function correctly in this state.",
          (int)replacementImportFunctions.size(),
          (int)numSuccessfullyReplaced);
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
    }
  }
} // namespace Xidi
