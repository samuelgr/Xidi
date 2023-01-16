/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ImportApiXInput.cpp
 *   Implementations of functions for accessing the XInput API imported from
 *   the native XInput library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"
#include "ImportApiXInput.h"
#include "Message.h"

#include <array>
#include <mutex>


namespace Xidi
{
    namespace ImportApiXInput
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Holds pointers to all the functions imported from the native XInput library.
        /// Exposes them as both an array of typeless pointers and a named structure of type-specific pointers.

        union UImportTable
        {
            struct
            {
                DWORD(WINAPI* XInputGetState)(DWORD, XINPUT_STATE*);
                DWORD(WINAPI* XInputSetState)(DWORD, XINPUT_VIBRATION*);
            } named;

            const void* ptr[sizeof(named) / sizeof(const void*)];
        };
        static_assert(sizeof(UImportTable::named) == sizeof(UImportTable::ptr), "Element size mismatch.");


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// Holds the imported WinMM API function addresses.
        static UImportTable importTable;


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Shows an error and terminates the process in the event of failure to import a particular function from the import library.
        /// @param [in] libraryName Name of the library from which the import is being attempted.
        /// @param [in] functionName Name of the function whose import attempt failed.
        static void TerminateProcessBecauseImportFailed(LPCWSTR libraryName, LPCWSTR functionName)
        {
            Message::OutputFormatted(Message::ESeverity::ForcedInteractiveError, L"Import library \"%s\" is missing XInput function \"%s\".\n\nXidi cannot function without it.", libraryName, functionName);
            TerminateProcess(Globals::GetCurrentProcessHandle(), (UINT)-1);
        }

        /// Shows an error and terminates the process in the event of failure to load any XInput library.
        static void TerminateProcessBecauseNoXInputLibraryLoaded(void)
        {
            Message::Output(Message::ESeverity::ForcedInteractiveError, L"Failed to load an XInput library.\n\nXidi cannot function without it.");
            TerminateProcess(Globals::GetCurrentProcessHandle(), (UINT)-1);
        }


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "ImportApiXInput.h" for documentation.

        void Initialize(void)
        {
            static std::once_flag initializeFlag;
            std::call_once(initializeFlag, []() -> void
                {
                    // Try loading each possible DLL, in order from most preferred to least preferred.
                    constexpr std::array kXInputLibraryNamesOrdered = {L"xinput1_4.dll", L"xinput1_3.dll", L"xinput1_2.dll", L"xinput1_1.dll", L"xinput9_1_0.dll"};

                    for (const auto& kXInputLibraryName : kXInputLibraryNamesOrdered)
                    {
                        // Initialize the import table.
                        ZeroMemory(&importTable, sizeof(importTable));

                        Message::OutputFormatted(Message::ESeverity::Info, L"Attempting to import XInput functions from %s.", kXInputLibraryName);
                        HMODULE loadedLibrary = LoadLibraryEx(kXInputLibraryName, nullptr, 0);                        
                        if (nullptr == loadedLibrary)
                        {
                            Message::OutputFormatted(Message::ESeverity::Warning, L"Failed to import XInput functions from %s.", kXInputLibraryName);
                            continue;
                        }

                        // Attempt to obtain the addresses of all imported API functions.
                        FARPROC procAddress = nullptr;

                        procAddress = GetProcAddress(loadedLibrary, "XInputGetState");
                        if (nullptr == procAddress) TerminateProcessBecauseImportFailed(kXInputLibraryName, L"XInputGetState");
                        importTable.named.XInputGetState = (DWORD(WINAPI*)(DWORD, XINPUT_STATE*))procAddress;

                        procAddress = GetProcAddress(loadedLibrary, "XInputSetState");
                        if (nullptr == procAddress) TerminateProcessBecauseImportFailed(kXInputLibraryName, L"XInputSetState");
                        importTable.named.XInputSetState = (DWORD(WINAPI*)(DWORD, XINPUT_VIBRATION*))procAddress;

                        // Initialization complete.
                        Message::OutputFormatted(Message::ESeverity::Info, L"Successfully initialized imported XInput functions.");
                        return;
                    }

                    TerminateProcessBecauseNoXInputLibraryLoaded();
                }
            );
        }

        // --------

        DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
        {
            Initialize();
            return importTable.named.XInputGetState(dwUserIndex, pState);
        }

        // --------

        DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
        {
            Initialize();
            return importTable.named.XInputSetState(dwUserIndex, pVibration);
        }
    }
}
