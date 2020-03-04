/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ImportApiWinMM.cpp
 *   Implementation of importing the API from the WinMM library.
 *****************************************************************************/

#include "ApiWindows.h"
#include "Globals.h"
#include "ImportApiWinMM.h"
#include "Log.h"

#include <string>

using namespace Xidi;


// -------- CLASS VARIABLES ------------------------------------------------ //
// See "ImportApiWinMM.h" for documentation.

ImportApiWinMM::SImportTable ImportApiWinMM::importTable;
BOOL ImportApiWinMM::importTableIsInitialized = FALSE;


// -------- CLASS METHODS -------------------------------------------------- //
// See "ImportApiWinMM.h" for documentation.

void ImportApiWinMM::Initialize(void)
{
    if (FALSE == importTableIsInitialized)
    {
        // Initialize the import table.
        ZeroMemory(&importTable, sizeof(importTable));
        
        // Obtain the full library path string.
        std::wstring libraryPath;
        Globals::FillWinMMLibraryPath(libraryPath);
        
        // Attempt to load the library.
        LogInitializeLibraryPath(libraryPath.c_str());
        HMODULE loadedLibrary = LoadLibraryEx(libraryPath.c_str(), NULL, 0);
        if (NULL == loadedLibrary)
        {
            LogInitializeFailed();
            return;
        }
        
        // Attempt to obtain the addresses of all imported API functions.
        FARPROC procAddress = NULL;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "CloseDriver");
        if (NULL == procAddress) LogImportFailed(_T("CloseDriver"));
        importTable.CloseDriver = (LRESULT(WINAPI*)(HDRVR, LPARAM, LPARAM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DefDriverProc");
        if (NULL == procAddress) LogImportFailed(_T("DefDriverProc"));
        importTable.DefDriverProc = (LRESULT(WINAPI*)(DWORD_PTR, HDRVR, UINT, LONG, LONG))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DriverCallback");
        if (NULL == procAddress) LogImportFailed(_T("DriverCallback"));
        importTable.DriverCallback = (BOOL(WINAPI*)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DrvGetModuleHandle");
        if (NULL == procAddress) LogImportFailed(_T("DrvGetModuleHandle"));
        importTable.DrvGetModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "GetDriverModuleHandle");
        if (NULL == procAddress) LogImportFailed(_T("GetDriverModuleHandle"));
        importTable.GetDriverModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "OpenDriver");
        if (NULL == procAddress) LogImportFailed(_T("OpenDriver"));
        importTable.OpenDriver = (HDRVR(WINAPI*)(LPCWSTR, LPCWSTR, LPARAM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "PlaySoundA");
        if (NULL == procAddress) LogImportFailed(_T("PlaySoundA"));
        importTable.PlaySoundA = (BOOL(WINAPI*)(LPCSTR, HMODULE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "PlaySoundW");
        if (NULL == procAddress) LogImportFailed(_T("PlaySoundW"));
        importTable.PlaySoundW = (BOOL(WINAPI*)(LPCWSTR, HMODULE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "SendDriverMessage");
        if (NULL == procAddress) LogImportFailed(_T("SendDriverMessage"));
        importTable.SendDriverMessage = (LRESULT(WINAPI*)(HDRVR, UINT, LPARAM, LPARAM))procAddress;

        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("auxGetDevCapsA"));
        importTable.auxGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("auxGetDevCapsW"));
        importTable.auxGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("auxGetNumDevs"));
        importTable.auxGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
        if (NULL == procAddress) LogImportFailed(_T("auxGetVolume"));
        importTable.auxGetVolume = (MMRESULT(WINAPI*)(UINT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
        if (NULL == procAddress) LogImportFailed(_T("auxOutMessage"));
        importTable.auxOutMessage = (MMRESULT(WINAPI*)(UINT, UINT, DWORD_PTR, DWORD_PTR))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
        if (NULL == procAddress) LogImportFailed(_T("auxSetVolume"));
        importTable.auxSetVolume = (MMRESULT(WINAPI*)(UINT, DWORD))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
        if (NULL == procAddress) LogImportFailed(_T("joyConfigChanged"));
        importTable.joyConfigChanged = (MMRESULT(WINAPI*)(DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("joyGetDevCapsA"));
        importTable.joyGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("joyGetDevCapsW"));
        importTable.joyGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("joyGetNumDevs"));
        importTable.joyGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
        if (NULL == procAddress) LogImportFailed(_T("joyGetPos"));
        importTable.joyGetPos = (MMRESULT(WINAPI*)(UINT, LPJOYINFO))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
        if (NULL == procAddress) LogImportFailed(_T("joyGetPosEx"));
        importTable.joyGetPosEx = (MMRESULT(WINAPI*)(UINT, LPJOYINFOEX))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
        if (NULL == procAddress) LogImportFailed(_T("joyGetThreshold"));
        importTable.joyGetThreshold = (MMRESULT(WINAPI*)(UINT, LPUINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
        if (NULL == procAddress) LogImportFailed(_T("joyReleaseCapture"));
        importTable.joyReleaseCapture = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
        if (NULL == procAddress) LogImportFailed(_T("joySetCapture"));
        importTable.joySetCapture = (MMRESULT(WINAPI*)(HWND, UINT, UINT, BOOL))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
        if (NULL == procAddress) LogImportFailed(_T("joySetThreshold"));
        importTable.joySetThreshold = (MMRESULT(WINAPI*)(UINT, UINT))procAddress;
        
        // ---------

        procAddress = GetProcAddress(loadedLibrary, "mciDriverNotify");
        if (NULL == procAddress) LogImportFailed(_T("mciDriverNotify"));
        importTable.mciDriverNotify = (decltype(importTable.mciDriverNotify))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciDriverYield");
        if (NULL == procAddress) LogImportFailed(_T("mciDriverYield"));
        importTable.mciDriverYield = (decltype(importTable.mciDriverYield))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciExecute");
        if (NULL == procAddress) LogImportFailed(_T("mciExecute"));
        importTable.mciExecute = (decltype(importTable.mciExecute))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciFreeCommandResource");
        if (NULL == procAddress) LogImportFailed(_T("mciFreeCommandResource"));
        importTable.mciFreeCommandResource = (decltype(importTable.mciFreeCommandResource))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetCreatorTask");
        if (NULL == procAddress) LogImportFailed(_T("mciGetCreatorTask"));
        importTable.mciGetCreatorTask = (decltype(importTable.mciGetCreatorTask))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDA");
        if (NULL == procAddress) LogImportFailed(_T("mciGetDeviceIDA"));
        importTable.mciGetDeviceIDA = (decltype(importTable.mciGetDeviceIDA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDW");
        if (NULL == procAddress) LogImportFailed(_T("mciGetDeviceIDW"));
        importTable.mciGetDeviceIDW = (decltype(importTable.mciGetDeviceIDW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDA");
        if (NULL == procAddress) LogImportFailed(_T("mciGetDeviceIDFromElementIDA"));
        importTable.mciGetDeviceIDFromElementIDA = (decltype(importTable.mciGetDeviceIDFromElementIDA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDW");
        if (NULL == procAddress) LogImportFailed(_T("mciGetDeviceIDFromElementIDW"));
        importTable.mciGetDeviceIDFromElementIDW = (decltype(importTable.mciGetDeviceIDFromElementIDW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDriverData");
        if (NULL == procAddress) LogImportFailed(_T("mciGetDriverData"));
        importTable.mciGetDriverData = (decltype(importTable.mciGetDriverData))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringA");
        if (NULL == procAddress) LogImportFailed(_T("mciGetErrorStringA"));
        importTable.mciGetErrorStringA = (decltype(importTable.mciGetErrorStringA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringW");
        if (NULL == procAddress) LogImportFailed(_T("mciGetErrorStringW"));
        importTable.mciGetErrorStringW = (decltype(importTable.mciGetErrorStringW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetYieldProc");
        if (NULL == procAddress) LogImportFailed(_T("mciGetYieldProc"));
        importTable.mciGetYieldProc = (decltype(importTable.mciGetYieldProc))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciLoadCommandResource");
        if (NULL == procAddress) LogImportFailed(_T("mciLoadCommandResource"));
        importTable.mciLoadCommandResource = (decltype(importTable.mciLoadCommandResource))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendCommandA");
        if (NULL == procAddress) LogImportFailed(_T("mciSendCommandA"));
        importTable.mciSendCommandA = (decltype(importTable.mciSendCommandA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendCommandW");
        if (NULL == procAddress) LogImportFailed(_T("mciSendCommandW"));
        importTable.mciSendCommandW = (decltype(importTable.mciSendCommandW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendStringA");
        if (NULL == procAddress) LogImportFailed(_T("mciSendStringA"));
        importTable.mciSendStringA = (decltype(importTable.mciSendStringA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendStringW");
        if (NULL == procAddress) LogImportFailed(_T("mciSendStringW"));
        importTable.mciSendStringW = (decltype(importTable.mciSendStringW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSetDriverData");
        if (NULL == procAddress) LogImportFailed(_T("mciSetDriverData"));
        importTable.mciSetDriverData = (decltype(importTable.mciSetDriverData))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSetYieldProc");
        if (NULL == procAddress) LogImportFailed(_T("mciSetYieldProc"));
        importTable.mciSetYieldProc = (decltype(importTable.mciSetYieldProc))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiConnect");
        if (NULL == procAddress) LogImportFailed(_T("midiConnect"));
        importTable.midiConnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiDisconnect");
        if (NULL == procAddress) LogImportFailed(_T("midiDisconnect"));
        importTable.midiDisconnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiInAddBuffer");
        if (NULL == procAddress) LogImportFailed(_T("midiInAddBuffer"));
        importTable.midiInAddBuffer = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInClose");
        if (NULL == procAddress) LogImportFailed(_T("midiInClose"));
        importTable.midiInClose = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("midiInGetDevCapsA"));
        importTable.midiInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("midiInGetDevCapsW"));
        importTable.midiInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(_T("midiInGetErrorTextA"));
        importTable.midiInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(_T("midiInGetErrorTextW"));
        importTable.midiInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetID");
        if (NULL == procAddress) LogImportFailed(_T("midiInGetID"));
        importTable.midiInGetID = (MMRESULT(WINAPI*)(HMIDIIN, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("midiInGetNumDevs"));
        importTable.midiInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInMessage");
        if (NULL == procAddress) LogImportFailed(_T("midiInMessage"));
        importTable.midiInMessage = (DWORD(WINAPI*)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInOpen");
        if (NULL == procAddress) LogImportFailed(_T("midiInOpen"));
        importTable.midiInOpen = (MMRESULT(WINAPI*)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInPrepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("midiInPrepareHeader"));
        importTable.midiInPrepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInReset");
        if (NULL == procAddress) LogImportFailed(_T("midiInReset"));
        importTable.midiInReset = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInStart");
        if (NULL == procAddress) LogImportFailed(_T("midiInStart"));
        importTable.midiInStart = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInStop");
        if (NULL == procAddress) LogImportFailed(_T("midiInStop"));
        importTable.midiInStop = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("midiInUnprepareHeader"));
        importTable.midiInUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;
        
        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiOutCacheDrumPatches");
        if (NULL == procAddress) LogImportFailed(_T("midiOutCacheDrumPatches"));
        importTable.midiOutCacheDrumPatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutCachePatches");
        if (NULL == procAddress) LogImportFailed(_T("midiOutCachePatches"));
        importTable.midiOutCachePatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutClose");
        if (NULL == procAddress) LogImportFailed(_T("midiOutClose"));
        importTable.midiOutClose = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetDevCapsA"));
        importTable.midiOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetDevCapsW"));
        importTable.midiOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetErrorTextA"));
        importTable.midiOutGetErrorTextA = (UINT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetErrorTextW"));
        importTable.midiOutGetErrorTextW = (UINT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetID");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetID"));
        importTable.midiOutGetID = (MMRESULT(WINAPI*)(HMIDIOUT, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetNumDevs"));
        importTable.midiOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetVolume");
        if (NULL == procAddress) LogImportFailed(_T("midiOutGetVolume"));
        importTable.midiOutGetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutLongMsg");
        if (NULL == procAddress) LogImportFailed(_T("midiOutLongMsg"));
        importTable.midiOutLongMsg = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutMessage");
        if (NULL == procAddress) LogImportFailed(_T("midiOutMessage"));
        importTable.midiOutMessage = (DWORD(WINAPI*)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutOpen");
        if (NULL == procAddress) LogImportFailed(_T("midiOutOpen"));
        importTable.midiOutOpen = (MMRESULT(WINAPI*)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutPrepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("midiOutPrepareHeader"));
        importTable.midiOutPrepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutReset");
        if (NULL == procAddress) LogImportFailed(_T("midiOutReset"));
        importTable.midiOutReset = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutSetVolume");
        if (NULL == procAddress) LogImportFailed(_T("midiOutSetVolume"));
        importTable.midiOutSetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutShortMsg");
        if (NULL == procAddress) LogImportFailed(_T("midiOutShortMsg"));
        importTable.midiOutShortMsg = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("midiOutUnprepareHeader"));
        importTable.midiOutUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiStreamClose");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamClose"));
        importTable.midiStreamClose = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamOpen");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamOpen"));
        importTable.midiStreamOpen = (MMRESULT(WINAPI*)(LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamOut");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamOut"));
        importTable.midiStreamOut = (MMRESULT(WINAPI*)(HMIDISTRM, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamPause");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamPause"));
        importTable.midiStreamPause = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamPosition");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamPosition"));
        importTable.midiStreamPosition = (MMRESULT(WINAPI*)(HMIDISTRM, LPMMTIME, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamProperty");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamProperty"));
        importTable.midiStreamProperty = (MMRESULT(WINAPI*)(HMIDISTRM, LPBYTE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamRestart");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamRestart"));
        importTable.midiStreamRestart = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamStop");
        if (NULL == procAddress) LogImportFailed(_T("midiStreamStop"));
        importTable.midiStreamStop = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "mixerClose");
        if (NULL == procAddress) LogImportFailed(_T("mixerClose"));
        importTable.mixerClose = (MMRESULT(WINAPI*)(HMIXER))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsA");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetControlDetailsA"));
        importTable.mixerGetControlDetailsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsW");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetControlDetailsW"));
        importTable.mixerGetControlDetailsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetDevCapsA"));
        importTable.mixerGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetDevCapsW"));
        importTable.mixerGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetID");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetID"));
        importTable.mixerGetID = (MMRESULT(WINAPI*)(HMIXEROBJ, UINT*, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsA");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetLineControlsA"));
        importTable.mixerGetLineControlsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsW");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetLineControlsW"));
        importTable.mixerGetLineControlsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoA");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetLineInfoA"));
        importTable.mixerGetLineInfoA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoW");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetLineInfoW"));
        importTable.mixerGetLineInfoW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("mixerGetNumDevs"));
        importTable.mixerGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerMessage");
        if (NULL == procAddress) LogImportFailed(_T("mixerMessage"));
        importTable.mixerMessage = (DWORD(WINAPI*)(HMIXER, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerOpen");
        if (NULL == procAddress) LogImportFailed(_T("mixerOpen"));
        importTable.mixerOpen = (MMRESULT(WINAPI*)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerSetControlDetails");
        if (NULL == procAddress) LogImportFailed(_T("mixerSetControlDetails"));
        importTable.mixerSetControlDetails = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "mmioAdvance");
        if (NULL == procAddress) LogImportFailed(_T("mmioAdvance"));
        importTable.mmioAdvance = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioAscend");
        if (NULL == procAddress) LogImportFailed(_T("mmioAscend"));
        importTable.mmioAscend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioClose");
        if (NULL == procAddress) LogImportFailed(_T("mmioClose"));
        importTable.mmioClose = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioCreateChunk");
        if (NULL == procAddress) LogImportFailed(_T("mmioCreateChunk"));
        importTable.mmioCreateChunk = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioDescend");
        if (NULL == procAddress) LogImportFailed(_T("mmioDescend"));
        importTable.mmioDescend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioFlush");
        if (NULL == procAddress) LogImportFailed(_T("mmioFlush"));
        importTable.mmioFlush = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioGetInfo");
        if (NULL == procAddress) LogImportFailed(_T("mmioGetInfo"));
        importTable.mmioGetInfo = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcA");
        if (NULL == procAddress) LogImportFailed(_T("mmioInstallIOProcA"));
        importTable.mmioInstallIOProcA = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcW");
        if (NULL == procAddress) LogImportFailed(_T("mmioInstallIOProcW"));
        importTable.mmioInstallIOProcW = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioOpenA");
        if (NULL == procAddress) LogImportFailed(_T("mmioOpenA"));
        importTable.mmioOpenA = (HMMIO(WINAPI*)(LPSTR, LPMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioOpenW");
        if (NULL == procAddress) LogImportFailed(_T("mmioOpenW"));
        importTable.mmioOpenW = (HMMIO(WINAPI*)(LPWSTR, LPMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioRead");
        if (NULL == procAddress) LogImportFailed(_T("mmioRead"));
        importTable.mmioRead = (LONG(WINAPI*)(HMMIO, HPSTR, LONG))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioRenameA");
        if (NULL == procAddress) LogImportFailed(_T("mmioRenameA"));
        importTable.mmioRenameA = (MMRESULT(WINAPI*)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioRenameW");
        if (NULL == procAddress) LogImportFailed(_T("mmioRenameW"));
        importTable.mmioRenameW = (MMRESULT(WINAPI*)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSeek");
        if (NULL == procAddress) LogImportFailed(_T("mmioSeek"));
        importTable.mmioSeek = (LONG(WINAPI*)(HMMIO, LONG, int))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSendMessage");
        if (NULL == procAddress) LogImportFailed(_T("mmioSendMessage"));
        importTable.mmioSendMessage = (LRESULT(WINAPI*)(HMMIO, UINT, LPARAM, LPARAM))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSetBuffer");
        if (NULL == procAddress) LogImportFailed(_T("mmioSetBuffer"));
        importTable.mmioSetBuffer = (MMRESULT(WINAPI*)(HMMIO, LPSTR, LONG, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioSetInfo");
        if (NULL == procAddress) LogImportFailed(_T("mmioSetInfo"));
        importTable.mmioSetInfo = (MMRESULT(WINAPI*)(HMMIO, LPCMMIOINFO, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCA");
        if (NULL == procAddress) LogImportFailed(_T("mmioStringToFOURCCA"));
        importTable.mmioStringToFOURCCA = (FOURCC(WINAPI*)(LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCW");
        if (NULL == procAddress) LogImportFailed(_T("mmioStringToFOURCCW"));
        importTable.mmioStringToFOURCCW = (FOURCC(WINAPI*)(LPCWSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "mmioWrite");
        if (NULL == procAddress) LogImportFailed(_T("mmioWrite"));
        importTable.mmioWrite = (LONG(WINAPI*)(HMMIO, const char*, LONG))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundA");
        if (NULL == procAddress) LogImportFailed(_T("sndPlaySoundA"));
        importTable.sndPlaySoundA = (BOOL(WINAPI*)(LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundW");
        if (NULL == procAddress) LogImportFailed(_T("sndPlaySoundW"));
        importTable.sndPlaySoundW = (BOOL(WINAPI*)(LPCWSTR, UINT))procAddress;
        
        // ---------

        procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
        if (NULL == procAddress) LogImportFailed(_T("timeBeginPeriod"));
        importTable.timeBeginPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
        if (NULL == procAddress) LogImportFailed(_T("timeEndPeriod"));
        importTable.timeEndPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
        if (NULL == procAddress) LogImportFailed(_T("timeGetDevCaps"));
        importTable.timeGetDevCaps = (MMRESULT(WINAPI*)(LPTIMECAPS, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
        if (NULL == procAddress) LogImportFailed(_T("timeGetSystemTime"));
        importTable.timeGetSystemTime = (MMRESULT(WINAPI*)(LPMMTIME, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
        if (NULL == procAddress) LogImportFailed(_T("timeGetTime"));
        importTable.timeGetTime = (DWORD(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
        if (NULL == procAddress) LogImportFailed(_T("timeKillEvent"));
        importTable.timeKillEvent = (MMRESULT(WINAPI*)(UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
        if (NULL == procAddress) LogImportFailed(_T("timeSetEvent"));
        importTable.timeSetEvent = (MMRESULT(WINAPI*)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "waveInAddBuffer");
        if (NULL == procAddress) LogImportFailed(_T("waveInAddBuffer"));
        importTable.waveInAddBuffer = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInClose");
        if (NULL == procAddress) LogImportFailed(_T("waveInClose"));
        importTable.waveInClose = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetDevCapsA"));
        importTable.waveInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetDevCapsW"));
        importTable.waveInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetErrorTextA"));
        importTable.waveInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetErrorTextW"));
        importTable.waveInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetID");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetID"));
        importTable.waveInGetID = (MMRESULT(WINAPI*)(HWAVEIN, LPUINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetNumDevs"));
        importTable.waveInGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInGetPosition");
        if (NULL == procAddress) LogImportFailed(_T("waveInGetPosition"));
        importTable.waveInGetPosition = (MMRESULT(WINAPI*)(HWAVEIN, LPMMTIME, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInMessage");
        if (NULL == procAddress) LogImportFailed(_T("waveInMessage"));
        importTable.waveInMessage = (DWORD(WINAPI*)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInOpen");
        if (NULL == procAddress) LogImportFailed(_T("waveInOpen"));
        importTable.waveInOpen = (MMRESULT(WINAPI*)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInPrepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("waveInPrepareHeader"));
        importTable.waveInPrepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInReset");
        if (NULL == procAddress) LogImportFailed(_T("waveInReset"));
        importTable.waveInReset = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInStart");
        if (NULL == procAddress) LogImportFailed(_T("waveInStart"));
        importTable.waveInStart = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInStop");
        if (NULL == procAddress) LogImportFailed(_T("waveInStop"));
        importTable.waveInStop = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveInUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("waveInUnprepareHeader"));
        importTable.waveInUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;
        
        // ---------
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutBreakLoop");
        if (NULL == procAddress) LogImportFailed(_T("waveOutBreakLoop"));
        importTable.waveOutBreakLoop = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutClose");
        if (NULL == procAddress) LogImportFailed(_T("waveOutClose"));
        importTable.waveOutClose = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetDevCapsA"));
        importTable.waveOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSA, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetDevCapsW"));
        importTable.waveOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSW, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetErrorTextA"));
        importTable.waveOutGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetErrorTextW"));
        importTable.waveOutGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetID");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetID"));
        importTable.waveOutGetID = (MMRESULT(WINAPI*)(HWAVEOUT, LPUINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetNumDevs");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetNumDevs"));
        importTable.waveOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPitch");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetPitch"));
        importTable.waveOutGetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPlaybackRate");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetPlaybackRate"));
        importTable.waveOutGetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPosition");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetPosition"));
        importTable.waveOutGetPosition = (MMRESULT(WINAPI*)(HWAVEOUT, LPMMTIME, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutGetVolume");
        if (NULL == procAddress) LogImportFailed(_T("waveOutGetVolume"));
        importTable.waveOutGetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutMessage");
        if (NULL == procAddress) LogImportFailed(_T("waveOutMessage"));
        importTable.waveOutMessage = (DWORD(WINAPI*)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutOpen");
        if (NULL == procAddress) LogImportFailed(_T("waveOutOpen"));
        importTable.waveOutOpen = (MMRESULT(WINAPI*)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutPause");
        if (NULL == procAddress) LogImportFailed(_T("waveOutPause"));
        importTable.waveOutPause = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutPrepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("waveOutPrepareHeader"));
        importTable.waveOutPrepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutReset");
        if (NULL == procAddress) LogImportFailed(_T("waveOutReset"));
        importTable.waveOutReset = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutRestart");
        if (NULL == procAddress) LogImportFailed(_T("waveOutRestart"));
        importTable.waveOutRestart = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutSetPitch");
        if (NULL == procAddress) LogImportFailed(_T("waveOutSetPitch"));
        importTable.waveOutSetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutSetPlaybackRate");
        if (NULL == procAddress) LogImportFailed(_T("waveOutSetPlaybackRate"));
        importTable.waveOutSetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutSetVolume");
        if (NULL == procAddress) LogImportFailed(_T("waveOutSetVolume"));
        importTable.waveOutSetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(_T("waveOutUnprepareHeader"));
        importTable.waveOutUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;
        
        procAddress = GetProcAddress(loadedLibrary, "waveOutWrite");
        if (NULL == procAddress) LogImportFailed(_T("waveOutWrite"));
        importTable.waveOutWrite = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;
        
        // Initialization complete.
        importTableIsInitialized = TRUE;
        LogInitializeSucceeded();
    }
}

// ---------

LRESULT ImportApiWinMM::CloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2)
{
    Initialize();

    if (NULL == importTable.CloseDriver)
        LogMissingFunctionCalled(_T("CloseDriver"));

    return importTable.CloseDriver(hdrvr, lParam1, lParam2);
}

// ---------

LRESULT ImportApiWinMM::DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
{
    Initialize();

    if (NULL == importTable.DefDriverProc)
        LogMissingFunctionCalled(_T("DefDriverProc"));

    return importTable.DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
}

// ---------

BOOL ImportApiWinMM::DriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
    Initialize();

    if (NULL == importTable.DriverCallback)
        LogMissingFunctionCalled(_T("DriverCallback"));

    return importTable.DriverCallback(dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
}

// ---------

HMODULE ImportApiWinMM::DrvGetModuleHandle(HDRVR hDriver)
{
    Initialize();

    if (NULL == importTable.DrvGetModuleHandle)
        LogMissingFunctionCalled(_T("DrvGetModuleHandle"));

    return importTable.DrvGetModuleHandle(hDriver);
}

// ---------

HMODULE ImportApiWinMM::GetDriverModuleHandle(HDRVR hdrvr)
{
    Initialize();

    if (NULL == importTable.GetDriverModuleHandle)
        LogMissingFunctionCalled(_T("GetDriverModuleHandle"));

    return importTable.GetDriverModuleHandle(hdrvr);
}

// ---------

HDRVR ImportApiWinMM::OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
{
    Initialize();

    if (NULL == importTable.OpenDriver)
        LogMissingFunctionCalled(_T("OpenDriver"));

    return importTable.OpenDriver(lpDriverName, lpSectionName, lParam);
}

// ---------

BOOL ImportApiWinMM::PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    Initialize();

    if (NULL == importTable.PlaySoundA)
        LogMissingFunctionCalled(_T("PlaySoundA"));

    return importTable.PlaySoundA(pszSound, hmod, fdwSound);
}

// ---------

BOOL ImportApiWinMM::PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    Initialize();

    if (NULL == importTable.PlaySoundW)
        LogMissingFunctionCalled(_T("PlaySoundW"));

    return importTable.PlaySoundW(pszSound, hmod, fdwSound);
}

// ---------

LRESULT ImportApiWinMM::SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    Initialize();

    if (NULL == importTable.SendDriverMessage)
        LogMissingFunctionCalled(_T("SendDriverMessage"));

    return importTable.SendDriverMessage(hdrvr, msg, lParam1, lParam2);
}

// ---------

MMRESULT ImportApiWinMM::auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
{
    Initialize();

    if (NULL == importTable.auxGetDevCapsA)
        LogMissingFunctionCalled(_T("auxGetDevCapsA"));

    return importTable.auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
}

// ---------

MMRESULT ImportApiWinMM::auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
{
    Initialize();

    if (NULL == importTable.auxGetDevCapsW)
        LogMissingFunctionCalled(_T("auxGetDevCapsW"));

    return importTable.auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
}

// ---------

UINT ImportApiWinMM::auxGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.auxGetNumDevs)
        LogMissingFunctionCalled(_T("auxGetNumDevs"));

    return importTable.auxGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
{
    Initialize();

    if (NULL == importTable.auxGetVolume)
        LogMissingFunctionCalled(_T("auxGetVolume"));

    return importTable.auxGetVolume(uDeviceID, lpdwVolume);
}

// ---------

MMRESULT ImportApiWinMM::auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.auxOutMessage)
        LogMissingFunctionCalled(_T("auxOutMessage"));

    return importTable.auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::auxSetVolume(UINT uDeviceID, DWORD dwVolume)
{
    Initialize();

    if (NULL == importTable.auxSetVolume)
        LogMissingFunctionCalled(_T("auxSetVolume"));

    return importTable.auxSetVolume(uDeviceID, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::joyConfigChanged(DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.joyConfigChanged)
        LogMissingFunctionCalled(_T("joyConfigChanged"));

    return importTable.joyConfigChanged(dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    Initialize();

    if (NULL == importTable.joyGetDevCapsA)
        LogMissingFunctionCalled(_T("joyGetDevCapsA"));

    return importTable.joyGetDevCapsA(uJoyID, pjc, cbjc);
}

// ---------

MMRESULT ImportApiWinMM::joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    Initialize();

    if (NULL == importTable.joyGetDevCapsW)
        LogMissingFunctionCalled(_T("joyGetDevCapsW"));

    return importTable.joyGetDevCapsW(uJoyID, pjc, cbjc);
}

// ---------

UINT ImportApiWinMM::joyGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.joyGetNumDevs)
        LogMissingFunctionCalled(_T("joyGetNumDevs"));

    return importTable.joyGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::joyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    Initialize();

    if (NULL == importTable.joyGetPos)
        LogMissingFunctionCalled(_T("joyGetPos"));

    return importTable.joyGetPos(uJoyID, pji);
}

// ---------

MMRESULT ImportApiWinMM::joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    Initialize();

    if (NULL == importTable.joyGetPosEx)
        LogMissingFunctionCalled(_T("joyGetPosEx"));

    return importTable.joyGetPosEx(uJoyID, pji);
}

// ---------

MMRESULT ImportApiWinMM::joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    Initialize();

    if (NULL == importTable.joyGetThreshold)
        LogMissingFunctionCalled(_T("joyGetThreshold"));

    return importTable.joyGetThreshold(uJoyID, puThreshold);
}

// ---------

MMRESULT ImportApiWinMM::joyReleaseCapture(UINT uJoyID)
{
    Initialize();

    if (NULL == importTable.joyReleaseCapture)
        LogMissingFunctionCalled(_T("joyReleaseCapture"));

    return importTable.joyReleaseCapture(uJoyID);
}

// ---------

MMRESULT ImportApiWinMM::joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    Initialize();

    if (NULL == importTable.joySetCapture)
        LogMissingFunctionCalled(_T("joySetCapture"));

    return importTable.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
}

// ---------

MMRESULT ImportApiWinMM::joySetThreshold(UINT uJoyID, UINT uThreshold)
{
    Initialize();

    if (NULL == importTable.joySetThreshold)
        LogMissingFunctionCalled(_T("joySetThreshold"));

    return importTable.joySetThreshold(uJoyID, uThreshold);
}

// ---------

BOOL ImportApiWinMM::mciDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
{
    Initialize();

    if (NULL == importTable.mciDriverNotify)
        LogMissingFunctionCalled(_T("mciDriverNotify"));

    return importTable.mciDriverNotify(hwndCallback, IDDevice, uStatus);
}

// ---------

UINT ImportApiWinMM::mciDriverYield(MCIDEVICEID IDDevice)
{
    Initialize();

    if (NULL == importTable.mciDriverYield)
        LogMissingFunctionCalled(_T("mciDriverYield"));

    return importTable.mciDriverYield(IDDevice);
}

// ---------

BOOL ImportApiWinMM::mciExecute(LPCSTR pszCommand)
{
    Initialize();

    if (NULL == importTable.mciExecute)
        LogMissingFunctionCalled(_T("mciExecute"));

    return importTable.mciExecute(pszCommand);
}

// ---------

BOOL ImportApiWinMM::mciFreeCommandResource(UINT uResource)
{
    Initialize();

    if (NULL == importTable.mciFreeCommandResource)
        LogMissingFunctionCalled(_T("mciFreeCommandResource"));

    return importTable.mciFreeCommandResource(uResource);
}

// ---------

HANDLE ImportApiWinMM::mciGetCreatorTask(MCIDEVICEID IDDevice)
{
    Initialize();

    if (NULL == importTable.mciGetCreatorTask)
        LogMissingFunctionCalled(_T("mciGetCreatorTask"));

    return importTable.mciGetCreatorTask(IDDevice);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDA(LPCSTR lpszDevice)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDA)
        LogMissingFunctionCalled(_T("mciGetDeviceIDA"));

    return importTable.mciGetDeviceIDA(lpszDevice);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDW(LPCWSTR lpszDevice)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDW)
        LogMissingFunctionCalled(_T("mciGetDeviceIDW"));

    return importTable.mciGetDeviceIDW(lpszDevice);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDFromElementIDA)
        LogMissingFunctionCalled(_T("mciGetDeviceIDFromElementIDA"));

    return importTable.mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDFromElementIDW)
        LogMissingFunctionCalled(_T("mciGetDeviceIDFromElementIDW"));

    return importTable.mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
}

// ---------

DWORD_PTR ImportApiWinMM::mciGetDriverData(MCIDEVICEID IDDevice)
{
    Initialize();

    if (NULL == importTable.mciGetDriverData)
        LogMissingFunctionCalled(_T("mciGetDriverData"));

    return importTable.mciGetDriverData(IDDevice);
}

// ---------

BOOL ImportApiWinMM::mciGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
{
    Initialize();

    if (NULL == importTable.mciGetErrorStringA)
        LogMissingFunctionCalled(_T("mciGetErrorStringA"));

    return importTable.mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
}

// ---------

BOOL ImportApiWinMM::mciGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
{
    Initialize();

    if (NULL == importTable.mciGetErrorStringW)
        LogMissingFunctionCalled(_T("mciGetErrorStringW"));

    return importTable.mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
}

// ---------

YIELDPROC ImportApiWinMM::mciGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
{
    Initialize();

    if (NULL == importTable.mciGetYieldProc)
        LogMissingFunctionCalled(_T("mciGetYieldProc"));

    return importTable.mciGetYieldProc(IDDevice, lpdwYieldData);
}

// ---------

UINT ImportApiWinMM::mciLoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
{
    Initialize();

    if (NULL == importTable.mciLoadCommandResource)
        LogMissingFunctionCalled(_T("mciLoadCommandResource"));

    return importTable.mciLoadCommandResource(hInst, lpwstrResourceName, uType);
}

// ---------

MCIERROR ImportApiWinMM::mciSendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
    Initialize();

    if (NULL == importTable.mciSendCommandA)
        LogMissingFunctionCalled(_T("mciSendCommandA"));

    return importTable.mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
}

// ---------

MCIERROR ImportApiWinMM::mciSendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
    Initialize();

    if (NULL == importTable.mciSendCommandW)
        LogMissingFunctionCalled(_T("mciSendCommandW"));

    return importTable.mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
}

// ---------

MCIERROR ImportApiWinMM::mciSendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    Initialize();

    if (NULL == importTable.mciSendStringA)
        LogMissingFunctionCalled(_T("mciSendStringA"));

    return importTable.mciSendStringA(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
}

// ---------

MCIERROR ImportApiWinMM::mciSendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    Initialize();

    if (NULL == importTable.mciSendStringW)
        LogMissingFunctionCalled(_T("mciSendStringW"));

    return importTable.mciSendStringW(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
}

// ---------

BOOL ImportApiWinMM::mciSetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
{
    Initialize();

    if (NULL == importTable.mciSetDriverData)
        LogMissingFunctionCalled(_T("mciSetDriverData"));

    return importTable.mciSetDriverData(IDDevice, data);
}

// ---------

UINT ImportApiWinMM::mciSetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
{
    Initialize();

    if (NULL == importTable.mciSetYieldProc)
        LogMissingFunctionCalled(_T("mciSetYieldProc"));

    return importTable.mciSetYieldProc(IDDevice, yp, dwYieldData);
}

// ---------

MMRESULT ImportApiWinMM::midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
{
    Initialize();

    if (NULL == importTable.midiConnect)
        LogMissingFunctionCalled(_T("midiConnect"));

    return importTable.midiConnect(hMidi, hmo, pReserved);
}

// ---------

MMRESULT ImportApiWinMM::midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
{
    Initialize();

    if (NULL == importTable.midiDisconnect)
        LogMissingFunctionCalled(_T("midiDisconnect"));

    return importTable.midiDisconnect(hMidi, hmo, pReserved);
}

// ---------

MMRESULT ImportApiWinMM::midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    Initialize();

    if (NULL == importTable.midiInAddBuffer)
        LogMissingFunctionCalled(_T("midiInAddBuffer"));

    return importTable.midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiInClose(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInClose)
        LogMissingFunctionCalled(_T("midiInClose"));

    return importTable.midiInClose(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
{
    Initialize();

    if (NULL == importTable.midiInGetDevCapsA)
        LogMissingFunctionCalled(_T("midiInGetDevCapsA"));

    return importTable.midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
{
    Initialize();

    if (NULL == importTable.midiInGetDevCapsW)
        LogMissingFunctionCalled(_T("midiInGetDevCapsW"));

    return importTable.midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiInGetErrorTextA)
        LogMissingFunctionCalled(_T("midiInGetErrorTextA"));

    return importTable.midiInGetErrorTextA(wError, lpText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiInGetErrorTextW)
        LogMissingFunctionCalled(_T("midiInGetErrorTextW"));

    return importTable.midiInGetErrorTextW(wError, lpText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.midiInGetID)
        LogMissingFunctionCalled(_T("midiInGetID"));

    return importTable.midiInGetID(hmi, puDeviceID);
}

// ---------

UINT ImportApiWinMM::midiInGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.midiInGetNumDevs)
        LogMissingFunctionCalled(_T("midiInGetNumDevs"));

    return importTable.midiInGetNumDevs();
}

// ---------

DWORD ImportApiWinMM::midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    Initialize();

    if (NULL == importTable.midiInMessage)
        LogMissingFunctionCalled(_T("midiInMessage"));

    return importTable.midiInMessage(deviceID, msg, dw1, dw2);
}

// ---------

MMRESULT ImportApiWinMM::midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.midiInOpen)
        LogMissingFunctionCalled(_T("midiInOpen"));

    return importTable.midiInOpen(lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    Initialize();

    if (NULL == importTable.midiInPrepareHeader)
        LogMissingFunctionCalled(_T("midiInPrepareHeader"));

    return importTable.midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiInReset(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInReset)
        LogMissingFunctionCalled(_T("midiInReset"));

    return importTable.midiInReset(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInStart(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInStart)
        LogMissingFunctionCalled(_T("midiInStart"));

    return importTable.midiInStart(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInStop(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInStop)
        LogMissingFunctionCalled(_T("midiInStop"));

    return importTable.midiInStop(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    Initialize();

    if (NULL == importTable.midiInUnprepareHeader)
        LogMissingFunctionCalled(_T("midiInUnprepareHeader"));

    return importTable.midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.midiOutCacheDrumPatches)
        LogMissingFunctionCalled(_T("midiOutCacheDrumPatches"));

    return importTable.midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.midiOutCachePatches)
        LogMissingFunctionCalled(_T("midiOutCachePatches"));

    return importTable.midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiOutClose(HMIDIOUT hmo)
{
    Initialize();

    if (NULL == importTable.midiOutClose)
        LogMissingFunctionCalled(_T("midiOutClose"));

    return importTable.midiOutClose(hmo);
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
{
    Initialize();

    if (NULL == importTable.midiOutGetDevCapsA)
        LogMissingFunctionCalled(_T("midiOutGetDevCapsA"));

    return importTable.midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
{
    Initialize();

    if (NULL == importTable.midiOutGetDevCapsW)
        LogMissingFunctionCalled(_T("midiOutGetDevCapsW"));

    return importTable.midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
}

// ---------

UINT ImportApiWinMM::midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiOutGetErrorTextA)
        LogMissingFunctionCalled(_T("midiOutGetErrorTextA"));

    return importTable.midiOutGetErrorTextA(mmrError, lpText, cchText);
}

// ---------

UINT ImportApiWinMM::midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiOutGetErrorTextW)
        LogMissingFunctionCalled(_T("midiOutGetErrorTextW"));

    return importTable.midiOutGetErrorTextW(mmrError, lpText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.midiOutGetID)
        LogMissingFunctionCalled(_T("midiOutGetID"));

    return importTable.midiOutGetID(hmo, puDeviceID);
}

// ---------

UINT ImportApiWinMM::midiOutGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.midiOutGetNumDevs)
        LogMissingFunctionCalled(_T("midiOutGetNumDevs"));

    return importTable.midiOutGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
{
    Initialize();

    if (NULL == importTable.midiOutGetVolume)
        LogMissingFunctionCalled(_T("midiOutGetVolume"));

    return importTable.midiOutGetVolume(hmo, lpdwVolume);
}

// ---------

MMRESULT ImportApiWinMM::midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    Initialize();

    if (NULL == importTable.midiOutLongMsg)
        LogMissingFunctionCalled(_T("midiOutLongMsg"));

    return importTable.midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

DWORD ImportApiWinMM::midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    Initialize();

    if (NULL == importTable.midiOutMessage)
        LogMissingFunctionCalled(_T("midiOutMessage"));

    return importTable.midiOutMessage(deviceID, msg, dw1, dw2);
}

// ---------

MMRESULT ImportApiWinMM::midiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.midiOutOpen)
        LogMissingFunctionCalled(_T("midiOutOpen"));

    return importTable.midiOutOpen(lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    Initialize();

    if (NULL == importTable.midiOutPrepareHeader)
        LogMissingFunctionCalled(_T("midiOutPrepareHeader"));

    return importTable.midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiOutReset(HMIDIOUT hmo)
{
    Initialize();

    if (NULL == importTable.midiOutReset)
        LogMissingFunctionCalled(_T("midiOutReset"));

    return importTable.midiOutReset(hmo);
}

// ---------

MMRESULT ImportApiWinMM::midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
{
    Initialize();

    if (NULL == importTable.midiOutSetVolume)
        LogMissingFunctionCalled(_T("midiOutSetVolume"));

    return importTable.midiOutSetVolume(hmo, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
{
    Initialize();

    if (NULL == importTable.midiOutShortMsg)
        LogMissingFunctionCalled(_T("midiOutShortMsg"));

    return importTable.midiOutShortMsg(hmo, dwMsg);
}

// ---------

MMRESULT ImportApiWinMM::midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    Initialize();

    if (NULL == importTable.midiOutUnprepareHeader)
        LogMissingFunctionCalled(_T("midiOutUnprepareHeader"));

    return importTable.midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamClose(HMIDISTRM hStream)
{
    Initialize();

    if (NULL == importTable.midiStreamClose)
        LogMissingFunctionCalled(_T("midiStreamClose"));

    return importTable.midiStreamClose(hStream);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.midiStreamOpen)
        LogMissingFunctionCalled(_T("midiStreamOpen"));

    return importTable.midiStreamOpen(lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
{
    Initialize();

    if (NULL == importTable.midiStreamOut)
        LogMissingFunctionCalled(_T("midiStreamOut"));

    return importTable.midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamPause(HMIDISTRM hms)
{
    Initialize();

    if (NULL == importTable.midiStreamPause)
        LogMissingFunctionCalled(_T("midiStreamPause"));

    return importTable.midiStreamPause(hms);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.midiStreamPosition)
        LogMissingFunctionCalled(_T("midiStreamPosition"));

    return importTable.midiStreamPosition(hms, pmmt, cbmmt);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
{
    Initialize();

    if (NULL == importTable.midiStreamProperty)
        LogMissingFunctionCalled(_T("midiStreamProperty"));

    return importTable.midiStreamProperty(hm, lppropdata, dwProperty);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamRestart(HMIDISTRM hms)
{
    Initialize();

    if (NULL == importTable.midiStreamRestart)
        LogMissingFunctionCalled(_T("midiStreamRestart"));

    return importTable.midiStreamRestart(hms);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamStop(HMIDISTRM hms)
{
    Initialize();

    if (NULL == importTable.midiStreamStop)
        LogMissingFunctionCalled(_T("midiStreamStop"));

    return importTable.midiStreamStop(hms);
}

// ---------

MMRESULT ImportApiWinMM::mixerClose(HMIXER hmx)
{
    Initialize();

    if (NULL == importTable.mixerClose)
        LogMissingFunctionCalled(_T("mixerClose"));

    return importTable.mixerClose(hmx);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    Initialize();

    if (NULL == importTable.mixerGetControlDetailsA)
        LogMissingFunctionCalled(_T("mixerGetControlDetailsA"));

    return importTable.mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    Initialize();

    if (NULL == importTable.mixerGetControlDetailsW)
        LogMissingFunctionCalled(_T("mixerGetControlDetailsW"));

    return importTable.mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
{
    Initialize();

    if (NULL == importTable.mixerGetDevCapsA)
        LogMissingFunctionCalled(_T("mixerGetDevCapsA"));

    return importTable.mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
{
    Initialize();

    if (NULL == importTable.mixerGetDevCapsW)
        LogMissingFunctionCalled(_T("mixerGetDevCapsW"));

    return importTable.mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
{
    Initialize();

    if (NULL == importTable.mixerGetID)
        LogMissingFunctionCalled(_T("mixerGetID"));

    return importTable.mixerGetID(hmxobj, puMxId, fdwId);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
{
    Initialize();

    if (NULL == importTable.mixerGetLineControlsA)
        LogMissingFunctionCalled(_T("mixerGetLineControlsA"));

    return importTable.mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
{
    Initialize();

    if (NULL == importTable.mixerGetLineControlsW)
        LogMissingFunctionCalled(_T("mixerGetLineControlsW"));

    return importTable.mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
{
    Initialize();

    if (NULL == importTable.mixerGetLineInfoA)
        LogMissingFunctionCalled(_T("mixerGetLineInfoA"));

    return importTable.mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
{
    Initialize();

    if (NULL == importTable.mixerGetLineInfoW)
        LogMissingFunctionCalled(_T("mixerGetLineInfoW"));

    return importTable.mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
}

// ---------

UINT ImportApiWinMM::mixerGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.mixerGetNumDevs)
        LogMissingFunctionCalled(_T("mixerGetNumDevs"));

    return importTable.mixerGetNumDevs();
}

// ---------

DWORD ImportApiWinMM::mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.mixerMessage)
        LogMissingFunctionCalled(_T("mixerMessage"));

    return importTable.mixerMessage(driverID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.mixerOpen)
        LogMissingFunctionCalled(_T("mixerOpen"));

    return importTable.mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    Initialize();

    if (NULL == importTable.mixerSetControlDetails)
        LogMissingFunctionCalled(_T("mixerSetControlDetails"));

    return importTable.mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT ImportApiWinMM::mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioAdvance)
        LogMissingFunctionCalled(_T("mmioAdvance"));

    return importTable.mmioAdvance(hmmio, lpmmioinfo, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioAscend)
        LogMissingFunctionCalled(_T("mmioAscend"));

    return importTable.mmioAscend(hmmio, lpck, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioClose(HMMIO hmmio, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioClose)
        LogMissingFunctionCalled(_T("mmioClose"));

    return importTable.mmioClose(hmmio, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioCreateChunk)
        LogMissingFunctionCalled(_T("mmioCreateChunk"));

    return importTable.mmioCreateChunk(hmmio, lpck, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioDescend)
        LogMissingFunctionCalled(_T("mmioDescend"));

    return importTable.mmioDescend(hmmio, lpck, lpckParent, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioFlush(HMMIO hmmio, UINT fuFlush)
{
    Initialize();

    if (NULL == importTable.mmioFlush)
        LogMissingFunctionCalled(_T("mmioFlush"));

    return importTable.mmioFlush(hmmio, fuFlush);
}

// ---------


MMRESULT ImportApiWinMM::mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioGetInfo)
        LogMissingFunctionCalled(_T("mmioGetInfo"));

    return importTable.mmioGetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


LPMMIOPROC ImportApiWinMM::mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.mmioInstallIOProcA)
        LogMissingFunctionCalled(_T("mmioInstallIOProcA"));

    return importTable.mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
}

// ---------


LPMMIOPROC ImportApiWinMM::mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.mmioInstallIOProcW)
        LogMissingFunctionCalled(_T("mmioInstallIOProcW"));

    return importTable.mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
}

// ---------


HMMIO ImportApiWinMM::mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    Initialize();

    if (NULL == importTable.mmioOpenA)
        LogMissingFunctionCalled(_T("mmioOpenA"));

    return importTable.mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


HMMIO ImportApiWinMM::mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    Initialize();

    if (NULL == importTable.mmioOpenW)
        LogMissingFunctionCalled(_T("mmioOpenW"));

    return importTable.mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


LONG ImportApiWinMM::mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
{
    Initialize();

    if (NULL == importTable.mmioRead)
        LogMissingFunctionCalled(_T("mmioRead"));

    return importTable.mmioRead(hmmio, pch, cch);
}

// ---------


MMRESULT ImportApiWinMM::mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    Initialize();

    if (NULL == importTable.mmioRenameA)
        LogMissingFunctionCalled(_T("mmioRenameA"));

    return importTable.mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    Initialize();

    if (NULL == importTable.mmioRenameW)
        LogMissingFunctionCalled(_T("mmioRenameW"));

    return importTable.mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


LONG ImportApiWinMM::mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
{
    Initialize();

    if (NULL == importTable.mmioSeek)
        LogMissingFunctionCalled(_T("mmioSeek"));

    return importTable.mmioSeek(hmmio, lOffset, iOrigin);
}

// ---------


LRESULT ImportApiWinMM::mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
    Initialize();

    if (NULL == importTable.mmioSendMessage)
        LogMissingFunctionCalled(_T("mmioSendMessage"));

    return importTable.mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
}

// ---------


MMRESULT ImportApiWinMM::mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioSetBuffer)
        LogMissingFunctionCalled(_T("mmioSetBuffer"));

    return importTable.mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioSetInfo)
        LogMissingFunctionCalled(_T("mmioSetInfo"));

    return importTable.mmioSetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


FOURCC ImportApiWinMM::mmioStringToFOURCCA(LPCSTR sz, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioStringToFOURCCA)
        LogMissingFunctionCalled(_T("mmioStringToFOURCCA"));

    return importTable.mmioStringToFOURCCA(sz, wFlags);
}

// ---------


FOURCC ImportApiWinMM::mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioStringToFOURCCW)
        LogMissingFunctionCalled(_T("mmioStringToFOURCCW"));

    return importTable.mmioStringToFOURCCW(sz, wFlags);
}

// ---------


LONG ImportApiWinMM::mmioWrite(HMMIO hmmio, const char* pch, LONG cch)
{
    Initialize();

    if (NULL == importTable.mmioWrite)
        LogMissingFunctionCalled(_T("mmioWrite"));

    return importTable.mmioWrite(hmmio, pch, cch);
}

// ---------

BOOL ImportApiWinMM::sndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
{
    Initialize();

    if (NULL == importTable.sndPlaySoundA)
        LogMissingFunctionCalled(_T("sndPlaySoundA"));

    return importTable.sndPlaySoundA(lpszSound, fuSound);
}

// ---------

BOOL ImportApiWinMM::sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
{
    Initialize();

    if (NULL == importTable.sndPlaySoundW)
        LogMissingFunctionCalled(_T("sndPlaySoundW"));

    return importTable.sndPlaySoundW(lpszSound, fuSound);
}

// ---------

MMRESULT ImportApiWinMM::timeBeginPeriod(UINT uPeriod)
{
    Initialize();

    if (NULL == importTable.timeBeginPeriod)
        LogMissingFunctionCalled(_T("timeBeginPeriod"));

    return importTable.timeBeginPeriod(uPeriod);
}

// ---------

MMRESULT ImportApiWinMM::timeEndPeriod(UINT uPeriod)
{
    Initialize();

    if (NULL == importTable.timeEndPeriod)
        LogMissingFunctionCalled(_T("timeEndPeriod"));

    return importTable.timeEndPeriod(uPeriod);
}

// ---------

MMRESULT ImportApiWinMM::timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    Initialize();

    if (NULL == importTable.timeGetDevCaps)
        LogMissingFunctionCalled(_T("timeGetDevCaps"));

    return importTable.timeGetDevCaps(ptc, cbtc);
}

// ---------

MMRESULT ImportApiWinMM::timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.timeGetSystemTime)
        LogMissingFunctionCalled(_T("timeGetSystemTime"));

    return importTable.timeGetSystemTime(pmmt, cbmmt);
}

// ---------

DWORD ImportApiWinMM::timeGetTime(void)
{
    Initialize();

    if (NULL == importTable.timeGetTime)
        LogMissingFunctionCalled(_T("timeGetTime"));

    return importTable.timeGetTime();
}

// ---------

MMRESULT ImportApiWinMM::timeKillEvent(UINT uTimerID)
{
    Initialize();

    if (NULL == importTable.timeKillEvent)
        LogMissingFunctionCalled(_T("timeKillEvent"));

    return importTable.timeKillEvent(uTimerID);
}

// ---------

MMRESULT ImportApiWinMM::timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
    Initialize();

    if (NULL == importTable.timeSetEvent)
        LogMissingFunctionCalled(_T("timeSetEvent"));

    return importTable.timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
}

// ---------

MMRESULT ImportApiWinMM::waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveInAddBuffer)
        LogMissingFunctionCalled(_T("waveInAddBuffer"));

    return importTable.waveInAddBuffer(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveInClose(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInClose)
        LogMissingFunctionCalled(_T("waveInClose"));

    return importTable.waveInClose(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
{
    Initialize();

    if (NULL == importTable.waveInGetDevCapsA)
        LogMissingFunctionCalled(_T("waveInGetDevCapsA"));

    return importTable.waveInGetDevCapsA(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
{
    Initialize();

    if (NULL == importTable.waveInGetDevCapsW)
        LogMissingFunctionCalled(_T("waveInGetDevCapsW"));

    return importTable.waveInGetDevCapsW(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveInGetErrorTextA)
        LogMissingFunctionCalled(_T("waveInGetErrorTextA"));

    return importTable.waveInGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveInGetErrorTextW)
        LogMissingFunctionCalled(_T("waveInGetErrorTextW"));

    return importTable.waveInGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.waveInGetID)
        LogMissingFunctionCalled(_T("waveInGetID"));

    return importTable.waveInGetID(hwi, puDeviceID);
}

// ---------

UINT ImportApiWinMM::waveInGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.waveInGetNumDevs)
        LogMissingFunctionCalled(_T("waveInGetNumDevs"));

    return importTable.waveInGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.waveInGetPosition)
        LogMissingFunctionCalled(_T("waveInGetPosition"));

    return importTable.waveInGetPosition(hwi, pmmt, cbmmt);
}

