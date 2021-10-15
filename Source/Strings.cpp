/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file Strings.cpp
 *   Implementation of functions for manipulating Xidi-specific strings.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "Strings.h"
#include "TemporaryBuffer.h"

#include <cstdlib>
#include <intrin.h>
#include <mutex>
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
        static const std::wstring& GetProductName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    const wchar_t* stringStart = nullptr;
                    int stringLength = LoadString(Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, (wchar_t*)&stringStart, 0);

                    while ((stringLength > 0) && (L'\0' == stringStart[stringLength - 1]))
                        stringLength -= 1;

                    if (stringLength > 0)
                        initString.assign(stringStart, &stringStart[stringLength]);
                }
            );

            return initString;
        }

        /// Generates the value for kStrFormName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetFormName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
#ifdef IDS_XIDI_FORM_NAME
                    const wchar_t* stringStart = nullptr;
                    int stringLength = LoadString(Globals::GetInstanceHandle(), IDS_XIDI_FORM_NAME, (wchar_t*)&stringStart, 0);

                    while ((stringLength > 0) && (L'\0' == stringStart[stringLength - 1]))
                        stringLength -= 1;

                    if (stringLength > 0)
                        initString.assign(stringStart, &stringStart[stringLength]);
#else
                    initString.assign(L"");
