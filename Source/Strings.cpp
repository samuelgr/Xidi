/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
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

#include "ApiWindows.h"
#include "ControllerTypes.h"
#include "Globals.h"
#include "TemporaryBuffer.h"

namespace Xidi
{
  namespace Strings
  {
    /// File extension for a configuration file.
    static constexpr std::wstring_view kStrConfigurationFileExtension = L".ini";

    /// File extension for a log file.
    static constexpr std::wstring_view kStrLogFileExtension = L".log";

    /// Converts a single character to lowercase.
    /// Default implementation does nothing useful.
    /// @tparam CharType Character type.
    /// @param [in] c Character to convert.
    /// @return Null character, as the default implementation does nothing useful.
    template <typename CharType> static inline CharType ToLowercase(CharType c)
    {
      return L'\0';
    }

    /// Converts a single narrow character to lowercase.
    /// @tparam CharType Character type.
    /// @param [in] c Character to convert.
    /// @return Lowercase version of the input, if a conversion is possible, or the same character
    /// as the input otherwise.
    template <> char static inline ToLowercase(char c)
    {
      return std::tolower(c);
    }

    /// Converts a single wide character to lowercase.
    /// Default implementation does nothing useful.
    /// @tparam CharType Character type.
    /// @param [in] c Character to convert.
    /// @return Lowercase version of the input, if a conversion is possible, or the same character
    /// as the input otherwise.
    template <> wchar_t static inline ToLowercase(wchar_t c)
    {
      return std::towlower(c);
    }

    /// Generates the value for kStrProductName; see documentation of this run-time constant for
    /// more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetProductName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            const wchar_t* stringStart = nullptr;
            int stringLength = LoadString(
                Globals::GetInstanceHandle(), IDS_XIDI_PRODUCT_NAME, (wchar_t*)&stringStart, 0);

            while ((stringLength > 0) && (L'\0' == stringStart[stringLength - 1]))
              stringLength -= 1;

            if (stringLength > 0) initString.assign(stringStart, &stringStart[stringLength]);
          });

      return initString;
    }

    /// Generates the value for kStrFormName; see documentation of this run-time constant for more
    /// information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetFormName(void)
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
                Globals::GetInstanceHandle(), IDS_XIDI_FORM_NAME, (wchar_t*)&stringStart, 0);

            while ((stringLength > 0) && (L'\0' == stringStart[stringLength - 1]))
              stringLength -= 1;

            if (stringLength > 0) initString.assign(stringStart, &stringStart[stringLength]);
#else
            initString.assign(L"");
