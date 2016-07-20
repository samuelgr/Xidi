/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ImportApiWinMM.cpp
 *      Implementation of importing the API from the WinMM library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "ImportApiWinMM.h"

using namespace Xidi;


// -------- CONSTANTS ------------------------------------------------------ //
// See "ImportApiWinMM.h" for documentation.

const TCHAR* const ImportApiWinMM::kWinMMLibraryName = _T("\\winmm.dll");
const DWORD ImportApiWinMM::kWinMMLibraryLength = 10;


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
        // A path must be specified directly since the system has already loaded this DLL of the same name.
        TCHAR libraryName[1024];
        GetSystemDirectory(libraryName, 512);
        _tcsncat_s(libraryName, _countof(libraryName), kWinMMLibraryName, kWinMMLibraryLength);

        // Attempt to load the library.
        HMODULE loadedLibrary = LoadLibraryEx(libraryName, NULL, 0);
        if (NULL == loadedLibrary) return MMSYSERR_ERROR;

        // Attempt to obtain the addresses of all imported API functions.
        FARPROC procAddress = NULL;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.auxGetDevCapsA = (MMRESULT(WINAPI *)(UINT_PTR, LPAUXCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.auxGetDevCapsW = (MMRESULT(WINAPI *)(UINT_PTR, LPAUXCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.auxGetNumDevs = (UINT(WINAPI *)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.auxGetVolume = (MMRESULT(WINAPI *)(UINT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.auxOutMessage = (MMRESULT(WINAPI *)(UINT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.auxSetVolume = (MMRESULT(WINAPI *)(UINT, DWORD))procAddress;

        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyConfigChanged = (MMRESULT(WINAPI *)(DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyGetDevCapsA = (MMRESULT(WINAPI *)(UINT_PTR, LPJOYCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyGetDevCapsW = (MMRESULT(WINAPI *)(UINT_PTR, LPJOYCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyGetNumDevs = (UINT(WINAPI *)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyGetPos = (MMRESULT(WINAPI *)(UINT, LPJOYINFO))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyGetPosEx = (MMRESULT(WINAPI *)(UINT, LPJOYINFOEX))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyGetThreshold = (MMRESULT(WINAPI *)(UINT, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joyReleaseCapture = (MMRESULT(WINAPI *)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joySetCapture = (MMRESULT(WINAPI *)(HWND, UINT, UINT, BOOL))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.joySetThreshold = (MMRESULT(WINAPI *)(UINT, UINT))procAddress;

        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeBeginPeriod = (MMRESULT(WINAPI *)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeEndPeriod = (MMRESULT(WINAPI *)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeGetDevCaps = (MMRESULT(WINAPI *)(LPTIMECAPS, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeGetSystemTime = (MMRESULT(WINAPI *)(LPMMTIME, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeGetTime = (DWORD(WINAPI *)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeKillEvent = (MMRESULT(WINAPI *)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
        if (NULL == procAddress) return MMSYSERR_ERROR;
        importTable.timeSetEvent = (MMRESULT(WINAPI *)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT))procAddress;

        // Initialization complete.
        importTableIsInitialized = TRUE;
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
