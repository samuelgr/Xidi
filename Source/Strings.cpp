/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file Strings.cpp
 *   Implementation of functions for manipulating Xidi-specific strings.
 **************************************************************************************************/

#include "Strings.h"

#include <intrin.h>
#include <sal.h>

#include <cctype>
#include <cstdlib>
#include <cwctype>
#include <mutex>
#include <string>
#include <string_view>

#include <Infra/Core/ProcessInfo.h>
#include <Infra/Core/Strings.h>
#include <Infra/Core/TemporaryBuffer.h>

#include "ApiWindows.h"
#include "ControllerTypes.h"

namespace Xidi
{
  namespace Strings
  {
    /// File extension for a configuration file.
    static constexpr std::wstring_view kStrConfigurationFileExtension = L".ini";

    /// File extension for a log file.
    static constexpr std::wstring_view kStrLogFileExtension = L".log";

    std::wstring_view GetFormName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
#ifdef IDS_XIDI_FORM_NAME
            const wchar_t* stringStart = nullptr;
            int stringLength = LoadString(
                Infra::ProcessInfo::GetThisModuleInstanceHandle(),
                IDS_XIDI_FORM_NAME,
                (wchar_t*)&stringStart,
                0);

            while ((stringLength > 0) && (L'\0' == stringStart[stringLength - 1]))
              stringLength -= 1;

            if (stringLength > 0) initString.assign(stringStart, &stringStart[stringLength]);
#else
            initString.assign(Infra::ProcessInfo::GetProductName());
#endif
          });

      return initString;
    }

    std::wstring_view GetSystemDirectoryName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            Infra::TemporaryBuffer<wchar_t> buf;
            const UINT numChars = GetSystemDirectory(buf.Data(), buf.Capacity() - 1);

            if (L'\\' != buf[numChars - 1])
            {
              buf[numChars] = L'\\';
              buf[numChars + 1] = L'\0';
            }

            initString.assign(buf.Data());
          });

      return initString;
    }

    std::wstring_view GetSystemLibraryFilenameDirectInput(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view pieces[] = {GetSystemDirectoryName(), kStrLibraryNameDirectInput};

            size_t totalLength = 0;
            for (int i = 0; i < _countof(pieces); ++i)
              totalLength += pieces[i].length();

            initString.reserve(1 + totalLength);

            for (int i = 0; i < _countof(pieces); ++i)
              initString.append(pieces[i]);
          });

      return initString;
    }

    std::wstring_view GetSystemLibraryFilenameDirectInput8(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view pieces[] = {GetSystemDirectoryName(), kStrLibraryNameDirectInput8};

            size_t totalLength = 0;
            for (int i = 0; i < _countof(pieces); ++i)
              totalLength += pieces[i].length();

            initString.reserve(1 + totalLength);

            for (int i = 0; i < _countof(pieces); ++i)
              initString.append(pieces[i]);
          });

      return initString;
    }

    std::wstring_view GetSystemLibraryFilenameWinMM(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view pieces[] = {GetSystemDirectoryName(), kStrLibraryNameWinMM};

            size_t totalLength = 0;
            for (int i = 0; i < _countof(pieces); ++i)
              totalLength += pieces[i].length();

            initString.reserve(1 + totalLength);

            for (int i = 0; i < _countof(pieces); ++i)
              initString.append(pieces[i]);
          });

      return initString;
    }

    std::wstring_view GetXidiMainLibraryFilename(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view pieces[] = {
                Infra::ProcessInfo::GetThisModuleDirectoryName(),
                L"\\",
                Infra::ProcessInfo::GetProductName(),
                L".",
#ifdef _WIN64
                L"64",
#else
                L"32",
#endif
                L".dll"};

            size_t totalLength = 0;
            for (int i = 0; i < _countof(pieces); ++i)
              totalLength += pieces[i].length();

            initString.reserve(1 + totalLength);

            for (int i = 0; i < _countof(pieces); ++i)
              initString.append(pieces[i]);
          });

      return initString;
    }

    const wchar_t* AxisTypeString(Controller::EAxis axis)
    {
      switch (axis)
      {
        case Controller::EAxis::X:
          return L"X";
        case Controller::EAxis::Y:
          return L"Y";
        case Controller::EAxis::Z:
          return L"Z";
        case Controller::EAxis::RotX:
          return L"RotX";
        case Controller::EAxis::RotY:
          return L"RotY";
        case Controller::EAxis::RotZ:
          return L"RotZ";
      }

      return L"?";
    }

    Infra::TemporaryString GuidToString(const GUID& guid)
    {
      return Infra::Strings::Format(
          L"{%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx}",
          guid.Data1,
          guid.Data2,
          guid.Data3,
          guid.Data4[0],
          guid.Data4[1],
          guid.Data4[2],
          guid.Data4[3],
          guid.Data4[4],
          guid.Data4[5],
          guid.Data4[6],
          guid.Data4[7]);
    }

    std::wstring_view MapperTypeConfigurationNameString(
        Controller::TControllerIdentifier controllerIdentifier)
    {
      static std::wstring initStrings[Controller::kPhysicalControllerCount];
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            Infra::TemporaryString perControllerMapperTypeString;

            for (Controller::TControllerIdentifier i = 0; i < _countof(initStrings); ++i)
            {
              perControllerMapperTypeString.Clear();
              perControllerMapperTypeString << kStrConfigurationSettingMapperType
                                            << kCharConfigurationSettingSeparator << (1 + i);
              initStrings[i] = perControllerMapperTypeString;
            }
          });

      if (controllerIdentifier >= Controller::kPhysicalControllerCount) return std::wstring_view();

      return initStrings[controllerIdentifier];
    }
  } // namespace Strings
} // namespace Xidi
