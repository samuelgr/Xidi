/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file ImportApiWinMM.cpp
 *   Implementations of functions for accessing the WinMM API imported from
 *   the native WinMM library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Configuration.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "Message.h"
#include "Strings.h"

#include <mutex>
#include <string_view>


namespace Xidi
{
    namespace ImportApiWinMM
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Holds pointers to all the functions imported from the native WinMM library.
        struct SImportTable
        {
            LRESULT(WINAPI* CloseDriver)(HDRVR, LPARAM, LPARAM);
            LRESULT(WINAPI* DefDriverProc)(DWORD_PTR, HDRVR, UINT, LONG, LONG);
            BOOL(WINAPI* DriverCallback)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD);
            HMODULE(WINAPI* DrvGetModuleHandle)(HDRVR);
            HMODULE(WINAPI* GetDriverModuleHandle)(HDRVR);
            HDRVR(WINAPI* OpenDriver)(LPCWSTR, LPCWSTR, LPARAM);
            BOOL(WINAPI* PlaySoundA)(LPCSTR, HMODULE, DWORD);
            BOOL(WINAPI* PlaySoundW)(LPCWSTR, HMODULE, DWORD);
            LRESULT(WINAPI* SendDriverMessage)(HDRVR, UINT, LPARAM, LPARAM);

            MMRESULT(WINAPI* auxGetDevCapsA)(UINT_PTR, LPAUXCAPSA, UINT);
            MMRESULT(WINAPI* auxGetDevCapsW)(UINT_PTR, LPAUXCAPSW, UINT);
            UINT(WINAPI* auxGetNumDevs)(void);
            MMRESULT(WINAPI* auxGetVolume)(UINT, LPDWORD);
            MMRESULT(WINAPI* auxOutMessage)(UINT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT(WINAPI* auxSetVolume)(UINT, DWORD);

            MMRESULT(WINAPI* joyConfigChanged)(DWORD);
            MMRESULT(WINAPI* joyGetDevCapsA)(UINT_PTR, LPJOYCAPSA, UINT);
            MMRESULT(WINAPI* joyGetDevCapsW)(UINT_PTR, LPJOYCAPSW, UINT);
            UINT(WINAPI* joyGetNumDevs)(void);
            MMRESULT(WINAPI* joyGetPos)(UINT, LPJOYINFO);
            MMRESULT(WINAPI* joyGetPosEx)(UINT, LPJOYINFOEX);
            MMRESULT(WINAPI* joyGetThreshold)(UINT, LPUINT);
            MMRESULT(WINAPI* joyReleaseCapture)(UINT);
            MMRESULT(WINAPI* joySetCapture)(HWND, UINT, UINT, BOOL);
            MMRESULT(WINAPI* joySetThreshold)(UINT, UINT);

            BOOL(WINAPI* mciDriverNotify)(HWND, MCIDEVICEID, UINT);
            UINT(WINAPI* mciDriverYield)(MCIDEVICEID);
            BOOL(WINAPI* mciExecute)(LPCSTR);
            BOOL(WINAPI* mciFreeCommandResource)(UINT);
            HANDLE(WINAPI* mciGetCreatorTask)(MCIDEVICEID);
            MCIDEVICEID(WINAPI* mciGetDeviceIDA)(LPCSTR);
            MCIDEVICEID(WINAPI* mciGetDeviceIDW)(LPCWSTR);
            MCIDEVICEID(WINAPI* mciGetDeviceIDFromElementIDA)(DWORD, LPCSTR);
            MCIDEVICEID(WINAPI* mciGetDeviceIDFromElementIDW)(DWORD, LPCWSTR);
            DWORD_PTR(WINAPI* mciGetDriverData)(MCIDEVICEID);
            BOOL(WINAPI* mciGetErrorStringA)(DWORD, LPCSTR, UINT);
            BOOL(WINAPI* mciGetErrorStringW)(DWORD, LPWSTR, UINT);
            YIELDPROC(WINAPI* mciGetYieldProc)(MCIDEVICEID, LPDWORD);
            UINT(WINAPI* mciLoadCommandResource)(HINSTANCE, LPCWSTR, UINT);
            MCIERROR(WINAPI* mciSendCommandA)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
            MCIERROR(WINAPI* mciSendCommandW)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
            MCIERROR(WINAPI* mciSendStringA)(LPCSTR, LPSTR, UINT, HANDLE);
            MCIERROR(WINAPI* mciSendStringW)(LPCWSTR, LPWSTR, UINT, HANDLE);
            BOOL(WINAPI* mciSetDriverData)(MCIDEVICEID, DWORD_PTR);
            UINT(WINAPI* mciSetYieldProc)(MCIDEVICEID, YIELDPROC, DWORD);

            MMRESULT(WINAPI* midiConnect)(HMIDI, HMIDIOUT, LPVOID);
            MMRESULT(WINAPI* midiDisconnect)(HMIDI, HMIDIOUT, LPVOID);

            MMRESULT(WINAPI* midiInAddBuffer)(HMIDIIN, LPMIDIHDR, UINT);
            MMRESULT(WINAPI* midiInClose)(HMIDIIN);
            MMRESULT(WINAPI* midiInGetDevCapsA)(UINT_PTR, LPMIDIINCAPSA, UINT);
            MMRESULT(WINAPI* midiInGetDevCapsW)(UINT_PTR, LPMIDIINCAPSW, UINT);
            MMRESULT(WINAPI* midiInGetErrorTextA)(MMRESULT, LPSTR, UINT);
            MMRESULT(WINAPI* midiInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT(WINAPI* midiInGetID)(HMIDIIN, LPUINT);
            UINT(WINAPI* midiInGetNumDevs)(void);
            DWORD(WINAPI* midiInMessage)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT(WINAPI* midiInOpen)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT(WINAPI* midiInPrepareHeader)(HMIDIIN, LPMIDIHDR, UINT);
            MMRESULT(WINAPI* midiInReset)(HMIDIIN);
            MMRESULT(WINAPI* midiInStart)(HMIDIIN);
            MMRESULT(WINAPI* midiInStop)(HMIDIIN);
            MMRESULT(WINAPI* midiInUnprepareHeader)(HMIDIIN, LPMIDIHDR, UINT);

            MMRESULT(WINAPI* midiOutCacheDrumPatches)(HMIDIOUT, UINT, WORD*, UINT);
            MMRESULT(WINAPI* midiOutCachePatches)(HMIDIOUT, UINT, WORD*, UINT);
            MMRESULT(WINAPI* midiOutClose)(HMIDIOUT);
            MMRESULT(WINAPI* midiOutGetDevCapsA)(UINT_PTR, LPMIDIOUTCAPSA, UINT);
            MMRESULT(WINAPI* midiOutGetDevCapsW)(UINT_PTR, LPMIDIOUTCAPSW, UINT);
            UINT(WINAPI* midiOutGetErrorTextA)(MMRESULT, LPSTR, UINT);
            UINT(WINAPI* midiOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT(WINAPI* midiOutGetID)(HMIDIOUT, LPUINT);
            UINT(WINAPI* midiOutGetNumDevs)(void);
            MMRESULT(WINAPI* midiOutGetVolume)(HMIDIOUT, LPDWORD);
            MMRESULT(WINAPI* midiOutLongMsg)(HMIDIOUT, LPMIDIHDR, UINT);
            DWORD(WINAPI* midiOutMessage)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT(WINAPI* midiOutOpen)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT(WINAPI* midiOutPrepareHeader)(HMIDIOUT, LPMIDIHDR, UINT);
            MMRESULT(WINAPI* midiOutReset)(HMIDIOUT);
            MMRESULT(WINAPI* midiOutSetVolume)(HMIDIOUT, DWORD);
            MMRESULT(WINAPI* midiOutShortMsg)(HMIDIOUT, DWORD);
            MMRESULT(WINAPI* midiOutUnprepareHeader)(HMIDIOUT, LPMIDIHDR, UINT);

            MMRESULT(WINAPI* midiStreamClose)(HMIDISTRM);
            MMRESULT(WINAPI* midiStreamOpen)(LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT(WINAPI* midiStreamOut)(HMIDISTRM, LPMIDIHDR, UINT);
            MMRESULT(WINAPI* midiStreamPause)(HMIDISTRM);
            MMRESULT(WINAPI* midiStreamPosition)(HMIDISTRM, LPMMTIME, UINT);
            MMRESULT(WINAPI* midiStreamProperty)(HMIDISTRM, LPBYTE, DWORD);
            MMRESULT(WINAPI* midiStreamRestart)(HMIDISTRM);
            MMRESULT(WINAPI* midiStreamStop)(HMIDISTRM);

            MMRESULT(WINAPI* mixerClose)(HMIXER);
            MMRESULT(WINAPI* mixerGetControlDetailsA)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);
            MMRESULT(WINAPI* mixerGetControlDetailsW)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);
            MMRESULT(WINAPI* mixerGetDevCapsA)(UINT_PTR, LPMIXERCAPS, UINT);
            MMRESULT(WINAPI* mixerGetDevCapsW)(UINT_PTR, LPMIXERCAPS, UINT);
            MMRESULT(WINAPI* mixerGetID)(HMIXEROBJ, UINT*, DWORD);
            MMRESULT(WINAPI* mixerGetLineControlsA)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD);
            MMRESULT(WINAPI* mixerGetLineControlsW)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD);
            MMRESULT(WINAPI* mixerGetLineInfoA)(HMIXEROBJ, LPMIXERLINE, DWORD);
            MMRESULT(WINAPI* mixerGetLineInfoW)(HMIXEROBJ, LPMIXERLINE, DWORD);
            UINT(WINAPI* mixerGetNumDevs)(void);
            DWORD(WINAPI* mixerMessage)(HMIXER, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT(WINAPI* mixerOpen)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT(WINAPI* mixerSetControlDetails)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);

