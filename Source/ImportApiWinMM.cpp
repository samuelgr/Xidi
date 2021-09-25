/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2021
 *************************************************************************//**
 * @file ImportApiWinMM.cpp
 *   Implementations of functions for accessing the WinMM API imported from
 *   the native WinMM library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ApiXidi.h"
#include "Configuration.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "Message.h"
#include "Strings.h"

#include <map>
#include <mutex>
#include <set>
#include <string_view>


// -------- MACROS --------------------------------------------------------- //

/// Computes the index of the specified named function in the pointer array of the import table.
#define IMPORT_TABLE_INDEX_OF(name)         (offsetof(UImportTable, named.##name) / sizeof(UImportTable::ptr[0]))


namespace Xidi
{
    namespace ImportApiWinMM
    {
        // -------- INTERNAL TYPES ----------------------------------------- //

        /// Holds pointers to all the functions imported from the native WinMM library.
        /// Exposes them as both an array of typeless pointers and a named structure of type-specific pointers.
        union UImportTable
        {
            struct
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
            } named;

            const void* ptr[sizeof(named) / sizeof(const void*)];
        };
        static_assert(sizeof(UImportTable::named) == sizeof(UImportTable::ptr), L"Element size mismatch.");


        // -------- INTERNAL VARIABLES ------------------------------------- //

        /// Holds the imported WinMM API function addresses.
        static UImportTable importTable;


