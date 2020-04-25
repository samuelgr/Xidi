/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file Strings.cpp
 *   Implementation of functions for manipulating Hookshot-specific strings.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"
#include "Strings.h"
#include "TemporaryBuffer.h"

#include <cstdlib>
#include <intrin.h>
#include <psapi.h>
#include <shlobj.h>
#include <sstream>
#include <string>
#include <string_view>


namespace Xidi
{
    namespace Strings
    {
        // -------- INTERNAL CONSTANTS ------------------------------------- //

        /// File extension for a configuration file.
        static constexpr std::wstring_view kStrConfigurationFileExtension = L".ini";

        /// File extension for a log file.
        static constexpr std::wstring_view kStrLogFileExtension = L".log";


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

        /// Generates the value for kStrProductName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetProductName(void)
        {
            TemporaryBuffer<wchar_t> buf;
            LoadString(Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, (wchar_t*)buf, buf.Count());

            return (std::wstring(buf));
        }

        /// Generates the value for kStrVersionName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetVersionName(void)
        {
            TemporaryBuffer<wchar_t> buf;
            LoadString(Globals::GetInstanceHandle(), IDS_XIDI_VERSION_NAME, (wchar_t*)buf, buf.Count());

            return (std::wstring(buf));
        }

        /// Generates the value for kStrExecutableBaseName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetExecutableBaseName(void)
        {
            TemporaryBuffer<wchar_t> buf;
            GetModuleFileName(nullptr, buf, (DWORD)buf.Count());

            wchar_t* executableBaseName = wcsrchr(buf, L'\\');
            if (nullptr == executableBaseName)
                executableBaseName = buf;
            else
                executableBaseName += 1;

            return (std::wstring(executableBaseName));
        }

        /// Generates the value for kStrExecutableDirectoryName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetExecutableDirectoryName(void)
        {
            TemporaryBuffer<wchar_t> buf;
            GetModuleFileName(nullptr, buf, (DWORD)buf.Count());

            wchar_t* const lastBackslash = wcsrchr(buf, L'\\');
            if (nullptr == lastBackslash)
                buf[0] = L'\0';
            else
                lastBackslash[1] = L'\0';

            return (std::wstring(buf));
        }

        /// Generates the value for kStrExecutableCompleteFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetExecutableCompleteFilename(void)
        {
            TemporaryBuffer<wchar_t> buf;
            GetModuleFileName(nullptr, buf, (DWORD)buf.Count());

            return (std::wstring(buf));
        }

        /// Generates the value for kStrSystemDirectoryName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetSystemDirectoryName(void)
        {
            TemporaryBuffer<wchar_t> buf;
            const UINT numChars = GetSystemDirectory(buf, buf.Count() - 1);

            if (L'\\' != buf[numChars - 1])
            {
                buf[numChars] = L'\\';
                buf[numChars + 1] = L'\0';
            }

            return (std::wstring(buf));
        }

        /// Generates the value for kStrSystemLibraryFilenameDirectInput; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetSystemLibraryFilenameDirectInput(void)
        {
            std::wstring libraryFilename = GetSystemDirectoryName();
            libraryFilename += kStrLibraryNameDirectInput;

            return libraryFilename;
        }

        /// Generates the value for kStrSystemLibraryFilenameDirectInput8; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetSystemLibraryFilenameDirectInput8(void)
        {
            std::wstring libraryFilename = GetSystemDirectoryName();
            libraryFilename += kStrLibraryNameDirectInput8;

            return libraryFilename;
        }

        /// Generates the value for kStrSystemLibraryFilenameWinMM; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetSystemLibraryFilenameWinMM(void)
        {
            std::wstring libraryFilename = GetSystemDirectoryName();
            libraryFilename += kStrLibraryNameWinMM;

            return libraryFilename;
        }

        /// Generates the value for kStrConfigurationFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetConfigurationFilename(void)
        {
            return GetExecutableDirectoryName() + GetProductName() + kStrConfigurationFileExtension.data();
        }

