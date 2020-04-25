/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Globals.cpp
 *   Implementation of accessors and mutators for global data items.
 *   Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Strings.h"

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

            /// Holds the path to a custom library that overrides the default import library for DirectInput functions.
            std::wstring gOverrideImportDirectInput;

            /// Holds the path to a custom library that overrides the default import library for DirectInput8 functions.
            std::wstring gOverrideImportDirectInput8;

            /// Holds the path to a custom library that overrides the default import library for WinMM functions.
            std::wstring gOverrideImportWinMM;


        private:
            // -------- CONSTRUCTION AND DESTRUCTION ----------------------- //

            /// Default constructor. Objects cannot be constructed externally.
            GlobalData(void) : gCurrentProcessHandle(GetCurrentProcess()), gCurrentProcessId(GetProcessId(GetCurrentProcess())), gSystemInformation(), gInstanceHandle(nullptr), gOverrideImportDirectInput(L""), gOverrideImportDirectInput8(L""), gOverrideImportWinMM(L"")
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


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Globals.h" for documentation.

        bool ApplyOverrideImportDirectInput(std::wstring& value)
        {
            bool validValue = !(value.empty());

            if (true == validValue)
                GlobalData::GetInstance().gOverrideImportDirectInput = value;

            return validValue;
        }

        // --------

        bool ApplyOverrideImportDirectInput8(std::wstring& value)
        {
            bool validValue = !(value.empty());

            if (true == validValue)
                GlobalData::GetInstance().gOverrideImportDirectInput8 = value;

            return validValue;
        }

        // --------

        bool ApplyOverrideImportWinMM(std::wstring& value)
        {
            bool validValue = !(value.empty());

            if (true == validValue)
                GlobalData::GetInstance().gOverrideImportWinMM = value;

            return validValue;
        }

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

        std::wstring_view GetLibraryPathDirectInput(void)
        {
            if (false == GlobalData::GetInstance().gOverrideImportDirectInput.empty())
                return GlobalData::GetInstance().gOverrideImportDirectInput;
            else
                return Strings::kStrSystemLibraryFilenameDirectInput;
        }

        // --------

        std::wstring_view GetLibraryPathDirectInput8(void)
        {
            if (false == GlobalData::GetInstance().gOverrideImportDirectInput8.empty())
                return GlobalData::GetInstance().gOverrideImportDirectInput8;
            else
                return Strings::kStrSystemLibraryFilenameDirectInput8;
        }

        // --------

        std::wstring_view GetLibraryPathWinMM(void)
        {
            if (false == GlobalData::GetInstance().gOverrideImportWinMM.empty())
                return GlobalData::GetInstance().gOverrideImportWinMM;
            else
                return Strings::kStrSystemLibraryFilenameWinMM;
        }

        // --------

        const SYSTEM_INFO& GetSystemInformation(void)
        {
            return GlobalData::GetInstance().gSystemInformation;
        }
    }
}
