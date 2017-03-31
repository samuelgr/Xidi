/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * ImportApiWinMM.h
 *      Declarations related to importing the API from the WinMM library.
 *****************************************************************************/

#pragma once

#include "ApiStdString.h"
#include "ApiWindows.h"


namespace Xidi
{
    // Enables access to the underlying system's WinMM API.
    // Dynamically loads the library and holds pointers to its methods.
    // Methods are intended to be called directly rather than through an instance.
    class ImportApiWinMM
    {
    public:
        // -------- TYPE DEFINITIONS ----------------------------------------------- //

        // Fields specify the addresses of the imported DirectInput API functions.
        struct SImportTable
        {
            MMRESULT    (WINAPI* auxGetDevCapsA)(UINT_PTR, LPAUXCAPSA, UINT);
            MMRESULT    (WINAPI* auxGetDevCapsW)(UINT_PTR, LPAUXCAPSW, UINT);
            UINT        (WINAPI* auxGetNumDevs)(void);
            MMRESULT    (WINAPI* auxGetVolume)(UINT, LPDWORD);
            MMRESULT    (WINAPI* auxOutMessage)(UINT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* auxSetVolume)(UINT, DWORD);

            MMRESULT    (WINAPI* joyConfigChanged)(DWORD);
            MMRESULT    (WINAPI* joyGetDevCapsA)(UINT_PTR, LPJOYCAPSA, UINT);
            MMRESULT    (WINAPI* joyGetDevCapsW)(UINT_PTR, LPJOYCAPSW, UINT);
            UINT        (WINAPI* joyGetNumDevs)(void);
            MMRESULT    (WINAPI* joyGetPos)(UINT, LPJOYINFO);
            MMRESULT    (WINAPI* joyGetPosEx)(UINT, LPJOYINFOEX);
            MMRESULT    (WINAPI* joyGetThreshold)(UINT, LPUINT);
            MMRESULT    (WINAPI* joyReleaseCapture)(UINT);
            MMRESULT    (WINAPI* joySetCapture)(HWND, UINT, UINT, BOOL);
            MMRESULT    (WINAPI* joySetThreshold)(UINT, UINT);

            MMRESULT    (WINAPI* midiConnect)(HMIDI, HMIDIOUT, LPVOID);
            MMRESULT    (WINAPI* midiDisconnect)(HMIDI, HMIDIOUT, LPVOID);
            
