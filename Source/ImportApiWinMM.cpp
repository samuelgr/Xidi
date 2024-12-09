/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
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

#include <Infra/Core/Message.h>
#include <Infra/Core/ProcessInfo.h>

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "Configuration.h"
#include "Globals.h"
#include "Strings.h"

/// Computes the index of the specified named function in the pointer array of the import table.
#define IMPORT_TABLE_INDEX_OF(name)                                                                \
  (offsetof(UImportTable, named.##name) / sizeof(UImportTable::ptr[0]))

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
        LRESULT(__stdcall* CloseDriver)(HDRVR, LPARAM, LPARAM);
        LRESULT(__stdcall* DefDriverProc)(DWORD_PTR, HDRVR, UINT, LONG, LONG);
        BOOL(__stdcall* DriverCallback)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD);
        HMODULE(__stdcall* DrvGetModuleHandle)(HDRVR);
        HMODULE(__stdcall* GetDriverModuleHandle)(HDRVR);
        HDRVR(__stdcall* OpenDriver)(LPCWSTR, LPCWSTR, LPARAM);
        BOOL(__stdcall* PlaySoundA)(LPCSTR, HMODULE, DWORD);
        BOOL(__stdcall* PlaySoundW)(LPCWSTR, HMODULE, DWORD);
        LRESULT(__stdcall* SendDriverMessage)(HDRVR, UINT, LPARAM, LPARAM);

        MMRESULT(__stdcall* auxGetDevCapsA)(UINT_PTR, LPAUXCAPSA, UINT);
        MMRESULT(__stdcall* auxGetDevCapsW)(UINT_PTR, LPAUXCAPSW, UINT);
        UINT(__stdcall* auxGetNumDevs)(void);
        MMRESULT(__stdcall* auxGetVolume)(UINT, LPDWORD);
        MMRESULT(__stdcall* auxOutMessage)(UINT, UINT, DWORD_PTR, DWORD_PTR);
        MMRESULT(__stdcall* auxSetVolume)(UINT, DWORD);

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

        BOOL(__stdcall* mciDriverNotify)(HWND, MCIDEVICEID, UINT);
        UINT(__stdcall* mciDriverYield)(MCIDEVICEID);
        BOOL(__stdcall* mciExecute)(LPCSTR);
        BOOL(__stdcall* mciFreeCommandResource)(UINT);
        HANDLE(__stdcall* mciGetCreatorTask)(MCIDEVICEID);
        MCIDEVICEID(__stdcall* mciGetDeviceIDA)(LPCSTR);
        MCIDEVICEID(__stdcall* mciGetDeviceIDW)(LPCWSTR);
        MCIDEVICEID(__stdcall* mciGetDeviceIDFromElementIDA)(DWORD, LPCSTR);
        MCIDEVICEID(__stdcall* mciGetDeviceIDFromElementIDW)(DWORD, LPCWSTR);
        DWORD_PTR(__stdcall* mciGetDriverData)(MCIDEVICEID);
        BOOL(__stdcall* mciGetErrorStringA)(DWORD, LPCSTR, UINT);
        BOOL(__stdcall* mciGetErrorStringW)(DWORD, LPWSTR, UINT);
        YIELDPROC(__stdcall* mciGetYieldProc)(MCIDEVICEID, LPDWORD);
        UINT(__stdcall* mciLoadCommandResource)(HINSTANCE, LPCWSTR, UINT);
        MCIERROR(__stdcall* mciSendCommandA)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
        MCIERROR(__stdcall* mciSendCommandW)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
        MCIERROR(__stdcall* mciSendStringA)(LPCSTR, LPSTR, UINT, HANDLE);
        MCIERROR(__stdcall* mciSendStringW)(LPCWSTR, LPWSTR, UINT, HANDLE);
        BOOL(__stdcall* mciSetDriverData)(MCIDEVICEID, DWORD_PTR);
        UINT(__stdcall* mciSetYieldProc)(MCIDEVICEID, YIELDPROC, DWORD);

        MMRESULT(__stdcall* midiConnect)(HMIDI, HMIDIOUT, LPVOID);
        MMRESULT(__stdcall* midiDisconnect)(HMIDI, HMIDIOUT, LPVOID);

        MMRESULT(__stdcall* midiInAddBuffer)(HMIDIIN, LPMIDIHDR, UINT);
        MMRESULT(__stdcall* midiInClose)(HMIDIIN);
        MMRESULT(__stdcall* midiInGetDevCapsA)(UINT_PTR, LPMIDIINCAPSA, UINT);
        MMRESULT(__stdcall* midiInGetDevCapsW)(UINT_PTR, LPMIDIINCAPSW, UINT);
        MMRESULT(__stdcall* midiInGetErrorTextA)(MMRESULT, LPSTR, UINT);
        MMRESULT(__stdcall* midiInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
        MMRESULT(__stdcall* midiInGetID)(HMIDIIN, LPUINT);
        UINT(__stdcall* midiInGetNumDevs)(void);
        DWORD(__stdcall* midiInMessage)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR);
        MMRESULT(__stdcall* midiInOpen)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD);
        MMRESULT(__stdcall* midiInPrepareHeader)(HMIDIIN, LPMIDIHDR, UINT);
        MMRESULT(__stdcall* midiInReset)(HMIDIIN);
        MMRESULT(__stdcall* midiInStart)(HMIDIIN);
        MMRESULT(__stdcall* midiInStop)(HMIDIIN);
        MMRESULT(__stdcall* midiInUnprepareHeader)(HMIDIIN, LPMIDIHDR, UINT);

        MMRESULT(__stdcall* midiOutCacheDrumPatches)(HMIDIOUT, UINT, WORD*, UINT);
        MMRESULT(__stdcall* midiOutCachePatches)(HMIDIOUT, UINT, WORD*, UINT);
        MMRESULT(__stdcall* midiOutClose)(HMIDIOUT);
        MMRESULT(__stdcall* midiOutGetDevCapsA)(UINT_PTR, LPMIDIOUTCAPSA, UINT);
        MMRESULT(__stdcall* midiOutGetDevCapsW)(UINT_PTR, LPMIDIOUTCAPSW, UINT);
        UINT(__stdcall* midiOutGetErrorTextA)(MMRESULT, LPSTR, UINT);
        UINT(__stdcall* midiOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
        MMRESULT(__stdcall* midiOutGetID)(HMIDIOUT, LPUINT);
        UINT(__stdcall* midiOutGetNumDevs)(void);
        MMRESULT(__stdcall* midiOutGetVolume)(HMIDIOUT, LPDWORD);
        MMRESULT(__stdcall* midiOutLongMsg)(HMIDIOUT, LPMIDIHDR, UINT);
        DWORD(__stdcall* midiOutMessage)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR);
        MMRESULT(__stdcall* midiOutOpen)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD);
        MMRESULT(__stdcall* midiOutPrepareHeader)(HMIDIOUT, LPMIDIHDR, UINT);
        MMRESULT(__stdcall* midiOutReset)(HMIDIOUT);
        MMRESULT(__stdcall* midiOutSetVolume)(HMIDIOUT, DWORD);
        MMRESULT(__stdcall* midiOutShortMsg)(HMIDIOUT, DWORD);
        MMRESULT(__stdcall* midiOutUnprepareHeader)(HMIDIOUT, LPMIDIHDR, UINT);

        MMRESULT(__stdcall* midiStreamClose)(HMIDISTRM);
        MMRESULT(__stdcall* midiStreamOpen)
        (LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD);
        MMRESULT(__stdcall* midiStreamOut)(HMIDISTRM, LPMIDIHDR, UINT);
        MMRESULT(__stdcall* midiStreamPause)(HMIDISTRM);
        MMRESULT(__stdcall* midiStreamPosition)(HMIDISTRM, LPMMTIME, UINT);
        MMRESULT(__stdcall* midiStreamProperty)(HMIDISTRM, LPBYTE, DWORD);
        MMRESULT(__stdcall* midiStreamRestart)(HMIDISTRM);
        MMRESULT(__stdcall* midiStreamStop)(HMIDISTRM);

        MMRESULT(__stdcall* mixerClose)(HMIXER);
        MMRESULT(__stdcall* mixerGetControlDetailsA)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);
        MMRESULT(__stdcall* mixerGetControlDetailsW)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);
        MMRESULT(__stdcall* mixerGetDevCapsA)(UINT_PTR, LPMIXERCAPS, UINT);
        MMRESULT(__stdcall* mixerGetDevCapsW)(UINT_PTR, LPMIXERCAPS, UINT);
        MMRESULT(__stdcall* mixerGetID)(HMIXEROBJ, UINT*, DWORD);
        MMRESULT(__stdcall* mixerGetLineControlsA)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD);
        MMRESULT(__stdcall* mixerGetLineControlsW)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD);
        MMRESULT(__stdcall* mixerGetLineInfoA)(HMIXEROBJ, LPMIXERLINE, DWORD);
        MMRESULT(__stdcall* mixerGetLineInfoW)(HMIXEROBJ, LPMIXERLINE, DWORD);
        UINT(__stdcall* mixerGetNumDevs)(void);
        DWORD(__stdcall* mixerMessage)(HMIXER, UINT, DWORD_PTR, DWORD_PTR);
        MMRESULT(__stdcall* mixerOpen)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD);
        MMRESULT(__stdcall* mixerSetControlDetails)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);

        MMRESULT(__stdcall* mmioAdvance)(HMMIO, LPMMIOINFO, UINT);
        MMRESULT(__stdcall* mmioAscend)(HMMIO, LPMMCKINFO, UINT);
        MMRESULT(__stdcall* mmioClose)(HMMIO, UINT);
        MMRESULT(__stdcall* mmioCreateChunk)(HMMIO, LPMMCKINFO, UINT);
        MMRESULT(__stdcall* mmioDescend)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT);
        MMRESULT(__stdcall* mmioFlush)(HMMIO, UINT);
        MMRESULT(__stdcall* mmioGetInfo)(HMMIO, LPMMIOINFO, UINT);
        LPMMIOPROC(__stdcall* mmioInstallIOProcA)(FOURCC, LPMMIOPROC, DWORD);
        LPMMIOPROC(__stdcall* mmioInstallIOProcW)(FOURCC, LPMMIOPROC, DWORD);
        HMMIO(__stdcall* mmioOpenA)(LPSTR, LPMMIOINFO, DWORD);
        HMMIO(__stdcall* mmioOpenW)(LPWSTR, LPMMIOINFO, DWORD);
        LONG(__stdcall* mmioRead)(HMMIO, HPSTR, LONG);
        MMRESULT(__stdcall* mmioRenameA)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD);
        MMRESULT(__stdcall* mmioRenameW)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD);
        LONG(__stdcall* mmioSeek)(HMMIO, LONG, int);
        LRESULT(__stdcall* mmioSendMessage)(HMMIO, UINT, LPARAM, LPARAM);
        MMRESULT(__stdcall* mmioSetBuffer)(HMMIO, LPSTR, LONG, UINT);
        MMRESULT(__stdcall* mmioSetInfo)(HMMIO, LPCMMIOINFO, UINT);
        FOURCC(__stdcall* mmioStringToFOURCCA)(LPCSTR, UINT);
        FOURCC(__stdcall* mmioStringToFOURCCW)(LPCWSTR, UINT);
        LONG(__stdcall* mmioWrite)(HMMIO, const char*, LONG);

        BOOL(__stdcall* sndPlaySoundA)(LPCSTR lpszSound, UINT fuSound);
        BOOL(__stdcall* sndPlaySoundW)(LPCWSTR lpszSound, UINT fuSound);

        MMRESULT(__stdcall* timeBeginPeriod)(UINT);
        MMRESULT(__stdcall* timeEndPeriod)(UINT);
        MMRESULT(__stdcall* timeGetDevCaps)(LPTIMECAPS, UINT);
        MMRESULT(__stdcall* timeGetSystemTime)(LPMMTIME, UINT);
        DWORD(__stdcall* timeGetTime)(void);
        MMRESULT(__stdcall* timeKillEvent)(UINT);
        MMRESULT(__stdcall* timeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);

        MMRESULT(__stdcall* waveInAddBuffer)(HWAVEIN, LPWAVEHDR, UINT);
        MMRESULT(__stdcall* waveInClose)(HWAVEIN);
        MMRESULT(__stdcall* waveInGetDevCapsA)(UINT_PTR, LPWAVEINCAPSA, UINT);
        MMRESULT(__stdcall* waveInGetDevCapsW)(UINT_PTR, LPWAVEINCAPSW, UINT);
        MMRESULT(__stdcall* waveInGetErrorTextA)(MMRESULT, LPCSTR, UINT);
        MMRESULT(__stdcall* waveInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
        MMRESULT(__stdcall* waveInGetID)(HWAVEIN, LPUINT);
        UINT(__stdcall* waveInGetNumDevs)(void);
        MMRESULT(__stdcall* waveInGetPosition)(HWAVEIN, LPMMTIME, UINT);
        DWORD(__stdcall* waveInMessage)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR);
        MMRESULT(__stdcall* waveInOpen)
        (LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
        MMRESULT(__stdcall* waveInPrepareHeader)(HWAVEIN, LPWAVEHDR, UINT);
        MMRESULT(__stdcall* waveInReset)(HWAVEIN);
        MMRESULT(__stdcall* waveInStart)(HWAVEIN);
        MMRESULT(__stdcall* waveInStop)(HWAVEIN);
        MMRESULT(__stdcall* waveInUnprepareHeader)(HWAVEIN, LPWAVEHDR, UINT);

        MMRESULT(__stdcall* waveOutBreakLoop)(HWAVEOUT);
        MMRESULT(__stdcall* waveOutClose)(HWAVEOUT);
        MMRESULT(__stdcall* waveOutGetDevCapsA)(UINT_PTR, LPWAVEOUTCAPSA, UINT);
        MMRESULT(__stdcall* waveOutGetDevCapsW)(UINT_PTR, LPWAVEOUTCAPSW, UINT);
        MMRESULT(__stdcall* waveOutGetErrorTextA)(MMRESULT, LPCSTR, UINT);
        MMRESULT(__stdcall* waveOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
        MMRESULT(__stdcall* waveOutGetID)(HWAVEOUT, LPUINT);
        UINT(__stdcall* waveOutGetNumDevs)(void);
        MMRESULT(__stdcall* waveOutGetPitch)(HWAVEOUT, LPDWORD);
        MMRESULT(__stdcall* waveOutGetPlaybackRate)(HWAVEOUT, LPDWORD);
        MMRESULT(__stdcall* waveOutGetPosition)(HWAVEOUT, LPMMTIME, UINT);
        MMRESULT(__stdcall* waveOutGetVolume)(HWAVEOUT, LPDWORD);
        DWORD(__stdcall* waveOutMessage)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR);
        MMRESULT(__stdcall* waveOutOpen)
        (LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
        MMRESULT(__stdcall* waveOutPause)(HWAVEOUT);
        MMRESULT(__stdcall* waveOutPrepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
        MMRESULT(__stdcall* waveOutReset)(HWAVEOUT);
        MMRESULT(__stdcall* waveOutRestart)(HWAVEOUT);
        MMRESULT(__stdcall* waveOutSetPitch)(HWAVEOUT, DWORD);
        MMRESULT(__stdcall* waveOutSetPlaybackRate)(HWAVEOUT, DWORD);
        MMRESULT(__stdcall* waveOutSetVolume)(HWAVEOUT, DWORD);
        MMRESULT(__stdcall* waveOutUnprepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
        MMRESULT(__stdcall* waveOutWrite)(HWAVEOUT, LPWAVEHDR, UINT);
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
      return Globals::GetConfigurationData()
          .GetFirstStringValue(
              Strings::kStrConfigurationSectionImport, Strings::kStrConfigurationSettingImportWinMM)
          .value_or(Strings::kStrSystemLibraryFilenameWinMM);
    }

    /// Logs a warning event related to failure to import a particular function from the import
    /// library.
    /// @param [in] functionName Name of the function whose import attempt failed.
    static void LogImportFailed(LPCWSTR functionName)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Warning,
          L"Import library is missing WinMM function \"%s\". Attempts to call it will fail.",
          functionName);
    }

    /// Logs an error event related to a missing import function that has been invoked and then
    /// terminates the application.
    /// @param [in] functionName Name of the function that was invoked.
    static void TerminateAndLogMissingFunctionCalled(LPCWSTR functionName)
    {
      Infra::Message::OutputFormatted(
          Infra::Message::ESeverity::Error,
          L"Application has attempted to call missing WinMM import function \"%s\".",
          functionName);
      TerminateProcess(Infra::ProcessInfo::GetCurrentProcessHandle(), (UINT)-1);
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

            procAddress = GetProcAddress(loadedLibrary, "CloseDriver");
            if (nullptr == procAddress) LogImportFailed(L"CloseDriver");
            importTable.named.CloseDriver =
                reinterpret_cast<decltype(importTable.named.CloseDriver)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DefDriverProc");
            if (nullptr == procAddress) LogImportFailed(L"DefDriverProc");
            importTable.named.DefDriverProc =
                reinterpret_cast<decltype(importTable.named.DefDriverProc)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DriverCallback");
            if (nullptr == procAddress) LogImportFailed(L"DriverCallback");
            importTable.named.DriverCallback =
                reinterpret_cast<decltype(importTable.named.DriverCallback)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "DrvGetModuleHandle");
            if (nullptr == procAddress) LogImportFailed(L"DrvGetModuleHandle");
            importTable.named.DrvGetModuleHandle =
                reinterpret_cast<decltype(importTable.named.DrvGetModuleHandle)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "GetDriverModuleHandle");
            if (nullptr == procAddress) LogImportFailed(L"GetDriverModuleHandle");
            importTable.named.GetDriverModuleHandle =
                reinterpret_cast<decltype(importTable.named.GetDriverModuleHandle)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "OpenDriver");
            if (nullptr == procAddress) LogImportFailed(L"OpenDriver");
            importTable.named.OpenDriver =
                reinterpret_cast<decltype(importTable.named.OpenDriver)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "PlaySoundA");
            if (nullptr == procAddress) LogImportFailed(L"PlaySoundA");
            importTable.named.PlaySoundA =
                reinterpret_cast<decltype(importTable.named.PlaySoundA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "PlaySoundW");
            if (nullptr == procAddress) LogImportFailed(L"PlaySoundW");
            importTable.named.PlaySoundW =
                reinterpret_cast<decltype(importTable.named.PlaySoundW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "SendDriverMessage");
            if (nullptr == procAddress) LogImportFailed(L"SendDriverMessage");
            importTable.named.SendDriverMessage =
                reinterpret_cast<decltype(importTable.named.SendDriverMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"auxGetDevCapsA");
            importTable.named.auxGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.auxGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"auxGetDevCapsW");
            importTable.named.auxGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.auxGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"auxGetNumDevs");
            importTable.named.auxGetNumDevs =
                reinterpret_cast<decltype(importTable.named.auxGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
            if (nullptr == procAddress) LogImportFailed(L"auxGetVolume");
            importTable.named.auxGetVolume =
                reinterpret_cast<decltype(importTable.named.auxGetVolume)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
            if (nullptr == procAddress) LogImportFailed(L"auxOutMessage");
            importTable.named.auxOutMessage =
                reinterpret_cast<decltype(importTable.named.auxOutMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
            if (nullptr == procAddress) LogImportFailed(L"auxSetVolume");
            importTable.named.auxSetVolume =
                reinterpret_cast<decltype(importTable.named.auxSetVolume)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
            if (nullptr == procAddress) LogImportFailed(L"joyConfigChanged");
            importTable.named.joyConfigChanged =
                reinterpret_cast<decltype(importTable.named.joyConfigChanged)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"joyGetDevCapsA");
            importTable.named.joyGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.joyGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"joyGetDevCapsW");
            importTable.named.joyGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.joyGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"joyGetNumDevs");
            importTable.named.joyGetNumDevs =
                reinterpret_cast<decltype(importTable.named.joyGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
            if (nullptr == procAddress) LogImportFailed(L"joyGetPos");
            importTable.named.joyGetPos =
                reinterpret_cast<decltype(importTable.named.joyGetPos)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
            if (nullptr == procAddress) LogImportFailed(L"joyGetPosEx");
            importTable.named.joyGetPosEx =
                reinterpret_cast<decltype(importTable.named.joyGetPosEx)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
            if (nullptr == procAddress) LogImportFailed(L"joyGetThreshold");
            importTable.named.joyGetThreshold =
                reinterpret_cast<decltype(importTable.named.joyGetThreshold)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
            if (nullptr == procAddress) LogImportFailed(L"joyReleaseCapture");
            importTable.named.joyReleaseCapture =
                reinterpret_cast<decltype(importTable.named.joyReleaseCapture)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
            if (nullptr == procAddress) LogImportFailed(L"joySetCapture");
            importTable.named.joySetCapture =
                reinterpret_cast<decltype(importTable.named.joySetCapture)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
            if (nullptr == procAddress) LogImportFailed(L"joySetThreshold");
            importTable.named.joySetThreshold =
                reinterpret_cast<decltype(importTable.named.joySetThreshold)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciDriverNotify");
            if (nullptr == procAddress) LogImportFailed(L"mciDriverNotify");
            importTable.named.mciDriverNotify =
                reinterpret_cast<decltype(importTable.named.mciDriverNotify)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciDriverYield");
            if (nullptr == procAddress) LogImportFailed(L"mciDriverYield");
            importTable.named.mciDriverYield =
                reinterpret_cast<decltype(importTable.named.mciDriverYield)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciExecute");
            if (nullptr == procAddress) LogImportFailed(L"mciExecute");
            importTable.named.mciExecute =
                reinterpret_cast<decltype(importTable.named.mciExecute)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciFreeCommandResource");
            if (nullptr == procAddress) LogImportFailed(L"mciFreeCommandResource");
            importTable.named.mciFreeCommandResource =
                reinterpret_cast<decltype(importTable.named.mciFreeCommandResource)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetCreatorTask");
            if (nullptr == procAddress) LogImportFailed(L"mciGetCreatorTask");
            importTable.named.mciGetCreatorTask =
                reinterpret_cast<decltype(importTable.named.mciGetCreatorTask)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDA");
            if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDA");
            importTable.named.mciGetDeviceIDA =
                reinterpret_cast<decltype(importTable.named.mciGetDeviceIDA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDW");
            if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDW");
            importTable.named.mciGetDeviceIDW =
                reinterpret_cast<decltype(importTable.named.mciGetDeviceIDW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDA");
            if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDA");
            importTable.named.mciGetDeviceIDFromElementIDA =
                reinterpret_cast<decltype(importTable.named.mciGetDeviceIDFromElementIDA)>(
                    procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDW");
            if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDW");
            importTable.named.mciGetDeviceIDFromElementIDW =
                reinterpret_cast<decltype(importTable.named.mciGetDeviceIDFromElementIDW)>(
                    procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetDriverData");
            if (nullptr == procAddress) LogImportFailed(L"mciGetDriverData");
            importTable.named.mciGetDriverData =
                reinterpret_cast<decltype(importTable.named.mciGetDriverData)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringA");
            if (nullptr == procAddress) LogImportFailed(L"mciGetErrorStringA");
            importTable.named.mciGetErrorStringA =
                reinterpret_cast<decltype(importTable.named.mciGetErrorStringA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringW");
            if (nullptr == procAddress) LogImportFailed(L"mciGetErrorStringW");
            importTable.named.mciGetErrorStringW =
                reinterpret_cast<decltype(importTable.named.mciGetErrorStringW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciGetYieldProc");
            if (nullptr == procAddress) LogImportFailed(L"mciGetYieldProc");
            importTable.named.mciGetYieldProc =
                reinterpret_cast<decltype(importTable.named.mciGetYieldProc)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciLoadCommandResource");
            if (nullptr == procAddress) LogImportFailed(L"mciLoadCommandResource");
            importTable.named.mciLoadCommandResource =
                reinterpret_cast<decltype(importTable.named.mciLoadCommandResource)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciSendCommandA");
            if (nullptr == procAddress) LogImportFailed(L"mciSendCommandA");
            importTable.named.mciSendCommandA =
                reinterpret_cast<decltype(importTable.named.mciSendCommandA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciSendCommandW");
            if (nullptr == procAddress) LogImportFailed(L"mciSendCommandW");
            importTable.named.mciSendCommandW =
                reinterpret_cast<decltype(importTable.named.mciSendCommandW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciSendStringA");
            if (nullptr == procAddress) LogImportFailed(L"mciSendStringA");
            importTable.named.mciSendStringA =
                reinterpret_cast<decltype(importTable.named.mciSendStringA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciSendStringW");
            if (nullptr == procAddress) LogImportFailed(L"mciSendStringW");
            importTable.named.mciSendStringW =
                reinterpret_cast<decltype(importTable.named.mciSendStringW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciSetDriverData");
            if (nullptr == procAddress) LogImportFailed(L"mciSetDriverData");
            importTable.named.mciSetDriverData =
                reinterpret_cast<decltype(importTable.named.mciSetDriverData)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mciSetYieldProc");
            if (nullptr == procAddress) LogImportFailed(L"mciSetYieldProc");
            importTable.named.mciSetYieldProc =
                reinterpret_cast<decltype(importTable.named.mciSetYieldProc)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiConnect");
            if (nullptr == procAddress) LogImportFailed(L"midiConnect");
            importTable.named.midiConnect =
                reinterpret_cast<decltype(importTable.named.midiConnect)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiDisconnect");
            if (nullptr == procAddress) LogImportFailed(L"midiDisconnect");
            importTable.named.midiDisconnect =
                reinterpret_cast<decltype(importTable.named.midiDisconnect)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInAddBuffer");
            if (nullptr == procAddress) LogImportFailed(L"midiInAddBuffer");
            importTable.named.midiInAddBuffer =
                reinterpret_cast<decltype(importTable.named.midiInAddBuffer)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInClose");
            if (nullptr == procAddress) LogImportFailed(L"midiInClose");
            importTable.named.midiInClose =
                reinterpret_cast<decltype(importTable.named.midiInClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"midiInGetDevCapsA");
            importTable.named.midiInGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.midiInGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"midiInGetDevCapsW");
            importTable.named.midiInGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.midiInGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextA");
            if (nullptr == procAddress) LogImportFailed(L"midiInGetErrorTextA");
            importTable.named.midiInGetErrorTextA =
                reinterpret_cast<decltype(importTable.named.midiInGetErrorTextA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextW");
            if (nullptr == procAddress) LogImportFailed(L"midiInGetErrorTextW");
            importTable.named.midiInGetErrorTextW =
                reinterpret_cast<decltype(importTable.named.midiInGetErrorTextW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInGetID");
            if (nullptr == procAddress) LogImportFailed(L"midiInGetID");
            importTable.named.midiInGetID =
                reinterpret_cast<decltype(importTable.named.midiInGetID)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"midiInGetNumDevs");
            importTable.named.midiInGetNumDevs =
                reinterpret_cast<decltype(importTable.named.midiInGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInMessage");
            if (nullptr == procAddress) LogImportFailed(L"midiInMessage");
            importTable.named.midiInMessage =
                reinterpret_cast<decltype(importTable.named.midiInMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInOpen");
            if (nullptr == procAddress) LogImportFailed(L"midiInOpen");
            importTable.named.midiInOpen =
                reinterpret_cast<decltype(importTable.named.midiInOpen)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInPrepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"midiInPrepareHeader");
            importTable.named.midiInPrepareHeader =
                reinterpret_cast<decltype(importTable.named.midiInPrepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInReset");
            if (nullptr == procAddress) LogImportFailed(L"midiInReset");
            importTable.named.midiInReset =
                reinterpret_cast<decltype(importTable.named.midiInReset)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInStart");
            if (nullptr == procAddress) LogImportFailed(L"midiInStart");
            importTable.named.midiInStart =
                reinterpret_cast<decltype(importTable.named.midiInStart)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInStop");
            if (nullptr == procAddress) LogImportFailed(L"midiInStop");
            importTable.named.midiInStop =
                reinterpret_cast<decltype(importTable.named.midiInStop)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiInUnprepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"midiInUnprepareHeader");
            importTable.named.midiInUnprepareHeader =
                reinterpret_cast<decltype(importTable.named.midiInUnprepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutCacheDrumPatches");
            if (nullptr == procAddress) LogImportFailed(L"midiOutCacheDrumPatches");
            importTable.named.midiOutCacheDrumPatches =
                reinterpret_cast<decltype(importTable.named.midiOutCacheDrumPatches)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutCachePatches");
            if (nullptr == procAddress) LogImportFailed(L"midiOutCachePatches");
            importTable.named.midiOutCachePatches =
                reinterpret_cast<decltype(importTable.named.midiOutCachePatches)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutClose");
            if (nullptr == procAddress) LogImportFailed(L"midiOutClose");
            importTable.named.midiOutClose =
                reinterpret_cast<decltype(importTable.named.midiOutClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetDevCapsA");
            importTable.named.midiOutGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.midiOutGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetDevCapsW");
            importTable.named.midiOutGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.midiOutGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextA");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetErrorTextA");
            importTable.named.midiOutGetErrorTextA =
                reinterpret_cast<decltype(importTable.named.midiOutGetErrorTextA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextW");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetErrorTextW");
            importTable.named.midiOutGetErrorTextW =
                reinterpret_cast<decltype(importTable.named.midiOutGetErrorTextW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetID");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetID");
            importTable.named.midiOutGetID =
                reinterpret_cast<decltype(importTable.named.midiOutGetID)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetNumDevs");
            importTable.named.midiOutGetNumDevs =
                reinterpret_cast<decltype(importTable.named.midiOutGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutGetVolume");
            if (nullptr == procAddress) LogImportFailed(L"midiOutGetVolume");
            importTable.named.midiOutGetVolume =
                reinterpret_cast<decltype(importTable.named.midiOutGetVolume)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutLongMsg");
            if (nullptr == procAddress) LogImportFailed(L"midiOutLongMsg");
            importTable.named.midiOutLongMsg =
                reinterpret_cast<decltype(importTable.named.midiOutLongMsg)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutMessage");
            if (nullptr == procAddress) LogImportFailed(L"midiOutMessage");
            importTable.named.midiOutMessage =
                reinterpret_cast<decltype(importTable.named.midiOutMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutOpen");
            if (nullptr == procAddress) LogImportFailed(L"midiOutOpen");
            importTable.named.midiOutOpen =
                reinterpret_cast<decltype(importTable.named.midiOutOpen)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutPrepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"midiOutPrepareHeader");
            importTable.named.midiOutPrepareHeader =
                reinterpret_cast<decltype(importTable.named.midiOutPrepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutReset");
            if (nullptr == procAddress) LogImportFailed(L"midiOutReset");
            importTable.named.midiOutReset =
                reinterpret_cast<decltype(importTable.named.midiOutReset)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutSetVolume");
            if (nullptr == procAddress) LogImportFailed(L"midiOutSetVolume");
            importTable.named.midiOutSetVolume =
                reinterpret_cast<decltype(importTable.named.midiOutSetVolume)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutShortMsg");
            if (nullptr == procAddress) LogImportFailed(L"midiOutShortMsg");
            importTable.named.midiOutShortMsg =
                reinterpret_cast<decltype(importTable.named.midiOutShortMsg)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiOutUnprepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"midiOutUnprepareHeader");
            importTable.named.midiOutUnprepareHeader =
                reinterpret_cast<decltype(importTable.named.midiOutUnprepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamClose");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamClose");
            importTable.named.midiStreamClose =
                reinterpret_cast<decltype(importTable.named.midiStreamClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamOpen");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamOpen");
            importTable.named.midiStreamOpen =
                reinterpret_cast<decltype(importTable.named.midiStreamOpen)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamOut");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamOut");
            importTable.named.midiStreamOut =
                reinterpret_cast<decltype(importTable.named.midiStreamOut)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamPause");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamPause");
            importTable.named.midiStreamPause =
                reinterpret_cast<decltype(importTable.named.midiStreamPause)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamPosition");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamPosition");
            importTable.named.midiStreamPosition =
                reinterpret_cast<decltype(importTable.named.midiStreamPosition)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamProperty");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamProperty");
            importTable.named.midiStreamProperty =
                reinterpret_cast<decltype(importTable.named.midiStreamProperty)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamRestart");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamRestart");
            importTable.named.midiStreamRestart =
                reinterpret_cast<decltype(importTable.named.midiStreamRestart)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "midiStreamStop");
            if (nullptr == procAddress) LogImportFailed(L"midiStreamStop");
            importTable.named.midiStreamStop =
                reinterpret_cast<decltype(importTable.named.midiStreamStop)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerClose");
            if (nullptr == procAddress) LogImportFailed(L"mixerClose");
            importTable.named.mixerClose =
                reinterpret_cast<decltype(importTable.named.mixerClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsA");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetControlDetailsA");
            importTable.named.mixerGetControlDetailsA =
                reinterpret_cast<decltype(importTable.named.mixerGetControlDetailsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsW");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetControlDetailsW");
            importTable.named.mixerGetControlDetailsW =
                reinterpret_cast<decltype(importTable.named.mixerGetControlDetailsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetDevCapsA");
            importTable.named.mixerGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.mixerGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetDevCapsW");
            importTable.named.mixerGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.mixerGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetID");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetID");
            importTable.named.mixerGetID =
                reinterpret_cast<decltype(importTable.named.mixerGetID)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsA");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetLineControlsA");
            importTable.named.mixerGetLineControlsA =
                reinterpret_cast<decltype(importTable.named.mixerGetLineControlsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsW");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetLineControlsW");
            importTable.named.mixerGetLineControlsW =
                reinterpret_cast<decltype(importTable.named.mixerGetLineControlsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoA");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetLineInfoA");
            importTable.named.mixerGetLineInfoA =
                reinterpret_cast<decltype(importTable.named.mixerGetLineInfoA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoW");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetLineInfoW");
            importTable.named.mixerGetLineInfoW =
                reinterpret_cast<decltype(importTable.named.mixerGetLineInfoW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"mixerGetNumDevs");
            importTable.named.mixerGetNumDevs =
                reinterpret_cast<decltype(importTable.named.mixerGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerMessage");
            if (nullptr == procAddress) LogImportFailed(L"mixerMessage");
            importTable.named.mixerMessage =
                reinterpret_cast<decltype(importTable.named.mixerMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerOpen");
            if (nullptr == procAddress) LogImportFailed(L"mixerOpen");
            importTable.named.mixerOpen =
                reinterpret_cast<decltype(importTable.named.mixerOpen)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mixerSetControlDetails");
            if (nullptr == procAddress) LogImportFailed(L"mixerSetControlDetails");
            importTable.named.mixerSetControlDetails =
                reinterpret_cast<decltype(importTable.named.mixerSetControlDetails)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioAdvance");
            if (nullptr == procAddress) LogImportFailed(L"mmioAdvance");
            importTable.named.mmioAdvance =
                reinterpret_cast<decltype(importTable.named.mmioAdvance)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioAscend");
            if (nullptr == procAddress) LogImportFailed(L"mmioAscend");
            importTable.named.mmioAscend =
                reinterpret_cast<decltype(importTable.named.mmioAscend)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioClose");
            if (nullptr == procAddress) LogImportFailed(L"mmioClose");
            importTable.named.mmioClose =
                reinterpret_cast<decltype(importTable.named.mmioClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioCreateChunk");
            if (nullptr == procAddress) LogImportFailed(L"mmioCreateChunk");
            importTable.named.mmioCreateChunk =
                reinterpret_cast<decltype(importTable.named.mmioCreateChunk)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioDescend");
            if (nullptr == procAddress) LogImportFailed(L"mmioDescend");
            importTable.named.mmioDescend =
                reinterpret_cast<decltype(importTable.named.mmioDescend)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioFlush");
            if (nullptr == procAddress) LogImportFailed(L"mmioFlush");
            importTable.named.mmioFlush =
                reinterpret_cast<decltype(importTable.named.mmioFlush)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioGetInfo");
            if (nullptr == procAddress) LogImportFailed(L"mmioGetInfo");
            importTable.named.mmioGetInfo =
                reinterpret_cast<decltype(importTable.named.mmioGetInfo)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcA");
            if (nullptr == procAddress) LogImportFailed(L"mmioInstallIOProcA");
            importTable.named.mmioInstallIOProcA =
                reinterpret_cast<decltype(importTable.named.mmioInstallIOProcA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcW");
            if (nullptr == procAddress) LogImportFailed(L"mmioInstallIOProcW");
            importTable.named.mmioInstallIOProcW =
                reinterpret_cast<decltype(importTable.named.mmioInstallIOProcW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioOpenA");
            if (nullptr == procAddress) LogImportFailed(L"mmioOpenA");
            importTable.named.mmioOpenA =
                reinterpret_cast<decltype(importTable.named.mmioOpenA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioOpenW");
            if (nullptr == procAddress) LogImportFailed(L"mmioOpenW");
            importTable.named.mmioOpenW =
                reinterpret_cast<decltype(importTable.named.mmioOpenW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioRead");
            if (nullptr == procAddress) LogImportFailed(L"mmioRead");
            importTable.named.mmioRead =
                reinterpret_cast<decltype(importTable.named.mmioRead)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioRenameA");
            if (nullptr == procAddress) LogImportFailed(L"mmioRenameA");
            importTable.named.mmioRenameA =
                reinterpret_cast<decltype(importTable.named.mmioRenameA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioRenameW");
            if (nullptr == procAddress) LogImportFailed(L"mmioRenameW");
            importTable.named.mmioRenameW =
                reinterpret_cast<decltype(importTable.named.mmioRenameW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioSeek");
            if (nullptr == procAddress) LogImportFailed(L"mmioSeek");
            importTable.named.mmioSeek =
                reinterpret_cast<decltype(importTable.named.mmioSeek)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioSendMessage");
            if (nullptr == procAddress) LogImportFailed(L"mmioSendMessage");
            importTable.named.mmioSendMessage =
                reinterpret_cast<decltype(importTable.named.mmioSendMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioSetBuffer");
            if (nullptr == procAddress) LogImportFailed(L"mmioSetBuffer");
            importTable.named.mmioSetBuffer =
                reinterpret_cast<decltype(importTable.named.mmioSetBuffer)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioSetInfo");
            if (nullptr == procAddress) LogImportFailed(L"mmioSetInfo");
            importTable.named.mmioSetInfo =
                reinterpret_cast<decltype(importTable.named.mmioSetInfo)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCA");
            if (nullptr == procAddress) LogImportFailed(L"mmioStringToFOURCCA");
            importTable.named.mmioStringToFOURCCA =
                reinterpret_cast<decltype(importTable.named.mmioStringToFOURCCA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCW");
            if (nullptr == procAddress) LogImportFailed(L"mmioStringToFOURCCW");
            importTable.named.mmioStringToFOURCCW =
                reinterpret_cast<decltype(importTable.named.mmioStringToFOURCCW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "mmioWrite");
            if (nullptr == procAddress) LogImportFailed(L"mmioWrite");
            importTable.named.mmioWrite =
                reinterpret_cast<decltype(importTable.named.mmioWrite)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundA");
            if (nullptr == procAddress) LogImportFailed(L"sndPlaySoundA");
            importTable.named.sndPlaySoundA =
                reinterpret_cast<decltype(importTable.named.sndPlaySoundA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundW");
            if (nullptr == procAddress) LogImportFailed(L"sndPlaySoundW");
            importTable.named.sndPlaySoundW =
                reinterpret_cast<decltype(importTable.named.sndPlaySoundW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
            if (nullptr == procAddress) LogImportFailed(L"timeBeginPeriod");
            importTable.named.timeBeginPeriod =
                reinterpret_cast<decltype(importTable.named.timeBeginPeriod)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
            if (nullptr == procAddress) LogImportFailed(L"timeEndPeriod");
            importTable.named.timeEndPeriod =
                reinterpret_cast<decltype(importTable.named.timeEndPeriod)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
            if (nullptr == procAddress) LogImportFailed(L"timeGetDevCaps");
            importTable.named.timeGetDevCaps =
                reinterpret_cast<decltype(importTable.named.timeGetDevCaps)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
            if (nullptr == procAddress) LogImportFailed(L"timeGetSystemTime");
            importTable.named.timeGetSystemTime =
                reinterpret_cast<decltype(importTable.named.timeGetSystemTime)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
            if (nullptr == procAddress) LogImportFailed(L"timeGetTime");
            importTable.named.timeGetTime =
                reinterpret_cast<decltype(importTable.named.timeGetTime)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
            if (nullptr == procAddress) LogImportFailed(L"timeKillEvent");
            importTable.named.timeKillEvent =
                reinterpret_cast<decltype(importTable.named.timeKillEvent)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
            if (nullptr == procAddress) LogImportFailed(L"timeSetEvent");
            importTable.named.timeSetEvent =
                reinterpret_cast<decltype(importTable.named.timeSetEvent)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInAddBuffer");
            if (nullptr == procAddress) LogImportFailed(L"waveInAddBuffer");
            importTable.named.waveInAddBuffer =
                reinterpret_cast<decltype(importTable.named.waveInAddBuffer)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInClose");
            if (nullptr == procAddress) LogImportFailed(L"waveInClose");
            importTable.named.waveInClose =
                reinterpret_cast<decltype(importTable.named.waveInClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetDevCapsA");
            importTable.named.waveInGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.waveInGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetDevCapsW");
            importTable.named.waveInGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.waveInGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextA");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetErrorTextA");
            importTable.named.waveInGetErrorTextA =
                reinterpret_cast<decltype(importTable.named.waveInGetErrorTextA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextW");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetErrorTextW");
            importTable.named.waveInGetErrorTextW =
                reinterpret_cast<decltype(importTable.named.waveInGetErrorTextW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetID");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetID");
            importTable.named.waveInGetID =
                reinterpret_cast<decltype(importTable.named.waveInGetID)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetNumDevs");
            importTable.named.waveInGetNumDevs =
                reinterpret_cast<decltype(importTable.named.waveInGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInGetPosition");
            if (nullptr == procAddress) LogImportFailed(L"waveInGetPosition");
            importTable.named.waveInGetPosition =
                reinterpret_cast<decltype(importTable.named.waveInGetPosition)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInMessage");
            if (nullptr == procAddress) LogImportFailed(L"waveInMessage");
            importTable.named.waveInMessage =
                reinterpret_cast<decltype(importTable.named.waveInMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInOpen");
            if (nullptr == procAddress) LogImportFailed(L"waveInOpen");
            importTable.named.waveInOpen =
                reinterpret_cast<decltype(importTable.named.waveInOpen)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInPrepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"waveInPrepareHeader");
            importTable.named.waveInPrepareHeader =
                reinterpret_cast<decltype(importTable.named.waveInPrepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInReset");
            if (nullptr == procAddress) LogImportFailed(L"waveInReset");
            importTable.named.waveInReset =
                reinterpret_cast<decltype(importTable.named.waveInReset)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInStart");
            if (nullptr == procAddress) LogImportFailed(L"waveInStart");
            importTable.named.waveInStart =
                reinterpret_cast<decltype(importTable.named.waveInStart)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInStop");
            if (nullptr == procAddress) LogImportFailed(L"waveInStop");
            importTable.named.waveInStop =
                reinterpret_cast<decltype(importTable.named.waveInStop)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveInUnprepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"waveInUnprepareHeader");
            importTable.named.waveInUnprepareHeader =
                reinterpret_cast<decltype(importTable.named.waveInUnprepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutBreakLoop");
            if (nullptr == procAddress) LogImportFailed(L"waveOutBreakLoop");
            importTable.named.waveOutBreakLoop =
                reinterpret_cast<decltype(importTable.named.waveOutBreakLoop)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutClose");
            if (nullptr == procAddress) LogImportFailed(L"waveOutClose");
            importTable.named.waveOutClose =
                reinterpret_cast<decltype(importTable.named.waveOutClose)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsA");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetDevCapsA");
            importTable.named.waveOutGetDevCapsA =
                reinterpret_cast<decltype(importTable.named.waveOutGetDevCapsA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsW");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetDevCapsW");
            importTable.named.waveOutGetDevCapsW =
                reinterpret_cast<decltype(importTable.named.waveOutGetDevCapsW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextA");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetErrorTextA");
            importTable.named.waveOutGetErrorTextA =
                reinterpret_cast<decltype(importTable.named.waveOutGetErrorTextA)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextW");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetErrorTextW");
            importTable.named.waveOutGetErrorTextW =
                reinterpret_cast<decltype(importTable.named.waveOutGetErrorTextW)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetID");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetID");
            importTable.named.waveOutGetID =
                reinterpret_cast<decltype(importTable.named.waveOutGetID)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetNumDevs");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetNumDevs");
            importTable.named.waveOutGetNumDevs =
                reinterpret_cast<decltype(importTable.named.waveOutGetNumDevs)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetPitch");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetPitch");
            importTable.named.waveOutGetPitch =
                reinterpret_cast<decltype(importTable.named.waveOutGetPitch)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetPlaybackRate");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetPlaybackRate");
            importTable.named.waveOutGetPlaybackRate =
                reinterpret_cast<decltype(importTable.named.waveOutGetPlaybackRate)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetPosition");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetPosition");
            importTable.named.waveOutGetPosition =
                reinterpret_cast<decltype(importTable.named.waveOutGetPosition)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutGetVolume");
            if (nullptr == procAddress) LogImportFailed(L"waveOutGetVolume");
            importTable.named.waveOutGetVolume =
                reinterpret_cast<decltype(importTable.named.waveOutGetVolume)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutMessage");
            if (nullptr == procAddress) LogImportFailed(L"waveOutMessage");
            importTable.named.waveOutMessage =
                reinterpret_cast<decltype(importTable.named.waveOutMessage)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutOpen");
            if (nullptr == procAddress) LogImportFailed(L"waveOutOpen");
            importTable.named.waveOutOpen =
                reinterpret_cast<decltype(importTable.named.waveOutOpen)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutPause");
            if (nullptr == procAddress) LogImportFailed(L"waveOutPause");
            importTable.named.waveOutPause =
                reinterpret_cast<decltype(importTable.named.waveOutPause)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutPrepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"waveOutPrepareHeader");
            importTable.named.waveOutPrepareHeader =
                reinterpret_cast<decltype(importTable.named.waveOutPrepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutReset");
            if (nullptr == procAddress) LogImportFailed(L"waveOutReset");
            importTable.named.waveOutReset =
                reinterpret_cast<decltype(importTable.named.waveOutReset)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutRestart");
            if (nullptr == procAddress) LogImportFailed(L"waveOutRestart");
            importTable.named.waveOutRestart =
                reinterpret_cast<decltype(importTable.named.waveOutRestart)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutSetPitch");
            if (nullptr == procAddress) LogImportFailed(L"waveOutSetPitch");
            importTable.named.waveOutSetPitch =
                reinterpret_cast<decltype(importTable.named.waveOutSetPitch)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutSetPlaybackRate");
            if (nullptr == procAddress) LogImportFailed(L"waveOutSetPlaybackRate");
            importTable.named.waveOutSetPlaybackRate =
                reinterpret_cast<decltype(importTable.named.waveOutSetPlaybackRate)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutSetVolume");
            if (nullptr == procAddress) LogImportFailed(L"waveOutSetVolume");
            importTable.named.waveOutSetVolume =
                reinterpret_cast<decltype(importTable.named.waveOutSetVolume)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutUnprepareHeader");
            if (nullptr == procAddress) LogImportFailed(L"waveOutUnprepareHeader");
            importTable.named.waveOutUnprepareHeader =
                reinterpret_cast<decltype(importTable.named.waveOutUnprepareHeader)>(procAddress);

            procAddress = GetProcAddress(loadedLibrary, "waveOutWrite");
            if (nullptr == procAddress) LogImportFailed(L"waveOutWrite");
            importTable.named.waveOutWrite =
                reinterpret_cast<decltype(importTable.named.waveOutWrite)>(procAddress);

            // Initialization complete.
            Infra::Message::OutputFormatted(
                Infra::Message::ESeverity::Info,
                L"Successfully initialized imported WinMM functions.");
          });
    }

    LRESULT CloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2)
    {
      Initialize();

      if (nullptr == importTable.named.CloseDriver)
        TerminateAndLogMissingFunctionCalled(L"CloseDriver");

      return importTable.named.CloseDriver(hdrvr, lParam1, lParam2);
    }

    LRESULT DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
    {
      Initialize();

      if (nullptr == importTable.named.DefDriverProc)
        TerminateAndLogMissingFunctionCalled(L"DefDriverProc");

      return importTable.named.DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
    }

    BOOL DriverCallback(
        DWORD dwCallBack,
        DWORD dwFlags,
        HDRVR hdrvr,
        DWORD msg,
        DWORD dwUser,
        DWORD dwParam1,
        DWORD dwParam2)
    {
      Initialize();

      if (nullptr == importTable.named.DriverCallback)
        TerminateAndLogMissingFunctionCalled(L"DriverCallback");

      return importTable.named.DriverCallback(
          dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
    }

    HMODULE DrvGetModuleHandle(HDRVR hDriver)
    {
      Initialize();

      if (nullptr == importTable.named.DrvGetModuleHandle)
        TerminateAndLogMissingFunctionCalled(L"DrvGetModuleHandle");

      return importTable.named.DrvGetModuleHandle(hDriver);
    }

    HMODULE GetDriverModuleHandle(HDRVR hdrvr)
    {
      Initialize();

      if (nullptr == importTable.named.GetDriverModuleHandle)
        TerminateAndLogMissingFunctionCalled(L"GetDriverModuleHandle");

      return importTable.named.GetDriverModuleHandle(hdrvr);
    }

    HDRVR OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
    {
      Initialize();

      if (nullptr == importTable.named.OpenDriver)
        TerminateAndLogMissingFunctionCalled(L"OpenDriver");

      return importTable.named.OpenDriver(lpDriverName, lpSectionName, lParam);
    }

    BOOL PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
    {
      Initialize();

      if (nullptr == importTable.named.PlaySoundA)
        TerminateAndLogMissingFunctionCalled(L"PlaySoundA");

      return importTable.named.PlaySoundA(pszSound, hmod, fdwSound);
    }

    BOOL PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
    {
      Initialize();

      if (nullptr == importTable.named.PlaySoundW)
        TerminateAndLogMissingFunctionCalled(L"PlaySoundW");

      return importTable.named.PlaySoundW(pszSound, hmod, fdwSound);
    }

    LRESULT SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
    {
      Initialize();

      if (nullptr == importTable.named.SendDriverMessage)
        TerminateAndLogMissingFunctionCalled(L"SendDriverMessage");

      return importTable.named.SendDriverMessage(hdrvr, msg, lParam1, lParam2);
    }

    MMRESULT auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
    {
      Initialize();

      if (nullptr == importTable.named.auxGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"auxGetDevCapsA");

      return importTable.named.auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
    }

    MMRESULT auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
    {
      Initialize();

      if (nullptr == importTable.named.auxGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"auxGetDevCapsW");

      return importTable.named.auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
    }

    UINT auxGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.auxGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"auxGetNumDevs");

      return importTable.named.auxGetNumDevs();
    }

    MMRESULT auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
    {
      Initialize();

      if (nullptr == importTable.named.auxGetVolume)
        TerminateAndLogMissingFunctionCalled(L"auxGetVolume");

      return importTable.named.auxGetVolume(uDeviceID, lpdwVolume);
    }

    MMRESULT auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
      Initialize();

      if (nullptr == importTable.named.auxOutMessage)
        TerminateAndLogMissingFunctionCalled(L"auxOutMessage");

      return importTable.named.auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
    }

    MMRESULT auxSetVolume(UINT uDeviceID, DWORD dwVolume)
    {
      Initialize();

      if (nullptr == importTable.named.auxSetVolume)
        TerminateAndLogMissingFunctionCalled(L"auxSetVolume");

      return importTable.named.auxSetVolume(uDeviceID, dwVolume);
    }

    MMRESULT joyConfigChanged(DWORD dwFlags)
    {
      Initialize();

      if (nullptr == importTable.named.joyConfigChanged)
        TerminateAndLogMissingFunctionCalled(L"joyConfigChanged");

      return importTable.named.joyConfigChanged(dwFlags);
    }

    MMRESULT joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
    {
      Initialize();

      if (nullptr == importTable.named.joyGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"joyGetDevCapsA");

      return importTable.named.joyGetDevCapsA(uJoyID, pjc, cbjc);
    }

    MMRESULT joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
    {
      Initialize();

      if (nullptr == importTable.named.joyGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"joyGetDevCapsW");

      return importTable.named.joyGetDevCapsW(uJoyID, pjc, cbjc);
    }

    UINT joyGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.joyGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"joyGetNumDevs");

      return importTable.named.joyGetNumDevs();
    }

    MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji)
    {
      Initialize();

      if (nullptr == importTable.named.joyGetPos)
        TerminateAndLogMissingFunctionCalled(L"joyGetPos");

      return importTable.named.joyGetPos(uJoyID, pji);
    }

    MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
    {
      Initialize();

      if (nullptr == importTable.named.joyGetPosEx)
        TerminateAndLogMissingFunctionCalled(L"joyGetPosEx");

      return importTable.named.joyGetPosEx(uJoyID, pji);
    }

    MMRESULT joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
    {
      Initialize();

      if (nullptr == importTable.named.joyGetThreshold)
        TerminateAndLogMissingFunctionCalled(L"joyGetThreshold");

      return importTable.named.joyGetThreshold(uJoyID, puThreshold);
    }

    MMRESULT joyReleaseCapture(UINT uJoyID)
    {
      Initialize();

      if (nullptr == importTable.named.joyReleaseCapture)
        TerminateAndLogMissingFunctionCalled(L"joyReleaseCapture");

      return importTable.named.joyReleaseCapture(uJoyID);
    }

    MMRESULT joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
    {
      Initialize();

      if (nullptr == importTable.named.joySetCapture)
        TerminateAndLogMissingFunctionCalled(L"joySetCapture");

      return importTable.named.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
    }

    MMRESULT joySetThreshold(UINT uJoyID, UINT uThreshold)
    {
      Initialize();

      if (nullptr == importTable.named.joySetThreshold)
        TerminateAndLogMissingFunctionCalled(L"joySetThreshold");

      return importTable.named.joySetThreshold(uJoyID, uThreshold);
    }

    BOOL mciDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
    {
      Initialize();

      if (nullptr == importTable.named.mciDriverNotify)
        TerminateAndLogMissingFunctionCalled(L"mciDriverNotify");

      return importTable.named.mciDriverNotify(hwndCallback, IDDevice, uStatus);
    }

    UINT mciDriverYield(MCIDEVICEID IDDevice)
    {
      Initialize();

      if (nullptr == importTable.named.mciDriverYield)
        TerminateAndLogMissingFunctionCalled(L"mciDriverYield");

      return importTable.named.mciDriverYield(IDDevice);
    }

    BOOL mciExecute(LPCSTR pszCommand)
    {
      Initialize();

      if (nullptr == importTable.named.mciExecute)
        TerminateAndLogMissingFunctionCalled(L"mciExecute");

      return importTable.named.mciExecute(pszCommand);
    }

    BOOL mciFreeCommandResource(UINT uResource)
    {
      Initialize();

      if (nullptr == importTable.named.mciFreeCommandResource)
        TerminateAndLogMissingFunctionCalled(L"mciFreeCommandResource");

      return importTable.named.mciFreeCommandResource(uResource);
    }

    HANDLE mciGetCreatorTask(MCIDEVICEID IDDevice)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetCreatorTask)
        TerminateAndLogMissingFunctionCalled(L"mciGetCreatorTask");

      return importTable.named.mciGetCreatorTask(IDDevice);
    }

    MCIDEVICEID mciGetDeviceIDA(LPCSTR lpszDevice)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetDeviceIDA)
        TerminateAndLogMissingFunctionCalled(L"mciGetDeviceIDA");

      return importTable.named.mciGetDeviceIDA(lpszDevice);
    }

    MCIDEVICEID mciGetDeviceIDW(LPCWSTR lpszDevice)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetDeviceIDW)
        TerminateAndLogMissingFunctionCalled(L"mciGetDeviceIDW");

      return importTable.named.mciGetDeviceIDW(lpszDevice);
    }

    MCIDEVICEID mciGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetDeviceIDFromElementIDA)
        TerminateAndLogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDA");

      return importTable.named.mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
    }

    MCIDEVICEID mciGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetDeviceIDFromElementIDW)
        TerminateAndLogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDW");

      return importTable.named.mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
    }

    DWORD_PTR mciGetDriverData(MCIDEVICEID IDDevice)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetDriverData)
        TerminateAndLogMissingFunctionCalled(L"mciGetDriverData");

      return importTable.named.mciGetDriverData(IDDevice);
    }

    BOOL mciGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetErrorStringA)
        TerminateAndLogMissingFunctionCalled(L"mciGetErrorStringA");

      return importTable.named.mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
    }

    BOOL mciGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetErrorStringW)
        TerminateAndLogMissingFunctionCalled(L"mciGetErrorStringW");

      return importTable.named.mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
    }

    YIELDPROC mciGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
    {
      Initialize();

      if (nullptr == importTable.named.mciGetYieldProc)
        TerminateAndLogMissingFunctionCalled(L"mciGetYieldProc");

      return importTable.named.mciGetYieldProc(IDDevice, lpdwYieldData);
    }

    UINT mciLoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
    {
      Initialize();

      if (nullptr == importTable.named.mciLoadCommandResource)
        TerminateAndLogMissingFunctionCalled(L"mciLoadCommandResource");

      return importTable.named.mciLoadCommandResource(hInst, lpwstrResourceName, uType);
    }

    MCIERROR mciSendCommandA(
        MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
    {
      Initialize();

      if (nullptr == importTable.named.mciSendCommandA)
        TerminateAndLogMissingFunctionCalled(L"mciSendCommandA");

      return importTable.named.mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
    }

    MCIERROR mciSendCommandW(
        MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
    {
      Initialize();

      if (nullptr == importTable.named.mciSendCommandW)
        TerminateAndLogMissingFunctionCalled(L"mciSendCommandW");

      return importTable.named.mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
    }

    MCIERROR mciSendStringA(
        LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
    {
      Initialize();

      if (nullptr == importTable.named.mciSendStringA)
        TerminateAndLogMissingFunctionCalled(L"mciSendStringA");

      return importTable.named.mciSendStringA(
          lpszCommand, lpszReturnString, cchReturn, hwndCallback);
    }

    MCIERROR mciSendStringW(
        LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
    {
      Initialize();

      if (nullptr == importTable.named.mciSendStringW)
        TerminateAndLogMissingFunctionCalled(L"mciSendStringW");

      return importTable.named.mciSendStringW(
          lpszCommand, lpszReturnString, cchReturn, hwndCallback);
    }

    BOOL mciSetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
    {
      Initialize();

      if (nullptr == importTable.named.mciSetDriverData)
        TerminateAndLogMissingFunctionCalled(L"mciSetDriverData");

      return importTable.named.mciSetDriverData(IDDevice, data);
    }

    UINT mciSetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
    {
      Initialize();

      if (nullptr == importTable.named.mciSetYieldProc)
        TerminateAndLogMissingFunctionCalled(L"mciSetYieldProc");

      return importTable.named.mciSetYieldProc(IDDevice, yp, dwYieldData);
    }

    MMRESULT midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
    {
      Initialize();

      if (nullptr == importTable.named.midiConnect)
        TerminateAndLogMissingFunctionCalled(L"midiConnect");

      return importTable.named.midiConnect(hMidi, hmo, pReserved);
    }

    MMRESULT midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
    {
      Initialize();

      if (nullptr == importTable.named.midiDisconnect)
        TerminateAndLogMissingFunctionCalled(L"midiDisconnect");

      return importTable.named.midiDisconnect(hMidi, hmo, pReserved);
    }

    MMRESULT midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiInAddBuffer)
        TerminateAndLogMissingFunctionCalled(L"midiInAddBuffer");

      return importTable.named.midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
    }

    MMRESULT midiInClose(HMIDIIN hMidiIn)
    {
      Initialize();

      if (nullptr == importTable.named.midiInClose)
        TerminateAndLogMissingFunctionCalled(L"midiInClose");

      return importTable.named.midiInClose(hMidiIn);
    }

    MMRESULT midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
    {
      Initialize();

      if (nullptr == importTable.named.midiInGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"midiInGetDevCapsA");

      return importTable.named.midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
    }

    MMRESULT midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
    {
      Initialize();

      if (nullptr == importTable.named.midiInGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"midiInGetDevCapsW");

      return importTable.named.midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
    }

    MMRESULT midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.midiInGetErrorTextA)
        TerminateAndLogMissingFunctionCalled(L"midiInGetErrorTextA");

      return importTable.named.midiInGetErrorTextA(wError, lpText, cchText);
    }

    MMRESULT midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.midiInGetErrorTextW)
        TerminateAndLogMissingFunctionCalled(L"midiInGetErrorTextW");

      return importTable.named.midiInGetErrorTextW(wError, lpText, cchText);
    }

    MMRESULT midiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
    {
      Initialize();

      if (nullptr == importTable.named.midiInGetID)
        TerminateAndLogMissingFunctionCalled(L"midiInGetID");

      return importTable.named.midiInGetID(hmi, puDeviceID);
    }

    UINT midiInGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.midiInGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"midiInGetNumDevs");

      return importTable.named.midiInGetNumDevs();
    }

    DWORD midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
    {
      Initialize();

      if (nullptr == importTable.named.midiInMessage)
        TerminateAndLogMissingFunctionCalled(L"midiInMessage");

      return importTable.named.midiInMessage(deviceID, msg, dw1, dw2);
    }

    MMRESULT midiInOpen(
        LPHMIDIIN lphMidiIn,
        UINT uDeviceID,
        DWORD_PTR dwCallback,
        DWORD_PTR dwCallbackInstance,
        DWORD dwFlags)
    {
      Initialize();

      if (nullptr == importTable.named.midiInOpen)
        TerminateAndLogMissingFunctionCalled(L"midiInOpen");

      return importTable.named.midiInOpen(
          lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
    }

    MMRESULT midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiInPrepareHeader)
        TerminateAndLogMissingFunctionCalled(L"midiInPrepareHeader");

      return importTable.named.midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
    }

    MMRESULT midiInReset(HMIDIIN hMidiIn)
    {
      Initialize();

      if (nullptr == importTable.named.midiInReset)
        TerminateAndLogMissingFunctionCalled(L"midiInReset");

      return importTable.named.midiInReset(hMidiIn);
    }

    MMRESULT midiInStart(HMIDIIN hMidiIn)
    {
      Initialize();

      if (nullptr == importTable.named.midiInStart)
        TerminateAndLogMissingFunctionCalled(L"midiInStart");

      return importTable.named.midiInStart(hMidiIn);
    }

    MMRESULT midiInStop(HMIDIIN hMidiIn)
    {
      Initialize();

      if (nullptr == importTable.named.midiInStop)
        TerminateAndLogMissingFunctionCalled(L"midiInStop");

      return importTable.named.midiInStop(hMidiIn);
    }

    MMRESULT midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiInUnprepareHeader)
        TerminateAndLogMissingFunctionCalled(L"midiInUnprepareHeader");

      return importTable.named.midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
    }

    MMRESULT midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutCacheDrumPatches)
        TerminateAndLogMissingFunctionCalled(L"midiOutCacheDrumPatches");

      return importTable.named.midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
    }

    MMRESULT midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutCachePatches)
        TerminateAndLogMissingFunctionCalled(L"midiOutCachePatches");

      return importTable.named.midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
    }

    MMRESULT midiOutClose(HMIDIOUT hmo)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutClose)
        TerminateAndLogMissingFunctionCalled(L"midiOutClose");

      return importTable.named.midiOutClose(hmo);
    }

    MMRESULT midiOutGetDevCapsA(
        UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetDevCapsA");

      return importTable.named.midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
    }

    MMRESULT midiOutGetDevCapsW(
        UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetDevCapsW");

      return importTable.named.midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
    }

    UINT midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetErrorTextA)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetErrorTextA");

      return importTable.named.midiOutGetErrorTextA(mmrError, lpText, cchText);
    }

    UINT midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetErrorTextW)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetErrorTextW");

      return importTable.named.midiOutGetErrorTextW(mmrError, lpText, cchText);
    }

    MMRESULT midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetID)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetID");

      return importTable.named.midiOutGetID(hmo, puDeviceID);
    }

    UINT midiOutGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetNumDevs");

      return importTable.named.midiOutGetNumDevs();
    }

    MMRESULT midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutGetVolume)
        TerminateAndLogMissingFunctionCalled(L"midiOutGetVolume");

      return importTable.named.midiOutGetVolume(hmo, lpdwVolume);
    }

    MMRESULT midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutLongMsg)
        TerminateAndLogMissingFunctionCalled(L"midiOutLongMsg");

      return importTable.named.midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
    }

    DWORD midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutMessage)
        TerminateAndLogMissingFunctionCalled(L"midiOutMessage");

      return importTable.named.midiOutMessage(deviceID, msg, dw1, dw2);
    }

    MMRESULT midiOutOpen(
        LPHMIDIOUT lphmo,
        UINT uDeviceID,
        DWORD_PTR dwCallback,
        DWORD_PTR dwCallbackInstance,
        DWORD dwFlags)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutOpen)
        TerminateAndLogMissingFunctionCalled(L"midiOutOpen");

      return importTable.named.midiOutOpen(
          lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
    }

    MMRESULT midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutPrepareHeader)
        TerminateAndLogMissingFunctionCalled(L"midiOutPrepareHeader");

      return importTable.named.midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
    }

    MMRESULT midiOutReset(HMIDIOUT hmo)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutReset)
        TerminateAndLogMissingFunctionCalled(L"midiOutReset");

      return importTable.named.midiOutReset(hmo);
    }

    MMRESULT midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutSetVolume)
        TerminateAndLogMissingFunctionCalled(L"midiOutSetVolume");

      return importTable.named.midiOutSetVolume(hmo, dwVolume);
    }

    MMRESULT midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutShortMsg)
        TerminateAndLogMissingFunctionCalled(L"midiOutShortMsg");

      return importTable.named.midiOutShortMsg(hmo, dwMsg);
    }

    MMRESULT midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiOutUnprepareHeader)
        TerminateAndLogMissingFunctionCalled(L"midiOutUnprepareHeader");

      return importTable.named.midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
    }

    MMRESULT midiStreamClose(HMIDISTRM hStream)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamClose)
        TerminateAndLogMissingFunctionCalled(L"midiStreamClose");

      return importTable.named.midiStreamClose(hStream);
    }

    MMRESULT midiStreamOpen(
        LPHMIDISTRM lphStream,
        LPUINT puDeviceID,
        DWORD cMidi,
        DWORD_PTR dwCallback,
        DWORD_PTR dwInstance,
        DWORD fdwOpen)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamOpen)
        TerminateAndLogMissingFunctionCalled(L"midiStreamOpen");

      return importTable.named.midiStreamOpen(
          lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
    }

    MMRESULT midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamOut)
        TerminateAndLogMissingFunctionCalled(L"midiStreamOut");

      return importTable.named.midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
    }

    MMRESULT midiStreamPause(HMIDISTRM hms)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamPause)
        TerminateAndLogMissingFunctionCalled(L"midiStreamPause");

      return importTable.named.midiStreamPause(hms);
    }

    MMRESULT midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamPosition)
        TerminateAndLogMissingFunctionCalled(L"midiStreamPosition");

      return importTable.named.midiStreamPosition(hms, pmmt, cbmmt);
    }

    MMRESULT midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamProperty)
        TerminateAndLogMissingFunctionCalled(L"midiStreamProperty");

      return importTable.named.midiStreamProperty(hm, lppropdata, dwProperty);
    }

    MMRESULT midiStreamRestart(HMIDISTRM hms)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamRestart)
        TerminateAndLogMissingFunctionCalled(L"midiStreamRestart");

      return importTable.named.midiStreamRestart(hms);
    }

    MMRESULT midiStreamStop(HMIDISTRM hms)
    {
      Initialize();

      if (nullptr == importTable.named.midiStreamStop)
        TerminateAndLogMissingFunctionCalled(L"midiStreamStop");

      return importTable.named.midiStreamStop(hms);
    }

    MMRESULT mixerClose(HMIXER hmx)
    {
      Initialize();

      if (nullptr == importTable.named.mixerClose)
        TerminateAndLogMissingFunctionCalled(L"mixerClose");

      return importTable.named.mixerClose(hmx);
    }

    MMRESULT mixerGetControlDetailsA(
        HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetControlDetailsA)
        TerminateAndLogMissingFunctionCalled(L"mixerGetControlDetailsA");

      return importTable.named.mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
    }

    MMRESULT mixerGetControlDetailsW(
        HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetControlDetailsW)
        TerminateAndLogMissingFunctionCalled(L"mixerGetControlDetailsW");

      return importTable.named.mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
    }

    MMRESULT mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"mixerGetDevCapsA");

      return importTable.named.mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
    }

    MMRESULT mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"mixerGetDevCapsW");

      return importTable.named.mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
    }

    MMRESULT mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetID)
        TerminateAndLogMissingFunctionCalled(L"mixerGetID");

      return importTable.named.mixerGetID(hmxobj, puMxId, fdwId);
    }

    MMRESULT mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetLineControlsA)
        TerminateAndLogMissingFunctionCalled(L"mixerGetLineControlsA");

      return importTable.named.mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
    }

    MMRESULT mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetLineControlsW)
        TerminateAndLogMissingFunctionCalled(L"mixerGetLineControlsW");

      return importTable.named.mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
    }

    MMRESULT mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetLineInfoA)
        TerminateAndLogMissingFunctionCalled(L"mixerGetLineInfoA");

      return importTable.named.mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
    }

    MMRESULT mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetLineInfoW)
        TerminateAndLogMissingFunctionCalled(L"mixerGetLineInfoW");

      return importTable.named.mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
    }

    UINT mixerGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.mixerGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"mixerGetNumDevs");

      return importTable.named.mixerGetNumDevs();
    }

    DWORD mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
      Initialize();

      if (nullptr == importTable.named.mixerMessage)
        TerminateAndLogMissingFunctionCalled(L"mixerMessage");

      return importTable.named.mixerMessage(driverID, uMsg, dwParam1, dwParam2);
    }

    MMRESULT mixerOpen(
        LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
    {
      Initialize();

      if (nullptr == importTable.named.mixerOpen)
        TerminateAndLogMissingFunctionCalled(L"mixerOpen");

      return importTable.named.mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
    }

    MMRESULT mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
    {
      Initialize();

      if (nullptr == importTable.named.mixerSetControlDetails)
        TerminateAndLogMissingFunctionCalled(L"mixerSetControlDetails");

      return importTable.named.mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
    }

    MMRESULT mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioAdvance)
        TerminateAndLogMissingFunctionCalled(L"mmioAdvance");

      return importTable.named.mmioAdvance(hmmio, lpmmioinfo, wFlags);
    }

    MMRESULT mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioAscend)
        TerminateAndLogMissingFunctionCalled(L"mmioAscend");

      return importTable.named.mmioAscend(hmmio, lpck, wFlags);
    }

    MMRESULT mmioClose(HMMIO hmmio, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioClose)
        TerminateAndLogMissingFunctionCalled(L"mmioClose");

      return importTable.named.mmioClose(hmmio, wFlags);
    }

    MMRESULT mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioCreateChunk)
        TerminateAndLogMissingFunctionCalled(L"mmioCreateChunk");

      return importTable.named.mmioCreateChunk(hmmio, lpck, wFlags);
    }

    MMRESULT mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioDescend)
        TerminateAndLogMissingFunctionCalled(L"mmioDescend");

      return importTable.named.mmioDescend(hmmio, lpck, lpckParent, wFlags);
    }

    MMRESULT mmioFlush(HMMIO hmmio, UINT fuFlush)
    {
      Initialize();

      if (nullptr == importTable.named.mmioFlush)
        TerminateAndLogMissingFunctionCalled(L"mmioFlush");

      return importTable.named.mmioFlush(hmmio, fuFlush);
    }

    MMRESULT mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioGetInfo)
        TerminateAndLogMissingFunctionCalled(L"mmioGetInfo");

      return importTable.named.mmioGetInfo(hmmio, lpmmioinfo, wFlags);
    }

    LPMMIOPROC mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioInstallIOProcA)
        TerminateAndLogMissingFunctionCalled(L"mmioInstallIOProcA");

      return importTable.named.mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
    }

    LPMMIOPROC mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioInstallIOProcW)
        TerminateAndLogMissingFunctionCalled(L"mmioInstallIOProcW");

      return importTable.named.mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
    }

    HMMIO mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioOpenA)
        TerminateAndLogMissingFunctionCalled(L"mmioOpenA");

      return importTable.named.mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
    }

    HMMIO mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioOpenW)
        TerminateAndLogMissingFunctionCalled(L"mmioOpenW");

      return importTable.named.mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
    }

    LONG mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
    {
      Initialize();

      if (nullptr == importTable.named.mmioRead) TerminateAndLogMissingFunctionCalled(L"mmioRead");

      return importTable.named.mmioRead(hmmio, pch, cch);
    }

    MMRESULT mmioRenameA(
        LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioRenameA)
        TerminateAndLogMissingFunctionCalled(L"mmioRenameA");

      return importTable.named.mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
    }

    MMRESULT mmioRenameW(
        LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioRenameW)
        TerminateAndLogMissingFunctionCalled(L"mmioRenameW");

      return importTable.named.mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
    }

    LONG mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
    {
      Initialize();

      if (nullptr == importTable.named.mmioSeek) TerminateAndLogMissingFunctionCalled(L"mmioSeek");

      return importTable.named.mmioSeek(hmmio, lOffset, iOrigin);
    }

    LRESULT mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
    {
      Initialize();

      if (nullptr == importTable.named.mmioSendMessage)
        TerminateAndLogMissingFunctionCalled(L"mmioSendMessage");

      return importTable.named.mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
    }

    MMRESULT mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioSetBuffer)
        TerminateAndLogMissingFunctionCalled(L"mmioSetBuffer");

      return importTable.named.mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
    }

    MMRESULT mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioSetInfo)
        TerminateAndLogMissingFunctionCalled(L"mmioSetInfo");

      return importTable.named.mmioSetInfo(hmmio, lpmmioinfo, wFlags);
    }

    FOURCC mmioStringToFOURCCA(LPCSTR sz, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioStringToFOURCCA)
        TerminateAndLogMissingFunctionCalled(L"mmioStringToFOURCCA");

      return importTable.named.mmioStringToFOURCCA(sz, wFlags);
    }

    FOURCC mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags)
    {
      Initialize();

      if (nullptr == importTable.named.mmioStringToFOURCCW)
        TerminateAndLogMissingFunctionCalled(L"mmioStringToFOURCCW");

      return importTable.named.mmioStringToFOURCCW(sz, wFlags);
    }

    LONG mmioWrite(HMMIO hmmio, const char* pch, LONG cch)
    {
      Initialize();

      if (nullptr == importTable.named.mmioWrite)
        TerminateAndLogMissingFunctionCalled(L"mmioWrite");

      return importTable.named.mmioWrite(hmmio, pch, cch);
    }

    BOOL sndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
    {
      Initialize();

      if (nullptr == importTable.named.sndPlaySoundA)
        TerminateAndLogMissingFunctionCalled(L"sndPlaySoundA");

      return importTable.named.sndPlaySoundA(lpszSound, fuSound);
    }

    BOOL sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
    {
      Initialize();

      if (nullptr == importTable.named.sndPlaySoundW)
        TerminateAndLogMissingFunctionCalled(L"sndPlaySoundW");

      return importTable.named.sndPlaySoundW(lpszSound, fuSound);
    }

    MMRESULT timeBeginPeriod(UINT uPeriod)
    {
      Initialize();

      if (nullptr == importTable.named.timeBeginPeriod)
        TerminateAndLogMissingFunctionCalled(L"timeBeginPeriod");

      return importTable.named.timeBeginPeriod(uPeriod);
    }

    MMRESULT timeEndPeriod(UINT uPeriod)
    {
      Initialize();

      if (nullptr == importTable.named.timeEndPeriod)
        TerminateAndLogMissingFunctionCalled(L"timeEndPeriod");

      return importTable.named.timeEndPeriod(uPeriod);
    }

    MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
    {
      Initialize();

      if (nullptr == importTable.named.timeGetDevCaps)
        TerminateAndLogMissingFunctionCalled(L"timeGetDevCaps");

      return importTable.named.timeGetDevCaps(ptc, cbtc);
    }

    MMRESULT timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
    {
      Initialize();

      if (nullptr == importTable.named.timeGetSystemTime)
        TerminateAndLogMissingFunctionCalled(L"timeGetSystemTime");

      return importTable.named.timeGetSystemTime(pmmt, cbmmt);
    }

    DWORD timeGetTime(void)
    {
      Initialize();

      if (nullptr == importTable.named.timeGetTime)
        TerminateAndLogMissingFunctionCalled(L"timeGetTime");

      return importTable.named.timeGetTime();
    }

    MMRESULT timeKillEvent(UINT uTimerID)
    {
      Initialize();

      if (nullptr == importTable.named.timeKillEvent)
        TerminateAndLogMissingFunctionCalled(L"timeKillEvent");

      return importTable.named.timeKillEvent(uTimerID);
    }

    MMRESULT timeSetEvent(
        UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
    {
      Initialize();

      if (nullptr == importTable.named.timeSetEvent)
        TerminateAndLogMissingFunctionCalled(L"timeSetEvent");

      return importTable.named.timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
    }

    MMRESULT waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
    {
      Initialize();

      if (nullptr == importTable.named.waveInAddBuffer)
        TerminateAndLogMissingFunctionCalled(L"waveInAddBuffer");

      return importTable.named.waveInAddBuffer(hwi, pwh, cbwh);
    }

    MMRESULT waveInClose(HWAVEIN hwi)
    {
      Initialize();

      if (nullptr == importTable.named.waveInClose)
        TerminateAndLogMissingFunctionCalled(L"waveInClose");

      return importTable.named.waveInClose(hwi);
    }

    MMRESULT waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"waveInGetDevCapsA");

      return importTable.named.waveInGetDevCapsA(uDeviceID, pwic, cbwic);
    }

    MMRESULT waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"waveInGetDevCapsW");

      return importTable.named.waveInGetDevCapsW(uDeviceID, pwic, cbwic);
    }

    MMRESULT waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetErrorTextA)
        TerminateAndLogMissingFunctionCalled(L"waveInGetErrorTextA");

      return importTable.named.waveInGetErrorTextA(mmrError, pszText, cchText);
    }

    MMRESULT waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetErrorTextW)
        TerminateAndLogMissingFunctionCalled(L"waveInGetErrorTextW");

      return importTable.named.waveInGetErrorTextW(mmrError, pszText, cchText);
    }

    MMRESULT waveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetID)
        TerminateAndLogMissingFunctionCalled(L"waveInGetID");

      return importTable.named.waveInGetID(hwi, puDeviceID);
    }

    UINT waveInGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"waveInGetNumDevs");

      return importTable.named.waveInGetNumDevs();
    }

    MMRESULT waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
    {
      Initialize();

      if (nullptr == importTable.named.waveInGetPosition)
        TerminateAndLogMissingFunctionCalled(L"waveInGetPosition");

      return importTable.named.waveInGetPosition(hwi, pmmt, cbmmt);
    }

    DWORD waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
      Initialize();

      if (nullptr == importTable.named.waveInMessage)
        TerminateAndLogMissingFunctionCalled(L"waveInMessage");

      return importTable.named.waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
    }

    MMRESULT waveInOpen(
        LPHWAVEIN phwi,
        UINT uDeviceID,
        LPCWAVEFORMATEX pwfx,
        DWORD_PTR dwCallback,
        DWORD_PTR dwCallbackInstance,
        DWORD fdwOpen)
    {
      Initialize();

      if (nullptr == importTable.named.waveInOpen)
        TerminateAndLogMissingFunctionCalled(L"waveInOpen");

      return importTable.named.waveInOpen(
          phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
    }

    MMRESULT waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
    {
      Initialize();

      if (nullptr == importTable.named.waveInPrepareHeader)
        TerminateAndLogMissingFunctionCalled(L"waveInPrepareHeader");

      return importTable.named.waveInPrepareHeader(hwi, pwh, cbwh);
    }

    MMRESULT waveInReset(HWAVEIN hwi)
    {
      Initialize();

      if (nullptr == importTable.named.waveInReset)
        TerminateAndLogMissingFunctionCalled(L"waveInReset");

      return importTable.named.waveInReset(hwi);
    }

    MMRESULT waveInStart(HWAVEIN hwi)
    {
      Initialize();

      if (nullptr == importTable.named.waveInStart)
        TerminateAndLogMissingFunctionCalled(L"waveInStart");

      return importTable.named.waveInStart(hwi);
    }

    MMRESULT waveInStop(HWAVEIN hwi)
    {
      Initialize();

      if (nullptr == importTable.named.waveInStop)
        TerminateAndLogMissingFunctionCalled(L"waveInStop");

      return importTable.named.waveInStop(hwi);
    }

    MMRESULT waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
    {
      Initialize();

      if (nullptr == importTable.named.waveInUnprepareHeader)
        TerminateAndLogMissingFunctionCalled(L"waveInUnprepareHeader");

      return importTable.named.waveInUnprepareHeader(hwi, pwh, cbwh);
    }

    MMRESULT waveOutBreakLoop(HWAVEOUT hwo)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutBreakLoop)
        TerminateAndLogMissingFunctionCalled(L"waveOutBreakLoop");

      return importTable.named.waveOutBreakLoop(hwo);
    }

    MMRESULT waveOutClose(HWAVEOUT hwo)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutClose)
        TerminateAndLogMissingFunctionCalled(L"waveOutClose");

      return importTable.named.waveOutClose(hwo);
    }

    MMRESULT waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetDevCapsA)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetDevCapsA");

      return importTable.named.waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
    }

    MMRESULT waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetDevCapsW)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetDevCapsW");

      return importTable.named.waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
    }

    MMRESULT waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetErrorTextA)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetErrorTextA");

      return importTable.named.waveOutGetErrorTextA(mmrError, pszText, cchText);
    }

    MMRESULT waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetErrorTextW)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetErrorTextW");

      return importTable.named.waveOutGetErrorTextW(mmrError, pszText, cchText);
    }

    MMRESULT waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetID)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetID");

      return importTable.named.waveOutGetID(hwo, puDeviceID);
    }

    UINT waveOutGetNumDevs(void)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetNumDevs)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetNumDevs");

      return importTable.named.waveOutGetNumDevs();
    }

    MMRESULT waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetPitch)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetPitch");

      return importTable.named.waveOutGetPitch(hwo, pdwPitch);
    }

    MMRESULT waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetPlaybackRate)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetPlaybackRate");

      return importTable.named.waveOutGetPlaybackRate(hwo, pdwRate);
    }

    MMRESULT waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetPosition)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetPosition");

      return importTable.named.waveOutGetPosition(hwo, pmmt, cbmmt);
    }

    MMRESULT waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutGetVolume)
        TerminateAndLogMissingFunctionCalled(L"waveOutGetVolume");

      return importTable.named.waveOutGetVolume(hwo, pdwVolume);
    }

    DWORD waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutMessage)
        TerminateAndLogMissingFunctionCalled(L"waveOutMessage");

      return importTable.named.waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
    }

    MMRESULT waveOutOpen(
        LPHWAVEOUT phwo,
        UINT_PTR uDeviceID,
        LPWAVEFORMATEX pwfx,
        DWORD_PTR dwCallback,
        DWORD_PTR dwCallbackInstance,
        DWORD fdwOpen)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutOpen)
        TerminateAndLogMissingFunctionCalled(L"waveOutOpen");

      return importTable.named.waveOutOpen(
          phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
    }

    MMRESULT waveOutPause(HWAVEOUT hwo)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutPause)
        TerminateAndLogMissingFunctionCalled(L"waveOutPause");

      return importTable.named.waveOutPause(hwo);
    }

    MMRESULT waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutPrepareHeader)
        TerminateAndLogMissingFunctionCalled(L"waveOutPrepareHeader");

      return importTable.named.waveOutPrepareHeader(hwo, pwh, cbwh);
    }

    MMRESULT waveOutReset(HWAVEOUT hwo)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutReset)
        TerminateAndLogMissingFunctionCalled(L"waveOutReset");

      return importTable.named.waveOutReset(hwo);
    }

    MMRESULT waveOutRestart(HWAVEOUT hwo)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutRestart)
        TerminateAndLogMissingFunctionCalled(L"waveOutRestart");

      return importTable.named.waveOutRestart(hwo);
    }

    MMRESULT waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutSetPitch)
        TerminateAndLogMissingFunctionCalled(L"waveOutSetPitch");

      return importTable.named.waveOutSetPitch(hwo, dwPitch);
    }

    MMRESULT waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutSetPlaybackRate)
        TerminateAndLogMissingFunctionCalled(L"waveOutSetPlaybackRate");

      return importTable.named.waveOutSetPlaybackRate(hwo, dwRate);
    }

    MMRESULT waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutSetVolume)
        TerminateAndLogMissingFunctionCalled(L"waveOutSetVolume");

      return importTable.named.waveOutSetVolume(hwo, dwVolume);
    }

    MMRESULT waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutUnprepareHeader)
        TerminateAndLogMissingFunctionCalled(L"waveOutUnprepareHeader");

      return importTable.named.waveOutUnprepareHeader(hwo, pwh, cbwh);
    }

    MMRESULT waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
    {
      Initialize();

      if (nullptr == importTable.named.waveOutWrite)
        TerminateAndLogMissingFunctionCalled(L"waveOutWrite");

      return importTable.named.waveOutWrite(hwo, pwh, cbwh);
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