#endif
          });

      return initString;
    }

    /// Generates the value for kStrExecutableCompleteFilename; see documentation of this run-time
    /// constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetExecutableCompleteFilename(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            TemporaryBuffer<wchar_t> buf;
            GetModuleFileName(nullptr, buf.Data(), (DWORD)buf.Capacity());

            initString.assign(buf.Data());
          });

      return initString;
    }

    /// Generates the value for kStrExecutableBaseName; see documentation of this run-time constant
    /// for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetExecutableBaseName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view executableBaseName = GetExecutableCompleteFilename();

            const size_t lastBackslashPos = executableBaseName.find_last_of(L"\\");
            if (std::wstring_view::npos != lastBackslashPos)
              executableBaseName.remove_prefix(1 + lastBackslashPos);

            initString.assign(executableBaseName);
          });

      return initString;
    }

    /// Generates the value for kStrExecutableDirectoryName; see documentation of this run-time
    /// constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetExecutableDirectoryName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view executableDirectoryName = GetExecutableCompleteFilename();

            const size_t lastBackslashPos = executableDirectoryName.find_last_of(L"\\");
            if (std::wstring_view::npos != lastBackslashPos)
            {
              executableDirectoryName.remove_suffix(
                  executableDirectoryName.length() - lastBackslashPos - 1);
              initString.assign(executableDirectoryName);
            }
          });

      return initString;
    }

    /// Generates the value for kStrXidiCompleteFilename; see documentation of this run-time
    /// constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetXidiCompleteFilename(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            TemporaryBuffer<wchar_t> buf;
            GetModuleFileName(Globals::GetInstanceHandle(), buf.Data(), (DWORD)buf.Capacity());

            initString.assign(buf.Data());
          });

      return initString;
    }

    /// Generates the value for kStrXidiBaseName; see documentation of this run-time constant for
    /// more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetXidiBaseName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view executableBaseName = GetXidiCompleteFilename();

            const size_t lastBackslashPos = executableBaseName.find_last_of(L"\\");
            if (std::wstring_view::npos != lastBackslashPos)
              executableBaseName.remove_prefix(1 + lastBackslashPos);

            initString.assign(executableBaseName);
          });

      return initString;
    }

    /// Generates the value for kStrXidiDirectoryName; see documentation of this run-time constant
    /// for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetXidiDirectoryName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view executableDirectoryName = GetXidiCompleteFilename();

            const size_t lastBackslashPos = executableDirectoryName.find_last_of(L"\\");
            if (std::wstring_view::npos != lastBackslashPos)
            {
              executableDirectoryName.remove_suffix(
                  executableDirectoryName.length() - lastBackslashPos - 1);
              initString.assign(executableDirectoryName);
            }
          });

      return initString;
    }

    /// Generates the value for kStrSystemDirectoryName; see documentation of this run-time constant
    /// for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetSystemDirectoryName(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            TemporaryBuffer<wchar_t> buf;
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

    /// Generates the value for kStrSystemLibraryFilenameDirectInput; see documentation of this
    /// run-time constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetSystemLibraryFilenameDirectInput(void)
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

    /// Generates the value for kStrSystemLibraryFilenameDirectInput8; see documentation of this
    /// run-time constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetSystemLibraryFilenameDirectInput8(void)
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

    /// Generates the value for kStrSystemLibraryFilenameWinMM; see documentation of this run-time
    /// constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetSystemLibraryFilenameWinMM(void)
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

    /// Generates the value for kStrConfigurationFilename; see documentation of this run-time
    /// constant for more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetConfigurationFilename(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            std::wstring_view pieces[] = {
                GetXidiDirectoryName(), GetProductName(), kStrConfigurationFileExtension};

            size_t totalLength = 0;
            for (int i = 0; i < _countof(pieces); ++i)
              totalLength += pieces[i].length();

            initString.reserve(1 + totalLength);

            for (int i = 0; i < _countof(pieces); ++i)
              initString.append(pieces[i]);
          });

      return initString;
    }

    /// Generates the value for kStrLogFilename; see documentation of this run-time constant for
    /// more information.
    /// @return Corresponding run-time constant value.
    static const std::wstring& GetLogFilename(void)
    {
      static std::wstring initString;
      static std::once_flag initFlag;

      std::call_once(
          initFlag,
          []() -> void
          {
            TemporaryString logFilename;

            PWSTR knownFolderPath;
            const HRESULT result =
                SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &knownFolderPath);

            if (S_OK == result)
            {
              logFilename << knownFolderPath << L'\\';
              CoTaskMemFree(knownFolderPath);
            }

            logFilename << GetProductName().c_str() << L'_' << GetFormName().c_str() << L'_'
                        << GetExecutableBaseName().c_str() << L'_' << Globals::GetCurrentProcessId()
                        << kStrLogFileExtension;

            initString.assign(logFilename);
          });

      return initString;
    }

    extern const std::wstring_view kStrProductName(GetProductName());
    extern const std::wstring_view kStrFormName(GetFormName());
    extern const std::wstring_view kStrExecutableCompleteFilename(GetExecutableCompleteFilename());
    extern const std::wstring_view kStrExecutableBaseName(GetExecutableBaseName());
    extern const std::wstring_view kStrExecutableDirectoryName(GetExecutableDirectoryName());
    extern const std::wstring_view kStrXidiCompleteFilename(GetXidiCompleteFilename());
    extern const std::wstring_view kStrXidiBaseName(GetXidiBaseName());
    extern const std::wstring_view kStrXidiDirectoryName(GetXidiDirectoryName());
    extern const std::wstring_view kStrSystemDirectoryName(GetSystemDirectoryName());
    extern const std::wstring_view kStrSystemLibraryFilenameDirectInput(
        GetSystemLibraryFilenameDirectInput());
    extern const std::wstring_view kStrSystemLibraryFilenameDirectInput8(
        GetSystemLibraryFilenameDirectInput8());
    extern const std::wstring_view kStrSystemLibraryFilenameWinMM(GetSystemLibraryFilenameWinMM());
    extern const std::wstring_view kStrConfigurationFilename(GetConfigurationFilename());
    extern const std::wstring_view kStrLogFilename(GetLogFilename());

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

    template <typename CharType> bool EqualsCaseInsensitive(
        std::basic_string_view<CharType> strA, std::basic_string_view<CharType> strB)
    {
      if (strA.length() != strB.length()) return false;

      for (size_t i = 0; i < strA.length(); ++i)
      {
        if (ToLowercase(strA[i]) != ToLowercase(strB[i])) return false;
      }

      return true;
    }

    template bool EqualsCaseInsensitive<char>(std::string_view, std::string_view);
    template bool EqualsCaseInsensitive<wchar_t>(std::wstring_view, std::wstring_view);

    TemporaryString FormatString(_Printf_format_string_ const wchar_t* format, ...)
    {
      TemporaryString buf;

      va_list args;
      va_start(args, format);

      buf.UnsafeSetSize((size_t)vswprintf_s(buf.Data(), buf.Capacity(), format, args));

      va_end(args);

      return buf;
    }

    TemporaryString GuidToString(const GUID& guid)
    {
      return FormatString(
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
            TemporaryString perControllerMapperTypeString;

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

    TemporaryVector<std::wstring_view> SplitString(
        std::wstring_view stringToSplit, std::wstring_view delimiter)
    {
      TemporaryVector<std::wstring_view> stringPieces;

      auto beginIter = stringToSplit.cbegin();
      auto endIter = ((false == delimiter.empty()) ? beginIter : stringToSplit.cend());

      while ((stringPieces.Size() < stringPieces.Capacity()) && (stringToSplit.cend() != endIter))
      {
        std::wstring_view remainingStringToSplit(endIter, stringToSplit.cend());
        if (true == remainingStringToSplit.starts_with(delimiter))
        {
          stringPieces.EmplaceBack(beginIter, endIter);
          endIter += delimiter.length();
          beginIter = endIter;
        }
        else
        {
          endIter += 1;
        }
      }

      if (stringPieces.Size() < stringPieces.Capacity())
        stringPieces.EmplaceBack(beginIter, endIter);
      else
        stringPieces.Clear();

      return stringPieces;
    }

    TemporaryVector<std::wstring_view> SplitString(
        std::wstring_view stringToSplit, std::initializer_list<std::wstring_view> delimiters)
    {
      TemporaryVector<std::wstring_view> stringPieces;

      auto beginIter = stringToSplit.cbegin();
      auto endIter = ((false == std::empty(delimiters)) ? beginIter : stringToSplit.cend());

      while ((stringPieces.Size() < stringPieces.Capacity()) && (stringToSplit.cend() != endIter))
      {
        bool delimiterFound = false;
        std::wstring_view remainingStringToSplit(endIter, stringToSplit.cend());
        for (const auto& delimiter : delimiters)
        {
          if (true == remainingStringToSplit.starts_with(delimiter))
          {
            stringPieces.EmplaceBack(beginIter, endIter);
            endIter += delimiter.length();
            beginIter = endIter;
            delimiterFound = true;
            break;
          }
        }

        if (false == delimiterFound)
        {
          endIter += 1;
        }
      }

      if (stringPieces.Size() < stringPieces.Capacity())
        stringPieces.EmplaceBack(beginIter, endIter);
      else
        stringPieces.Clear();

      return stringPieces;
    }

    TemporaryString SystemErrorCodeString(const unsigned long systemErrorCode)
    {
      TemporaryString systemErrorString;
      DWORD systemErrorLength = FormatMessage(
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr,
          systemErrorCode,
          0,
          systemErrorString.Data(),
          systemErrorString.Capacity(),
          nullptr);

      if (0 == systemErrorLength)
      {
        systemErrorString = FormatString(L"System error %u.", (unsigned int)systemErrorCode);
      }
      else
      {
        for (; systemErrorLength > 0; --systemErrorLength)
        {
          if (L'\0' != systemErrorString[systemErrorLength] &&
              !iswspace(systemErrorString[systemErrorLength]))
            break;

          systemErrorString[systemErrorLength] = L'\0';
          systemErrorString.UnsafeSetSize(systemErrorLength);
        }
      }

      return systemErrorString;
    }
  } // namespace Strings
} // namespace Xidi
