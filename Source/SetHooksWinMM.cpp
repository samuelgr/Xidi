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

#include "Message.h"
#include "SetHooks.h"
#include "Strings.h"
#include "TemporaryBuffer.h"

#include <Hookshot/Hookshot.h>
#include <shlwapi.h>
#include <string>
#include <string_view>


namespace Xidi
{
    // -------- FUNCTIONS -------------------------------------------------- //
    // See "SetHooks.h" for documentation.

    void SetHooksWinMM(Hookshot::IHookshot* hookshot)
    {
        static const std::wstring kImportLibraryFilename(std::wstring(Strings::kStrXidiDirectoryName) + std::wstring(Strings::kStrLibraryNameWinMM));
        static const std::wstring_view kApiSetJoystickName = L"api-ms-win-mm-joystick-l1-1-0";

        Message::Output(Message::ESeverity::Info, L"Beginning to set hooks for WinMM.");

        // First, check preconditions.
        // System joystick functions are only hooked if the WinMM DLL file in the same directory as this hook module is not loaded but the system API set DLL is already loaded.
        if (TRUE == PathFileExists(kImportLibraryFilename.c_str()))
        {
            const HMODULE kImportLibraryModule = GetModuleHandle(kImportLibraryFilename.c_str());
            if ((nullptr == kImportLibraryModule) || (INVALID_HANDLE_VALUE == kImportLibraryModule))
            {
                Message::OutputFormatted(Message::ESeverity::Debug, L"WinMM DLL %s exists and is not loaded.", kImportLibraryFilename.c_str());

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
            }
            else
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

        // Next, load the WinMM DLL file in the same directory as this hook module.
        const HMODULE kImportLibraryModule = LoadLibrary(kImportLibraryFilename.c_str());
        if ((nullptr == kImportLibraryModule) || (INVALID_HANDLE_VALUE == kImportLibraryModule))
        {
            Message::OutputFormatted(Message::ESeverity::Error, L"Failed to load WinMM DLL %s. Unable to hook WinMM joystick functions.", kImportLibraryFilename.c_str());
            return;
        }
    }
}