        /// Generates the value for kStrLogFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static std::wstring GetLogFilename(void)
        {
            std::wstringstream logFilename;

            PWSTR knownFolderPath;
            const HRESULT result = SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &knownFolderPath);

            if (S_OK == result)
            {
                logFilename << knownFolderPath << L'\\';
                CoTaskMemFree(knownFolderPath);
            }

            logFilename << GetProductName() << L'_' << GetVersionName() << L'_' << GetExecutableBaseName() << L'_' << Globals::GetCurrentProcessId() << kStrLogFileExtension;

            return logFilename.str();
        }


        // -------- INTERNAL CONSTANTS ------------------------------------- //
        // Used to implement run-time constants; see "Strings.h" for documentation.

        static const std::wstring kStrProductNameImpl(GetProductName());

        static const std::wstring kStrVersionNameImpl(GetVersionName());

        static const std::wstring kStrExecutableBaseNameImpl(GetExecutableBaseName());

        static const std::wstring kStrExecutableDirectoryNameImpl(GetExecutableDirectoryName());

        static const std::wstring kStrExecutableCompleteFilenameImpl(GetExecutableCompleteFilename());

        static const std::wstring kStrSystemDirectoryNameImpl(GetSystemDirectoryName());

        static const std::wstring kStrSystemLibraryFilenameDirectInputImpl(GetSystemLibraryFilenameDirectInput());

        static const std::wstring kStrSystemLibraryFilenameDirectInput8Impl(GetSystemLibraryFilenameDirectInput8());

        static const std::wstring kStrSystemLibraryFilenameWinMMImpl(GetSystemLibraryFilenameWinMM());

        static const std::wstring kStrConfigurationFilenameImpl(GetConfigurationFilename());

        static const std::wstring kStrLogFilenameImpl(GetLogFilename());


        // -------- RUN-TIME CONSTANTS ------------------------------------- //
        // See "Strings.h" for documentation.

        extern const std::wstring_view kStrProductName(kStrProductNameImpl);

        extern const std::wstring_view kStrVersionName(kStrVersionNameImpl);

        extern const std::wstring_view kStrExecutableBaseName(kStrExecutableBaseNameImpl);

        extern const std::wstring_view kStrExecutableDirectoryName(kStrExecutableDirectoryNameImpl);

        extern const std::wstring_view kStrExecutableCompleteFilename(kStrExecutableCompleteFilenameImpl);

        extern const std::wstring_view kStrSystemDirectoryName(kStrSystemDirectoryNameImpl);

        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput(kStrSystemLibraryFilenameDirectInputImpl);

        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput8(kStrSystemLibraryFilenameDirectInput8Impl);

        extern const std::wstring_view kStrSystemLibraryFilenameWinMM(kStrSystemLibraryFilenameWinMMImpl);
        
        extern const std::wstring_view kStrConfigurationFilename(kStrConfigurationFilenameImpl);

        extern const std::wstring_view kStrLogFilename(kStrLogFilenameImpl);


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Strings.h" for documentation.

        std::wstring SystemErrorCodeString(const unsigned long systemErrorCode)
        {
            TemporaryBuffer<wchar_t> systemErrorString;
            DWORD systemErrorLength = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, systemErrorCode, 0, systemErrorString, systemErrorString.Count(), nullptr);

            if (0 == systemErrorLength)
            {
                swprintf_s(systemErrorString, systemErrorString.Count(), L"System error %u.", (unsigned int)systemErrorCode);
            }
            else
            {
                for (; systemErrorLength > 0; --systemErrorLength)
                {
                    if (L'\0' != systemErrorString[systemErrorLength] && !iswspace(systemErrorString[systemErrorLength]))
                        break;

                    systemErrorString[systemErrorLength] = L'\0';
                }
            }

            return std::wstring(systemErrorString);
        }
    }
}