        // -------- INTERNAL FUNCTIONS ------------------------------------- //

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
            std::call_once(initializeFlag, []() -> void
                {
                    // Initialize the import table.
                    ZeroMemory(&importTable, sizeof(importTable));

                    // Obtain the full library path string.
                    std::wstring_view libraryPath = GetImportLibraryPathWinMM();

                    // Attempt to load the library.
                    Message::OutputFormatted(Message::ESeverity::Debug, L"Attempting to import WinMM functions from %s.", libraryPath.data());
                    HMODULE loadedLibrary = LoadLibraryEx(libraryPath.data(), nullptr, 0);
                    if (nullptr == loadedLibrary)
                    {
                        Message::Output(Message::ESeverity::Error, L"Failed to initialize imported WinMM functions.");
                        return;
                    }

                    // Attempt to obtain the addresses of all imported API functions.
                    FARPROC procAddress = nullptr;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "CloseDriver");
                    if (nullptr == procAddress) LogImportFailed(L"CloseDriver");
                    importTable.named.CloseDriver = (LRESULT(WINAPI*)(HDRVR, LPARAM, LPARAM))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DefDriverProc");
                    if (nullptr == procAddress) LogImportFailed(L"DefDriverProc");
                    importTable.named.DefDriverProc = (LRESULT(WINAPI*)(DWORD_PTR, HDRVR, UINT, LONG, LONG))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DriverCallback");
                    if (nullptr == procAddress) LogImportFailed(L"DriverCallback");
                    importTable.named.DriverCallback = (BOOL(WINAPI*)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "DrvGetModuleHandle");
                    if (nullptr == procAddress) LogImportFailed(L"DrvGetModuleHandle");
                    importTable.named.DrvGetModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "GetDriverModuleHandle");
                    if (nullptr == procAddress) LogImportFailed(L"GetDriverModuleHandle");
                    importTable.named.GetDriverModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "OpenDriver");
                    if (nullptr == procAddress) LogImportFailed(L"OpenDriver");
                    importTable.named.OpenDriver = (HDRVR(WINAPI*)(LPCWSTR, LPCWSTR, LPARAM))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "PlaySoundA");
                    if (nullptr == procAddress) LogImportFailed(L"PlaySoundA");
                    importTable.named.PlaySoundA = (BOOL(WINAPI*)(LPCSTR, HMODULE, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "PlaySoundW");
                    if (nullptr == procAddress) LogImportFailed(L"PlaySoundW");
                    importTable.named.PlaySoundW = (BOOL(WINAPI*)(LPCWSTR, HMODULE, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "SendDriverMessage");
                    if (nullptr == procAddress) LogImportFailed(L"SendDriverMessage");
                    importTable.named.SendDriverMessage = (LRESULT(WINAPI*)(HDRVR, UINT, LPARAM, LPARAM))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"auxGetDevCapsA");
                    importTable.named.auxGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSA, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"auxGetDevCapsW");
                    importTable.named.auxGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSW, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"auxGetNumDevs");
                    importTable.named.auxGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
                    if (nullptr == procAddress) LogImportFailed(L"auxGetVolume");
                    importTable.named.auxGetVolume = (MMRESULT(WINAPI*)(UINT, LPDWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
                    if (nullptr == procAddress) LogImportFailed(L"auxOutMessage");
                    importTable.named.auxOutMessage = (MMRESULT(WINAPI*)(UINT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
                    if (nullptr == procAddress) LogImportFailed(L"auxSetVolume");
                    importTable.named.auxSetVolume = (MMRESULT(WINAPI*)(UINT, DWORD))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
                    if (nullptr == procAddress) LogImportFailed(L"joyConfigChanged");
                    importTable.named.joyConfigChanged = (MMRESULT(WINAPI*)(DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"joyGetDevCapsA");
                    importTable.named.joyGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSA, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"joyGetDevCapsW");
                    importTable.named.joyGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSW, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"joyGetNumDevs");
                    importTable.named.joyGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
                    if (nullptr == procAddress) LogImportFailed(L"joyGetPos");
                    importTable.named.joyGetPos = (MMRESULT(WINAPI*)(UINT, LPJOYINFO))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
                    if (nullptr == procAddress) LogImportFailed(L"joyGetPosEx");
                    importTable.named.joyGetPosEx = (MMRESULT(WINAPI*)(UINT, LPJOYINFOEX))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
                    if (nullptr == procAddress) LogImportFailed(L"joyGetThreshold");
                    importTable.named.joyGetThreshold = (MMRESULT(WINAPI*)(UINT, LPUINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
                    if (nullptr == procAddress) LogImportFailed(L"joyReleaseCapture");
                    importTable.named.joyReleaseCapture = (MMRESULT(WINAPI*)(UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
                    if (nullptr == procAddress) LogImportFailed(L"joySetCapture");
                    importTable.named.joySetCapture = (MMRESULT(WINAPI*)(HWND, UINT, UINT, BOOL))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
                    if (nullptr == procAddress) LogImportFailed(L"joySetThreshold");
                    importTable.named.joySetThreshold = (MMRESULT(WINAPI*)(UINT, UINT))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "mciDriverNotify");
                    if (nullptr == procAddress) LogImportFailed(L"mciDriverNotify");
                    importTable.named.mciDriverNotify = (decltype(importTable.named.mciDriverNotify))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciDriverYield");
                    if (nullptr == procAddress) LogImportFailed(L"mciDriverYield");
                    importTable.named.mciDriverYield = (decltype(importTable.named.mciDriverYield))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciExecute");
                    if (nullptr == procAddress) LogImportFailed(L"mciExecute");
                    importTable.named.mciExecute = (decltype(importTable.named.mciExecute))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciFreeCommandResource");
                    if (nullptr == procAddress) LogImportFailed(L"mciFreeCommandResource");
                    importTable.named.mciFreeCommandResource = (decltype(importTable.named.mciFreeCommandResource))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetCreatorTask");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetCreatorTask");
                    importTable.named.mciGetCreatorTask = (decltype(importTable.named.mciGetCreatorTask))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDA");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDA");
                    importTable.named.mciGetDeviceIDA = (decltype(importTable.named.mciGetDeviceIDA))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDW");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDW");
                    importTable.named.mciGetDeviceIDW = (decltype(importTable.named.mciGetDeviceIDW))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDA");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDA");
                    importTable.named.mciGetDeviceIDFromElementIDA = (decltype(importTable.named.mciGetDeviceIDFromElementIDA))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDW");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDW");
                    importTable.named.mciGetDeviceIDFromElementIDW = (decltype(importTable.named.mciGetDeviceIDFromElementIDW))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetDriverData");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetDriverData");
                    importTable.named.mciGetDriverData = (decltype(importTable.named.mciGetDriverData))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringA");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetErrorStringA");
                    importTable.named.mciGetErrorStringA = (decltype(importTable.named.mciGetErrorStringA))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringW");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetErrorStringW");
                    importTable.named.mciGetErrorStringW = (decltype(importTable.named.mciGetErrorStringW))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciGetYieldProc");
                    if (nullptr == procAddress) LogImportFailed(L"mciGetYieldProc");
                    importTable.named.mciGetYieldProc = (decltype(importTable.named.mciGetYieldProc))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciLoadCommandResource");
                    if (nullptr == procAddress) LogImportFailed(L"mciLoadCommandResource");
                    importTable.named.mciLoadCommandResource = (decltype(importTable.named.mciLoadCommandResource))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciSendCommandA");
                    if (nullptr == procAddress) LogImportFailed(L"mciSendCommandA");
                    importTable.named.mciSendCommandA = (decltype(importTable.named.mciSendCommandA))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciSendCommandW");
                    if (nullptr == procAddress) LogImportFailed(L"mciSendCommandW");
                    importTable.named.mciSendCommandW = (decltype(importTable.named.mciSendCommandW))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciSendStringA");
                    if (nullptr == procAddress) LogImportFailed(L"mciSendStringA");
                    importTable.named.mciSendStringA = (decltype(importTable.named.mciSendStringA))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciSendStringW");
                    if (nullptr == procAddress) LogImportFailed(L"mciSendStringW");
                    importTable.named.mciSendStringW = (decltype(importTable.named.mciSendStringW))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciSetDriverData");
                    if (nullptr == procAddress) LogImportFailed(L"mciSetDriverData");
                    importTable.named.mciSetDriverData = (decltype(importTable.named.mciSetDriverData))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mciSetYieldProc");
                    if (nullptr == procAddress) LogImportFailed(L"mciSetYieldProc");
                    importTable.named.mciSetYieldProc = (decltype(importTable.named.mciSetYieldProc))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "midiConnect");
                    if (nullptr == procAddress) LogImportFailed(L"midiConnect");
                    importTable.named.midiConnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiDisconnect");
                    if (nullptr == procAddress) LogImportFailed(L"midiDisconnect");
                    importTable.named.midiDisconnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "midiInAddBuffer");
                    if (nullptr == procAddress) LogImportFailed(L"midiInAddBuffer");
                    importTable.named.midiInAddBuffer = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInClose");
                    if (nullptr == procAddress) LogImportFailed(L"midiInClose");
                    importTable.named.midiInClose = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"midiInGetDevCapsA");
                    importTable.named.midiInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSA, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"midiInGetDevCapsW");
                    importTable.named.midiInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSW, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextA");
                    if (nullptr == procAddress) LogImportFailed(L"midiInGetErrorTextA");
                    importTable.named.midiInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextW");
                    if (nullptr == procAddress) LogImportFailed(L"midiInGetErrorTextW");
                    importTable.named.midiInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInGetID");
                    if (nullptr == procAddress) LogImportFailed(L"midiInGetID");
                    importTable.named.midiInGetID = (MMRESULT(WINAPI*)(HMIDIIN, LPUINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"midiInGetNumDevs");
                    importTable.named.midiInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInMessage");
                    if (nullptr == procAddress) LogImportFailed(L"midiInMessage");
                    importTable.named.midiInMessage = (DWORD(WINAPI*)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInOpen");
                    if (nullptr == procAddress) LogImportFailed(L"midiInOpen");
                    importTable.named.midiInOpen = (MMRESULT(WINAPI*)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInPrepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"midiInPrepareHeader");
                    importTable.named.midiInPrepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInReset");
                    if (nullptr == procAddress) LogImportFailed(L"midiInReset");
                    importTable.named.midiInReset = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInStart");
                    if (nullptr == procAddress) LogImportFailed(L"midiInStart");
                    importTable.named.midiInStart = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInStop");
                    if (nullptr == procAddress) LogImportFailed(L"midiInStop");
                    importTable.named.midiInStop = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiInUnprepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"midiInUnprepareHeader");
                    importTable.named.midiInUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "midiOutCacheDrumPatches");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutCacheDrumPatches");
                    importTable.named.midiOutCacheDrumPatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutCachePatches");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutCachePatches");
                    importTable.named.midiOutCachePatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutClose");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutClose");
                    importTable.named.midiOutClose = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetDevCapsA");
                    importTable.named.midiOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSA, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetDevCapsW");
                    importTable.named.midiOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSW, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextA");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetErrorTextA");
                    importTable.named.midiOutGetErrorTextA = (UINT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextW");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetErrorTextW");
                    importTable.named.midiOutGetErrorTextW = (UINT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetID");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetID");
                    importTable.named.midiOutGetID = (MMRESULT(WINAPI*)(HMIDIOUT, LPUINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetNumDevs");
                    importTable.named.midiOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutGetVolume");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutGetVolume");
                    importTable.named.midiOutGetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, LPDWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutLongMsg");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutLongMsg");
                    importTable.named.midiOutLongMsg = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutMessage");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutMessage");
                    importTable.named.midiOutMessage = (DWORD(WINAPI*)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutOpen");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutOpen");
                    importTable.named.midiOutOpen = (MMRESULT(WINAPI*)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutPrepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutPrepareHeader");
                    importTable.named.midiOutPrepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutReset");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutReset");
                    importTable.named.midiOutReset = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutSetVolume");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutSetVolume");
                    importTable.named.midiOutSetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutShortMsg");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutShortMsg");
                    importTable.named.midiOutShortMsg = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiOutUnprepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"midiOutUnprepareHeader");
                    importTable.named.midiOutUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamClose");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamClose");
                    importTable.named.midiStreamClose = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamOpen");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamOpen");
                    importTable.named.midiStreamOpen = (MMRESULT(WINAPI*)(LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamOut");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamOut");
                    importTable.named.midiStreamOut = (MMRESULT(WINAPI*)(HMIDISTRM, LPMIDIHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamPause");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamPause");
                    importTable.named.midiStreamPause = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamPosition");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamPosition");
                    importTable.named.midiStreamPosition = (MMRESULT(WINAPI*)(HMIDISTRM, LPMMTIME, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamProperty");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamProperty");
                    importTable.named.midiStreamProperty = (MMRESULT(WINAPI*)(HMIDISTRM, LPBYTE, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamRestart");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamRestart");
                    importTable.named.midiStreamRestart = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "midiStreamStop");
                    if (nullptr == procAddress) LogImportFailed(L"midiStreamStop");
                    importTable.named.midiStreamStop = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "mixerClose");
                    if (nullptr == procAddress) LogImportFailed(L"mixerClose");
                    importTable.named.mixerClose = (MMRESULT(WINAPI*)(HMIXER))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsA");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetControlDetailsA");
                    importTable.named.mixerGetControlDetailsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsW");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetControlDetailsW");
                    importTable.named.mixerGetControlDetailsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetDevCapsA");
                    importTable.named.mixerGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetDevCapsW");
                    importTable.named.mixerGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetID");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetID");
                    importTable.named.mixerGetID = (MMRESULT(WINAPI*)(HMIXEROBJ, UINT*, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsA");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetLineControlsA");
                    importTable.named.mixerGetLineControlsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsW");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetLineControlsW");
                    importTable.named.mixerGetLineControlsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoA");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetLineInfoA");
                    importTable.named.mixerGetLineInfoA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoW");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetLineInfoW");
                    importTable.named.mixerGetLineInfoW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"mixerGetNumDevs");
                    importTable.named.mixerGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerMessage");
                    if (nullptr == procAddress) LogImportFailed(L"mixerMessage");
                    importTable.named.mixerMessage = (DWORD(WINAPI*)(HMIXER, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerOpen");
                    if (nullptr == procAddress) LogImportFailed(L"mixerOpen");
                    importTable.named.mixerOpen = (MMRESULT(WINAPI*)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mixerSetControlDetails");
                    if (nullptr == procAddress) LogImportFailed(L"mixerSetControlDetails");
                    importTable.named.mixerSetControlDetails = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "mmioAdvance");
                    if (nullptr == procAddress) LogImportFailed(L"mmioAdvance");
                    importTable.named.mmioAdvance = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioAscend");
                    if (nullptr == procAddress) LogImportFailed(L"mmioAscend");
                    importTable.named.mmioAscend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioClose");
                    if (nullptr == procAddress) LogImportFailed(L"mmioClose");
                    importTable.named.mmioClose = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioCreateChunk");
                    if (nullptr == procAddress) LogImportFailed(L"mmioCreateChunk");
                    importTable.named.mmioCreateChunk = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioDescend");
                    if (nullptr == procAddress) LogImportFailed(L"mmioDescend");
                    importTable.named.mmioDescend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioFlush");
                    if (nullptr == procAddress) LogImportFailed(L"mmioFlush");
                    importTable.named.mmioFlush = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioGetInfo");
                    if (nullptr == procAddress) LogImportFailed(L"mmioGetInfo");
                    importTable.named.mmioGetInfo = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcA");
                    if (nullptr == procAddress) LogImportFailed(L"mmioInstallIOProcA");
                    importTable.named.mmioInstallIOProcA = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcW");
                    if (nullptr == procAddress) LogImportFailed(L"mmioInstallIOProcW");
                    importTable.named.mmioInstallIOProcW = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioOpenA");
                    if (nullptr == procAddress) LogImportFailed(L"mmioOpenA");
                    importTable.named.mmioOpenA = (HMMIO(WINAPI*)(LPSTR, LPMMIOINFO, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioOpenW");
                    if (nullptr == procAddress) LogImportFailed(L"mmioOpenW");
                    importTable.named.mmioOpenW = (HMMIO(WINAPI*)(LPWSTR, LPMMIOINFO, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioRead");
                    if (nullptr == procAddress) LogImportFailed(L"mmioRead");
                    importTable.named.mmioRead = (LONG(WINAPI*)(HMMIO, HPSTR, LONG))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioRenameA");
                    if (nullptr == procAddress) LogImportFailed(L"mmioRenameA");
                    importTable.named.mmioRenameA = (MMRESULT(WINAPI*)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioRenameW");
                    if (nullptr == procAddress) LogImportFailed(L"mmioRenameW");
                    importTable.named.mmioRenameW = (MMRESULT(WINAPI*)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioSeek");
                    if (nullptr == procAddress) LogImportFailed(L"mmioSeek");
                    importTable.named.mmioSeek = (LONG(WINAPI*)(HMMIO, LONG, int))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioSendMessage");
                    if (nullptr == procAddress) LogImportFailed(L"mmioSendMessage");
                    importTable.named.mmioSendMessage = (LRESULT(WINAPI*)(HMMIO, UINT, LPARAM, LPARAM))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioSetBuffer");
                    if (nullptr == procAddress) LogImportFailed(L"mmioSetBuffer");
                    importTable.named.mmioSetBuffer = (MMRESULT(WINAPI*)(HMMIO, LPSTR, LONG, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioSetInfo");
                    if (nullptr == procAddress) LogImportFailed(L"mmioSetInfo");
                    importTable.named.mmioSetInfo = (MMRESULT(WINAPI*)(HMMIO, LPCMMIOINFO, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCA");
                    if (nullptr == procAddress) LogImportFailed(L"mmioStringToFOURCCA");
                    importTable.named.mmioStringToFOURCCA = (FOURCC(WINAPI*)(LPCSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCW");
                    if (nullptr == procAddress) LogImportFailed(L"mmioStringToFOURCCW");
                    importTable.named.mmioStringToFOURCCW = (FOURCC(WINAPI*)(LPCWSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "mmioWrite");
                    if (nullptr == procAddress) LogImportFailed(L"mmioWrite");
                    importTable.named.mmioWrite = (LONG(WINAPI*)(HMMIO, const char*, LONG))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundA");
                    if (nullptr == procAddress) LogImportFailed(L"sndPlaySoundA");
                    importTable.named.sndPlaySoundA = (BOOL(WINAPI*)(LPCSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundW");
                    if (nullptr == procAddress) LogImportFailed(L"sndPlaySoundW");
                    importTable.named.sndPlaySoundW = (BOOL(WINAPI*)(LPCWSTR, UINT))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
                    if (nullptr == procAddress) LogImportFailed(L"timeBeginPeriod");
                    importTable.named.timeBeginPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
                    if (nullptr == procAddress) LogImportFailed(L"timeEndPeriod");
                    importTable.named.timeEndPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
                    if (nullptr == procAddress) LogImportFailed(L"timeGetDevCaps");
                    importTable.named.timeGetDevCaps = (MMRESULT(WINAPI*)(LPTIMECAPS, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
                    if (nullptr == procAddress) LogImportFailed(L"timeGetSystemTime");
                    importTable.named.timeGetSystemTime = (MMRESULT(WINAPI*)(LPMMTIME, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
                    if (nullptr == procAddress) LogImportFailed(L"timeGetTime");
                    importTable.named.timeGetTime = (DWORD(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
                    if (nullptr == procAddress) LogImportFailed(L"timeKillEvent");
                    importTable.named.timeKillEvent = (MMRESULT(WINAPI*)(UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
                    if (nullptr == procAddress) LogImportFailed(L"timeSetEvent");
                    importTable.named.timeSetEvent = (MMRESULT(WINAPI*)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "waveInAddBuffer");
                    if (nullptr == procAddress) LogImportFailed(L"waveInAddBuffer");
                    importTable.named.waveInAddBuffer = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInClose");
                    if (nullptr == procAddress) LogImportFailed(L"waveInClose");
                    importTable.named.waveInClose = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetDevCapsA");
                    importTable.named.waveInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSA, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetDevCapsW");
                    importTable.named.waveInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSW, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextA");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetErrorTextA");
                    importTable.named.waveInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextW");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetErrorTextW");
                    importTable.named.waveInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetID");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetID");
                    importTable.named.waveInGetID = (MMRESULT(WINAPI*)(HWAVEIN, LPUINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetNumDevs");
                    importTable.named.waveInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInGetPosition");
                    if (nullptr == procAddress) LogImportFailed(L"waveInGetPosition");
                    importTable.named.waveInGetPosition = (MMRESULT(WINAPI*)(HWAVEIN, LPMMTIME, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInMessage");
                    if (nullptr == procAddress) LogImportFailed(L"waveInMessage");
                    importTable.named.waveInMessage = (DWORD(WINAPI*)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInOpen");
                    if (nullptr == procAddress) LogImportFailed(L"waveInOpen");
                    importTable.named.waveInOpen = (MMRESULT(WINAPI*)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInPrepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"waveInPrepareHeader");
                    importTable.named.waveInPrepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInReset");
                    if (nullptr == procAddress) LogImportFailed(L"waveInReset");
                    importTable.named.waveInReset = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInStart");
                    if (nullptr == procAddress) LogImportFailed(L"waveInStart");
                    importTable.named.waveInStart = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInStop");
                    if (nullptr == procAddress) LogImportFailed(L"waveInStop");
                    importTable.named.waveInStop = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveInUnprepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"waveInUnprepareHeader");
                    importTable.named.waveInUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

                    // ---------

                    procAddress = GetProcAddress(loadedLibrary, "waveOutBreakLoop");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutBreakLoop");
                    importTable.named.waveOutBreakLoop = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutClose");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutClose");
                    importTable.named.waveOutClose = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsA");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetDevCapsA");
                    importTable.named.waveOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSA, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsW");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetDevCapsW");
                    importTable.named.waveOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSW, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextA");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetErrorTextA");
                    importTable.named.waveOutGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextW");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetErrorTextW");
                    importTable.named.waveOutGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetID");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetID");
                    importTable.named.waveOutGetID = (MMRESULT(WINAPI*)(HWAVEOUT, LPUINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetNumDevs");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetNumDevs");
                    importTable.named.waveOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetPitch");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetPitch");
                    importTable.named.waveOutGetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetPlaybackRate");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetPlaybackRate");
                    importTable.named.waveOutGetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetPosition");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetPosition");
                    importTable.named.waveOutGetPosition = (MMRESULT(WINAPI*)(HWAVEOUT, LPMMTIME, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutGetVolume");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutGetVolume");
                    importTable.named.waveOutGetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutMessage");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutMessage");
                    importTable.named.waveOutMessage = (DWORD(WINAPI*)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutOpen");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutOpen");
                    importTable.named.waveOutOpen = (MMRESULT(WINAPI*)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutPause");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutPause");
                    importTable.named.waveOutPause = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutPrepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutPrepareHeader");
                    importTable.named.waveOutPrepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutReset");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutReset");
                    importTable.named.waveOutReset = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutRestart");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutRestart");
                    importTable.named.waveOutRestart = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutSetPitch");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutSetPitch");
                    importTable.named.waveOutSetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutSetPlaybackRate");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutSetPlaybackRate");
                    importTable.named.waveOutSetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutSetVolume");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutSetVolume");
                    importTable.named.waveOutSetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutUnprepareHeader");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutUnprepareHeader");
                    importTable.named.waveOutUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

                    procAddress = GetProcAddress(loadedLibrary, "waveOutWrite");
                    if (nullptr == procAddress) LogImportFailed(L"waveOutWrite");
                    importTable.named.waveOutWrite = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

                    // Initialization complete.
                    Message::OutputFormatted(Message::ESeverity::Info, L"Successfully initialized imported WinMM functions.");
                }
            );
        }

        // ---------

        LRESULT CloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2)
        {
            Initialize();

            if (nullptr == importTable.named.CloseDriver)
                LogMissingFunctionCalled(L"CloseDriver");

            return importTable.named.CloseDriver(hdrvr, lParam1, lParam2);
        }

        // ---------

        LRESULT DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
        {
            Initialize();

            if (nullptr == importTable.named.DefDriverProc)
                LogMissingFunctionCalled(L"DefDriverProc");

            return importTable.named.DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
        }

        // ---------

        BOOL DriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
        {
            Initialize();

            if (nullptr == importTable.named.DriverCallback)
                LogMissingFunctionCalled(L"DriverCallback");

            return importTable.named.DriverCallback(dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
        }

        // ---------

        HMODULE DrvGetModuleHandle(HDRVR hDriver)
        {
            Initialize();

            if (nullptr == importTable.named.DrvGetModuleHandle)
                LogMissingFunctionCalled(L"DrvGetModuleHandle");

            return importTable.named.DrvGetModuleHandle(hDriver);
        }

        // ---------

        HMODULE GetDriverModuleHandle(HDRVR hdrvr)
        {
            Initialize();

            if (nullptr == importTable.named.GetDriverModuleHandle)
                LogMissingFunctionCalled(L"GetDriverModuleHandle");

            return importTable.named.GetDriverModuleHandle(hdrvr);
        }

        // ---------

        HDRVR OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
        {
            Initialize();

            if (nullptr == importTable.named.OpenDriver)
                LogMissingFunctionCalled(L"OpenDriver");

            return importTable.named.OpenDriver(lpDriverName, lpSectionName, lParam);
        }

        // ---------

        BOOL PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
        {
            Initialize();

            if (nullptr == importTable.named.PlaySoundA)
                LogMissingFunctionCalled(L"PlaySoundA");

            return importTable.named.PlaySoundA(pszSound, hmod, fdwSound);
        }

        // ---------

        BOOL PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
        {
            Initialize();

            if (nullptr == importTable.named.PlaySoundW)
                LogMissingFunctionCalled(L"PlaySoundW");

            return importTable.named.PlaySoundW(pszSound, hmod, fdwSound);
        }

        // ---------

        LRESULT SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
        {
            Initialize();

            if (nullptr == importTable.named.SendDriverMessage)
                LogMissingFunctionCalled(L"SendDriverMessage");

            return importTable.named.SendDriverMessage(hdrvr, msg, lParam1, lParam2);
        }

        // ---------

        MMRESULT auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
        {
            Initialize();

            if (nullptr == importTable.named.auxGetDevCapsA)
                LogMissingFunctionCalled(L"auxGetDevCapsA");

            return importTable.named.auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
        }

        // ---------

        MMRESULT auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
        {
            Initialize();

            if (nullptr == importTable.named.auxGetDevCapsW)
                LogMissingFunctionCalled(L"auxGetDevCapsW");

            return importTable.named.auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
        }

        // ---------

        UINT auxGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.auxGetNumDevs)
                LogMissingFunctionCalled(L"auxGetNumDevs");

            return importTable.named.auxGetNumDevs();
        }

        // ---------

        MMRESULT auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
        {
            Initialize();

            if (nullptr == importTable.named.auxGetVolume)
                LogMissingFunctionCalled(L"auxGetVolume");

            return importTable.named.auxGetVolume(uDeviceID, lpdwVolume);
        }

        // ---------

        MMRESULT auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.named.auxOutMessage)
                LogMissingFunctionCalled(L"auxOutMessage");

            return importTable.named.auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT auxSetVolume(UINT uDeviceID, DWORD dwVolume)
        {
            Initialize();

            if (nullptr == importTable.named.auxSetVolume)
                LogMissingFunctionCalled(L"auxSetVolume");

            return importTable.named.auxSetVolume(uDeviceID, dwVolume);
        }

        // ---------

        MMRESULT joyConfigChanged(DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.named.joyConfigChanged)
                LogMissingFunctionCalled(L"joyConfigChanged");

            return importTable.named.joyConfigChanged(dwFlags);
        }

        // ---------

        MMRESULT joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
        {
            Initialize();

            if (nullptr == importTable.named.joyGetDevCapsA)
                LogMissingFunctionCalled(L"joyGetDevCapsA");

            return importTable.named.joyGetDevCapsA(uJoyID, pjc, cbjc);
        }

        // ---------

        MMRESULT joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
        {
            Initialize();

            if (nullptr == importTable.named.joyGetDevCapsW)
                LogMissingFunctionCalled(L"joyGetDevCapsW");

            return importTable.named.joyGetDevCapsW(uJoyID, pjc, cbjc);
        }

        // ---------

        UINT joyGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.joyGetNumDevs)
                LogMissingFunctionCalled(L"joyGetNumDevs");

            return importTable.named.joyGetNumDevs();
        }

        // ---------

        MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji)
        {
            Initialize();

            if (nullptr == importTable.named.joyGetPos)
                LogMissingFunctionCalled(L"joyGetPos");

            return importTable.named.joyGetPos(uJoyID, pji);
        }

        // ---------

        MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
        {
            Initialize();

            if (nullptr == importTable.named.joyGetPosEx)
                LogMissingFunctionCalled(L"joyGetPosEx");

            return importTable.named.joyGetPosEx(uJoyID, pji);
        }

        // ---------

        MMRESULT joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
        {
            Initialize();

            if (nullptr == importTable.named.joyGetThreshold)
                LogMissingFunctionCalled(L"joyGetThreshold");

            return importTable.named.joyGetThreshold(uJoyID, puThreshold);
        }

        // ---------

        MMRESULT joyReleaseCapture(UINT uJoyID)
        {
            Initialize();

            if (nullptr == importTable.named.joyReleaseCapture)
                LogMissingFunctionCalled(L"joyReleaseCapture");

            return importTable.named.joyReleaseCapture(uJoyID);
        }

        // ---------

        MMRESULT joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
        {
            Initialize();

            if (nullptr == importTable.named.joySetCapture)
                LogMissingFunctionCalled(L"joySetCapture");

            return importTable.named.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
        }

        // ---------

        MMRESULT joySetThreshold(UINT uJoyID, UINT uThreshold)
        {
            Initialize();

            if (nullptr == importTable.named.joySetThreshold)
                LogMissingFunctionCalled(L"joySetThreshold");

            return importTable.named.joySetThreshold(uJoyID, uThreshold);
        }

        // ---------

        BOOL mciDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
        {
            Initialize();

            if (nullptr == importTable.named.mciDriverNotify)
                LogMissingFunctionCalled(L"mciDriverNotify");

            return importTable.named.mciDriverNotify(hwndCallback, IDDevice, uStatus);
        }

        // ---------

        UINT mciDriverYield(MCIDEVICEID IDDevice)
        {
            Initialize();

            if (nullptr == importTable.named.mciDriverYield)
                LogMissingFunctionCalled(L"mciDriverYield");

            return importTable.named.mciDriverYield(IDDevice);
        }

        // ---------

        BOOL mciExecute(LPCSTR pszCommand)
        {
            Initialize();

            if (nullptr == importTable.named.mciExecute)
                LogMissingFunctionCalled(L"mciExecute");

            return importTable.named.mciExecute(pszCommand);
        }

        // ---------

        BOOL mciFreeCommandResource(UINT uResource)
        {
            Initialize();

            if (nullptr == importTable.named.mciFreeCommandResource)
                LogMissingFunctionCalled(L"mciFreeCommandResource");

            return importTable.named.mciFreeCommandResource(uResource);
        }

        // ---------

        HANDLE mciGetCreatorTask(MCIDEVICEID IDDevice)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetCreatorTask)
                LogMissingFunctionCalled(L"mciGetCreatorTask");

            return importTable.named.mciGetCreatorTask(IDDevice);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDA(LPCSTR lpszDevice)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetDeviceIDA)
                LogMissingFunctionCalled(L"mciGetDeviceIDA");

            return importTable.named.mciGetDeviceIDA(lpszDevice);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDW(LPCWSTR lpszDevice)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetDeviceIDW)
                LogMissingFunctionCalled(L"mciGetDeviceIDW");

            return importTable.named.mciGetDeviceIDW(lpszDevice);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetDeviceIDFromElementIDA)
                LogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDA");

            return importTable.named.mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
        }

        // ---------

        MCIDEVICEID mciGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetDeviceIDFromElementIDW)
                LogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDW");

            return importTable.named.mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
        }

        // ---------

        DWORD_PTR mciGetDriverData(MCIDEVICEID IDDevice)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetDriverData)
                LogMissingFunctionCalled(L"mciGetDriverData");

            return importTable.named.mciGetDriverData(IDDevice);
        }

        // ---------

        BOOL mciGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetErrorStringA)
                LogMissingFunctionCalled(L"mciGetErrorStringA");

            return importTable.named.mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
        }

        // ---------

        BOOL mciGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetErrorStringW)
                LogMissingFunctionCalled(L"mciGetErrorStringW");

            return importTable.named.mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
        }

        // ---------

        YIELDPROC mciGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
        {
            Initialize();

            if (nullptr == importTable.named.mciGetYieldProc)
                LogMissingFunctionCalled(L"mciGetYieldProc");

            return importTable.named.mciGetYieldProc(IDDevice, lpdwYieldData);
        }

        // ---------

        UINT mciLoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
        {
            Initialize();

            if (nullptr == importTable.named.mciLoadCommandResource)
                LogMissingFunctionCalled(L"mciLoadCommandResource");

            return importTable.named.mciLoadCommandResource(hInst, lpwstrResourceName, uType);
        }

        // ---------

        MCIERROR mciSendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
        {
            Initialize();

            if (nullptr == importTable.named.mciSendCommandA)
                LogMissingFunctionCalled(L"mciSendCommandA");

            return importTable.named.mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
        }

        // ---------

        MCIERROR mciSendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
        {
            Initialize();

            if (nullptr == importTable.named.mciSendCommandW)
                LogMissingFunctionCalled(L"mciSendCommandW");

            return importTable.named.mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
        }

        // ---------

        MCIERROR mciSendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
        {
            Initialize();

            if (nullptr == importTable.named.mciSendStringA)
                LogMissingFunctionCalled(L"mciSendStringA");

            return importTable.named.mciSendStringA(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
        }

        // ---------

        MCIERROR mciSendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
        {
            Initialize();

            if (nullptr == importTable.named.mciSendStringW)
                LogMissingFunctionCalled(L"mciSendStringW");

            return importTable.named.mciSendStringW(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
        }

        // ---------

        BOOL mciSetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
        {
            Initialize();

            if (nullptr == importTable.named.mciSetDriverData)
                LogMissingFunctionCalled(L"mciSetDriverData");

            return importTable.named.mciSetDriverData(IDDevice, data);
        }

        // ---------

        UINT mciSetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
        {
            Initialize();

            if (nullptr == importTable.named.mciSetYieldProc)
                LogMissingFunctionCalled(L"mciSetYieldProc");

            return importTable.named.mciSetYieldProc(IDDevice, yp, dwYieldData);
        }

        // ---------

        MMRESULT midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
        {
            Initialize();

            if (nullptr == importTable.named.midiConnect)
                LogMissingFunctionCalled(L"midiConnect");

            return importTable.named.midiConnect(hMidi, hmo, pReserved);
        }

        // ---------

        MMRESULT midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
        {
            Initialize();

            if (nullptr == importTable.named.midiDisconnect)
                LogMissingFunctionCalled(L"midiDisconnect");

            return importTable.named.midiDisconnect(hMidi, hmo, pReserved);
        }

        // ---------

        MMRESULT midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiInAddBuffer)
                LogMissingFunctionCalled(L"midiInAddBuffer");

            return importTable.named.midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
        }

        // ---------

        MMRESULT midiInClose(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.named.midiInClose)
                LogMissingFunctionCalled(L"midiInClose");

            return importTable.named.midiInClose(hMidiIn);
        }

        // ---------

        MMRESULT midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
        {
            Initialize();

            if (nullptr == importTable.named.midiInGetDevCapsA)
                LogMissingFunctionCalled(L"midiInGetDevCapsA");

            return importTable.named.midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
        }

        // ---------

        MMRESULT midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
        {
            Initialize();

            if (nullptr == importTable.named.midiInGetDevCapsW)
                LogMissingFunctionCalled(L"midiInGetDevCapsW");

            return importTable.named.midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
        }

        // ---------

        MMRESULT midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.midiInGetErrorTextA)
                LogMissingFunctionCalled(L"midiInGetErrorTextA");

            return importTable.named.midiInGetErrorTextA(wError, lpText, cchText);
        }

        // ---------

        MMRESULT midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.midiInGetErrorTextW)
                LogMissingFunctionCalled(L"midiInGetErrorTextW");

            return importTable.named.midiInGetErrorTextW(wError, lpText, cchText);
        }

        // ---------

        MMRESULT midiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.named.midiInGetID)
                LogMissingFunctionCalled(L"midiInGetID");

            return importTable.named.midiInGetID(hmi, puDeviceID);
        }

        // ---------

        UINT midiInGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.midiInGetNumDevs)
                LogMissingFunctionCalled(L"midiInGetNumDevs");

            return importTable.named.midiInGetNumDevs();
        }

        // ---------

        DWORD midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
        {
            Initialize();

            if (nullptr == importTable.named.midiInMessage)
                LogMissingFunctionCalled(L"midiInMessage");

            return importTable.named.midiInMessage(deviceID, msg, dw1, dw2);
        }

        // ---------

        MMRESULT midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.named.midiInOpen)
                LogMissingFunctionCalled(L"midiInOpen");

            return importTable.named.midiInOpen(lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
        }

        // ---------

        MMRESULT midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiInPrepareHeader)
                LogMissingFunctionCalled(L"midiInPrepareHeader");

            return importTable.named.midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
        }

        // ---------

        MMRESULT midiInReset(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.named.midiInReset)
                LogMissingFunctionCalled(L"midiInReset");

            return importTable.named.midiInReset(hMidiIn);
        }

        // ---------

        MMRESULT midiInStart(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.named.midiInStart)
                LogMissingFunctionCalled(L"midiInStart");

            return importTable.named.midiInStart(hMidiIn);
        }

        // ---------

        MMRESULT midiInStop(HMIDIIN hMidiIn)
        {
            Initialize();

            if (nullptr == importTable.named.midiInStop)
                LogMissingFunctionCalled(L"midiInStop");

            return importTable.named.midiInStop(hMidiIn);
        }

        // ---------

        MMRESULT midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiInUnprepareHeader)
                LogMissingFunctionCalled(L"midiInUnprepareHeader");

            return importTable.named.midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
        }

        // ---------

        MMRESULT midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutCacheDrumPatches)
                LogMissingFunctionCalled(L"midiOutCacheDrumPatches");

            return importTable.named.midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
        }

        // ---------

        MMRESULT midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutCachePatches)
                LogMissingFunctionCalled(L"midiOutCachePatches");

            return importTable.named.midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
        }

        // ---------

        MMRESULT midiOutClose(HMIDIOUT hmo)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutClose)
                LogMissingFunctionCalled(L"midiOutClose");

            return importTable.named.midiOutClose(hmo);
        }

        // ---------

        MMRESULT midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetDevCapsA)
                LogMissingFunctionCalled(L"midiOutGetDevCapsA");

            return importTable.named.midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
        }

        // ---------

        MMRESULT midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetDevCapsW)
                LogMissingFunctionCalled(L"midiOutGetDevCapsW");

            return importTable.named.midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
        }

        // ---------

        UINT midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetErrorTextA)
                LogMissingFunctionCalled(L"midiOutGetErrorTextA");

            return importTable.named.midiOutGetErrorTextA(mmrError, lpText, cchText);
        }

        // ---------

        UINT midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetErrorTextW)
                LogMissingFunctionCalled(L"midiOutGetErrorTextW");

            return importTable.named.midiOutGetErrorTextW(mmrError, lpText, cchText);
        }

        // ---------

        MMRESULT midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetID)
                LogMissingFunctionCalled(L"midiOutGetID");

            return importTable.named.midiOutGetID(hmo, puDeviceID);
        }

        // ---------

        UINT midiOutGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetNumDevs)
                LogMissingFunctionCalled(L"midiOutGetNumDevs");

            return importTable.named.midiOutGetNumDevs();
        }

        // ---------

        MMRESULT midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutGetVolume)
                LogMissingFunctionCalled(L"midiOutGetVolume");

            return importTable.named.midiOutGetVolume(hmo, lpdwVolume);
        }

        // ---------

        MMRESULT midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutLongMsg)
                LogMissingFunctionCalled(L"midiOutLongMsg");

            return importTable.named.midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
        }

        // ---------

        DWORD midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutMessage)
                LogMissingFunctionCalled(L"midiOutMessage");

            return importTable.named.midiOutMessage(deviceID, msg, dw1, dw2);
        }

        // ---------

        MMRESULT midiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutOpen)
                LogMissingFunctionCalled(L"midiOutOpen");

            return importTable.named.midiOutOpen(lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
        }

        // ---------

        MMRESULT midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutPrepareHeader)
                LogMissingFunctionCalled(L"midiOutPrepareHeader");

            return importTable.named.midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
        }

        // ---------

        MMRESULT midiOutReset(HMIDIOUT hmo)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutReset)
                LogMissingFunctionCalled(L"midiOutReset");

            return importTable.named.midiOutReset(hmo);
        }

        // ---------

        MMRESULT midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutSetVolume)
                LogMissingFunctionCalled(L"midiOutSetVolume");

            return importTable.named.midiOutSetVolume(hmo, dwVolume);
        }

        // ---------

        MMRESULT midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutShortMsg)
                LogMissingFunctionCalled(L"midiOutShortMsg");

            return importTable.named.midiOutShortMsg(hmo, dwMsg);
        }

        // ---------

        MMRESULT midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiOutUnprepareHeader)
                LogMissingFunctionCalled(L"midiOutUnprepareHeader");

            return importTable.named.midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
        }

        // ---------

        MMRESULT midiStreamClose(HMIDISTRM hStream)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamClose)
                LogMissingFunctionCalled(L"midiStreamClose");

            return importTable.named.midiStreamClose(hStream);
        }

        // ---------

        MMRESULT midiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamOpen)
                LogMissingFunctionCalled(L"midiStreamOpen");

            return importTable.named.midiStreamOpen(lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
        }

        // ---------

        MMRESULT midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamOut)
                LogMissingFunctionCalled(L"midiStreamOut");

            return importTable.named.midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
        }

        // ---------

        MMRESULT midiStreamPause(HMIDISTRM hms)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamPause)
                LogMissingFunctionCalled(L"midiStreamPause");

            return importTable.named.midiStreamPause(hms);
        }

        // ---------

        MMRESULT midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamPosition)
                LogMissingFunctionCalled(L"midiStreamPosition");

            return importTable.named.midiStreamPosition(hms, pmmt, cbmmt);
        }

        // ---------

        MMRESULT midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamProperty)
                LogMissingFunctionCalled(L"midiStreamProperty");

            return importTable.named.midiStreamProperty(hm, lppropdata, dwProperty);
        }

        // ---------

        MMRESULT midiStreamRestart(HMIDISTRM hms)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamRestart)
                LogMissingFunctionCalled(L"midiStreamRestart");

            return importTable.named.midiStreamRestart(hms);
        }

        // ---------

        MMRESULT midiStreamStop(HMIDISTRM hms)
        {
            Initialize();

            if (nullptr == importTable.named.midiStreamStop)
                LogMissingFunctionCalled(L"midiStreamStop");

            return importTable.named.midiStreamStop(hms);
        }

        // ---------

        MMRESULT mixerClose(HMIXER hmx)
        {
            Initialize();

            if (nullptr == importTable.named.mixerClose)
                LogMissingFunctionCalled(L"mixerClose");

            return importTable.named.mixerClose(hmx);
        }

        // ---------

        MMRESULT mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetControlDetailsA)
                LogMissingFunctionCalled(L"mixerGetControlDetailsA");

            return importTable.named.mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
        }

        // ---------

        MMRESULT mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetControlDetailsW)
                LogMissingFunctionCalled(L"mixerGetControlDetailsW");

            return importTable.named.mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
        }

        // ---------

        MMRESULT mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetDevCapsA)
                LogMissingFunctionCalled(L"mixerGetDevCapsA");

            return importTable.named.mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
        }

        // ---------

        MMRESULT mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetDevCapsW)
                LogMissingFunctionCalled(L"mixerGetDevCapsW");

            return importTable.named.mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
        }

        // ---------

        MMRESULT mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetID)
                LogMissingFunctionCalled(L"mixerGetID");

            return importTable.named.mixerGetID(hmxobj, puMxId, fdwId);
        }

        // ---------

        MMRESULT mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetLineControlsA)
                LogMissingFunctionCalled(L"mixerGetLineControlsA");

            return importTable.named.mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
        }

        // ---------

        MMRESULT mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetLineControlsW)
                LogMissingFunctionCalled(L"mixerGetLineControlsW");

            return importTable.named.mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
        }

        // ---------

        MMRESULT mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetLineInfoA)
                LogMissingFunctionCalled(L"mixerGetLineInfoA");

            return importTable.named.mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
        }

        // ---------

        MMRESULT mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetLineInfoW)
                LogMissingFunctionCalled(L"mixerGetLineInfoW");

            return importTable.named.mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
        }

        // ---------

        UINT mixerGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.mixerGetNumDevs)
                LogMissingFunctionCalled(L"mixerGetNumDevs");

            return importTable.named.mixerGetNumDevs();
        }

        // ---------

        DWORD mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.named.mixerMessage)
                LogMissingFunctionCalled(L"mixerMessage");

            return importTable.named.mixerMessage(driverID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.named.mixerOpen)
                LogMissingFunctionCalled(L"mixerOpen");

            return importTable.named.mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
        }

        // ---------

        MMRESULT mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
        {
            Initialize();

            if (nullptr == importTable.named.mixerSetControlDetails)
                LogMissingFunctionCalled(L"mixerSetControlDetails");

            return importTable.named.mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
        }

        // ---------

        MMRESULT mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioAdvance)
                LogMissingFunctionCalled(L"mmioAdvance");

            return importTable.named.mmioAdvance(hmmio, lpmmioinfo, wFlags);
        }

        // ---------


        MMRESULT mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioAscend)
                LogMissingFunctionCalled(L"mmioAscend");

            return importTable.named.mmioAscend(hmmio, lpck, wFlags);
        }

        // ---------


        MMRESULT mmioClose(HMMIO hmmio, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioClose)
                LogMissingFunctionCalled(L"mmioClose");

            return importTable.named.mmioClose(hmmio, wFlags);
        }

        // ---------


        MMRESULT mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioCreateChunk)
                LogMissingFunctionCalled(L"mmioCreateChunk");

            return importTable.named.mmioCreateChunk(hmmio, lpck, wFlags);
        }

        // ---------


        MMRESULT mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioDescend)
                LogMissingFunctionCalled(L"mmioDescend");

            return importTable.named.mmioDescend(hmmio, lpck, lpckParent, wFlags);
        }

        // ---------


        MMRESULT mmioFlush(HMMIO hmmio, UINT fuFlush)
        {
            Initialize();

            if (nullptr == importTable.named.mmioFlush)
                LogMissingFunctionCalled(L"mmioFlush");

            return importTable.named.mmioFlush(hmmio, fuFlush);
        }

        // ---------


        MMRESULT mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioGetInfo)
                LogMissingFunctionCalled(L"mmioGetInfo");

            return importTable.named.mmioGetInfo(hmmio, lpmmioinfo, wFlags);
        }

        // ---------


        LPMMIOPROC mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioInstallIOProcA)
                LogMissingFunctionCalled(L"mmioInstallIOProcA");

            return importTable.named.mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
        }

        // ---------


        LPMMIOPROC mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioInstallIOProcW)
                LogMissingFunctionCalled(L"mmioInstallIOProcW");

            return importTable.named.mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
        }

        // ---------


        HMMIO mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioOpenA)
                LogMissingFunctionCalled(L"mmioOpenA");

            return importTable.named.mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
        }

        // ---------


        HMMIO mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioOpenW)
                LogMissingFunctionCalled(L"mmioOpenW");

            return importTable.named.mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
        }

        // ---------


        LONG mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
        {
            Initialize();

            if (nullptr == importTable.named.mmioRead)
                LogMissingFunctionCalled(L"mmioRead");

            return importTable.named.mmioRead(hmmio, pch, cch);
        }

        // ---------


        MMRESULT mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioRenameA)
                LogMissingFunctionCalled(L"mmioRenameA");

            return importTable.named.mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
        }

        // ---------


        MMRESULT mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioRenameW)
                LogMissingFunctionCalled(L"mmioRenameW");

            return importTable.named.mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
        }

        // ---------


        LONG mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
        {
            Initialize();

            if (nullptr == importTable.named.mmioSeek)
                LogMissingFunctionCalled(L"mmioSeek");

            return importTable.named.mmioSeek(hmmio, lOffset, iOrigin);
        }

        // ---------


        LRESULT mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
        {
            Initialize();

            if (nullptr == importTable.named.mmioSendMessage)
                LogMissingFunctionCalled(L"mmioSendMessage");

            return importTable.named.mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
        }

        // ---------


        MMRESULT mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioSetBuffer)
                LogMissingFunctionCalled(L"mmioSetBuffer");

            return importTable.named.mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
        }

        // ---------


        MMRESULT mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioSetInfo)
                LogMissingFunctionCalled(L"mmioSetInfo");

            return importTable.named.mmioSetInfo(hmmio, lpmmioinfo, wFlags);
        }

        // ---------


        FOURCC mmioStringToFOURCCA(LPCSTR sz, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioStringToFOURCCA)
                LogMissingFunctionCalled(L"mmioStringToFOURCCA");

            return importTable.named.mmioStringToFOURCCA(sz, wFlags);
        }

        // ---------


        FOURCC mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags)
        {
            Initialize();

            if (nullptr == importTable.named.mmioStringToFOURCCW)
                LogMissingFunctionCalled(L"mmioStringToFOURCCW");

            return importTable.named.mmioStringToFOURCCW(sz, wFlags);
        }

        // ---------


        LONG mmioWrite(HMMIO hmmio, const char* pch, LONG cch)
        {
            Initialize();

            if (nullptr == importTable.named.mmioWrite)
                LogMissingFunctionCalled(L"mmioWrite");

            return importTable.named.mmioWrite(hmmio, pch, cch);
        }

        // ---------

        BOOL sndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
        {
            Initialize();

            if (nullptr == importTable.named.sndPlaySoundA)
                LogMissingFunctionCalled(L"sndPlaySoundA");

            return importTable.named.sndPlaySoundA(lpszSound, fuSound);
        }

        // ---------

        BOOL sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
        {
            Initialize();

            if (nullptr == importTable.named.sndPlaySoundW)
                LogMissingFunctionCalled(L"sndPlaySoundW");

            return importTable.named.sndPlaySoundW(lpszSound, fuSound);
        }

        // ---------

        MMRESULT timeBeginPeriod(UINT uPeriod)
        {
            Initialize();

            if (nullptr == importTable.named.timeBeginPeriod)
                LogMissingFunctionCalled(L"timeBeginPeriod");

            return importTable.named.timeBeginPeriod(uPeriod);
        }

        // ---------

        MMRESULT timeEndPeriod(UINT uPeriod)
        {
            Initialize();

            if (nullptr == importTable.named.timeEndPeriod)
                LogMissingFunctionCalled(L"timeEndPeriod");

            return importTable.named.timeEndPeriod(uPeriod);
        }

        // ---------

        MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
        {
            Initialize();

            if (nullptr == importTable.named.timeGetDevCaps)
                LogMissingFunctionCalled(L"timeGetDevCaps");

            return importTable.named.timeGetDevCaps(ptc, cbtc);
        }

        // ---------

        MMRESULT timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.named.timeGetSystemTime)
                LogMissingFunctionCalled(L"timeGetSystemTime");

            return importTable.named.timeGetSystemTime(pmmt, cbmmt);
        }

        // ---------

        DWORD timeGetTime(void)
        {
            Initialize();

            if (nullptr == importTable.named.timeGetTime)
                LogMissingFunctionCalled(L"timeGetTime");

            return importTable.named.timeGetTime();
        }

        // ---------

        MMRESULT timeKillEvent(UINT uTimerID)
        {
            Initialize();

            if (nullptr == importTable.named.timeKillEvent)
                LogMissingFunctionCalled(L"timeKillEvent");

            return importTable.named.timeKillEvent(uTimerID);
        }

        // ---------

        MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
        {
            Initialize();

            if (nullptr == importTable.named.timeSetEvent)
                LogMissingFunctionCalled(L"timeSetEvent");

            return importTable.named.timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
        }

        // ---------

        MMRESULT waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.named.waveInAddBuffer)
                LogMissingFunctionCalled(L"waveInAddBuffer");

            return importTable.named.waveInAddBuffer(hwi, pwh, cbwh);
        }

        // ---------

        MMRESULT waveInClose(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.named.waveInClose)
                LogMissingFunctionCalled(L"waveInClose");

            return importTable.named.waveInClose(hwi);
        }

        // ---------

        MMRESULT waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetDevCapsA)
                LogMissingFunctionCalled(L"waveInGetDevCapsA");

            return importTable.named.waveInGetDevCapsA(uDeviceID, pwic, cbwic);
        }

        // ---------

        MMRESULT waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetDevCapsW)
                LogMissingFunctionCalled(L"waveInGetDevCapsW");

            return importTable.named.waveInGetDevCapsW(uDeviceID, pwic, cbwic);
        }

        // ---------

        MMRESULT waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetErrorTextA)
                LogMissingFunctionCalled(L"waveInGetErrorTextA");

            return importTable.named.waveInGetErrorTextA(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetErrorTextW)
                LogMissingFunctionCalled(L"waveInGetErrorTextW");

            return importTable.named.waveInGetErrorTextW(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetID)
                LogMissingFunctionCalled(L"waveInGetID");

            return importTable.named.waveInGetID(hwi, puDeviceID);
        }

        // ---------

        UINT waveInGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetNumDevs)
                LogMissingFunctionCalled(L"waveInGetNumDevs");

            return importTable.named.waveInGetNumDevs();
        }

        // ---------

        MMRESULT waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.named.waveInGetPosition)
                LogMissingFunctionCalled(L"waveInGetPosition");

            return importTable.named.waveInGetPosition(hwi, pmmt, cbmmt);
        }

        // ---------

        DWORD waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.named.waveInMessage)
                LogMissingFunctionCalled(L"waveInMessage");

            return importTable.named.waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.named.waveInOpen)
                LogMissingFunctionCalled(L"waveInOpen");

            return importTable.named.waveInOpen(phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
        }

        // ---------

        MMRESULT waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.named.waveInPrepareHeader)
                LogMissingFunctionCalled(L"waveInPrepareHeader");

            return importTable.named.waveInPrepareHeader(hwi, pwh, cbwh);
        }

        // ---------

        MMRESULT waveInReset(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.named.waveInReset)
                LogMissingFunctionCalled(L"waveInReset");

            return importTable.named.waveInReset(hwi);
        }

        // ---------

        MMRESULT waveInStart(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.named.waveInStart)
                LogMissingFunctionCalled(L"waveInStart");

            return importTable.named.waveInStart(hwi);
        }

        // ---------

        MMRESULT waveInStop(HWAVEIN hwi)
        {
            Initialize();

            if (nullptr == importTable.named.waveInStop)
                LogMissingFunctionCalled(L"waveInStop");

            return importTable.named.waveInStop(hwi);
        }

        // ---------

        MMRESULT waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.named.waveInUnprepareHeader)
                LogMissingFunctionCalled(L"waveInUnprepareHeader");

            return importTable.named.waveInUnprepareHeader(hwi, pwh, cbwh);
        }

        // ---------

        MMRESULT waveOutBreakLoop(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutBreakLoop)
                LogMissingFunctionCalled(L"waveOutBreakLoop");

            return importTable.named.waveOutBreakLoop(hwo);
        }

        // ---------

        MMRESULT waveOutClose(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutClose)
                LogMissingFunctionCalled(L"waveOutClose");

            return importTable.named.waveOutClose(hwo);
        }

        // ---------

        MMRESULT waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetDevCapsA)
                LogMissingFunctionCalled(L"waveOutGetDevCapsA");

            return importTable.named.waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
        }

        // ---------

        MMRESULT waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetDevCapsW)
                LogMissingFunctionCalled(L"waveOutGetDevCapsW");

            return importTable.named.waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
        }

        // ---------

        MMRESULT waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetErrorTextA)
                LogMissingFunctionCalled(L"waveOutGetErrorTextA");

            return importTable.named.waveOutGetErrorTextA(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetErrorTextW)
                LogMissingFunctionCalled(L"waveOutGetErrorTextW");

            return importTable.named.waveOutGetErrorTextW(mmrError, pszText, cchText);
        }

        // ---------

        MMRESULT waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetID)
                LogMissingFunctionCalled(L"waveOutGetID");

            return importTable.named.waveOutGetID(hwo, puDeviceID);
        }

        // ---------

        UINT waveOutGetNumDevs(void)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetNumDevs)
                LogMissingFunctionCalled(L"waveOutGetNumDevs");

            return importTable.named.waveOutGetNumDevs();
        }

        // ---------

        MMRESULT waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetPitch)
                LogMissingFunctionCalled(L"waveOutGetPitch");

            return importTable.named.waveOutGetPitch(hwo, pdwPitch);
        }

        // ---------

        MMRESULT waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetPlaybackRate)
                LogMissingFunctionCalled(L"waveOutGetPlaybackRate");

            return importTable.named.waveOutGetPlaybackRate(hwo, pdwRate);
        }

        // ---------

        MMRESULT waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetPosition)
                LogMissingFunctionCalled(L"waveOutGetPosition");

            return importTable.named.waveOutGetPosition(hwo, pmmt, cbmmt);
        }

        // ---------

        MMRESULT waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutGetVolume)
                LogMissingFunctionCalled(L"waveOutGetVolume");

            return importTable.named.waveOutGetVolume(hwo, pdwVolume);
        }

        // ---------

        DWORD waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutMessage)
                LogMissingFunctionCalled(L"waveOutMessage");

            return importTable.named.waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
        }

        // ---------

        MMRESULT waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutOpen)
                LogMissingFunctionCalled(L"waveOutOpen");

            return importTable.named.waveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
        }

        // ---------

        MMRESULT waveOutPause(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutPause)
                LogMissingFunctionCalled(L"waveOutPause");

            return importTable.named.waveOutPause(hwo);
        }

        // ---------

        MMRESULT waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutPrepareHeader)
                LogMissingFunctionCalled(L"waveOutPrepareHeader");

            return importTable.named.waveOutPrepareHeader(hwo, pwh, cbwh);
        }

        // ---------

        MMRESULT waveOutReset(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutReset)
                LogMissingFunctionCalled(L"waveOutReset");

            return importTable.named.waveOutReset(hwo);
        }

        // ---------

        MMRESULT waveOutRestart(HWAVEOUT hwo)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutRestart)
                LogMissingFunctionCalled(L"waveOutRestart");

            return importTable.named.waveOutRestart(hwo);
        }

        // ---------

        MMRESULT waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutSetPitch)
                LogMissingFunctionCalled(L"waveOutSetPitch");

            return importTable.named.waveOutSetPitch(hwo, dwPitch);
        }

        // ---------

        MMRESULT waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutSetPlaybackRate)
                LogMissingFunctionCalled(L"waveOutSetPlaybackRate");

            return importTable.named.waveOutSetPlaybackRate(hwo, dwRate);
        }

        // ---------

        MMRESULT waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutSetVolume)
                LogMissingFunctionCalled(L"waveOutSetVolume");

            return importTable.named.waveOutSetVolume(hwo, dwVolume);
        }

        // ---------

        MMRESULT waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutUnprepareHeader)
                LogMissingFunctionCalled(L"waveOutUnprepareHeader");

            return importTable.named.waveOutUnprepareHeader(hwo, pwh, cbwh);
        }

        // ---------

        MMRESULT waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
        {
            Initialize();

            if (nullptr == importTable.named.waveOutWrite)
                LogMissingFunctionCalled(L"waveOutWrite");

            return importTable.named.waveOutWrite(hwo, pwh, cbwh);
        }


        // -------- XIDI API ----------------------------------------------- //

        /// Implements the Xidi API interface #IImportFunctions.
        /// Allows joystick WinMM functions to be replaced.
        class JoystickFunctionReplacer : public Api::IImportFunctions
        {
        private:
            // -------- CLASS VARIABLES ------------------------------------ //

            /// Maps from replaceable joystick function name to array index in the import table.
            static const std::map<std::wstring_view, size_t> kReplaceableFunctions;


        public:
            // -------- CONCRETE INSTANCE METHODS -------------------------- //
            // See "ApiXidi.h" for documentation.

            virtual const std::set<std::wstring_view>& GetReplaceable(void) const
            {
                static std::set<std::wstring_view> initSet;
                static std::once_flag initFlag;

                std::call_once(initFlag, []() -> void
                    {
                        for (auto replaceableFunction : kReplaceableFunctions)
                            initSet.insert(replaceableFunction.first);
                    }
                );

                return initSet;
            }

            virtual size_t SetReplaceable(const std::map<std::wstring_view, const void*>& importFunctionTable)
            {
                Initialize();

                const std::wstring_view kLibraryPath = GetImportLibraryPathWinMM();
                size_t numReplaced = 0;

                for (auto newImportFunction : importFunctionTable)
                {
                    if (true == kReplaceableFunctions.contains(newImportFunction.first))
                    {
                        Message::OutputFormatted(Message::ESeverity::Debug, L"Import function \"%s\" has been replaced.", newImportFunction.first.data());
                        importTable.ptr[kReplaceableFunctions.at(newImportFunction.first)] = newImportFunction.second;
                        numReplaced += 1;
                    }
                }

                if (numReplaced > 0)
                    Message::OutputFormatted(Message::ESeverity::Warning, L"%d function(s) previously imported from %s have been replaced. Previously imported versions will not be used.", (int)numReplaced, kLibraryPath.data());

                return numReplaced;
            }
        };

        /// Maps from replaceable import function name to its pointer's positional index in the import table.
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
            {L"joySetThreshold", IMPORT_TABLE_INDEX_OF(joySetThreshold)}
        };

        /// Singleton Xidi API implementation object.
        static JoystickFunctionReplacer joystickFunctionReplacer;
    }
}
