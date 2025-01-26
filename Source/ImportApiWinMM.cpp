/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ImportApiWinMM.cpp
 *   Implementations of functions for accessing the WinMM API imported from
 *   the native WinMM library.
 **************************************************************************************************/

#include "ImportApiWinMM.h"

#include <map>
#include <mutex>
#include <set>
#include <string_view>

#include <Infra/Core/Configuration.h>
#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "DllFunctions.h"
#include "Globals.h"
#include "Strings.h"

/// Computes the index of the specified named function in the pointer array of the import table.
#define IMPORT_TABLE_INDEX_OF(name)                                                                \
  (offsetof(UImportTable, named.##name) / sizeof(UImportTable::ptr[0]))

/// Attempts to import a single function and save it into the import table.
#define TRY_IMPORT(libraryPath, libraryHandle, functionName)                                       \
  DllFunctions::TryImport(                                                                         \
      libraryPath,                                                                                 \
      loadedLibrary,                                                                               \
      #functionName,                                                                               \
      &importTable.ptr[IMPORT_TABLE_INDEX_OF(functionName)])

namespace Xidi
{
  namespace ImportApiWinMM
  {
    /// Holds pointers to all the functions imported from the native WinMM library.
    /// Exposes them as both an array of typeless pointers and a named structure of type-specific
    /// pointers.
    union UImportTable
    {
      struct
      {
        MMRESULT(__stdcall* joyConfigChanged)(DWORD);
        MMRESULT(__stdcall* joyGetDevCapsA)(UINT_PTR, LPJOYCAPSA, UINT);
        MMRESULT(__stdcall* joyGetDevCapsW)(UINT_PTR, LPJOYCAPSW, UINT);
        UINT(__stdcall* joyGetNumDevs)(void);
        MMRESULT(__stdcall* joyGetPos)(UINT, LPJOYINFO);
        MMRESULT(__stdcall* joyGetPosEx)(UINT, LPJOYINFOEX);
        MMRESULT(__stdcall* joyGetThreshold)(UINT, LPUINT);
        MMRESULT(__stdcall* joyReleaseCapture)(UINT);
        MMRESULT(__stdcall* joySetCapture)(HWND, UINT, UINT, BOOL);
        MMRESULT(__stdcall* joySetThreshold)(UINT, UINT);
        MMRESULT(__stdcall* timeBeginPeriod)(UINT);
        MMRESULT(__stdcall* timeGetDevCaps)(LPTIMECAPS, UINT);
        DWORD(__stdcall* timeGetTime)(void);
      } named;

      const void* ptr[sizeof(named) / sizeof(const void*)];
    };

    static_assert(
        sizeof(UImportTable::named) == sizeof(UImportTable::ptr), "Element size mismatch.");

    /// Holds the imported WinMM API function addresses.
    static UImportTable importTable;

    /// Retrieves the library path for the WinMM library that should be used for importing
    /// functions.
    /// @return Library path.
    static std::wstring_view GetImportLibraryPathWinMM(void)
    {
      return Globals::GetConfigurationData()[Strings::kStrConfigurationSectionImport]
                                            [Strings::kStrConfigurationSettingImportWinMM]
                                                .ValueOr(Strings::GetSystemLibraryFilenameWinMM());
    }

    void Initialize(void)
    {
      static std::once_flag initializeFlag;

      std::call_once(
          initializeFlag,
          []() -> void
          {
            // Initialize the import table.
            ZeroMemory(&importTable, sizeof(importTable));

            // Obtain the full library path string.
            std::wstring_view libraryPath = GetImportLibraryPathWinMM();

            // Attempt to load the library.
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"Attempting to import WinMM functions from %s.",
                libraryPath.data());
            HMODULE loadedLibrary = LoadLibraryEx(libraryPath.data(), nullptr, 0);
            if (nullptr == loadedLibrary)
            {
              Infra::Message::Output(
                  Infra::Message::ESeverity::Error,
                  L"Failed to initialize imported WinMM functions.");
              return;
            }

            // Attempt to obtain the addresses of all imported API functions.
            FARPROC procAddress = nullptr;

            TRY_IMPORT(libraryPath, loadedLibrary, joyConfigChanged);
            TRY_IMPORT(libraryPath, loadedLibrary, joyGetDevCapsA);
            TRY_IMPORT(libraryPath, loadedLibrary, joyGetDevCapsW);
            TRY_IMPORT(libraryPath, loadedLibrary, joyGetNumDevs);
            TRY_IMPORT(libraryPath, loadedLibrary, joyGetPos);
            TRY_IMPORT(libraryPath, loadedLibrary, joyGetPosEx);
            TRY_IMPORT(libraryPath, loadedLibrary, joyGetThreshold);
            TRY_IMPORT(libraryPath, loadedLibrary, joyReleaseCapture);
            TRY_IMPORT(libraryPath, loadedLibrary, joySetCapture);
            TRY_IMPORT(libraryPath, loadedLibrary, joySetThreshold);
            TRY_IMPORT(libraryPath, loadedLibrary, timeBeginPeriod);
            TRY_IMPORT(libraryPath, loadedLibrary, timeGetDevCaps);
            TRY_IMPORT(libraryPath, loadedLibrary, timeGetTime);

            // Initialization complete.
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Info,
                L"Successfully initialized imported WinMM functions.");
          });
    }

    MMRESULT joyConfigChanged(DWORD dwFlags)
    {
      if (nullptr == importTable.named.joyConfigChanged) Initialize();
      return importTable.named.joyConfigChanged(dwFlags);
    }

    MMRESULT joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
    {
      if (nullptr == importTable.named.joyGetDevCapsA) Initialize();
      return importTable.named.joyGetDevCapsA(uJoyID, pjc, cbjc);
    }

    MMRESULT joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
    {
      if (nullptr == importTable.named.joyGetDevCapsW) Initialize();
      return importTable.named.joyGetDevCapsW(uJoyID, pjc, cbjc);
    }

    UINT joyGetNumDevs(void)
    {
      if (nullptr == importTable.named.joyGetNumDevs) Initialize();
      return importTable.named.joyGetNumDevs();
    }

    MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji)
    {
      if (nullptr == importTable.named.joyGetPos) Initialize();
      return importTable.named.joyGetPos(uJoyID, pji);
    }

    MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
    {
      if (nullptr == importTable.named.joyGetPosEx) Initialize();
      return importTable.named.joyGetPosEx(uJoyID, pji);
    }

    MMRESULT joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
    {
      if (nullptr == importTable.named.joyGetThreshold) Initialize();
      return importTable.named.joyGetThreshold(uJoyID, puThreshold);
    }

    MMRESULT joyReleaseCapture(UINT uJoyID)
    {
      if (nullptr == importTable.named.joyReleaseCapture) Initialize();
      return importTable.named.joyReleaseCapture(uJoyID);
    }

    MMRESULT joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
    {
      if (nullptr == importTable.named.joySetCapture) Initialize();
      return importTable.named.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
    }

    MMRESULT joySetThreshold(UINT uJoyID, UINT uThreshold)
    {
      if (nullptr == importTable.named.joySetThreshold) Initialize();
      return importTable.named.joySetThreshold(uJoyID, uThreshold);
    }

    MMRESULT timeBeginPeriod(UINT uPeriod)
    {
      if (nullptr == importTable.named.timeBeginPeriod) Initialize();
      return importTable.named.timeBeginPeriod(uPeriod);
    }

    MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
    {
      if (nullptr == importTable.named.timeGetDevCaps) Initialize();
      return importTable.named.timeGetDevCaps(ptc, cbtc);
    }

    DWORD timeGetTime(void)
    {
      if (nullptr == importTable.named.timeGetTime) Initialize();
      return importTable.named.timeGetTime();
    }

    /// Implements the Xidi API interface #IImportFunctions.
    /// Allows joystick WinMM functions to be replaced.
    class JoystickFunctionReplacer : public Api::IImportFunctions
    {
    private:

      /// Maps from replaceable joystick function name to array index in the import table.
      static const std::map<std::wstring_view, size_t> kReplaceableFunctions;

    public:

      const std::set<std::wstring_view>& GetReplaceable(void) const override
      {
        static std::set<std::wstring_view> initSet;
        static std::once_flag initFlag;

        std::call_once(
            initFlag,
            []() -> void
            {
              for (const auto& replaceableFunction : kReplaceableFunctions)
                initSet.insert(replaceableFunction.first);
            });

        return initSet;
      }

      size_t SetReplaceable(
          const std::map<std::wstring_view, const void*>& importFunctionTable) override
      {
        Initialize();

        std::wstring_view libraryPath = GetImportLibraryPathWinMM();
        size_t numReplaced = 0;

        for (const auto& newImportFunction : importFunctionTable)
        {
          if (true == kReplaceableFunctions.contains(newImportFunction.first))
          {
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Debug,
                L"Import function \"%s\" has been replaced.",
                newImportFunction.first.data());
            importTable.ptr[kReplaceableFunctions.at(newImportFunction.first)] =
                newImportFunction.second;
            numReplaced += 1;
          }
        }

        if (numReplaced > 0)
          Infra::Message::OutputFormatted(
              Infra::Message::ESeverity::Warning,
              L"%d function(s) previously imported from %s have been replaced. Previously imported versions will not be used.",
              (int)numReplaced,
              libraryPath.data());

        return numReplaced;
      }
    };

    /// Maps from replaceable import function name to its pointer's positional index in the import
    /// table.
    const std::map<std::wstring_view, size_t> JoystickFunctionReplacer::kReplaceableFunctions = {
        {L"joyConfigChanged", IMPORT_TABLE_INDEX_OF(joyConfigChanged)},
        {L"joyGetDevCapsA", IMPORT_TABLE_INDEX_OF(joyGetDevCapsA)},
        {L"joyGetDevCapsW", IMPORT_TABLE_INDEX_OF(joyGetDevCapsW)},
        {L"joyGetNumDevs", IMPORT_TABLE_INDEX_OF(joyGetNumDevs)},
        {L"joyGetPos", IMPORT_TABLE_INDEX_OF(joyGetPos)},
        {L"joyGetPosEx", IMPORT_TABLE_INDEX_OF(joyGetPosEx)},
        {L"joyGetThreshold", IMPORT_TABLE_INDEX_OF(joyGetThreshold)},
        {L"joyReleaseCapture", IMPORT_TABLE_INDEX_OF(joyReleaseCapture)},
        {L"joySetCapture", IMPORT_TABLE_INDEX_OF(joySetCapture)},
        {L"joySetThreshold", IMPORT_TABLE_INDEX_OF(joySetThreshold)}};

    /// Singleton Xidi API implementation object.
    static JoystickFunctionReplacer joystickFunctionReplacer;
  } // namespace ImportApiWinMM
} // namespace Xidi
