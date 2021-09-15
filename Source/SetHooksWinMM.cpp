/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file SetHooksWinMM.cpp
 *   Implementation of all functionality for setting WinMM hooks.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "Globals.h"
#include "Message.h"
#include "SetHooks.h"
#include "Strings.h"
#include "TemporaryBuffer.h"

#include <Hookshot/Hookshot.h>
#include <map>
#include <shlwapi.h>
#include <string>
#include <string_view>


namespace Xidi
{
    // -------- FUNCTIONS -------------------------------------------------- //
    // See "SetHooks.h" for documentation.

    void SetHooksWinMM(Hookshot::IHookshot* hookshot)
    {
        Message::Output(Message::ESeverity::Info, L"Beginning to set hooks for WinMM.");

        // First precondition.
        // System joystick functions are only hooked if the system API set DLL is already loaded.
        static const std::wstring_view kApiSetJoystickName = L"api-ms-win-mm-joystick-l1-1-0";
        const HMODULE kSystemLibraryHandle = GetModuleHandle(kApiSetJoystickName.data());
        if ((nullptr != kSystemLibraryHandle) && (INVALID_HANDLE_VALUE != kSystemLibraryHandle))
        {
            TemporaryBuffer<wchar_t> systemModuleName;
            GetModuleFileName(kSystemLibraryHandle, systemModuleName, systemModuleName.Count());
            Message::OutputFormatted(Message::ESeverity::Debug, L"System API set '%s' is already loaded as %s.", kApiSetJoystickName.data(), &systemModuleName[0]);
        }
        else
        {
            Message::OutputFormatted(Message::ESeverity::Debug, L"System API set '%s' is not loaded. Not attempting to hook WinMM joystick functions.", kApiSetJoystickName.data());
            return;
        }

        // Second precondition.
        // System joystick functions are only hooked if there exists a WinMM DLL in the same directory as this hook module and it is not already loaded.
        static const std::wstring kImportLibraryFilename(std::wstring(Strings::kStrXidiDirectoryName) + std::wstring(Strings::kStrLibraryNameWinMM));
        if (TRUE == PathFileExists(kImportLibraryFilename.c_str()))
        {
            const HMODULE kImportLibraryHandle = GetModuleHandle(kImportLibraryFilename.c_str());
            if ((nullptr != kImportLibraryHandle) && (INVALID_HANDLE_VALUE != kImportLibraryHandle))
            {
                Message::OutputFormatted(Message::ESeverity::Debug, L"WinMM DLL %s exists and is already loaded. Not attempting to hook WinMM joystick functions.", kImportLibraryFilename.c_str());
                return;
            }
        }
        else
        {
            Message::OutputFormatted(Message::ESeverity::Debug, L"WinMM DLL %s does not exist. Not attempting to hook WinMM joystick functions.", kImportLibraryFilename.c_str());
            return;
        }

        // Once all preconditions are satisfied, attempt to load the WinMM DLL in the same directory as this hook module.
        // Then use Hookshot to redirect all WinMM joystick API functions provided by the system library to the Xidi library.
        const HMODULE kImportLibraryHandle = LoadLibrary(kImportLibraryFilename.c_str());
        if ((nullptr == kImportLibraryHandle) || (INVALID_HANDLE_VALUE == kImportLibraryHandle))
        {
            Message::OutputFormatted(Message::ESeverity::Error, L"Failed to load WinMM DLL %s. Unable to hook WinMM joystick functions.", kImportLibraryFilename.c_str());
            return;
        }
        else
        {
            Message::OutputFormatted(Message::ESeverity::Debug, L"Successfully loaded WinMM DLL %s.", kImportLibraryFilename.c_str());
        }

        const Xidi::Api::TGetInterfaceFunc funcXidiApiGetInterface = (Xidi::Api::TGetInterfaceFunc)GetProcAddress(kImportLibraryHandle, "XidiApiGetInterface");
        if (nullptr == funcXidiApiGetInterface)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Unloading WinMM DLL %s because it is missing one or more required Xidi API entry points.", kImportLibraryFilename.c_str());
            FreeLibrary(kImportLibraryHandle);
            return;
        }

        Xidi::Api::IImportFunctions* const importFunctions = (Xidi::Api::IImportFunctions*)funcXidiApiGetInterface(Xidi::Api::EClass::ImportFunctions);
        if (nullptr == importFunctions)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Unloading WinMM DLL %s because it does not support the required Xidi API interface.", kImportLibraryFilename.c_str());
            FreeLibrary(kImportLibraryHandle);
            return;
        }

        std::map<std::wstring_view, const void*> replacementImportFunctions;
        for (auto importFunctionName : importFunctions->GetReplaceable())
        {
            TemporaryBuffer<char> importFunctionNameAscii;
            wcstombs_s(nullptr, importFunctionNameAscii, importFunctionNameAscii.Count(), importFunctionName.data(), importFunctionNameAscii.Size());

            void* const kSystemFunc = GetProcAddress(kSystemLibraryHandle, importFunctionNameAscii);
            if (nullptr == kSystemFunc)
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"Function %s is missing from the system API set module.", importFunctionName.data());
                continue;
            }

            void* const kImportFunc = GetProcAddress(kImportLibraryHandle, importFunctionNameAscii);
            if (nullptr == kImportFunc)
            {
                Message::OutputFormatted(Message::ESeverity::Warning, L"Function %s is missing from the WinMM DLL file %s.", importFunctionName.data(), kImportLibraryFilename.c_str());
                continue;
            }

            const Hookshot::EResult kHookResult = hookshot->CreateHook(kSystemFunc, kImportFunc);
            OutputSetHookResult(importFunctionName.data(), kHookResult);
            if (false == Hookshot::SuccessfulResult(kHookResult))
                continue;

            replacementImportFunctions[importFunctionName.data()] = hookshot->GetOriginalFunction(kSystemFunc);
        }

        const size_t kNumSuccessfullyReplaced = importFunctions->SetReplaceable(replacementImportFunctions);
        if (replacementImportFunctions.size() == kNumSuccessfullyReplaced)
        {
            // Every hooked function has its original version successfully submitted to Xidi.
            // This is important because Xidi invokes the functions it invokes from the system, and the addresses it uses need to provide the system functionality.
            Message::OutputFormatted(Message::ESeverity::Debug, L"Hooked and successfully replaced the import addresses for %d function(s).", (int)kNumSuccessfullyReplaced);
        }
        else
        {
            // It is a serious error to have hooked system functions but only replaced the import addresses on a strict subset of them.
            // Xidi invokes the functions it imports from the system, and failure to replace the import addresses could lead to infinite accidental recursion because the system functions are redirected to Xidi.
            // Thus, the application is practically guaranteed to freeze or crash.
            Message::OutputFormatted(Message::ESeverity::ForcedInteractiveError, L"Hooked %d function(s) but only successfully replaced the import addresses for %d of them. The application will not function correctly in this state.", (int)replacementImportFunctions.size(), (int)kNumSuccessfullyReplaced);
            TerminateProcess(Globals::GetCurrentProcessHandle(), (UINT)-1);
        }
    }
}
