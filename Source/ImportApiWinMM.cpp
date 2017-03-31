/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * ImportApiWinMM.cpp
 *      Implementation of importing the API from the WinMM library.
 *****************************************************************************/

#include "ApiStdString.h"
#include "ApiWindows.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "Log.h"

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "ImportApiWinMM.h" for documentation.

ImportApiWinMM::SImportTable ImportApiWinMM::importTable;
BOOL ImportApiWinMM::importTableIsInitialized = FALSE;


// -------- CLASS METHODS -------------------------------------------------- //
// See "ImportApiWinMM.h" for documentation.

MMRESULT ImportApiWinMM::Initialize(void)
{
    if (FALSE == importTableIsInitialized)
    {
        // Initialize the import table.
        ZeroMemory(&importTable, sizeof(importTable));
        
        // Obtain the full library path string.
        StdString libraryPath;
        Globals::FillWinMMLibraryPath(libraryPath);
        
        // Attempt to load the library.
        LogInitializeLibraryPath(libraryPath.c_str());
        HMODULE loadedLibrary = LoadLibraryEx(libraryPath.c_str(), NULL, 0);
        if (NULL == loadedLibrary) return LogInitializeFailed();
        
        // Attempt to obtain the addresses of all imported API functions.
        FARPROC procAddress = NULL;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.auxGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.auxGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.auxGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.auxGetVolume = (MMRESULT(WINAPI*)(UINT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.auxOutMessage = (MMRESULT(WINAPI*)(UINT, UINT, DWORD_PTR, DWORD_PTR))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.auxSetVolume = (MMRESULT(WINAPI*)(UINT, DWORD))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyConfigChanged = (MMRESULT(WINAPI*)(DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyGetPos = (MMRESULT(WINAPI*)(UINT, LPJOYINFO))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyGetPosEx = (MMRESULT(WINAPI*)(UINT, LPJOYINFOEX))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyGetThreshold = (MMRESULT(WINAPI*)(UINT, LPUINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joyReleaseCapture = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joySetCapture = (MMRESULT(WINAPI*)(HWND, UINT, UINT, BOOL))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.joySetThreshold = (MMRESULT(WINAPI*)(UINT, UINT))procAddress;
        
        // ---------

        procAddress = GetProcAddress(loadedLibrary, "mmioAdvance");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioAdvance = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioAscend");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioAscend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioClose");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioClose = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioCreateChunk");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioCreateChunk = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioDescend");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioDescend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioFlush");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioFlush = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioGetInfo");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioGetInfo = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioInstallIOProcA = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioInstallIOProcW = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioOpenA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioOpenA = (HMMIO(WINAPI*)(LPSTR, LPMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioOpenW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioOpenW = (HMMIO(WINAPI*)(LPWSTR, LPMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioRead");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioRead = (LONG(WINAPI*)(HMMIO, HPSTR, LONG))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioRenameA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioRenameA = (MMRESULT(WINAPI*)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioRenameW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioRenameW = (MMRESULT(WINAPI*)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSeek");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioSeek = (LONG(WINAPI*)(HMMIO, LONG, int))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSendMessage");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioSendMessage = (LRESULT(WINAPI*)(HMMIO, UINT, LPARAM, LPARAM))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSetBuffer");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioSetBuffer = (MMRESULT(WINAPI*)(HMMIO, LPSTR, LONG, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSetInfo");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioSetInfo = (MMRESULT(WINAPI*)(HMMIO, LPCMMIOINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioStringToFOURCCA = (FOURCC(WINAPI*)(LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioStringToFOURCCW = (FOURCC(WINAPI*)(LPCWSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioWrite");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.mmioWrite = (LONG(WINAPI*)(HMMIO, const char*, LONG))procAddress;
        
        // ---------

        procAddress = GetProcAddress(loadedLibrary, "PlaySoundA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.PlaySoundA = (BOOL(WINAPI*)(LPCSTR, HMODULE, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "PlaySoundW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.PlaySoundW = (BOOL(WINAPI*)(LPCWSTR, HMODULE, DWORD))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.sndPlaySoundA = (BOOL(WINAPI*)(LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.sndPlaySoundW = (BOOL(WINAPI*)(LPCWSTR, UINT))procAddress;
        
        // ---------

        procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeBeginPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeEndPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeGetDevCaps = (MMRESULT(WINAPI*)(LPTIMECAPS, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeGetSystemTime = (MMRESULT(WINAPI*)(LPMMTIME, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeGetTime = (DWORD(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeKillEvent = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.timeSetEvent = (MMRESULT(WINAPI*)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "waveInAddBuffer");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInAddBuffer = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInClose");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInClose = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetID");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetID = (MMRESULT(WINAPI*)(HWAVEIN, LPUINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetNumDevs");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetPosition");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInGetPosition = (MMRESULT(WINAPI*)(HWAVEIN, LPMMTIME, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInMessage");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInMessage = (DWORD(WINAPI*)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInOpen");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInOpen = (MMRESULT(WINAPI*)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInPrepareHeader");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInPrepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInReset");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInReset = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInStart");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInStart = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInStop");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInStop = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInUnprepareHeader");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveInUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutBreakLoop");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutBreakLoop = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutClose");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutClose = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextA");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextW");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetID");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetID = (MMRESULT(WINAPI*)(HWAVEOUT, LPUINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetNumDevs");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPitch");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPlaybackRate");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPosition");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetPosition = (MMRESULT(WINAPI*)(HWAVEOUT, LPMMTIME, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetVolume");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutGetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutMessage");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutMessage = (DWORD(WINAPI*)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutOpen");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutOpen = (MMRESULT(WINAPI*)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutPause");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutPause = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutPrepareHeader");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutPrepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutReset");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutReset = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutRestart");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutRestart = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutSetPitch");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutSetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutSetPlaybackRate");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutSetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutSetVolume");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutSetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutUnprepareHeader");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutWrite");
        if (NULL == procAddress) return LogInitializeFailed();
        importTable.waveOutWrite = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;
        
        // Initialization complete.
        importTableIsInitialized = TRUE;
        LogInitializeSucceeded();
    }

    return MMSYSERR_NOERROR;
}

// ---------

MMRESULT ImportApiWinMM::auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
}

// ---------

MMRESULT ImportApiWinMM::auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
}

// ---------

UINT ImportApiWinMM::auxGetNumDevs(void)
{
    if (MMSYSERR_NOERROR != Initialize())
        return 0;

    return importTable.auxGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.auxGetVolume(uDeviceID, lpdwVolume);
}

// ---------

MMRESULT ImportApiWinMM::auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::auxSetVolume(UINT uDeviceID, DWORD dwVolume)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.auxSetVolume(uDeviceID, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::joyConfigChanged(DWORD dwFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyConfigChanged(dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyGetDevCapsA(uJoyID, pjc, cbjc);
}

// ---------

MMRESULT ImportApiWinMM::joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyGetDevCapsW(uJoyID, pjc, cbjc);
}

// ---------

UINT ImportApiWinMM::joyGetNumDevs(void)
{
    if (MMSYSERR_NOERROR != Initialize())
        return 0;

    return importTable.joyGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::joyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyGetPos(uJoyID, pji);
}

// ---------

MMRESULT ImportApiWinMM::joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyGetPosEx(uJoyID, pji);
}

// ---------

MMRESULT ImportApiWinMM::joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyGetThreshold(uJoyID, puThreshold);
}

// ---------

MMRESULT ImportApiWinMM::joyReleaseCapture(UINT uJoyID)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joyReleaseCapture(uJoyID);
}

// ---------

MMRESULT ImportApiWinMM::joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
}

// ---------

MMRESULT ImportApiWinMM::joySetThreshold(UINT uJoyID, UINT uThreshold)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.joySetThreshold(uJoyID, uThreshold);
}

// ---------

MMRESULT ImportApiWinMM::mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioAdvance(hmmio, lpmmioinfo, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioAscend(hmmio, lpck, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioClose(HMMIO hmmio, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioClose(hmmio, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioCreateChunk(hmmio, lpck, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioDescend(hmmio, lpck, lpckParent, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioFlush(HMMIO hmmio, UINT fuFlush)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioFlush(hmmio, fuFlush);
}

// ---------


MMRESULT ImportApiWinMM::mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioGetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


LPMMIOPROC ImportApiWinMM::mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return NULL;

    return importTable.mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
}

// ---------


LPMMIOPROC ImportApiWinMM::mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return NULL;

    return importTable.mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
}

// ---------


HMMIO ImportApiWinMM::mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return NULL;

    return importTable.mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


HMMIO ImportApiWinMM::mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return NULL;

    return importTable.mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


LONG ImportApiWinMM::mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
{
    if (MMSYSERR_NOERROR != Initialize())
        return -1;

    return importTable.mmioRead(hmmio, pch, cch);
}

// ---------


MMRESULT ImportApiWinMM::mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


LONG ImportApiWinMM::mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
{
    if (MMSYSERR_NOERROR != Initialize())
        return -1;

    return importTable.mmioSeek(hmmio, lOffset, iOrigin);
}

// ---------


LRESULT ImportApiWinMM::mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
    if (MMSYSERR_NOERROR != Initialize())
        return 0;

    return importTable.mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
}

// ---------


MMRESULT ImportApiWinMM::mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.mmioSetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


FOURCC ImportApiWinMM::mmioStringToFOURCCA(LPCSTR sz, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return 0;

    return importTable.mmioStringToFOURCCA(sz, wFlags);
}

// ---------


FOURCC ImportApiWinMM::mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags)
{
    if (MMSYSERR_NOERROR != Initialize())
        return 0;

    return importTable.mmioStringToFOURCCW(sz, wFlags);
}

// ---------


LONG ImportApiWinMM::mmioWrite(HMMIO hmmio, const char* pch, LONG cch)
{
    if (MMSYSERR_NOERROR != Initialize())
        return -1;

    return importTable.mmioWrite(hmmio, pch, cch);
}

// ---------

BOOL ImportApiWinMM::PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    if (MMSYSERR_NOERROR != Initialize())
        return FALSE;

    return importTable.PlaySoundA(pszSound, hmod, fdwSound);
}

// ---------

BOOL ImportApiWinMM::PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    if (MMSYSERR_NOERROR != Initialize())
        return FALSE;

    return importTable.PlaySoundW(pszSound, hmod, fdwSound);
}

// ---------

BOOL ImportApiWinMM::sndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
{
    if (MMSYSERR_NOERROR != Initialize())
        return FALSE;

    return importTable.sndPlaySoundA(lpszSound, fuSound);
}

// ---------

BOOL ImportApiWinMM::sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
{
    if (MMSYSERR_NOERROR != Initialize())
        return FALSE;

    return importTable.sndPlaySoundW(lpszSound, fuSound);
}

// ---------

MMRESULT ImportApiWinMM::timeBeginPeriod(UINT uPeriod)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.timeBeginPeriod(uPeriod);
}

// ---------

MMRESULT ImportApiWinMM::timeEndPeriod(UINT uPeriod)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.timeEndPeriod(uPeriod);
}

