/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Globals.h
 *   Declaration of a namespace for storing and retrieving global data.
 *   Intended for miscellaneous data elements with no other suitable place.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"

#include <string>
#include <string_view>


namespace Xidi
{
    namespace Globals
    {
        // -------- FUNCTIONS ---------------------------------------------- //

        /// Applies a setting that specifies a custom path for the import library for DirectInput functions.
        /// @param [in] value Setting value to be applied.
        /// @return `true` if setting could be applied, `false` otherwise.
        bool ApplyOverrideImportDirectInput(std::wstring& value);

        /// Applies a setting that specifies a custom path for the import library for DirectInput8 functions.
        /// @param [in] value Setting value to be applied.
        /// @return `true` if setting could be applied, `false` otherwise.
        bool ApplyOverrideImportDirectInput8(std::wstring& value);

        /// Applies a setting that specifies a custom path for the import library for WinMM functions.
        /// @param [in] value Setting value to be applied.
        /// @return `true` if setting could be applied, `false` otherwise.
        bool ApplyOverrideImportWinMM(std::wstring& value);

        /// Retrieves a pseudohandle to the current process.
        /// @return Current process pseudohandle.
        HANDLE GetCurrentProcessHandle(void);

        /// Retrieves the PID of the current process.
        /// @return Current process PID.
        DWORD GetCurrentProcessId(void);

        /// Retrieves the handle of the instance that represents the current running form of this code.
        /// @return Instance handle for this code.
        HINSTANCE GetInstanceHandle(void);

        /// Retrieves the library path for the DirectInput library that should be used for importing functions.
        /// @return Library path.
        std::wstring_view GetLibraryPathDirectInput(void);

        /// Retrieves the library path for the DirectInput8 library that should be used for importing functions.
        /// @return Library path.
        std::wstring_view GetLibraryPathDirectInput8(void);

        /// Retrieves the library path for the WinMM library that should be used for importing functions.
        /// @return Library path.
        std::wstring_view GetLibraryPathWinMM(void);

        /// Retrieves information on the current system. This includes architecture, page size, and so on.
        /// @return Reference to a read-only structure containing system information.
        const SYSTEM_INFO& GetSystemInformation(void);
    }
}