            MMRESULT(WINAPI* mmioAdvance)(HMMIO, LPMMIOINFO, UINT);
            MMRESULT(WINAPI* mmioAscend)(HMMIO, LPMMCKINFO, UINT);
            MMRESULT(WINAPI* mmioClose)(HMMIO, UINT);
            MMRESULT(WINAPI* mmioCreateChunk)(HMMIO, LPMMCKINFO, UINT);
            MMRESULT(WINAPI* mmioDescend)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT);
            MMRESULT(WINAPI* mmioFlush)(HMMIO, UINT);
            MMRESULT(WINAPI* mmioGetInfo)(HMMIO, LPMMIOINFO, UINT);
            LPMMIOPROC(WINAPI* mmioInstallIOProcA)(FOURCC, LPMMIOPROC, DWORD);
            LPMMIOPROC(WINAPI* mmioInstallIOProcW)(FOURCC, LPMMIOPROC, DWORD);
            HMMIO(WINAPI* mmioOpenA)(LPSTR, LPMMIOINFO, DWORD);
            HMMIO(WINAPI* mmioOpenW)(LPWSTR, LPMMIOINFO, DWORD);
            LONG(WINAPI* mmioRead)(HMMIO, HPSTR, LONG);
            MMRESULT(WINAPI* mmioRenameA)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD);
            MMRESULT(WINAPI* mmioRenameW)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD);
            LONG(WINAPI* mmioSeek)(HMMIO, LONG, int);
            LRESULT(WINAPI* mmioSendMessage)(HMMIO, UINT, LPARAM, LPARAM);
            MMRESULT(WINAPI* mmioSetBuffer)(HMMIO, LPSTR, LONG, UINT);
            MMRESULT(WINAPI* mmioSetInfo)(HMMIO, LPCMMIOINFO, UINT);
            FOURCC(WINAPI* mmioStringToFOURCCA)(LPCSTR, UINT);
            FOURCC(WINAPI* mmioStringToFOURCCW)(LPCWSTR, UINT);
            LONG(WINAPI* mmioWrite)(HMMIO, const char*, LONG);

            BOOL(WINAPI* sndPlaySoundA)(LPCSTR lpszSound, UINT fuSound);
            BOOL(WINAPI* sndPlaySoundW)(LPCWSTR lpszSound, UINT fuSound);

            MMRESULT(WINAPI* timeBeginPeriod)(UINT);
            MMRESULT(WINAPI* timeEndPeriod)(UINT);
            MMRESULT(WINAPI* timeGetDevCaps)(LPTIMECAPS, UINT);
            MMRESULT(WINAPI* timeGetSystemTime)(LPMMTIME, UINT);
            DWORD(WINAPI* timeGetTime)(void);
            MMRESULT(WINAPI* timeKillEvent)(UINT);
            MMRESULT(WINAPI* timeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);

            MMRESULT(WINAPI* waveInAddBuffer)(HWAVEIN, LPWAVEHDR, UINT);
            MMRESULT(WINAPI* waveInClose)(HWAVEIN);
            MMRESULT(WINAPI* waveInGetDevCapsA)(UINT_PTR, LPWAVEINCAPSA, UINT);
            MMRESULT(WINAPI* waveInGetDevCapsW)(UINT_PTR, LPWAVEINCAPSW, UINT);
            MMRESULT(WINAPI* waveInGetErrorTextA)(MMRESULT, LPCSTR, UINT);
            MMRESULT(WINAPI* waveInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT(WINAPI* waveInGetID)(HWAVEIN, LPUINT);
            UINT(WINAPI* waveInGetNumDevs)(void);
            MMRESULT(WINAPI* waveInGetPosition)(HWAVEIN, LPMMTIME, UINT);
            DWORD(WINAPI* waveInMessage)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT(WINAPI* waveInOpen)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT(WINAPI* waveInPrepareHeader)(HWAVEIN, LPWAVEHDR, UINT);
            MMRESULT(WINAPI* waveInReset)(HWAVEIN);
            MMRESULT(WINAPI* waveInStart)(HWAVEIN);
            MMRESULT(WINAPI* waveInStop)(HWAVEIN);
            MMRESULT(WINAPI* waveInUnprepareHeader)(HWAVEIN, LPWAVEHDR, UINT);

            MMRESULT(WINAPI* waveOutBreakLoop)(HWAVEOUT);
            MMRESULT(WINAPI* waveOutClose)(HWAVEOUT);
            MMRESULT(WINAPI* waveOutGetDevCapsA)(UINT_PTR, LPWAVEOUTCAPSA, UINT);
            MMRESULT(WINAPI* waveOutGetDevCapsW)(UINT_PTR, LPWAVEOUTCAPSW, UINT);
            MMRESULT(WINAPI* waveOutGetErrorTextA)(MMRESULT, LPCSTR, UINT);
            MMRESULT(WINAPI* waveOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT(WINAPI* waveOutGetID)(HWAVEOUT, LPUINT);
            UINT(WINAPI* waveOutGetNumDevs)(void);
            MMRESULT(WINAPI* waveOutGetPitch)(HWAVEOUT, LPDWORD);
            MMRESULT(WINAPI* waveOutGetPlaybackRate)(HWAVEOUT, LPDWORD);
            MMRESULT(WINAPI* waveOutGetPosition)(HWAVEOUT, LPMMTIME, UINT);
            MMRESULT(WINAPI* waveOutGetVolume)(HWAVEOUT, LPDWORD);
            DWORD(WINAPI* waveOutMessage)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT(WINAPI* waveOutOpen)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT(WINAPI* waveOutPause)(HWAVEOUT);
            MMRESULT(WINAPI* waveOutPrepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
            MMRESULT(WINAPI* waveOutReset)(HWAVEOUT);
            MMRESULT(WINAPI* waveOutRestart)(HWAVEOUT);
            MMRESULT(WINAPI* waveOutSetPitch)(HWAVEOUT, DWORD);
            MMRESULT(WINAPI* waveOutSetPlaybackRate)(HWAVEOUT, DWORD);
            MMRESULT(WINAPI* waveOutSetVolume)(HWAVEOUT, DWORD);
            MMRESULT(WINAPI* waveOutUnprepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
            MMRESULT(WINAPI* waveOutWrite)(HWAVEOUT, LPWAVEHDR, UINT);
        };


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// Holds the imported WinMM API function addresses.
        static SImportTable importTable;


        // -------- INTERNAL FUNCTIONS --------------------------------------------- //

        /// Retrieves the library path for the WinMM library that should be used for importing functions.
        /// @return Library path.
        static std::wstring_view GetImportLibraryPathWinMM(void)
        {
            const Configuration::Configuration& config = Globals::GetConfiguration();

            if ((true == config.IsDataValid()) && (true == config.GetData().SectionNamePairExists(Strings::kStrConfigurationSectionImport, Strings::kStrConfigurationSettingImportWinMM)))
            {
                return config.GetData()[Strings::kStrConfigurationSectionImport][Strings::kStrConfigurationSettingImportWinMM].FirstValue().GetStringValue();
            }
            else
            {
                return Strings::kStrSystemLibraryFilenameWinMM;
            }
        }

        /// Logs a warning event related to failure to import a particular function from the import library.
        /// @param [in] functionName Name of the function whose import attempt failed.
        static void LogImportFailed(LPCWSTR functionName)
        {
            Message::OutputFormatted(Message::ESeverity::Warning, L"Import library is missing WinMM function \"%s\". Attempts to call it will fail.", functionName);
        }

        /// Logs a debug event related to attempting to load the system-provided library for importing functions.
        /// @param [in] libraryPath Path of the library that was loaded.
        static void LogInitializeLibraryPath(LPCWSTR libraryPath)
        {
            Message::OutputFormatted(Message::ESeverity::Debug, L"Attempting to import WinMM functions from \"%s\".", libraryPath);
        }

        /// Logs an error event related to failure to initialize the import table because the import library could not be loaded.
        /// @param [in] libraryPath Path of the library that was loaded.
        static void LogInitializeFailed(void)
        {
            Message::Output(Message::ESeverity::Error, L"Failed to initialize imported WinMM functions.");
        }

        /// Logs an informational event related to successful initialization of the import table.
        static void LogInitializeSucceeded(void)
        {
            Message::OutputFormatted(Message::ESeverity::Info, L"Successfully initialized imported WinMM functions.");
        }

        /// Logs an error event related to a missing import function that has been invoked.
        /// @param [in] functionName Name of the function that was invoked.
        static void LogMissingFunctionCalled(LPCWSTR functionName)
        {
            Message::OutputFormatted(Message::ESeverity::Error, L"Application has attempted to call missing WinMM import function \"%s\".", functionName);
        }

        
        // -------- FUNCTIONS ---------------------------------------------- //
        // See "ImportApiWinMM.h" for documentation.

        void Initialize(void)
        {
            static std::once_flag initializeFlag;
            std::call_once(initializeFlag, []() {
                // Initialize the import table.
                ZeroMemory(&importTable, sizeof(importTable));

                // Obtain the full library path string.
                std::wstring_view libraryPath = GetImportLibraryPathWinMM();

                // Attempt to load the library.
                LogInitializeLibraryPath(libraryPath.data());
                HMODULE loadedLibrary = LoadLibraryEx(libraryPath.data(), nullptr, 0);
                if (nullptr == loadedLibrary)
                {
                    LogInitializeFailed();
                    return;
                }

                // Attempt to obtain the addresses of all imported API functions.
                FARPROC procAddress = nullptr;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "CloseDriver");
                if (nullptr == procAddress) LogImportFailed(L"CloseDriver");
                importTable.CloseDriver = (LRESULT(WINAPI*)(HDRVR, LPARAM, LPARAM))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "DefDriverProc");
                if (nullptr == procAddress) LogImportFailed(L"DefDriverProc");
                importTable.DefDriverProc = (LRESULT(WINAPI*)(DWORD_PTR, HDRVR, UINT, LONG, LONG))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "DriverCallback");
                if (nullptr == procAddress) LogImportFailed(L"DriverCallback");
                importTable.DriverCallback = (BOOL(WINAPI*)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "DrvGetModuleHandle");
                if (nullptr == procAddress) LogImportFailed(L"DrvGetModuleHandle");
                importTable.DrvGetModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "GetDriverModuleHandle");
                if (nullptr == procAddress) LogImportFailed(L"GetDriverModuleHandle");
                importTable.GetDriverModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "OpenDriver");
                if (nullptr == procAddress) LogImportFailed(L"OpenDriver");
                importTable.OpenDriver = (HDRVR(WINAPI*)(LPCWSTR, LPCWSTR, LPARAM))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "PlaySoundA");
                if (nullptr == procAddress) LogImportFailed(L"PlaySoundA");
                importTable.PlaySoundA = (BOOL(WINAPI*)(LPCSTR, HMODULE, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "PlaySoundW");
                if (nullptr == procAddress) LogImportFailed(L"PlaySoundW");
                importTable.PlaySoundW = (BOOL(WINAPI*)(LPCWSTR, HMODULE, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "SendDriverMessage");
                if (nullptr == procAddress) LogImportFailed(L"SendDriverMessage");
                importTable.SendDriverMessage = (LRESULT(WINAPI*)(HDRVR, UINT, LPARAM, LPARAM))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"auxGetDevCapsA");
                importTable.auxGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSA, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"auxGetDevCapsW");
                importTable.auxGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSW, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"auxGetNumDevs");
                importTable.auxGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
                if (nullptr == procAddress) LogImportFailed(L"auxGetVolume");
                importTable.auxGetVolume = (MMRESULT(WINAPI*)(UINT, LPDWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
                if (nullptr == procAddress) LogImportFailed(L"auxOutMessage");
                importTable.auxOutMessage = (MMRESULT(WINAPI*)(UINT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
                if (nullptr == procAddress) LogImportFailed(L"auxSetVolume");
                importTable.auxSetVolume = (MMRESULT(WINAPI*)(UINT, DWORD))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
                if (nullptr == procAddress) LogImportFailed(L"joyConfigChanged");
                importTable.joyConfigChanged = (MMRESULT(WINAPI*)(DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"joyGetDevCapsA");
                importTable.joyGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSA, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"joyGetDevCapsW");
                importTable.joyGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSW, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"joyGetNumDevs");
                importTable.joyGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
                if (nullptr == procAddress) LogImportFailed(L"joyGetPos");
                importTable.joyGetPos = (MMRESULT(WINAPI*)(UINT, LPJOYINFO))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
                if (nullptr == procAddress) LogImportFailed(L"joyGetPosEx");
                importTable.joyGetPosEx = (MMRESULT(WINAPI*)(UINT, LPJOYINFOEX))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
                if (nullptr == procAddress) LogImportFailed(L"joyGetThreshold");
                importTable.joyGetThreshold = (MMRESULT(WINAPI*)(UINT, LPUINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
                if (nullptr == procAddress) LogImportFailed(L"joyReleaseCapture");
                importTable.joyReleaseCapture = (MMRESULT(WINAPI*)(UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
                if (nullptr == procAddress) LogImportFailed(L"joySetCapture");
                importTable.joySetCapture = (MMRESULT(WINAPI*)(HWND, UINT, UINT, BOOL))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
                if (nullptr == procAddress) LogImportFailed(L"joySetThreshold");
                importTable.joySetThreshold = (MMRESULT(WINAPI*)(UINT, UINT))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "mciDriverNotify");
                if (nullptr == procAddress) LogImportFailed(L"mciDriverNotify");
                importTable.mciDriverNotify = (decltype(importTable.mciDriverNotify))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciDriverYield");
                if (nullptr == procAddress) LogImportFailed(L"mciDriverYield");
                importTable.mciDriverYield = (decltype(importTable.mciDriverYield))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciExecute");
                if (nullptr == procAddress) LogImportFailed(L"mciExecute");
                importTable.mciExecute = (decltype(importTable.mciExecute))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciFreeCommandResource");
                if (nullptr == procAddress) LogImportFailed(L"mciFreeCommandResource");
                importTable.mciFreeCommandResource = (decltype(importTable.mciFreeCommandResource))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetCreatorTask");
                if (nullptr == procAddress) LogImportFailed(L"mciGetCreatorTask");
                importTable.mciGetCreatorTask = (decltype(importTable.mciGetCreatorTask))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDA");
                if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDA");
                importTable.mciGetDeviceIDA = (decltype(importTable.mciGetDeviceIDA))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDW");
                if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDW");
                importTable.mciGetDeviceIDW = (decltype(importTable.mciGetDeviceIDW))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDA");
                if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDA");
                importTable.mciGetDeviceIDFromElementIDA = (decltype(importTable.mciGetDeviceIDFromElementIDA))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDW");
                if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDW");
                importTable.mciGetDeviceIDFromElementIDW = (decltype(importTable.mciGetDeviceIDFromElementIDW))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetDriverData");
                if (nullptr == procAddress) LogImportFailed(L"mciGetDriverData");
                importTable.mciGetDriverData = (decltype(importTable.mciGetDriverData))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringA");
                if (nullptr == procAddress) LogImportFailed(L"mciGetErrorStringA");
                importTable.mciGetErrorStringA = (decltype(importTable.mciGetErrorStringA))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringW");
                if (nullptr == procAddress) LogImportFailed(L"mciGetErrorStringW");
                importTable.mciGetErrorStringW = (decltype(importTable.mciGetErrorStringW))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciGetYieldProc");
                if (nullptr == procAddress) LogImportFailed(L"mciGetYieldProc");
                importTable.mciGetYieldProc = (decltype(importTable.mciGetYieldProc))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciLoadCommandResource");
                if (nullptr == procAddress) LogImportFailed(L"mciLoadCommandResource");
                importTable.mciLoadCommandResource = (decltype(importTable.mciLoadCommandResource))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciSendCommandA");
                if (nullptr == procAddress) LogImportFailed(L"mciSendCommandA");
                importTable.mciSendCommandA = (decltype(importTable.mciSendCommandA))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciSendCommandW");
                if (nullptr == procAddress) LogImportFailed(L"mciSendCommandW");
                importTable.mciSendCommandW = (decltype(importTable.mciSendCommandW))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciSendStringA");
                if (nullptr == procAddress) LogImportFailed(L"mciSendStringA");
                importTable.mciSendStringA = (decltype(importTable.mciSendStringA))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciSendStringW");
                if (nullptr == procAddress) LogImportFailed(L"mciSendStringW");
                importTable.mciSendStringW = (decltype(importTable.mciSendStringW))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciSetDriverData");
                if (nullptr == procAddress) LogImportFailed(L"mciSetDriverData");
                importTable.mciSetDriverData = (decltype(importTable.mciSetDriverData))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mciSetYieldProc");
                if (nullptr == procAddress) LogImportFailed(L"mciSetYieldProc");
                importTable.mciSetYieldProc = (decltype(importTable.mciSetYieldProc))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "midiConnect");
                if (nullptr == procAddress) LogImportFailed(L"midiConnect");
                importTable.midiConnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiDisconnect");
                if (nullptr == procAddress) LogImportFailed(L"midiDisconnect");
                importTable.midiDisconnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "midiInAddBuffer");
                if (nullptr == procAddress) LogImportFailed(L"midiInAddBuffer");
                importTable.midiInAddBuffer = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInClose");
                if (nullptr == procAddress) LogImportFailed(L"midiInClose");
                importTable.midiInClose = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"midiInGetDevCapsA");
                importTable.midiInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSA, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"midiInGetDevCapsW");
                importTable.midiInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSW, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextA");
                if (nullptr == procAddress) LogImportFailed(L"midiInGetErrorTextA");
                importTable.midiInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextW");
                if (nullptr == procAddress) LogImportFailed(L"midiInGetErrorTextW");
                importTable.midiInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInGetID");
                if (nullptr == procAddress) LogImportFailed(L"midiInGetID");
                importTable.midiInGetID = (MMRESULT(WINAPI*)(HMIDIIN, LPUINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"midiInGetNumDevs");
                importTable.midiInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInMessage");
                if (nullptr == procAddress) LogImportFailed(L"midiInMessage");
                importTable.midiInMessage = (DWORD(WINAPI*)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInOpen");
                if (nullptr == procAddress) LogImportFailed(L"midiInOpen");
                importTable.midiInOpen = (MMRESULT(WINAPI*)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInPrepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"midiInPrepareHeader");
                importTable.midiInPrepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInReset");
                if (nullptr == procAddress) LogImportFailed(L"midiInReset");
                importTable.midiInReset = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInStart");
                if (nullptr == procAddress) LogImportFailed(L"midiInStart");
                importTable.midiInStart = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInStop");
                if (nullptr == procAddress) LogImportFailed(L"midiInStop");
                importTable.midiInStop = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiInUnprepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"midiInUnprepareHeader");
                importTable.midiInUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "midiOutCacheDrumPatches");
                if (nullptr == procAddress) LogImportFailed(L"midiOutCacheDrumPatches");
                importTable.midiOutCacheDrumPatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutCachePatches");
                if (nullptr == procAddress) LogImportFailed(L"midiOutCachePatches");
                importTable.midiOutCachePatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutClose");
                if (nullptr == procAddress) LogImportFailed(L"midiOutClose");
                importTable.midiOutClose = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetDevCapsA");
                importTable.midiOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSA, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetDevCapsW");
                importTable.midiOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSW, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextA");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetErrorTextA");
                importTable.midiOutGetErrorTextA = (UINT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextW");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetErrorTextW");
                importTable.midiOutGetErrorTextW = (UINT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetID");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetID");
                importTable.midiOutGetID = (MMRESULT(WINAPI*)(HMIDIOUT, LPUINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetNumDevs");
                importTable.midiOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutGetVolume");
                if (nullptr == procAddress) LogImportFailed(L"midiOutGetVolume");
                importTable.midiOutGetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, LPDWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutLongMsg");
                if (nullptr == procAddress) LogImportFailed(L"midiOutLongMsg");
                importTable.midiOutLongMsg = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutMessage");
                if (nullptr == procAddress) LogImportFailed(L"midiOutMessage");
                importTable.midiOutMessage = (DWORD(WINAPI*)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutOpen");
                if (nullptr == procAddress) LogImportFailed(L"midiOutOpen");
                importTable.midiOutOpen = (MMRESULT(WINAPI*)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutPrepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"midiOutPrepareHeader");
                importTable.midiOutPrepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutReset");
                if (nullptr == procAddress) LogImportFailed(L"midiOutReset");
                importTable.midiOutReset = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutSetVolume");
                if (nullptr == procAddress) LogImportFailed(L"midiOutSetVolume");
                importTable.midiOutSetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutShortMsg");
                if (nullptr == procAddress) LogImportFailed(L"midiOutShortMsg");
                importTable.midiOutShortMsg = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiOutUnprepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"midiOutUnprepareHeader");
                importTable.midiOutUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "midiStreamClose");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamClose");
                importTable.midiStreamClose = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamOpen");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamOpen");
                importTable.midiStreamOpen = (MMRESULT(WINAPI*)(LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamOut");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamOut");
                importTable.midiStreamOut = (MMRESULT(WINAPI*)(HMIDISTRM, LPMIDIHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamPause");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamPause");
                importTable.midiStreamPause = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamPosition");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamPosition");
                importTable.midiStreamPosition = (MMRESULT(WINAPI*)(HMIDISTRM, LPMMTIME, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamProperty");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamProperty");
                importTable.midiStreamProperty = (MMRESULT(WINAPI*)(HMIDISTRM, LPBYTE, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamRestart");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamRestart");
                importTable.midiStreamRestart = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "midiStreamStop");
                if (nullptr == procAddress) LogImportFailed(L"midiStreamStop");
                importTable.midiStreamStop = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "mixerClose");
                if (nullptr == procAddress) LogImportFailed(L"mixerClose");
                importTable.mixerClose = (MMRESULT(WINAPI*)(HMIXER))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsA");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetControlDetailsA");
                importTable.mixerGetControlDetailsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsW");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetControlDetailsW");
                importTable.mixerGetControlDetailsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetDevCapsA");
                importTable.mixerGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetDevCapsW");
                importTable.mixerGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetID");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetID");
                importTable.mixerGetID = (MMRESULT(WINAPI*)(HMIXEROBJ, UINT*, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsA");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetLineControlsA");
                importTable.mixerGetLineControlsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsW");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetLineControlsW");
                importTable.mixerGetLineControlsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoA");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetLineInfoA");
                importTable.mixerGetLineInfoA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoW");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetLineInfoW");
                importTable.mixerGetLineInfoW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"mixerGetNumDevs");
                importTable.mixerGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerMessage");
                if (nullptr == procAddress) LogImportFailed(L"mixerMessage");
                importTable.mixerMessage = (DWORD(WINAPI*)(HMIXER, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerOpen");
                if (nullptr == procAddress) LogImportFailed(L"mixerOpen");
                importTable.mixerOpen = (MMRESULT(WINAPI*)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mixerSetControlDetails");
                if (nullptr == procAddress) LogImportFailed(L"mixerSetControlDetails");
                importTable.mixerSetControlDetails = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "mmioAdvance");
                if (nullptr == procAddress) LogImportFailed(L"mmioAdvance");
                importTable.mmioAdvance = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioAscend");
                if (nullptr == procAddress) LogImportFailed(L"mmioAscend");
                importTable.mmioAscend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioClose");
                if (nullptr == procAddress) LogImportFailed(L"mmioClose");
                importTable.mmioClose = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioCreateChunk");
                if (nullptr == procAddress) LogImportFailed(L"mmioCreateChunk");
                importTable.mmioCreateChunk = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioDescend");
                if (nullptr == procAddress) LogImportFailed(L"mmioDescend");
                importTable.mmioDescend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioFlush");
                if (nullptr == procAddress) LogImportFailed(L"mmioFlush");
                importTable.mmioFlush = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioGetInfo");
                if (nullptr == procAddress) LogImportFailed(L"mmioGetInfo");
                importTable.mmioGetInfo = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcA");
                if (nullptr == procAddress) LogImportFailed(L"mmioInstallIOProcA");
                importTable.mmioInstallIOProcA = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcW");
                if (nullptr == procAddress) LogImportFailed(L"mmioInstallIOProcW");
                importTable.mmioInstallIOProcW = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioOpenA");
                if (nullptr == procAddress) LogImportFailed(L"mmioOpenA");
                importTable.mmioOpenA = (HMMIO(WINAPI*)(LPSTR, LPMMIOINFO, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioOpenW");
                if (nullptr == procAddress) LogImportFailed(L"mmioOpenW");
                importTable.mmioOpenW = (HMMIO(WINAPI*)(LPWSTR, LPMMIOINFO, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioRead");
                if (nullptr == procAddress) LogImportFailed(L"mmioRead");
                importTable.mmioRead = (LONG(WINAPI*)(HMMIO, HPSTR, LONG))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioRenameA");
                if (nullptr == procAddress) LogImportFailed(L"mmioRenameA");
                importTable.mmioRenameA = (MMRESULT(WINAPI*)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioRenameW");
                if (nullptr == procAddress) LogImportFailed(L"mmioRenameW");
                importTable.mmioRenameW = (MMRESULT(WINAPI*)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioSeek");
                if (nullptr == procAddress) LogImportFailed(L"mmioSeek");
                importTable.mmioSeek = (LONG(WINAPI*)(HMMIO, LONG, int))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioSendMessage");
                if (nullptr == procAddress) LogImportFailed(L"mmioSendMessage");
                importTable.mmioSendMessage = (LRESULT(WINAPI*)(HMMIO, UINT, LPARAM, LPARAM))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioSetBuffer");
                if (nullptr == procAddress) LogImportFailed(L"mmioSetBuffer");
                importTable.mmioSetBuffer = (MMRESULT(WINAPI*)(HMMIO, LPSTR, LONG, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioSetInfo");
                if (nullptr == procAddress) LogImportFailed(L"mmioSetInfo");
                importTable.mmioSetInfo = (MMRESULT(WINAPI*)(HMMIO, LPCMMIOINFO, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCA");
                if (nullptr == procAddress) LogImportFailed(L"mmioStringToFOURCCA");
                importTable.mmioStringToFOURCCA = (FOURCC(WINAPI*)(LPCSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCW");
                if (nullptr == procAddress) LogImportFailed(L"mmioStringToFOURCCW");
                importTable.mmioStringToFOURCCW = (FOURCC(WINAPI*)(LPCWSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "mmioWrite");
                if (nullptr == procAddress) LogImportFailed(L"mmioWrite");
                importTable.mmioWrite = (LONG(WINAPI*)(HMMIO, const char*, LONG))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundA");
                if (nullptr == procAddress) LogImportFailed(L"sndPlaySoundA");
                importTable.sndPlaySoundA = (BOOL(WINAPI*)(LPCSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundW");
                if (nullptr == procAddress) LogImportFailed(L"sndPlaySoundW");
                importTable.sndPlaySoundW = (BOOL(WINAPI*)(LPCWSTR, UINT))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
                if (nullptr == procAddress) LogImportFailed(L"timeBeginPeriod");
                importTable.timeBeginPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
                if (nullptr == procAddress) LogImportFailed(L"timeEndPeriod");
                importTable.timeEndPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
                if (nullptr == procAddress) LogImportFailed(L"timeGetDevCaps");
                importTable.timeGetDevCaps = (MMRESULT(WINAPI*)(LPTIMECAPS, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
                if (nullptr == procAddress) LogImportFailed(L"timeGetSystemTime");
                importTable.timeGetSystemTime = (MMRESULT(WINAPI*)(LPMMTIME, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
                if (nullptr == procAddress) LogImportFailed(L"timeGetTime");
                importTable.timeGetTime = (DWORD(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
                if (nullptr == procAddress) LogImportFailed(L"timeKillEvent");
                importTable.timeKillEvent = (MMRESULT(WINAPI*)(UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
                if (nullptr == procAddress) LogImportFailed(L"timeSetEvent");
                importTable.timeSetEvent = (MMRESULT(WINAPI*)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "waveInAddBuffer");
                if (nullptr == procAddress) LogImportFailed(L"waveInAddBuffer");
                importTable.waveInAddBuffer = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInClose");
                if (nullptr == procAddress) LogImportFailed(L"waveInClose");
                importTable.waveInClose = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetDevCapsA");
                importTable.waveInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSA, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetDevCapsW");
                importTable.waveInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSW, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextA");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetErrorTextA");
                importTable.waveInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextW");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetErrorTextW");
                importTable.waveInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetID");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetID");
                importTable.waveInGetID = (MMRESULT(WINAPI*)(HWAVEIN, LPUINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetNumDevs");
                importTable.waveInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInGetPosition");
                if (nullptr == procAddress) LogImportFailed(L"waveInGetPosition");
                importTable.waveInGetPosition = (MMRESULT(WINAPI*)(HWAVEIN, LPMMTIME, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInMessage");
                if (nullptr == procAddress) LogImportFailed(L"waveInMessage");
                importTable.waveInMessage = (DWORD(WINAPI*)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInOpen");
                if (nullptr == procAddress) LogImportFailed(L"waveInOpen");
                importTable.waveInOpen = (MMRESULT(WINAPI*)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInPrepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"waveInPrepareHeader");
                importTable.waveInPrepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInReset");
                if (nullptr == procAddress) LogImportFailed(L"waveInReset");
                importTable.waveInReset = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInStart");
                if (nullptr == procAddress) LogImportFailed(L"waveInStart");
                importTable.waveInStart = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInStop");
                if (nullptr == procAddress) LogImportFailed(L"waveInStop");
                importTable.waveInStop = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveInUnprepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"waveInUnprepareHeader");
                importTable.waveInUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

                // ---------

                procAddress = GetProcAddress(loadedLibrary, "waveOutBreakLoop");
                if (nullptr == procAddress) LogImportFailed(L"waveOutBreakLoop");
                importTable.waveOutBreakLoop = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutClose");
                if (nullptr == procAddress) LogImportFailed(L"waveOutClose");
                importTable.waveOutClose = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsA");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetDevCapsA");
                importTable.waveOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSA, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsW");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetDevCapsW");
                importTable.waveOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSW, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextA");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetErrorTextA");
                importTable.waveOutGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextW");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetErrorTextW");
                importTable.waveOutGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetID");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetID");
                importTable.waveOutGetID = (MMRESULT(WINAPI*)(HWAVEOUT, LPUINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetNumDevs");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetNumDevs");
                importTable.waveOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetPitch");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetPitch");
                importTable.waveOutGetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetPlaybackRate");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetPlaybackRate");
                importTable.waveOutGetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetPosition");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetPosition");
                importTable.waveOutGetPosition = (MMRESULT(WINAPI*)(HWAVEOUT, LPMMTIME, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutGetVolume");
                if (nullptr == procAddress) LogImportFailed(L"waveOutGetVolume");
                importTable.waveOutGetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutMessage");
                if (nullptr == procAddress) LogImportFailed(L"waveOutMessage");
                importTable.waveOutMessage = (DWORD(WINAPI*)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutOpen");
                if (nullptr == procAddress) LogImportFailed(L"waveOutOpen");
                importTable.waveOutOpen = (MMRESULT(WINAPI*)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutPause");
                if (nullptr == procAddress) LogImportFailed(L"waveOutPause");
                importTable.waveOutPause = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutPrepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"waveOutPrepareHeader");
                importTable.waveOutPrepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutReset");
                if (nullptr == procAddress) LogImportFailed(L"waveOutReset");
                importTable.waveOutReset = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutRestart");
                if (nullptr == procAddress) LogImportFailed(L"waveOutRestart");
                importTable.waveOutRestart = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutSetPitch");
                if (nullptr == procAddress) LogImportFailed(L"waveOutSetPitch");
                importTable.waveOutSetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutSetPlaybackRate");
                if (nullptr == procAddress) LogImportFailed(L"waveOutSetPlaybackRate");
                importTable.waveOutSetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutSetVolume");
                if (nullptr == procAddress) LogImportFailed(L"waveOutSetVolume");
                importTable.waveOutSetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutUnprepareHeader");
                if (nullptr == procAddress) LogImportFailed(L"waveOutUnprepareHeader");
                importTable.waveOutUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

                procAddress = GetProcAddress(loadedLibrary, "waveOutWrite");
                if (nullptr == procAddress) LogImportFailed(L"waveOutWrite");
                importTable.waveOutWrite = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

                // Initialization complete.
                LogInitializeSucceeded();
            });
        }

        // ---------

        LRESULT CloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2)
        {
            Initialize();

            if (nullptr == importTable.CloseDriver)
                LogMissingFunctionCalled(L"CloseDriver");

            return importTable.CloseDriver(hdrvr, lParam1, lParam2);
        }

        // ---------

        LRESULT DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
        {
            Initialize();

            if (nullptr == importTable.DefDriverProc)
                LogMissingFunctionCalled(L"DefDriverProc");

            return importTable.DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
        }

        // ---------

        BOOL DriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
        {
            Initialize();

            if (nullptr == importTable.DriverCallback)
                LogMissingFunctionCalled(L"DriverCallback");

            return importTable.DriverCallback(dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
        }

        // ---------

        HMODULE DrvGetModuleHandle(HDRVR hDriver)
        {
            Initialize();

            if (nullptr == importTable.DrvGetModuleHandle)
                LogMissingFunctionCalled(L"DrvGetModuleHandle");

            return importTable.DrvGetModuleHandle(hDriver);
        }

        // ---------

        HMODULE GetDriverModuleHandle(HDRVR hdrvr)
        {
            Initialize();

            if (nullptr == importTable.GetDriverModuleHandle)
                LogMissingFunctionCalled(L"GetDriverModuleHandle");

            return importTable.GetDriverModuleHandle(hdrvr);
        }

        // ---------

        HDRVR OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
        {
            Initialize();

            if (nullptr == importTable.OpenDriver)
                LogMissingFunctionCalled(L"OpenDriver");

            return importTable.OpenDriver(lpDriverName, lpSectionName, lParam);
        }

        // ---------

        BOOL PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
        {
            Initialize();

            if (nullptr == importTable.PlaySoundA)
                LogMissingFunctionCalled(L"PlaySoundA");

            return importTable.PlaySoundA(pszSound, hmod, fdwSound);
        }

        // ---------

        BOOL PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
        {
            Initialize();

            if (nullptr == importTable.PlaySoundW)
                LogMissingFunctionCalled(L"PlaySoundW");

            return importTable.PlaySoundW(pszSound, hmod, fdwSound);
        }

        // ---------

        LRESULT SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
        {
            Initialize();

            if (nullptr == importTable.SendDriverMessage)
                LogMissingFunctionCalled(L"SendDriverMessage");

            return importTable.SendDriverMessage(hdrvr, msg, lParam1, lParam2);
        }

        // ---------

        MMRESULT auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
        {
            Initialize();

            if (nullptr == importTable.auxGetDevCapsA)
                LogMissingFunctionCalled(L"auxGetDevCapsA");

            return importTable.auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
        }

        // ---------

        MMRESULT auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
        {
            Initialize();

            if (nullptr == importTable.auxGetDevCapsW)
                LogMissingFunctionCalled(L"auxGetDevCapsW");

            return importTable.auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
        }

        // ---------

        UINT auxGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.auxGetNumDevs)
                LogMissingFunctionCalled(L"auxGetNumDevs");

            return importTable.auxGetNumDevs();
        }

        // ---------

        MMRESULT auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
        {
            Initialize();

            if (nullptr == importTable.auxGetVolume)
                LogMissingFunctionCalled(L"auxGetVolume");

            return importTable.auxGetVolume(uDeviceID, lpdwVolume);
        }

        // ---------

        MMRESULT auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.auxOutMessage)
                LogMissingFunctionCalled(L"auxOutMessage");

            return importTable.auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT auxSetVolume(UINT uDeviceID, DWORD dwVolume)
        {
            Initialize();

            if (nullptr == importTable.auxSetVolume)
                LogMissingFunctionCalled(L"auxSetVolume");

            return importTable.auxSetVolume(uDeviceID, dwVolume);
        }

        // ---------

        MMRESULT joyConfigChanged(DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.joyConfigChanged)
                LogMissingFunctionCalled(L"joyConfigChanged");

            return importTable.joyConfigChanged(dwFlags);
        }

        // ---------

        MMRESULT joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
        {
            Initialize();

            if (nullptr == importTable.joyGetDevCapsA)
                LogMissingFunctionCalled(L"joyGetDevCapsA");

            return importTable.joyGetDevCapsA(uJoyID, pjc, cbjc);
        }

        // ---------

        MMRESULT joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
        {
            Initialize();

            if (nullptr == importTable.joyGetDevCapsW)
                LogMissingFunctionCalled(L"joyGetDevCapsW");

            return importTable.joyGetDevCapsW(uJoyID, pjc, cbjc);
        }

        // ---------

        UINT joyGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.joyGetNumDevs)
                LogMissingFunctionCalled(L"joyGetNumDevs");

            return importTable.joyGetNumDevs();
        }

        // ---------

        MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji)
        {
            Initialize();

            if (nullptr == importTable.joyGetPos)
                LogMissingFunctionCalled(L"joyGetPos");

            return importTable.joyGetPos(uJoyID, pji);
        }

        // ---------

        MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
        {
            Initialize();

            if (nullptr == importTable.joyGetPosEx)
                LogMissingFunctionCalled(L"joyGetPosEx");

            return importTable.joyGetPosEx(uJoyID, pji);
        }

        // ---------

        MMRESULT joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
        {
            Initialize();

            if (nullptr == importTable.joyGetThreshold)
                LogMissingFunctionCalled(L"joyGetThreshold");

            return importTable.joyGetThreshold(uJoyID, puThreshold);
        }

        // ---------

        MMRESULT joyReleaseCapture(UINT uJoyID)
        {
            Initialize();

            if (nullptr == importTable.joyReleaseCapture)
                LogMissingFunctionCalled(L"joyReleaseCapture");

            return importTable.joyReleaseCapture(uJoyID);
        }

        // ---------

        MMRESULT joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
        {
            Initialize();

            if (nullptr == importTable.joySetCapture)
                LogMissingFunctionCalled(L"joySetCapture");

            return importTable.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
        }

        // ---------

        MMRESULT joySetThreshold(UINT uJoyID, UINT uThreshold)
        {
            Initialize();

            if (nullptr == importTable.joySetThreshold)
                LogMissingFunctionCalled(L"joySetThreshold");

            return importTable.joySetThreshold(uJoyID, uThreshold);
        }

        // ---------

        BOOL mciDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
        {
            Initialize();

            if (nullptr == importTable.mciDriverNotify)
                LogMissingFunctionCalled(L"mciDriverNotify");

            return importTable.mciDriverNotify(hwndCallback, IDDevice, uStatus);
        }

        // ---------

        UINT mciDriverYield(MCIDEVICEID IDDevice)
        {
            Initialize();

            if (nullptr == importTable.mciDriverYield)
                LogMissingFunctionCalled(L"mciDriverYield");

            return importTable.mciDriverYield(IDDevice);
        }

        // ---------

        BOOL mciExecute(LPCSTR pszCommand)
        {
            Initialize();

            if (nullptr == importTable.mciExecute)
                LogMissingFunctionCalled(L"mciExecute");

            return importTable.mciExecute(pszCommand);
        }

        // ---------

        BOOL mciFreeCommandResource(UINT uResource)
        {
            Initialize();

            if (nullptr == importTable.mciFreeCommandResource)
                LogMissingFunctionCalled(L"mciFreeCommandResource");

            return importTable.mciFreeCommandResource(uResource);
        }

        // ---------

        HANDLE mciGetCreatorTask(MCIDEVICEID IDDevice)
        {
            Initialize();

            if (nullptr == importTable.mciGetCreatorTask)
                LogMissingFunctionCalled(L"mciGetCreatorTask");

            return importTable.mciGetCreatorTask(IDDevice);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDA(LPCSTR lpszDevice)
        {
            Initialize();

            if (nullptr == importTable.mciGetDeviceIDA)
                LogMissingFunctionCalled(L"mciGetDeviceIDA");

            return importTable.mciGetDeviceIDA(lpszDevice);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDW(LPCWSTR lpszDevice)
        {
            Initialize();

            if (nullptr == importTable.mciGetDeviceIDW)
                LogMissingFunctionCalled(L"mciGetDeviceIDW");

            return importTable.mciGetDeviceIDW(lpszDevice);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType)
        {
            Initialize();

            if (nullptr == importTable.mciGetDeviceIDFromElementIDA)
                LogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDA");

            return importTable.mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType)
        {
            Initialize();

            if (nullptr == importTable.mciGetDeviceIDFromElementIDW)
                LogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDW");

            return importTable.mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
        }

        // ---------

        DWORD_PTR mciGetDriverData(MCIDEVICEID IDDevice)
        {
            Initialize();

            if (nullptr == importTable.mciGetDriverData)
                LogMissingFunctionCalled(L"mciGetDriverData");

            return importTable.mciGetDriverData(IDDevice);
        }

        // ---------

        BOOL mciGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
        {
            Initialize();

            if (nullptr == importTable.mciGetErrorStringA)
                LogMissingFunctionCalled(L"mciGetErrorStringA");

            return importTable.mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
        }

        // ---------

        BOOL mciGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
        {
            Initialize();

            if (nullptr == importTable.mciGetErrorStringW)
                LogMissingFunctionCalled(L"mciGetErrorStringW");

            return importTable.mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
        }

        // ---------

        YIELDPROC mciGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
        {
            Initialize();

            if (nullptr == importTable.mciGetYieldProc)
                LogMissingFunctionCalled(L"mciGetYieldProc");

            return importTable.mciGetYieldProc(IDDevice, lpdwYieldData);
        }

        // ---------

        UINT mciLoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
        {
            Initialize();

            if (nullptr == importTable.mciLoadCommandResource)
                LogMissingFunctionCalled(L"mciLoadCommandResource");

            return importTable.mciLoadCommandResource(hInst, lpwstrResourceName, uType);
        }

        // ---------

        MCIERROR mciSendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
        {
            Initialize();

            if (nullptr == importTable.mciSendCommandA)
                LogMissingFunctionCalled(L"mciSendCommandA");

            return importTable.mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
        }

        // ---------

        MCIERROR mciSendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
        {
            Initialize();

            if (nullptr == importTable.mciSendCommandW)
                LogMissingFunctionCalled(L"mciSendCommandW");

            return importTable.mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
        }

        // ---------

        MCIERROR mciSendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
        {
            Initialize();

            if (nullptr == importTable.mciSendStringA)
                LogMissingFunctionCalled(L"mciSendStringA");

            return importTable.mciSendStringA(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
        }

        // ---------

        MCIERROR mciSendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
        {
            Initialize();

            if (nullptr == importTable.mciSendStringW)
                LogMissingFunctionCalled(L"mciSendStringW");

            return importTable.mciSendStringW(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
        }

        // ---------

        BOOL mciSetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
        {
            Initialize();

            if (nullptr == importTable.mciSetDriverData)
                LogMissingFunctionCalled(L"mciSetDriverData");

            return importTable.mciSetDriverData(IDDevice, data);
        }

        // ---------

        UINT mciSetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
        {
            Initialize();

            if (nullptr == importTable.mciSetYieldProc)
                LogMissingFunctionCalled(L"mciSetYieldProc");

            return importTable.mciSetYieldProc(IDDevice, yp, dwYieldData);
        }

        // ---------

        MMRESULT midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
        {
            Initialize();

            if (nullptr == importTable.midiConnect)
                LogMissingFunctionCalled(L"midiConnect");

            return importTable.midiConnect(hMidi, hmo, pReserved);
        }

        // ---------

        MMRESULT midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
        {
            Initialize();

            if (nullptr == importTable.midiDisconnect)
                LogMissingFunctionCalled(L"midiDisconnect");

            return importTable.midiDisconnect(hMidi, hmo, pReserved);
        }

        // ---------

        MMRESULT midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
        {
            Initialize();

            if (nullptr == importTable.midiInAddBuffer)
                LogMissingFunctionCalled(L"midiInAddBuffer");

            return importTable.midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
        }

        // ---------

        MMRESULT midiInClose(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.midiInClose)
                LogMissingFunctionCalled(L"midiInClose");

            return importTable.midiInClose(hMidiIn);
        }

        // ---------

        MMRESULT midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
        {
            Initialize();

            if (nullptr == importTable.midiInGetDevCapsA)
                LogMissingFunctionCalled(L"midiInGetDevCapsA");

            return importTable.midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
        }

        // ---------

        MMRESULT midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
        {
            Initialize();

            if (nullptr == importTable.midiInGetDevCapsW)
                LogMissingFunctionCalled(L"midiInGetDevCapsW");

            return importTable.midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
        }

        // ---------

        MMRESULT midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.midiInGetErrorTextA)
                LogMissingFunctionCalled(L"midiInGetErrorTextA");

            return importTable.midiInGetErrorTextA(wError, lpText, cchText);
        }

        // ---------

        MMRESULT midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.midiInGetErrorTextW)
                LogMissingFunctionCalled(L"midiInGetErrorTextW");

            return importTable.midiInGetErrorTextW(wError, lpText, cchText);
        }

        // ---------

        MMRESULT midiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.midiInGetID)
                LogMissingFunctionCalled(L"midiInGetID");

            return importTable.midiInGetID(hmi, puDeviceID);
        }

        // ---------

        UINT midiInGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.midiInGetNumDevs)
                LogMissingFunctionCalled(L"midiInGetNumDevs");

            return importTable.midiInGetNumDevs();
        }

        // ---------

        DWORD midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
        {
            Initialize();

            if (nullptr == importTable.midiInMessage)
                LogMissingFunctionCalled(L"midiInMessage");

            return importTable.midiInMessage(deviceID, msg, dw1, dw2);
        }

        // ---------

        MMRESULT midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.midiInOpen)
                LogMissingFunctionCalled(L"midiInOpen");

            return importTable.midiInOpen(lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
        }

        // ---------

        MMRESULT midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
        {
            Initialize();

            if (nullptr == importTable.midiInPrepareHeader)
                LogMissingFunctionCalled(L"midiInPrepareHeader");

            return importTable.midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
        }

        // ---------

        MMRESULT midiInReset(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.midiInReset)
                LogMissingFunctionCalled(L"midiInReset");

            return importTable.midiInReset(hMidiIn);
        }

        // ---------

        MMRESULT midiInStart(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.midiInStart)
                LogMissingFunctionCalled(L"midiInStart");

            return importTable.midiInStart(hMidiIn);
        }

        // ---------

        MMRESULT midiInStop(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.midiInStop)
                LogMissingFunctionCalled(L"midiInStop");

            return importTable.midiInStop(hMidiIn);
        }

        // ---------

        MMRESULT midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
        {
            Initialize();

            if (nullptr == importTable.midiInUnprepareHeader)
                LogMissingFunctionCalled(L"midiInUnprepareHeader");

            return importTable.midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
        }

        // ---------

        MMRESULT midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.midiOutCacheDrumPatches)
                LogMissingFunctionCalled(L"midiOutCacheDrumPatches");

            return importTable.midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
        }

        // ---------

        MMRESULT midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.midiOutCachePatches)
                LogMissingFunctionCalled(L"midiOutCachePatches");

            return importTable.midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
        }

        // ---------

        MMRESULT midiOutClose(HMIDIOUT hmo)
        {
            Initialize();

            if (nullptr == importTable.midiOutClose)
                LogMissingFunctionCalled(L"midiOutClose");

            return importTable.midiOutClose(hmo);
        }

        // ---------

        MMRESULT midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetDevCapsA)
                LogMissingFunctionCalled(L"midiOutGetDevCapsA");

            return importTable.midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
        }

        // ---------

        MMRESULT midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetDevCapsW)
                LogMissingFunctionCalled(L"midiOutGetDevCapsW");

            return importTable.midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
        }

        // ---------

        UINT midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetErrorTextA)
                LogMissingFunctionCalled(L"midiOutGetErrorTextA");

            return importTable.midiOutGetErrorTextA(mmrError, lpText, cchText);
        }

        // ---------

        UINT midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetErrorTextW)
                LogMissingFunctionCalled(L"midiOutGetErrorTextW");

            return importTable.midiOutGetErrorTextW(mmrError, lpText, cchText);
        }

        // ---------

        MMRESULT midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetID)
                LogMissingFunctionCalled(L"midiOutGetID");

            return importTable.midiOutGetID(hmo, puDeviceID);
        }

        // ---------

        UINT midiOutGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetNumDevs)
                LogMissingFunctionCalled(L"midiOutGetNumDevs");

            return importTable.midiOutGetNumDevs();
        }

        // ---------

        MMRESULT midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
        {
            Initialize();

            if (nullptr == importTable.midiOutGetVolume)
                LogMissingFunctionCalled(L"midiOutGetVolume");

            return importTable.midiOutGetVolume(hmo, lpdwVolume);
        }

        // ---------

        MMRESULT midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
        {
            Initialize();

            if (nullptr == importTable.midiOutLongMsg)
                LogMissingFunctionCalled(L"midiOutLongMsg");

            return importTable.midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
        }

        // ---------

        DWORD midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
        {
            Initialize();

            if (nullptr == importTable.midiOutMessage)
                LogMissingFunctionCalled(L"midiOutMessage");

            return importTable.midiOutMessage(deviceID, msg, dw1, dw2);
        }

        // ---------

        MMRESULT midiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.midiOutOpen)
                LogMissingFunctionCalled(L"midiOutOpen");

            return importTable.midiOutOpen(lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
        }

        // ---------

        MMRESULT midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
        {
            Initialize();

            if (nullptr == importTable.midiOutPrepareHeader)
                LogMissingFunctionCalled(L"midiOutPrepareHeader");

            return importTable.midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
        }

        // ---------

        MMRESULT midiOutReset(HMIDIOUT hmo)
        {
            Initialize();

            if (nullptr == importTable.midiOutReset)
                LogMissingFunctionCalled(L"midiOutReset");

            return importTable.midiOutReset(hmo);
        }

        // ---------

        MMRESULT midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
        {
            Initialize();

            if (nullptr == importTable.midiOutSetVolume)
                LogMissingFunctionCalled(L"midiOutSetVolume");

            return importTable.midiOutSetVolume(hmo, dwVolume);
        }

        // ---------

        MMRESULT midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
        {
            Initialize();

            if (nullptr == importTable.midiOutShortMsg)
                LogMissingFunctionCalled(L"midiOutShortMsg");

            return importTable.midiOutShortMsg(hmo, dwMsg);
        }

        // ---------

        MMRESULT midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
        {
            Initialize();

            if (nullptr == importTable.midiOutUnprepareHeader)
                LogMissingFunctionCalled(L"midiOutUnprepareHeader");

            return importTable.midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
        }

        // ---------

        MMRESULT midiStreamClose(HMIDISTRM hStream)
        {
            Initialize();

            if (nullptr == importTable.midiStreamClose)
                LogMissingFunctionCalled(L"midiStreamClose");

            return importTable.midiStreamClose(hStream);
        }

        // ---------

        MMRESULT midiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.midiStreamOpen)
                LogMissingFunctionCalled(L"midiStreamOpen");

            return importTable.midiStreamOpen(lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
        }

        // ---------

        MMRESULT midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
        {
            Initialize();

            if (nullptr == importTable.midiStreamOut)
                LogMissingFunctionCalled(L"midiStreamOut");

            return importTable.midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
        }

        // ---------

        MMRESULT midiStreamPause(HMIDISTRM hms)
        {
            Initialize();

            if (nullptr == importTable.midiStreamPause)
                LogMissingFunctionCalled(L"midiStreamPause");

            return importTable.midiStreamPause(hms);
        }

        // ---------

        MMRESULT midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.midiStreamPosition)
                LogMissingFunctionCalled(L"midiStreamPosition");

            return importTable.midiStreamPosition(hms, pmmt, cbmmt);
        }

        // ---------

        MMRESULT midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
        {
            Initialize();

            if (nullptr == importTable.midiStreamProperty)
                LogMissingFunctionCalled(L"midiStreamProperty");

            return importTable.midiStreamProperty(hm, lppropdata, dwProperty);
        }

        // ---------

        MMRESULT midiStreamRestart(HMIDISTRM hms)
        {
            Initialize();

            if (nullptr == importTable.midiStreamRestart)
                LogMissingFunctionCalled(L"midiStreamRestart");

            return importTable.midiStreamRestart(hms);
        }

        // ---------

        MMRESULT midiStreamStop(HMIDISTRM hms)
        {
            Initialize();

            if (nullptr == importTable.midiStreamStop)
                LogMissingFunctionCalled(L"midiStreamStop");

            return importTable.midiStreamStop(hms);
        }

        // ---------

        MMRESULT mixerClose(HMIXER hmx)
        {
            Initialize();

            if (nullptr == importTable.mixerClose)
                LogMissingFunctionCalled(L"mixerClose");

            return importTable.mixerClose(hmx);
        }

        // ---------

        MMRESULT mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
        {
            Initialize();

            if (nullptr == importTable.mixerGetControlDetailsA)
                LogMissingFunctionCalled(L"mixerGetControlDetailsA");

            return importTable.mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
        }

        // ---------

        MMRESULT mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
        {
            Initialize();

            if (nullptr == importTable.mixerGetControlDetailsW)
                LogMissingFunctionCalled(L"mixerGetControlDetailsW");

            return importTable.mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
        }

        // ---------

        MMRESULT mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
        {
            Initialize();

            if (nullptr == importTable.mixerGetDevCapsA)
                LogMissingFunctionCalled(L"mixerGetDevCapsA");

            return importTable.mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
        }

        // ---------

        MMRESULT mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
        {
            Initialize();

            if (nullptr == importTable.mixerGetDevCapsW)
                LogMissingFunctionCalled(L"mixerGetDevCapsW");

            return importTable.mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
        }

        // ---------

        MMRESULT mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
        {
            Initialize();

            if (nullptr == importTable.mixerGetID)
                LogMissingFunctionCalled(L"mixerGetID");

            return importTable.mixerGetID(hmxobj, puMxId, fdwId);
        }

        // ---------

        MMRESULT mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
        {
            Initialize();

            if (nullptr == importTable.mixerGetLineControlsA)
                LogMissingFunctionCalled(L"mixerGetLineControlsA");

            return importTable.mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
        }

        // ---------

        MMRESULT mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
        {
            Initialize();

            if (nullptr == importTable.mixerGetLineControlsW)
                LogMissingFunctionCalled(L"mixerGetLineControlsW");

            return importTable.mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
        }

        // ---------

        MMRESULT mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
        {
            Initialize();

            if (nullptr == importTable.mixerGetLineInfoA)
                LogMissingFunctionCalled(L"mixerGetLineInfoA");

            return importTable.mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
        }

        // ---------

        MMRESULT mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
        {
            Initialize();

            if (nullptr == importTable.mixerGetLineInfoW)
                LogMissingFunctionCalled(L"mixerGetLineInfoW");

            return importTable.mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
        }

        // ---------

        UINT mixerGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.mixerGetNumDevs)
                LogMissingFunctionCalled(L"mixerGetNumDevs");

            return importTable.mixerGetNumDevs();
        }

        // ---------

        DWORD mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.mixerMessage)
                LogMissingFunctionCalled(L"mixerMessage");

            return importTable.mixerMessage(driverID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.mixerOpen)
                LogMissingFunctionCalled(L"mixerOpen");

            return importTable.mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
        }

        // ---------

        MMRESULT mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
        {
            Initialize();

            if (nullptr == importTable.mixerSetControlDetails)
                LogMissingFunctionCalled(L"mixerSetControlDetails");

            return importTable.mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
        }

        // ---------

        MMRESULT mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioAdvance)
                LogMissingFunctionCalled(L"mmioAdvance");

            return importTable.mmioAdvance(hmmio, lpmmioinfo, wFlags);
        }

        // ---------


        MMRESULT mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioAscend)
                LogMissingFunctionCalled(L"mmioAscend");

            return importTable.mmioAscend(hmmio, lpck, wFlags);
        }

        // ---------


        MMRESULT mmioClose(HMMIO hmmio, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioClose)
                LogMissingFunctionCalled(L"mmioClose");

            return importTable.mmioClose(hmmio, wFlags);
        }

        // ---------


        MMRESULT mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioCreateChunk)
                LogMissingFunctionCalled(L"mmioCreateChunk");

            return importTable.mmioCreateChunk(hmmio, lpck, wFlags);
        }

        // ---------


        MMRESULT mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioDescend)
                LogMissingFunctionCalled(L"mmioDescend");

            return importTable.mmioDescend(hmmio, lpck, lpckParent, wFlags);
        }

        // ---------


        MMRESULT mmioFlush(HMMIO hmmio, UINT fuFlush)
        {
            Initialize();

            if (nullptr == importTable.mmioFlush)
                LogMissingFunctionCalled(L"mmioFlush");

            return importTable.mmioFlush(hmmio, fuFlush);
        }

        // ---------


        MMRESULT mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioGetInfo)
                LogMissingFunctionCalled(L"mmioGetInfo");

            return importTable.mmioGetInfo(hmmio, lpmmioinfo, wFlags);
        }

        // ---------


        LPMMIOPROC mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioInstallIOProcA)
                LogMissingFunctionCalled(L"mmioInstallIOProcA");

            return importTable.mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
        }

        // ---------


        LPMMIOPROC mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioInstallIOProcW)
                LogMissingFunctionCalled(L"mmioInstallIOProcW");

            return importTable.mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
        }

        // ---------


        HMMIO mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioOpenA)
                LogMissingFunctionCalled(L"mmioOpenA");

            return importTable.mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
        }

        // ---------


        HMMIO mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioOpenW)
                LogMissingFunctionCalled(L"mmioOpenW");

            return importTable.mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
        }

        // ---------


        LONG mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
        {
            Initialize();

            if (nullptr == importTable.mmioRead)
                LogMissingFunctionCalled(L"mmioRead");

            return importTable.mmioRead(hmmio, pch, cch);
        }

        // ---------


        MMRESULT mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioRenameA)
                LogMissingFunctionCalled(L"mmioRenameA");

            return importTable.mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
        }

        // ---------


        MMRESULT mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioRenameW)
                LogMissingFunctionCalled(L"mmioRenameW");

            return importTable.mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
        }

        // ---------


        LONG mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
        {
            Initialize();

            if (nullptr == importTable.mmioSeek)
                LogMissingFunctionCalled(L"mmioSeek");

            return importTable.mmioSeek(hmmio, lOffset, iOrigin);
        }

        // ---------


        LRESULT mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
        {
            Initialize();

            if (nullptr == importTable.mmioSendMessage)
                LogMissingFunctionCalled(L"mmioSendMessage");

            return importTable.mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
        }

        // ---------


        MMRESULT mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioSetBuffer)
                LogMissingFunctionCalled(L"mmioSetBuffer");

            return importTable.mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
        }

        // ---------


        MMRESULT mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioSetInfo)
                LogMissingFunctionCalled(L"mmioSetInfo");

            return importTable.mmioSetInfo(hmmio, lpmmioinfo, wFlags);
        }

        // ---------


        FOURCC mmioStringToFOURCCA(LPCSTR sz, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioStringToFOURCCA)
                LogMissingFunctionCalled(L"mmioStringToFOURCCA");

            return importTable.mmioStringToFOURCCA(sz, wFlags);
        }

        // ---------


        FOURCC mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.mmioStringToFOURCCW)
                LogMissingFunctionCalled(L"mmioStringToFOURCCW");

            return importTable.mmioStringToFOURCCW(sz, wFlags);
        }

        // ---------


        LONG mmioWrite(HMMIO hmmio, const char* pch, LONG cch)
        {
            Initialize();

            if (nullptr == importTable.mmioWrite)
                LogMissingFunctionCalled(L"mmioWrite");

            return importTable.mmioWrite(hmmio, pch, cch);
        }

        // ---------

        BOOL sndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
        {
            Initialize();

            if (nullptr == importTable.sndPlaySoundA)
                LogMissingFunctionCalled(L"sndPlaySoundA");

            return importTable.sndPlaySoundA(lpszSound, fuSound);
        }

        // ---------

        BOOL sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
        {
            Initialize();

            if (nullptr == importTable.sndPlaySoundW)
                LogMissingFunctionCalled(L"sndPlaySoundW");

            return importTable.sndPlaySoundW(lpszSound, fuSound);
        }

        // ---------

        MMRESULT timeBeginPeriod(UINT uPeriod)
        {
            Initialize();

            if (nullptr == importTable.timeBeginPeriod)
                LogMissingFunctionCalled(L"timeBeginPeriod");

            return importTable.timeBeginPeriod(uPeriod);
        }

        // ---------

        MMRESULT timeEndPeriod(UINT uPeriod)
        {
            Initialize();

            if (nullptr == importTable.timeEndPeriod)
                LogMissingFunctionCalled(L"timeEndPeriod");

            return importTable.timeEndPeriod(uPeriod);
        }

        // ---------

        MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
        {
            Initialize();

            if (nullptr == importTable.timeGetDevCaps)
                LogMissingFunctionCalled(L"timeGetDevCaps");

            return importTable.timeGetDevCaps(ptc, cbtc);
        }

        // ---------

        MMRESULT timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.timeGetSystemTime)
                LogMissingFunctionCalled(L"timeGetSystemTime");

            return importTable.timeGetSystemTime(pmmt, cbmmt);
        }

        // ---------

        DWORD timeGetTime(void)
        {
            Initialize();

            if (nullptr == importTable.timeGetTime)
                LogMissingFunctionCalled(L"timeGetTime");

            return importTable.timeGetTime();
        }

        // ---------

        MMRESULT timeKillEvent(UINT uTimerID)
        {
            Initialize();

            if (nullptr == importTable.timeKillEvent)
                LogMissingFunctionCalled(L"timeKillEvent");

            return importTable.timeKillEvent(uTimerID);
        }

        // ---------

        MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
        {
            Initialize();

            if (nullptr == importTable.timeSetEvent)
                LogMissingFunctionCalled(L"timeSetEvent");

            return importTable.timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
        }

        // ---------

        MMRESULT waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.waveInAddBuffer)
                LogMissingFunctionCalled(L"waveInAddBuffer");

            return importTable.waveInAddBuffer(hwi, pwh, cbwh);
        }

        // ---------

        MMRESULT waveInClose(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.waveInClose)
                LogMissingFunctionCalled(L"waveInClose");

            return importTable.waveInClose(hwi);
        }

        // ---------

        MMRESULT waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
        {
            Initialize();

            if (nullptr == importTable.waveInGetDevCapsA)
                LogMissingFunctionCalled(L"waveInGetDevCapsA");

            return importTable.waveInGetDevCapsA(uDeviceID, pwic, cbwic);
        }

        // ---------

        MMRESULT waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
        {
            Initialize();

            if (nullptr == importTable.waveInGetDevCapsW)
                LogMissingFunctionCalled(L"waveInGetDevCapsW");

            return importTable.waveInGetDevCapsW(uDeviceID, pwic, cbwic);
        }

        // ---------

        MMRESULT waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.waveInGetErrorTextA)
                LogMissingFunctionCalled(L"waveInGetErrorTextA");

            return importTable.waveInGetErrorTextA(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.waveInGetErrorTextW)
                LogMissingFunctionCalled(L"waveInGetErrorTextW");

            return importTable.waveInGetErrorTextW(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.waveInGetID)
                LogMissingFunctionCalled(L"waveInGetID");

            return importTable.waveInGetID(hwi, puDeviceID);
        }

        // ---------

        UINT waveInGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.waveInGetNumDevs)
                LogMissingFunctionCalled(L"waveInGetNumDevs");

            return importTable.waveInGetNumDevs();
        }

        // ---------

        MMRESULT waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.waveInGetPosition)
                LogMissingFunctionCalled(L"waveInGetPosition");

            return importTable.waveInGetPosition(hwi, pmmt, cbmmt);
        }

        // ---------

        DWORD waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.waveInMessage)
                LogMissingFunctionCalled(L"waveInMessage");

            return importTable.waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.waveInOpen)
                LogMissingFunctionCalled(L"waveInOpen");

            return importTable.waveInOpen(phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
        }

        // ---------

        MMRESULT waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.waveInPrepareHeader)
                LogMissingFunctionCalled(L"waveInPrepareHeader");

            return importTable.waveInPrepareHeader(hwi, pwh, cbwh);
        }

        // ---------

        MMRESULT waveInReset(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.waveInReset)
                LogMissingFunctionCalled(L"waveInReset");

            return importTable.waveInReset(hwi);
        }

        // ---------

        MMRESULT waveInStart(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.waveInStart)
                LogMissingFunctionCalled(L"waveInStart");

            return importTable.waveInStart(hwi);
        }

        // ---------

        MMRESULT waveInStop(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.waveInStop)
                LogMissingFunctionCalled(L"waveInStop");

            return importTable.waveInStop(hwi);
        }

        // ---------

        MMRESULT waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.waveInUnprepareHeader)
                LogMissingFunctionCalled(L"waveInUnprepareHeader");

            return importTable.waveInUnprepareHeader(hwi, pwh, cbwh);
        }

        // ---------

        MMRESULT waveOutBreakLoop(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.waveOutBreakLoop)
                LogMissingFunctionCalled(L"waveOutBreakLoop");

            return importTable.waveOutBreakLoop(hwo);
        }

        // ---------

        MMRESULT waveOutClose(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.waveOutClose)
                LogMissingFunctionCalled(L"waveOutClose");

            return importTable.waveOutClose(hwo);
        }

        // ---------

        MMRESULT waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetDevCapsA)
                LogMissingFunctionCalled(L"waveOutGetDevCapsA");

            return importTable.waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
        }

        // ---------

        MMRESULT waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetDevCapsW)
                LogMissingFunctionCalled(L"waveOutGetDevCapsW");

            return importTable.waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
        }

        // ---------

        MMRESULT waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetErrorTextA)
                LogMissingFunctionCalled(L"waveOutGetErrorTextA");

            return importTable.waveOutGetErrorTextA(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetErrorTextW)
                LogMissingFunctionCalled(L"waveOutGetErrorTextW");

            return importTable.waveOutGetErrorTextW(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetID)
                LogMissingFunctionCalled(L"waveOutGetID");

            return importTable.waveOutGetID(hwo, puDeviceID);
        }

        // ---------

        UINT waveOutGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetNumDevs)
                LogMissingFunctionCalled(L"waveOutGetNumDevs");

            return importTable.waveOutGetNumDevs();
        }

        // ---------

        MMRESULT waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetPitch)
                LogMissingFunctionCalled(L"waveOutGetPitch");

            return importTable.waveOutGetPitch(hwo, pdwPitch);
        }

        // ---------

        MMRESULT waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetPlaybackRate)
                LogMissingFunctionCalled(L"waveOutGetPlaybackRate");

            return importTable.waveOutGetPlaybackRate(hwo, pdwRate);
        }

        // ---------

        MMRESULT waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetPosition)
                LogMissingFunctionCalled(L"waveOutGetPosition");

            return importTable.waveOutGetPosition(hwo, pmmt, cbmmt);
        }

        // ---------

        MMRESULT waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
        {
            Initialize();

            if (nullptr == importTable.waveOutGetVolume)
                LogMissingFunctionCalled(L"waveOutGetVolume");

            return importTable.waveOutGetVolume(hwo, pdwVolume);
        }

        // ---------

        DWORD waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.waveOutMessage)
                LogMissingFunctionCalled(L"waveOutMessage");

            return importTable.waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.waveOutOpen)
                LogMissingFunctionCalled(L"waveOutOpen");

            return importTable.waveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
        }

        // ---------

        MMRESULT waveOutPause(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.waveOutPause)
                LogMissingFunctionCalled(L"waveOutPause");

            return importTable.waveOutPause(hwo);
        }

        // ---------

        MMRESULT waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.waveOutPrepareHeader)
                LogMissingFunctionCalled(L"waveOutPrepareHeader");

            return importTable.waveOutPrepareHeader(hwo, pwh, cbwh);
        }

        // ---------

        MMRESULT waveOutReset(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.waveOutReset)
                LogMissingFunctionCalled(L"waveOutReset");

            return importTable.waveOutReset(hwo);
        }

        // ---------

        MMRESULT waveOutRestart(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.waveOutRestart)
                LogMissingFunctionCalled(L"waveOutRestart");

            return importTable.waveOutRestart(hwo);
        }

        // ---------

        MMRESULT waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
        {
            Initialize();

            if (nullptr == importTable.waveOutSetPitch)
                LogMissingFunctionCalled(L"waveOutSetPitch");

            return importTable.waveOutSetPitch(hwo, dwPitch);
        }

        // ---------

        MMRESULT waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
        {
            Initialize();

            if (nullptr == importTable.waveOutSetPlaybackRate)
                LogMissingFunctionCalled(L"waveOutSetPlaybackRate");

            return importTable.waveOutSetPlaybackRate(hwo, dwRate);
        }

        // ---------

        MMRESULT waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
        {
            Initialize();

            if (nullptr == importTable.waveOutSetVolume)
                LogMissingFunctionCalled(L"waveOutSetVolume");

            return importTable.waveOutSetVolume(hwo, dwVolume);
        }

        // ---------

        MMRESULT waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.waveOutUnprepareHeader)
                LogMissingFunctionCalled(L"waveOutUnprepareHeader");

            return importTable.waveOutUnprepareHeader(hwo, pwh, cbwh);
        }

        // ---------

        MMRESULT waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.waveOutWrite)
                LogMissingFunctionCalled(L"waveOutWrite");

            return importTable.waveOutWrite(hwo, pwh, cbwh);
        }
    }
}