            MMRESULT    (WINAPI* mmioAdvance)(HMMIO, LPMMIOINFO, UINT);
            MMRESULT    (WINAPI* mmioAscend)(HMMIO, LPMMCKINFO, UINT);
            MMRESULT    (WINAPI* mmioClose)(HMMIO, UINT);
            MMRESULT    (WINAPI* mmioCreateChunk)(HMMIO, LPMMCKINFO, UINT);
            MMRESULT    (WINAPI* mmioDescend)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT);
            MMRESULT    (WINAPI* mmioFlush)(HMMIO, UINT);
            MMRESULT    (WINAPI* mmioGetInfo)(HMMIO, LPMMIOINFO, UINT);
            LPMMIOPROC  (WINAPI* mmioInstallIOProcA)(FOURCC, LPMMIOPROC, DWORD);
            LPMMIOPROC  (WINAPI* mmioInstallIOProcW)(FOURCC, LPMMIOPROC, DWORD);
            HMMIO       (WINAPI* mmioOpenA)(LPSTR, LPMMIOINFO, DWORD);
            HMMIO       (WINAPI* mmioOpenW)(LPWSTR, LPMMIOINFO, DWORD);
            LONG        (WINAPI* mmioRead)(HMMIO, HPSTR, LONG);
            MMRESULT    (WINAPI* mmioRenameA)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD);
            MMRESULT    (WINAPI* mmioRenameW)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD);
            LONG        (WINAPI* mmioSeek)(HMMIO, LONG, int);
            LRESULT     (WINAPI* mmioSendMessage)(HMMIO, UINT, LPARAM, LPARAM);
            MMRESULT    (WINAPI* mmioSetBuffer)(HMMIO, LPSTR, LONG, UINT);
            MMRESULT    (WINAPI* mmioSetInfo)(HMMIO, LPCMMIOINFO, UINT);
            FOURCC      (WINAPI* mmioStringToFOURCCA)(LPCSTR, UINT);
            FOURCC      (WINAPI* mmioStringToFOURCCW)(LPCWSTR, UINT);
            LONG        (WINAPI* mmioWrite)(HMMIO, const char*, LONG);
            
            BOOL        (WINAPI* PlaySoundA)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
            BOOL        (WINAPI* PlaySoundW)(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
            
            BOOL        (WINAPI* sndPlaySoundA)(LPCSTR lpszSound, UINT fuSound);
            BOOL        (WINAPI* sndPlaySoundW)(LPCWSTR lpszSound, UINT fuSound);
            
            MMRESULT    (WINAPI* timeBeginPeriod)(UINT);
            MMRESULT    (WINAPI* timeEndPeriod)(UINT);
            MMRESULT    (WINAPI* timeGetDevCaps)(LPTIMECAPS, UINT);
            MMRESULT    (WINAPI* timeGetSystemTime)(LPMMTIME, UINT);
            DWORD       (WINAPI* timeGetTime)(void);
            MMRESULT    (WINAPI* timeKillEvent)(UINT);
            MMRESULT    (WINAPI* timeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);

            MMRESULT    (WINAPI* waveInAddBuffer)(HWAVEIN, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveInClose)(HWAVEIN);
            MMRESULT    (WINAPI* waveInGetDevCapsA)(UINT_PTR, LPWAVEINCAPSA, UINT);
            MMRESULT    (WINAPI* waveInGetDevCapsW)(UINT_PTR, LPWAVEINCAPSW, UINT);
            MMRESULT    (WINAPI* waveInGetErrorTextA)(MMRESULT, LPCSTR, UINT);
            MMRESULT    (WINAPI* waveInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT    (WINAPI* waveInGetID)(HWAVEIN, LPUINT);
            UINT        (WINAPI* waveInGetNumDevs)(void);
            MMRESULT    (WINAPI* waveInGetPosition)(HWAVEIN, LPMMTIME, UINT);
            DWORD       (WINAPI* waveInMessage)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* waveInOpen)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* waveInPrepareHeader)(HWAVEIN, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveInReset)(HWAVEIN);
            MMRESULT    (WINAPI* waveInStart)(HWAVEIN);
            MMRESULT    (WINAPI* waveInStop)(HWAVEIN);
            MMRESULT    (WINAPI* waveInUnprepareHeader)(HWAVEIN, LPWAVEHDR, UINT);

            MMRESULT    (WINAPI* waveOutBreakLoop)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutClose)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutGetDevCapsA)(UINT_PTR, LPWAVEOUTCAPSA, UINT);
            MMRESULT    (WINAPI* waveOutGetDevCapsW)(UINT_PTR, LPWAVEOUTCAPSW, UINT);
            MMRESULT    (WINAPI* waveOutGetErrorTextA)(MMRESULT, LPCSTR, UINT);
            MMRESULT    (WINAPI* waveOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT    (WINAPI* waveOutGetID)(HWAVEOUT, LPUINT);
            UINT        (WINAPI* waveOutGetNumDevs)(void);
            MMRESULT    (WINAPI* waveOutGetPitch)(HWAVEOUT, LPDWORD);
            MMRESULT    (WINAPI* waveOutGetPlaybackRate)(HWAVEOUT, LPDWORD);
            MMRESULT    (WINAPI* waveOutGetPosition)(HWAVEOUT, LPMMTIME, UINT);
            MMRESULT    (WINAPI* waveOutGetVolume)(HWAVEOUT, LPDWORD);
            DWORD       (WINAPI* waveOutMessage)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* waveOutOpen)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* waveOutPause)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutPrepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveOutReset)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutRestart)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutSetPitch)(HWAVEOUT, DWORD);
            MMRESULT    (WINAPI* waveOutSetPlaybackRate)(HWAVEOUT, DWORD);
            MMRESULT    (WINAPI* waveOutSetVolume)(HWAVEOUT, DWORD);
            MMRESULT    (WINAPI* waveOutUnprepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveOutWrite)(HWAVEOUT, LPWAVEHDR, UINT);
        };
        
        
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Holds the imported DirectInput API function addresses.
        static SImportTable importTable;

        // Specifies whether or not the import table has been initialized.
        static BOOL importTableIsInitialized;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        ImportApiWinMM();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Dynamically loads the WinMM library and sets up all imported function calls.
        // Returns MMSYSERR_NOERROR on success and MMSYSERR_ERROR on failure.
        static MMRESULT Initialize(void);

        // Calls the imported function auxDevGetCapsA.
        static MMRESULT auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);

        // Calls the imported function auxDevGetCapsW.
        static MMRESULT auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);

        // Calls the imported function auxGetNumDevs.
        static UINT auxGetNumDevs(void);

        // Calls the imported function auxGetVolume.
        static MMRESULT auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);

        // Calls the imported function auxOutMessage.
        static MMRESULT auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        // Calls the imported function auxSetVolume.
        static MMRESULT auxSetVolume(UINT uDeviceID, DWORD dwVolume);
        
        // Calls the imported function joyConfigChanged.
        static MMRESULT joyConfigChanged(DWORD dwFlags);

        // Calls the imported function joyGetDevCapsA.
        static MMRESULT joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);

        // Calls the imported function joyGetDevCapsW.
        static MMRESULT joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);

        // Calls the imported function joyGetNumDevs.
        static UINT joyGetNumDevs(void);

        // Calls the imported function joyGetPos.
        static MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji);

        // Calls the imported function joyGetPosEx.
        static MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);

        // Calls the imported function joyGetThreshold.
        static MMRESULT joyGetThreshold(UINT uJoyID, LPUINT puThreshold);

        // Calls the imported function joyReleaseCapture.
        static MMRESULT joyReleaseCapture(UINT uJoyID);

        // Calls the imported function joySetCapture.
        static MMRESULT joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);

        // Calls the imported function joySetThreshold.
        static MMRESULT joySetThreshold(UINT uJoyID, UINT uThreshold);

        // Calls the imported function midiConnect.
        static MMRESULT midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);

        // Calls the imported function midiDisconnect.
        static MMRESULT midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);

        // Calls the imported function mmioAdvance.
        static MMRESULT mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);

        // Calls the imported function mmioAscend.
        static MMRESULT mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);

        // Calls the imported function mmioClose.
        static MMRESULT mmioClose(HMMIO hmmio, UINT wFlags);

        // Calls the imported function mmioCreateChunk.
        static MMRESULT mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);

        // Calls the imported function mmioDescend.
        static MMRESULT mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags);

        // Calls the imported function mmioFlush.
        static MMRESULT mmioFlush(HMMIO hmmio, UINT fuFlush);

        // Calls the imported function mmioGetInfo.
        static MMRESULT mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);

        // Calls the imported function mmioInstallIOProcA.
        static LPMMIOPROC mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);

        // Calls the imported function mmioInstallIOProcW.
        static LPMMIOPROC mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);

        // Calls the imported function mmioOpenA.
        static HMMIO mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);

        // Calls the imported function mmioOpenW.
        static HMMIO mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);

        // Calls the imported function mmioRead.
        static LONG mmioRead(HMMIO hmmio, HPSTR pch, LONG cch);

        // Calls the imported function mmioRenameA.
        static MMRESULT mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);

        // Calls the imported function mmioRenameW.
        static MMRESULT mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);

        // Calls the imported function mmioSeek.
        static LONG mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin);

        // Calls the imported function mmioSendMessage.
        static LRESULT mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2);

        // Calls the imported function mmioSetBuffer.
        static MMRESULT mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags);

        // Calls the imported function mmioSetInfo.
        static MMRESULT mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags);

        // Calls the imported function mmioStringToFOURCCA.
        static FOURCC mmioStringToFOURCCA(LPCSTR sz, UINT wFlags);

        // Calls the imported function mmioStringToFOURCCW.
        static FOURCC mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags);

        // Calls the imported function mmioWrite.
        static LONG mmioWrite(HMMIO hmmio, const char* pch, LONG cch);

        // Calls the imported function PlaySoundA.
        static BOOL PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
        
        // Calls the imported function PlaySoundW.
        static BOOL PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
        
        // Calls the imported function sndPlaySoundA.
        static BOOL sndPlaySoundA(LPCSTR lpszSound, UINT fuSound);
        
        // Calls the imported function sndPlaySoundW.
        static BOOL sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound);
        
        // Calls the imported function timeBeginPeriod.
        static MMRESULT timeBeginPeriod(UINT uPeriod);

        // Calls the imported function timeEndPeriod.
        static MMRESULT timeEndPeriod(UINT uPeriod);

        // Calls the imported function timeGetDevCaps.
        static MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);

        // Calls the imported function timeGetSystemTime.
        static MMRESULT timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);

        // Calls the imported function timeGetTime.
        static DWORD timeGetTime(void);

        // Calls the imported function timeKillEvent.
        static MMRESULT timeKillEvent(UINT uTimerID);

        // Calls the imported function timeSetEvent.
        static MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);

        // Calls the imported function waveInAddBuffer.
        static MMRESULT waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

        // Calls the imported function waveInClose.
        static MMRESULT waveInClose(HWAVEIN hwi);

        // Calls the imported function waveInGetDevCapsA.
        static MMRESULT waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);

        // Calls the imported function waveInGetDevCapsW.
        static MMRESULT waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);

        // Calls the imported function waveInGetErrorTextA.
        static MMRESULT waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);

        // Calls the imported function waveInGetErrorTextW.
        static MMRESULT waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);

        // Calls the imported function waveInGetID.
        static MMRESULT waveInGetID(HWAVEIN hwi, LPUINT puDeviceID);

        // Calls the imported function waveInGetNumDevs.
        static UINT waveInGetNumDevs(void);

        // Calls the imported function waveInGetPosition.
        static MMRESULT waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);

        // Calls the imported function waveInMessage.
        static DWORD waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        // Calls the imported function waveInOpen.
        static MMRESULT waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);

        // Calls the imported function waveInPrepareHeader.
        static MMRESULT waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

        // Calls the imported function waveInReset.
        static MMRESULT waveInReset(HWAVEIN hwi);

        // Calls the imported function waveInStart.
        static MMRESULT waveInStart(HWAVEIN hwi);

        // Calls the imported function waveInStop.
        static MMRESULT waveInStop(HWAVEIN hwi);

        // Calls the imported function waveInUnprepareHeader.
        static MMRESULT waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

        // Calls the imported function waveOutBreakLoop.
        static MMRESULT waveOutBreakLoop(HWAVEOUT hwo);

        // Calls the imported function waveOutClose.
        static MMRESULT waveOutClose(HWAVEOUT hwo);

        // Calls the imported function waveOutGetDevCapsA.
        static MMRESULT waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);

        // Calls the imported function waveOutGetDevCapsW.
        static MMRESULT waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);

        // Calls the imported function waveOutGetErrorTextA.
        static MMRESULT waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);

        // Calls the imported function waveOutGetErrorTextW.
        static MMRESULT waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);

        // Calls the imported function waveOutGetID.
        static MMRESULT waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);

        // Calls the imported function waveOutGetNumDevs.
        static UINT waveOutGetNumDevs(void);

        // Calls the imported function waveOutGetPitch.
        static MMRESULT waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);

        // Calls the imported function waveOutGetPlaybackRate.
        static MMRESULT waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);

        // Calls the imported function waveOutGetPosition.
        static MMRESULT waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);

        // Calls the imported function waveOutGetVolume.
        static MMRESULT waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);

        // Calls the imported function waveOutMessage.
        static DWORD waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        // Calls the imported function waveOutOpen.
        static MMRESULT waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);

        // Calls the imported function waveOutPause.
        static MMRESULT waveOutPause(HWAVEOUT hwo);

        // Calls the imported function waveOutPrepareHeader.
        static MMRESULT waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

        // Calls the imported function waveOutReset.
        static MMRESULT waveOutReset(HWAVEOUT hwo);

        // Calls the imported function waveOutRestart.
        static MMRESULT waveOutRestart(HWAVEOUT hwo);

        // Calls the imported function waveOutSetPitch.
        static MMRESULT waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);

        // Calls the imported function waveOutSetPlaybackRate.
        static MMRESULT waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);

        // Calls the imported function waveOutSetVolume.
        static MMRESULT waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);

        // Calls the imported function waveOutUnprepareHeader.
        static MMRESULT waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

        // Calls the imported function waveOutWrite.
        static MMRESULT waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
        
        
    private:
        // -------- HELPERS -------------------------------------------------------- //

        // Logs a debug event related to attempting to load the system-provided library for importing functions.
        static void LogInitializeLibraryPath(LPCTSTR libraryPath);
        
        // Logs an error event related to failure to initialize the import table.
        // Returns MMSYSERR_ERROR unconditionally.
        static MMRESULT LogInitializeFailed(void);

        // Logs an informational event related to successful initialization of the import table.
        static void LogInitializeSucceeded(void);
    };
}