// ---------

MMRESULT ImportApiWinMM::timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.timeGetDevCaps(ptc, cbtc);
}

// ---------

MMRESULT ImportApiWinMM::timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.timeGetSystemTime(pmmt, cbmmt);
}

// ---------

DWORD ImportApiWinMM::timeGetTime(void)
{
    if (MMSYSERR_NOERROR != Initialize())
        return 0;

    return importTable.timeGetTime();
}

// ---------

MMRESULT ImportApiWinMM::timeKillEvent(UINT uTimerID)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.timeKillEvent(uTimerID);
}

// ---------

MMRESULT ImportApiWinMM::timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
}

// ---------

MMRESULT ImportApiWinMM::waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInAddBuffer(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveInClose(HWAVEIN hwi)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInClose(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetDevCapsA(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetDevCapsW(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetID(hwi, puDeviceID);
}

// ---------

UINT ImportApiWinMM::waveInGetNumDevs(void)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInGetPosition(hwi, pmmt, cbmmt);
}

// ---------

DWORD ImportApiWinMM::waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInOpen(phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInPrepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveInReset(HWAVEIN hwi)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInReset(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInStart(HWAVEIN hwi)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInStart(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInStop(HWAVEIN hwi)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInStop(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveInUnprepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutBreakLoop(HWAVEOUT hwo)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutBreakLoop(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutClose(HWAVEOUT hwo)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutClose(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetID(hwo, puDeviceID);
}

// ---------

UINT ImportApiWinMM::waveOutGetNumDevs(void)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetPitch(hwo, pdwPitch);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetPlaybackRate(hwo, pdwRate);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetPosition(hwo, pmmt, cbmmt);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutGetVolume(hwo, pdwVolume);
}

// ---------

DWORD ImportApiWinMM::waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::waveOutPause(HWAVEOUT hwo)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutPause(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;

    return importTable.waveOutPrepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutReset(HWAVEOUT hwo)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutReset(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutRestart(HWAVEOUT hwo)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutRestart(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutSetPitch(hwo, dwPitch);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutSetPlaybackRate(hwo, dwRate);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutSetVolume(hwo, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutUnprepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    if (MMSYSERR_NOERROR != Initialize())
        return MMSYSERR_ERROR;
    
    return importTable.waveOutWrite(hwo, pwh, cbwh);
}


// -------- HELPERS -------------------------------------------------------- //
// See "ImportApiWinMM.h" for documentation.

void ImportApiWinMM::LogInitializeLibraryPath(LPCTSTR libraryPath)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelDebug, IDS_XIDI_IMPORTAPIWINMM_INIT_PATH_FORMAT, libraryPath);
}

// --------

MMRESULT ImportApiWinMM::LogInitializeFailed(void)
{
    Log::WriteLogMessageFromResource(ELogLevel::LogLevelError, IDS_XIDI_IMPORTAPIWINMM_INIT_FAILED);
    return MMSYSERR_ERROR;
}

// --------

void ImportApiWinMM::LogInitializeSucceeded(void)
{
    Log::WriteFormattedLogMessageFromResource(ELogLevel::LogLevelInfo, IDS_XIDI_IMPORTAPIWINMM_INIT_SUCCEEDED);
}