// ---------

DWORD ImportApiWinMM::waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.waveInMessage)
        LogMissingFunctionCalled(_T("waveInMessage"));

    return importTable.waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.waveInOpen)
        LogMissingFunctionCalled(_T("waveInOpen"));

    return importTable.waveInOpen(phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveInPrepareHeader)
        LogMissingFunctionCalled(_T("waveInPrepareHeader"));

    return importTable.waveInPrepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveInReset(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInReset)
        LogMissingFunctionCalled(_T("waveInReset"));

    return importTable.waveInReset(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInStart(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInStart)
        LogMissingFunctionCalled(_T("waveInStart"));

    return importTable.waveInStart(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInStop(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInStop)
        LogMissingFunctionCalled(_T("waveInStop"));

    return importTable.waveInStop(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveInUnprepareHeader)
        LogMissingFunctionCalled(_T("waveInUnprepareHeader"));

    return importTable.waveInUnprepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutBreakLoop(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutBreakLoop)
        LogMissingFunctionCalled(_T("waveOutBreakLoop"));

    return importTable.waveOutBreakLoop(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutClose(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutClose)
        LogMissingFunctionCalled(_T("waveOutClose"));

    return importTable.waveOutClose(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
{
    Initialize();

    if (NULL == importTable.waveOutGetDevCapsA)
        LogMissingFunctionCalled(_T("waveOutGetDevCapsA"));

    return importTable.waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
{
    Initialize();

    if (NULL == importTable.waveOutGetDevCapsW)
        LogMissingFunctionCalled(_T("waveOutGetDevCapsW"));

    return importTable.waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveOutGetErrorTextA)
        LogMissingFunctionCalled(_T("waveOutGetErrorTextA"));

    return importTable.waveOutGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveOutGetErrorTextW)
        LogMissingFunctionCalled(_T("waveOutGetErrorTextW"));

    return importTable.waveOutGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.waveOutGetID)
        LogMissingFunctionCalled(_T("waveOutGetID"));

    return importTable.waveOutGetID(hwo, puDeviceID);
}

// ---------

UINT ImportApiWinMM::waveOutGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.waveOutGetNumDevs)
        LogMissingFunctionCalled(_T("waveOutGetNumDevs"));

    return importTable.waveOutGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
{
    Initialize();

    if (NULL == importTable.waveOutGetPitch)
        LogMissingFunctionCalled(_T("waveOutGetPitch"));

    return importTable.waveOutGetPitch(hwo, pdwPitch);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
{
    Initialize();

    if (NULL == importTable.waveOutGetPlaybackRate)
        LogMissingFunctionCalled(_T("waveOutGetPlaybackRate"));

    return importTable.waveOutGetPlaybackRate(hwo, pdwRate);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.waveOutGetPosition)
        LogMissingFunctionCalled(_T("waveOutGetPosition"));

    return importTable.waveOutGetPosition(hwo, pmmt, cbmmt);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
{
    Initialize();

    if (NULL == importTable.waveOutGetVolume)
        LogMissingFunctionCalled(_T("waveOutGetVolume"));

    return importTable.waveOutGetVolume(hwo, pdwVolume);
}

// ---------

DWORD ImportApiWinMM::waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.waveOutMessage)
        LogMissingFunctionCalled(_T("waveOutMessage"));

    return importTable.waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.waveOutOpen)
        LogMissingFunctionCalled(_T("waveOutOpen"));

    return importTable.waveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::waveOutPause(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutPause)
        LogMissingFunctionCalled(_T("waveOutPause"));

    return importTable.waveOutPause(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveOutPrepareHeader)
        LogMissingFunctionCalled(_T("waveOutPrepareHeader"));

    return importTable.waveOutPrepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutReset(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutReset)
        LogMissingFunctionCalled(_T("waveOutReset"));
    
    return importTable.waveOutReset(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutRestart(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutRestart)
        LogMissingFunctionCalled(_T("waveOutRestart"));
    
    return importTable.waveOutRestart(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
{
    Initialize();

    if (NULL == importTable.waveOutSetPitch)
        LogMissingFunctionCalled(_T("waveOutSetPitch"));
    
    return importTable.waveOutSetPitch(hwo, dwPitch);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
{
    Initialize();

    if (NULL == importTable.waveOutSetPlaybackRate)
        LogMissingFunctionCalled(_T("waveOutSetPlaybackRate"));
    
    return importTable.waveOutSetPlaybackRate(hwo, dwRate);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
{
    Initialize();

    if (NULL == importTable.waveOutSetVolume)
        LogMissingFunctionCalled(_T("waveOutSetVolume"));
    
    return importTable.waveOutSetVolume(hwo, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveOutUnprepareHeader)
        LogMissingFunctionCalled(_T("waveOutUnprepareHeader"));
    
    return importTable.waveOutUnprepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveOutWrite)
        LogMissingFunctionCalled(_T("waveOutWrite"));
    
    return importTable.waveOutWrite(hwo, pwh, cbwh);
}


// -------- HELPERS -------------------------------------------------------- //
// See "ImportApiWinMM.h" for documentation.

void ImportApiWinMM::LogImportFailed(LPCTSTR functionName)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, _T("Import library is missing WinMM function \"%s\". Attempts to call it will fail."), functionName);
}

// --------

void ImportApiWinMM::LogInitializeLibraryPath(LPCTSTR libraryPath)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, _T("Attempting to import WinMM functions from \"%s\"."), libraryPath);
}

// --------

void ImportApiWinMM::LogInitializeFailed(void)
{
    Log::WriteLogMessage(ELogLevel::LogLevelError, _T("Failed to initialize imported WinMM functions."));
}

// --------

void ImportApiWinMM::LogInitializeSucceeded(void)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, _T("Successfully initialized imported WinMM functions."));
}

// --------

void ImportApiWinMM::LogMissingFunctionCalled(LPCTSTR functionName)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, _T("Application has attempted to call missing WinMM import function \"%s\"."), functionName);
}