#endif
                }
            );

            return initString;
        }

        /// Generates the value for kStrExecutableCompleteFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetExecutableCompleteFilename(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    TemporaryBuffer<wchar_t> buf;
                    GetModuleFileName(nullptr, buf, (DWORD)buf.Count());

                    initString.assign(buf);
                }
            );

            return initString;
        }

        /// Generates the value for kStrExecutableBaseName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetExecutableBaseName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view executableBaseName = GetExecutableCompleteFilename();

                    const size_t lastBackslashPos = executableBaseName.find_last_of(L"\\");
                    if (std::wstring_view::npos != lastBackslashPos)
                        executableBaseName.remove_prefix(1 + lastBackslashPos);

                    initString.assign(executableBaseName);
                }
            );

            return initString;
        }

        /// Generates the value for kStrExecutableDirectoryName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetExecutableDirectoryName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view executableDirectoryName = GetExecutableCompleteFilename();

                    const size_t lastBackslashPos = executableDirectoryName.find_last_of(L"\\");
                    if (std::wstring_view::npos != lastBackslashPos)
                    {
                        executableDirectoryName.remove_suffix(executableDirectoryName.length() - lastBackslashPos - 1);
                        initString.assign(executableDirectoryName);
                    }
                }
            );

            return initString;
        }

        /// Generates the value for kStrXidiCompleteFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetXidiCompleteFilename(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    TemporaryBuffer<wchar_t> buf;
                    GetModuleFileName(Globals::GetInstanceHandle(), buf, (DWORD)buf.Count());

                    initString.assign(buf);
                }
            );

            return initString;
        }

        /// Generates the value for kStrXidiBaseName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetXidiBaseName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view executableBaseName = GetXidiCompleteFilename();

                    const size_t lastBackslashPos = executableBaseName.find_last_of(L"\\");
                    if (std::wstring_view::npos != lastBackslashPos)
                        executableBaseName.remove_prefix(1 + lastBackslashPos);

                    initString.assign(executableBaseName);
                }
            );

            return initString;
        }

        /// Generates the value for kStrXidiDirectoryName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetXidiDirectoryName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view executableDirectoryName = GetXidiCompleteFilename();

                    const size_t lastBackslashPos = executableDirectoryName.find_last_of(L"\\");
                    if (std::wstring_view::npos != lastBackslashPos)
                    {
                        executableDirectoryName.remove_suffix(executableDirectoryName.length() - lastBackslashPos - 1);
                        initString.assign(executableDirectoryName);
                    }
                }
            );

            return initString;
        }

        /// Generates the value for kStrSystemDirectoryName; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetSystemDirectoryName(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    TemporaryBuffer<wchar_t> buf;
                    const UINT numChars = GetSystemDirectory(buf, buf.Count() - 1);

                    if (L'\\' != buf[numChars - 1])
                    {
                        buf[numChars] = L'\\';
                        buf[numChars + 1] = L'\0';
                    }

                    initString.assign(buf);
                }
            );

            return initString;
        }

        /// Generates the value for kStrSystemLibraryFilenameDirectInput; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetSystemLibraryFilenameDirectInput(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view pieces[] = {GetSystemDirectoryName(), kStrLibraryNameDirectInput};

                    size_t totalLength = 0;
                    for (int i = 0; i < _countof(pieces); ++i)
                        totalLength += pieces[i].length();

                    initString.reserve(1 + totalLength);

                    for (int i = 0; i < _countof(pieces); ++i)
                        initString.append(pieces[i]);
                }
            );

            return initString;
        }

        /// Generates the value for kStrSystemLibraryFilenameDirectInput8; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetSystemLibraryFilenameDirectInput8(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view pieces[] = {GetSystemDirectoryName(), kStrLibraryNameDirectInput8};

                    size_t totalLength = 0;
                    for (int i = 0; i < _countof(pieces); ++i)
                        totalLength += pieces[i].length();

                    initString.reserve(1 + totalLength);

                    for (int i = 0; i < _countof(pieces); ++i)
                        initString.append(pieces[i]);
                }
            );

            return initString;
        }

        /// Generates the value for kStrSystemLibraryFilenameWinMM; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetSystemLibraryFilenameWinMM(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view pieces[] = {GetSystemDirectoryName(), kStrLibraryNameWinMM};

                    size_t totalLength = 0;
                    for (int i = 0; i < _countof(pieces); ++i)
                        totalLength += pieces[i].length();

                    initString.reserve(1 + totalLength);

                    for (int i = 0; i < _countof(pieces); ++i)
                        initString.append(pieces[i]);
                }
            );

            return initString;
        }

        /// Generates the value for kStrConfigurationFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetConfigurationFilename(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstring_view pieces[] = {GetXidiDirectoryName(), GetProductName(), kStrConfigurationFileExtension};

                    size_t totalLength = 0;
                    for (int i = 0; i < _countof(pieces); ++i)
                        totalLength += pieces[i].length();

                    initString.reserve(1 + totalLength);

                    for (int i = 0; i < _countof(pieces); ++i)
                        initString.append(pieces[i]);
                }
            );

            return initString;
        }

        /// Generates the value for kStrLogFilename; see documentation of this run-time constant for more information.
        /// @return Corresponding run-time constant value.
        static const std::wstring& GetLogFilename(void)
        {
            static std::wstring initString;
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    std::wstringstream logFilename;

                    PWSTR knownFolderPath;
                    const HRESULT result = SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &knownFolderPath);

                    if (S_OK == result)
                    {
                        logFilename << knownFolderPath << L'\\';
                        CoTaskMemFree(knownFolderPath);
                    }

                    logFilename << GetProductName().c_str() << L'_' << GetFormName().c_str() << L'_' << GetExecutableBaseName().c_str() << L'_' << Globals::GetCurrentProcessId() << kStrLogFileExtension;

                    initString.assign(logFilename.str());
                }
            );

            return initString;
        }


        // -------- RUN-TIME CONSTANTS ------------------------------------- //
        // See "Strings.h" for documentation.

        extern const std::wstring_view kStrProductName(GetProductName());
        extern const std::wstring_view kStrFormName(GetFormName());
        extern const std::wstring_view kStrExecutableCompleteFilename(GetExecutableCompleteFilename());
        extern const std::wstring_view kStrExecutableBaseName(GetExecutableBaseName());
        extern const std::wstring_view kStrExecutableDirectoryName(GetExecutableDirectoryName());
        extern const std::wstring_view kStrXidiCompleteFilename(GetXidiCompleteFilename());
        extern const std::wstring_view kStrXidiBaseName(GetXidiBaseName());
        extern const std::wstring_view kStrXidiDirectoryName(GetXidiDirectoryName());
        extern const std::wstring_view kStrSystemDirectoryName(GetSystemDirectoryName());
        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput(GetSystemLibraryFilenameDirectInput());
        extern const std::wstring_view kStrSystemLibraryFilenameDirectInput8(GetSystemLibraryFilenameDirectInput8());
        extern const std::wstring_view kStrSystemLibraryFilenameWinMM(GetSystemLibraryFilenameWinMM());
        extern const std::wstring_view kStrConfigurationFilename(GetConfigurationFilename());
        extern const std::wstring_view kStrLogFilename(GetLogFilename());


        // -------- FUNCTIONS ---------------------------------------------- //
        // See "Strings.h" for documentation.

        std::wstring_view MapperTypeConfigurationNameString(Controller::TControllerIdentifier controllerIdentifier)
        {
            static std::wstring initStrings[Controller::kPhysicalControllerCount];
            static std::once_flag initFlag;

            std::call_once(initFlag, []() -> void
                {
                    for (Controller::TControllerIdentifier i = 0; i < _countof(initStrings); ++i)
                    {
                        std::wstringstream perControllerMapperTypeString;
                        perControllerMapperTypeString << kStrConfigurationSettingMapperType << kCharConfigurationSettingSeparator << (1 + i);
                        initStrings[i] = perControllerMapperTypeString.str();
                    }
                }
            );

            if (controllerIdentifier >= Controller::kPhysicalControllerCount)
                return std::wstring_view();

            return initStrings[controllerIdentifier];
        }

        // --------

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
