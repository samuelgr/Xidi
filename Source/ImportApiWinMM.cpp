/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
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
        if (NULL == procAddress) LogImportFailed(L"CloseDriver");
        importTable.CloseDriver = (LRESULT(WINAPI*)(HDRVR, LPARAM, LPARAM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DefDriverProc");
        if (NULL == procAddress) LogImportFailed(L"DefDriverProc");
        importTable.DefDriverProc = (LRESULT(WINAPI*)(DWORD_PTR, HDRVR, UINT, LONG, LONG))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DriverCallback");
        if (NULL == procAddress) LogImportFailed(L"DriverCallback");
        importTable.DriverCallback = (BOOL(WINAPI*)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "DrvGetModuleHandle");
        if (NULL == procAddress) LogImportFailed(L"DrvGetModuleHandle");
        importTable.DrvGetModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "GetDriverModuleHandle");
        if (NULL == procAddress) LogImportFailed(L"GetDriverModuleHandle");
        importTable.GetDriverModuleHandle = (HMODULE(WINAPI*)(HDRVR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "OpenDriver");
        if (NULL == procAddress) LogImportFailed(L"OpenDriver");
        importTable.OpenDriver = (HDRVR(WINAPI*)(LPCWSTR, LPCWSTR, LPARAM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "PlaySoundA");
        if (NULL == procAddress) LogImportFailed(L"PlaySoundA");
        importTable.PlaySoundA = (BOOL(WINAPI*)(LPCSTR, HMODULE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "PlaySoundW");
        if (NULL == procAddress) LogImportFailed(L"PlaySoundW");
        importTable.PlaySoundW = (BOOL(WINAPI*)(LPCWSTR, HMODULE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "SendDriverMessage");
        if (NULL == procAddress) LogImportFailed(L"SendDriverMessage");
        importTable.SendDriverMessage = (LRESULT(WINAPI*)(HDRVR, UINT, LPARAM, LPARAM))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"auxGetDevCapsA");
        importTable.auxGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"auxGetDevCapsW");
        importTable.auxGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPAUXCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"auxGetNumDevs");
        importTable.auxGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxGetVolume");
        if (NULL == procAddress) LogImportFailed(L"auxGetVolume");
        importTable.auxGetVolume = (MMRESULT(WINAPI*)(UINT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxOutMessage");
        if (NULL == procAddress) LogImportFailed(L"auxOutMessage");
        importTable.auxOutMessage = (MMRESULT(WINAPI*)(UINT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "auxSetVolume");
        if (NULL == procAddress) LogImportFailed(L"auxSetVolume");
        importTable.auxSetVolume = (MMRESULT(WINAPI*)(UINT, DWORD))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "joyConfigChanged");
        if (NULL == procAddress) LogImportFailed(L"joyConfigChanged");
        importTable.joyConfigChanged = (MMRESULT(WINAPI*)(DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"joyGetDevCapsA");
        importTable.joyGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"joyGetDevCapsW");
        importTable.joyGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPJOYCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"joyGetNumDevs");
        importTable.joyGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetPos");
        if (NULL == procAddress) LogImportFailed(L"joyGetPos");
        importTable.joyGetPos = (MMRESULT(WINAPI*)(UINT, LPJOYINFO))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetPosEx");
        if (NULL == procAddress) LogImportFailed(L"joyGetPosEx");
        importTable.joyGetPosEx = (MMRESULT(WINAPI*)(UINT, LPJOYINFOEX))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyGetThreshold");
        if (NULL == procAddress) LogImportFailed(L"joyGetThreshold");
        importTable.joyGetThreshold = (MMRESULT(WINAPI*)(UINT, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joyReleaseCapture");
        if (NULL == procAddress) LogImportFailed(L"joyReleaseCapture");
        importTable.joyReleaseCapture = (MMRESULT(WINAPI*)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joySetCapture");
        if (NULL == procAddress) LogImportFailed(L"joySetCapture");
        importTable.joySetCapture = (MMRESULT(WINAPI*)(HWND, UINT, UINT, BOOL))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "joySetThreshold");
        if (NULL == procAddress) LogImportFailed(L"joySetThreshold");
        importTable.joySetThreshold = (MMRESULT(WINAPI*)(UINT, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "mciDriverNotify");
        if (NULL == procAddress) LogImportFailed(L"mciDriverNotify");
        importTable.mciDriverNotify = (decltype(importTable.mciDriverNotify))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciDriverYield");
        if (NULL == procAddress) LogImportFailed(L"mciDriverYield");
        importTable.mciDriverYield = (decltype(importTable.mciDriverYield))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciExecute");
        if (NULL == procAddress) LogImportFailed(L"mciExecute");
        importTable.mciExecute = (decltype(importTable.mciExecute))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciFreeCommandResource");
        if (NULL == procAddress) LogImportFailed(L"mciFreeCommandResource");
        importTable.mciFreeCommandResource = (decltype(importTable.mciFreeCommandResource))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetCreatorTask");
        if (NULL == procAddress) LogImportFailed(L"mciGetCreatorTask");
        importTable.mciGetCreatorTask = (decltype(importTable.mciGetCreatorTask))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDA");
        if (NULL == procAddress) LogImportFailed(L"mciGetDeviceIDA");
        importTable.mciGetDeviceIDA = (decltype(importTable.mciGetDeviceIDA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDW");
        if (NULL == procAddress) LogImportFailed(L"mciGetDeviceIDW");
        importTable.mciGetDeviceIDW = (decltype(importTable.mciGetDeviceIDW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDA");
        if (NULL == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDA");
        importTable.mciGetDeviceIDFromElementIDA = (decltype(importTable.mciGetDeviceIDFromElementIDA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDeviceIDFromElementIDW");
        if (NULL == procAddress) LogImportFailed(L"mciGetDeviceIDFromElementIDW");
        importTable.mciGetDeviceIDFromElementIDW = (decltype(importTable.mciGetDeviceIDFromElementIDW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetDriverData");
        if (NULL == procAddress) LogImportFailed(L"mciGetDriverData");
        importTable.mciGetDriverData = (decltype(importTable.mciGetDriverData))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringA");
        if (NULL == procAddress) LogImportFailed(L"mciGetErrorStringA");
        importTable.mciGetErrorStringA = (decltype(importTable.mciGetErrorStringA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetErrorStringW");
        if (NULL == procAddress) LogImportFailed(L"mciGetErrorStringW");
        importTable.mciGetErrorStringW = (decltype(importTable.mciGetErrorStringW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciGetYieldProc");
        if (NULL == procAddress) LogImportFailed(L"mciGetYieldProc");
        importTable.mciGetYieldProc = (decltype(importTable.mciGetYieldProc))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciLoadCommandResource");
        if (NULL == procAddress) LogImportFailed(L"mciLoadCommandResource");
        importTable.mciLoadCommandResource = (decltype(importTable.mciLoadCommandResource))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendCommandA");
        if (NULL == procAddress) LogImportFailed(L"mciSendCommandA");
        importTable.mciSendCommandA = (decltype(importTable.mciSendCommandA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendCommandW");
        if (NULL == procAddress) LogImportFailed(L"mciSendCommandW");
        importTable.mciSendCommandW = (decltype(importTable.mciSendCommandW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendStringA");
        if (NULL == procAddress) LogImportFailed(L"mciSendStringA");
        importTable.mciSendStringA = (decltype(importTable.mciSendStringA))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSendStringW");
        if (NULL == procAddress) LogImportFailed(L"mciSendStringW");
        importTable.mciSendStringW = (decltype(importTable.mciSendStringW))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSetDriverData");
        if (NULL == procAddress) LogImportFailed(L"mciSetDriverData");
        importTable.mciSetDriverData = (decltype(importTable.mciSetDriverData))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mciSetYieldProc");
        if (NULL == procAddress) LogImportFailed(L"mciSetYieldProc");
        importTable.mciSetYieldProc = (decltype(importTable.mciSetYieldProc))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiConnect");
        if (NULL == procAddress) LogImportFailed(L"midiConnect");
        importTable.midiConnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiDisconnect");
        if (NULL == procAddress) LogImportFailed(L"midiDisconnect");
        importTable.midiDisconnect = (MMRESULT(WINAPI*)(HMIDI, HMIDIOUT, LPVOID))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiInAddBuffer");
        if (NULL == procAddress) LogImportFailed(L"midiInAddBuffer");
        importTable.midiInAddBuffer = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInClose");
        if (NULL == procAddress) LogImportFailed(L"midiInClose");
        importTable.midiInClose = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"midiInGetDevCapsA");
        importTable.midiInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"midiInGetDevCapsW");
        importTable.midiInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIINCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(L"midiInGetErrorTextA");
        importTable.midiInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(L"midiInGetErrorTextW");
        importTable.midiInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetID");
        if (NULL == procAddress) LogImportFailed(L"midiInGetID");
        importTable.midiInGetID = (MMRESULT(WINAPI*)(HMIDIIN, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"midiInGetNumDevs");
        importTable.midiInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInMessage");
        if (NULL == procAddress) LogImportFailed(L"midiInMessage");
        importTable.midiInMessage = (DWORD(WINAPI*)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInOpen");
        if (NULL == procAddress) LogImportFailed(L"midiInOpen");
        importTable.midiInOpen = (MMRESULT(WINAPI*)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInPrepareHeader");
        if (NULL == procAddress) LogImportFailed(L"midiInPrepareHeader");
        importTable.midiInPrepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInReset");
        if (NULL == procAddress) LogImportFailed(L"midiInReset");
        importTable.midiInReset = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInStart");
        if (NULL == procAddress) LogImportFailed(L"midiInStart");
        importTable.midiInStart = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInStop");
        if (NULL == procAddress) LogImportFailed(L"midiInStop");
        importTable.midiInStop = (MMRESULT(WINAPI*)(HMIDIIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiInUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(L"midiInUnprepareHeader");
        importTable.midiInUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIIN, LPMIDIHDR, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiOutCacheDrumPatches");
        if (NULL == procAddress) LogImportFailed(L"midiOutCacheDrumPatches");
        importTable.midiOutCacheDrumPatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutCachePatches");
        if (NULL == procAddress) LogImportFailed(L"midiOutCachePatches");
        importTable.midiOutCachePatches = (MMRESULT(WINAPI*)(HMIDIOUT, UINT, WORD*, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutClose");
        if (NULL == procAddress) LogImportFailed(L"midiOutClose");
        importTable.midiOutClose = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetDevCapsA");
        importTable.midiOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetDevCapsW");
        importTable.midiOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIDIOUTCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetErrorTextA");
        importTable.midiOutGetErrorTextA = (UINT(WINAPI*)(MMRESULT, LPSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetErrorTextW");
        importTable.midiOutGetErrorTextW = (UINT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetID");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetID");
        importTable.midiOutGetID = (MMRESULT(WINAPI*)(HMIDIOUT, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetNumDevs");
        importTable.midiOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutGetVolume");
        if (NULL == procAddress) LogImportFailed(L"midiOutGetVolume");
        importTable.midiOutGetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutLongMsg");
        if (NULL == procAddress) LogImportFailed(L"midiOutLongMsg");
        importTable.midiOutLongMsg = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutMessage");
        if (NULL == procAddress) LogImportFailed(L"midiOutMessage");
        importTable.midiOutMessage = (DWORD(WINAPI*)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutOpen");
        if (NULL == procAddress) LogImportFailed(L"midiOutOpen");
        importTable.midiOutOpen = (MMRESULT(WINAPI*)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutPrepareHeader");
        if (NULL == procAddress) LogImportFailed(L"midiOutPrepareHeader");
        importTable.midiOutPrepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutReset");
        if (NULL == procAddress) LogImportFailed(L"midiOutReset");
        importTable.midiOutReset = (MMRESULT(WINAPI*)(HMIDIOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutSetVolume");
        if (NULL == procAddress) LogImportFailed(L"midiOutSetVolume");
        importTable.midiOutSetVolume = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutShortMsg");
        if (NULL == procAddress) LogImportFailed(L"midiOutShortMsg");
        importTable.midiOutShortMsg = (MMRESULT(WINAPI*)(HMIDIOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiOutUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(L"midiOutUnprepareHeader");
        importTable.midiOutUnprepareHeader = (MMRESULT(WINAPI*)(HMIDIOUT, LPMIDIHDR, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "midiStreamClose");
        if (NULL == procAddress) LogImportFailed(L"midiStreamClose");
        importTable.midiStreamClose = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamOpen");
        if (NULL == procAddress) LogImportFailed(L"midiStreamOpen");
        importTable.midiStreamOpen = (MMRESULT(WINAPI*)(LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamOut");
        if (NULL == procAddress) LogImportFailed(L"midiStreamOut");
        importTable.midiStreamOut = (MMRESULT(WINAPI*)(HMIDISTRM, LPMIDIHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamPause");
        if (NULL == procAddress) LogImportFailed(L"midiStreamPause");
        importTable.midiStreamPause = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamPosition");
        if (NULL == procAddress) LogImportFailed(L"midiStreamPosition");
        importTable.midiStreamPosition = (MMRESULT(WINAPI*)(HMIDISTRM, LPMMTIME, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamProperty");
        if (NULL == procAddress) LogImportFailed(L"midiStreamProperty");
        importTable.midiStreamProperty = (MMRESULT(WINAPI*)(HMIDISTRM, LPBYTE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamRestart");
        if (NULL == procAddress) LogImportFailed(L"midiStreamRestart");
        importTable.midiStreamRestart = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "midiStreamStop");
        if (NULL == procAddress) LogImportFailed(L"midiStreamStop");
        importTable.midiStreamStop = (MMRESULT(WINAPI*)(HMIDISTRM))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "mixerClose");
        if (NULL == procAddress) LogImportFailed(L"mixerClose");
        importTable.mixerClose = (MMRESULT(WINAPI*)(HMIXER))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsA");
        if (NULL == procAddress) LogImportFailed(L"mixerGetControlDetailsA");
        importTable.mixerGetControlDetailsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetControlDetailsW");
        if (NULL == procAddress) LogImportFailed(L"mixerGetControlDetailsW");
        importTable.mixerGetControlDetailsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"mixerGetDevCapsA");
        importTable.mixerGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"mixerGetDevCapsW");
        importTable.mixerGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPMIXERCAPS, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetID");
        if (NULL == procAddress) LogImportFailed(L"mixerGetID");
        importTable.mixerGetID = (MMRESULT(WINAPI*)(HMIXEROBJ, UINT*, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsA");
        if (NULL == procAddress) LogImportFailed(L"mixerGetLineControlsA");
        importTable.mixerGetLineControlsA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineControlsW");
        if (NULL == procAddress) LogImportFailed(L"mixerGetLineControlsW");
        importTable.mixerGetLineControlsW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoA");
        if (NULL == procAddress) LogImportFailed(L"mixerGetLineInfoA");
        importTable.mixerGetLineInfoA = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetLineInfoW");
        if (NULL == procAddress) LogImportFailed(L"mixerGetLineInfoW");
        importTable.mixerGetLineInfoW = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERLINE, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"mixerGetNumDevs");
        importTable.mixerGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerMessage");
        if (NULL == procAddress) LogImportFailed(L"mixerMessage");
        importTable.mixerMessage = (DWORD(WINAPI*)(HMIXER, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerOpen");
        if (NULL == procAddress) LogImportFailed(L"mixerOpen");
        importTable.mixerOpen = (MMRESULT(WINAPI*)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mixerSetControlDetails");
        if (NULL == procAddress) LogImportFailed(L"mixerSetControlDetails");
        importTable.mixerSetControlDetails = (MMRESULT(WINAPI*)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "mmioAdvance");
        if (NULL == procAddress) LogImportFailed(L"mmioAdvance");
        importTable.mmioAdvance = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioAscend");
        if (NULL == procAddress) LogImportFailed(L"mmioAscend");
        importTable.mmioAscend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioClose");
        if (NULL == procAddress) LogImportFailed(L"mmioClose");
        importTable.mmioClose = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioCreateChunk");
        if (NULL == procAddress) LogImportFailed(L"mmioCreateChunk");
        importTable.mmioCreateChunk = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioDescend");
        if (NULL == procAddress) LogImportFailed(L"mmioDescend");
        importTable.mmioDescend = (MMRESULT(WINAPI*)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioFlush");
        if (NULL == procAddress) LogImportFailed(L"mmioFlush");
        importTable.mmioFlush = (MMRESULT(WINAPI*)(HMMIO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioGetInfo");
        if (NULL == procAddress) LogImportFailed(L"mmioGetInfo");
        importTable.mmioGetInfo = (MMRESULT(WINAPI*)(HMMIO, LPMMIOINFO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcA");
        if (NULL == procAddress) LogImportFailed(L"mmioInstallIOProcA");
        importTable.mmioInstallIOProcA = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioInstallIOProcW");
        if (NULL == procAddress) LogImportFailed(L"mmioInstallIOProcW");
        importTable.mmioInstallIOProcW = (LPMMIOPROC(WINAPI*)(FOURCC, LPMMIOPROC, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioOpenA");
        if (NULL == procAddress) LogImportFailed(L"mmioOpenA");
        importTable.mmioOpenA = (HMMIO(WINAPI*)(LPSTR, LPMMIOINFO, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioOpenW");
        if (NULL == procAddress) LogImportFailed(L"mmioOpenW");
        importTable.mmioOpenW = (HMMIO(WINAPI*)(LPWSTR, LPMMIOINFO, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioRead");
        if (NULL == procAddress) LogImportFailed(L"mmioRead");
        importTable.mmioRead = (LONG(WINAPI*)(HMMIO, HPSTR, LONG))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioRenameA");
        if (NULL == procAddress) LogImportFailed(L"mmioRenameA");
        importTable.mmioRenameA = (MMRESULT(WINAPI*)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioRenameW");
        if (NULL == procAddress) LogImportFailed(L"mmioRenameW");
        importTable.mmioRenameW = (MMRESULT(WINAPI*)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioSeek");
        if (NULL == procAddress) LogImportFailed(L"mmioSeek");
        importTable.mmioSeek = (LONG(WINAPI*)(HMMIO, LONG, int))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioSendMessage");
        if (NULL == procAddress) LogImportFailed(L"mmioSendMessage");
        importTable.mmioSendMessage = (LRESULT(WINAPI*)(HMMIO, UINT, LPARAM, LPARAM))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioSetBuffer");
        if (NULL == procAddress) LogImportFailed(L"mmioSetBuffer");
        importTable.mmioSetBuffer = (MMRESULT(WINAPI*)(HMMIO, LPSTR, LONG, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioSetInfo");
        if (NULL == procAddress) LogImportFailed(L"mmioSetInfo");
        importTable.mmioSetInfo = (MMRESULT(WINAPI*)(HMMIO, LPCMMIOINFO, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCA");
        if (NULL == procAddress) LogImportFailed(L"mmioStringToFOURCCA");
        importTable.mmioStringToFOURCCA = (FOURCC(WINAPI*)(LPCSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioStringToFOURCCW");
        if (NULL == procAddress) LogImportFailed(L"mmioStringToFOURCCW");
        importTable.mmioStringToFOURCCW = (FOURCC(WINAPI*)(LPCWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "mmioWrite");
        if (NULL == procAddress) LogImportFailed(L"mmioWrite");
        importTable.mmioWrite = (LONG(WINAPI*)(HMMIO, const char*, LONG))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundA");
        if (NULL == procAddress) LogImportFailed(L"sndPlaySoundA");
        importTable.sndPlaySoundA = (BOOL(WINAPI*)(LPCSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "sndPlaySoundW");
        if (NULL == procAddress) LogImportFailed(L"sndPlaySoundW");
        importTable.sndPlaySoundW = (BOOL(WINAPI*)(LPCWSTR, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "timeBeginPeriod");
        if (NULL == procAddress) LogImportFailed(L"timeBeginPeriod");
        importTable.timeBeginPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeEndPeriod");
        if (NULL == procAddress) LogImportFailed(L"timeEndPeriod");
        importTable.timeEndPeriod = (MMRESULT(WINAPI*)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeGetDevCaps");
        if (NULL == procAddress) LogImportFailed(L"timeGetDevCaps");
        importTable.timeGetDevCaps = (MMRESULT(WINAPI*)(LPTIMECAPS, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeGetSystemTime");
        if (NULL == procAddress) LogImportFailed(L"timeGetSystemTime");
        importTable.timeGetSystemTime = (MMRESULT(WINAPI*)(LPMMTIME, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeGetTime");
        if (NULL == procAddress) LogImportFailed(L"timeGetTime");
        importTable.timeGetTime = (DWORD(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeKillEvent");
        if (NULL == procAddress) LogImportFailed(L"timeKillEvent");
        importTable.timeKillEvent = (MMRESULT(WINAPI*)(UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "timeSetEvent");
        if (NULL == procAddress) LogImportFailed(L"timeSetEvent");
        importTable.timeSetEvent = (MMRESULT(WINAPI*)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "waveInAddBuffer");
        if (NULL == procAddress) LogImportFailed(L"waveInAddBuffer");
        importTable.waveInAddBuffer = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInClose");
        if (NULL == procAddress) LogImportFailed(L"waveInClose");
        importTable.waveInClose = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"waveInGetDevCapsA");
        importTable.waveInGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"waveInGetDevCapsW");
        importTable.waveInGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEINCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(L"waveInGetErrorTextA");
        importTable.waveInGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(L"waveInGetErrorTextW");
        importTable.waveInGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetID");
        if (NULL == procAddress) LogImportFailed(L"waveInGetID");
        importTable.waveInGetID = (MMRESULT(WINAPI*)(HWAVEIN, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"waveInGetNumDevs");
        importTable.waveInGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInGetPosition");
        if (NULL == procAddress) LogImportFailed(L"waveInGetPosition");
        importTable.waveInGetPosition = (MMRESULT(WINAPI*)(HWAVEIN, LPMMTIME, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInMessage");
        if (NULL == procAddress) LogImportFailed(L"waveInMessage");
        importTable.waveInMessage = (DWORD(WINAPI*)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInOpen");
        if (NULL == procAddress) LogImportFailed(L"waveInOpen");
        importTable.waveInOpen = (MMRESULT(WINAPI*)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInPrepareHeader");
        if (NULL == procAddress) LogImportFailed(L"waveInPrepareHeader");
        importTable.waveInPrepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInReset");
        if (NULL == procAddress) LogImportFailed(L"waveInReset");
        importTable.waveInReset = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInStart");
        if (NULL == procAddress) LogImportFailed(L"waveInStart");
        importTable.waveInStart = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInStop");
        if (NULL == procAddress) LogImportFailed(L"waveInStop");
        importTable.waveInStop = (MMRESULT(WINAPI*)(HWAVEIN))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveInUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(L"waveInUnprepareHeader");
        importTable.waveInUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEIN, LPWAVEHDR, UINT))procAddress;

        // ---------

        procAddress = GetProcAddress(loadedLibrary, "waveOutBreakLoop");
        if (NULL == procAddress) LogImportFailed(L"waveOutBreakLoop");
        importTable.waveOutBreakLoop = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutClose");
        if (NULL == procAddress) LogImportFailed(L"waveOutClose");
        importTable.waveOutClose = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsA");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetDevCapsA");
        importTable.waveOutGetDevCapsA = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSA, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetDevCapsW");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetDevCapsW");
        importTable.waveOutGetDevCapsW = (MMRESULT(WINAPI*)(UINT_PTR, LPWAVEOUTCAPSW, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextA");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetErrorTextA");
        importTable.waveOutGetErrorTextA = (MMRESULT(WINAPI*)(MMRESULT, LPCSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetErrorTextW");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetErrorTextW");
        importTable.waveOutGetErrorTextW = (MMRESULT(WINAPI*)(MMRESULT, LPWSTR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetID");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetID");
        importTable.waveOutGetID = (MMRESULT(WINAPI*)(HWAVEOUT, LPUINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetNumDevs");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetNumDevs");
        importTable.waveOutGetNumDevs = (UINT(WINAPI*)(void))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPitch");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetPitch");
        importTable.waveOutGetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPlaybackRate");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetPlaybackRate");
        importTable.waveOutGetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetPosition");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetPosition");
        importTable.waveOutGetPosition = (MMRESULT(WINAPI*)(HWAVEOUT, LPMMTIME, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutGetVolume");
        if (NULL == procAddress) LogImportFailed(L"waveOutGetVolume");
        importTable.waveOutGetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, LPDWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutMessage");
        if (NULL == procAddress) LogImportFailed(L"waveOutMessage");
        importTable.waveOutMessage = (DWORD(WINAPI*)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutOpen");
        if (NULL == procAddress) LogImportFailed(L"waveOutOpen");
        importTable.waveOutOpen = (MMRESULT(WINAPI*)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutPause");
        if (NULL == procAddress) LogImportFailed(L"waveOutPause");
        importTable.waveOutPause = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutPrepareHeader");
        if (NULL == procAddress) LogImportFailed(L"waveOutPrepareHeader");
        importTable.waveOutPrepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutReset");
        if (NULL == procAddress) LogImportFailed(L"waveOutReset");
        importTable.waveOutReset = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutRestart");
        if (NULL == procAddress) LogImportFailed(L"waveOutRestart");
        importTable.waveOutRestart = (MMRESULT(WINAPI*)(HWAVEOUT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutSetPitch");
        if (NULL == procAddress) LogImportFailed(L"waveOutSetPitch");
        importTable.waveOutSetPitch = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutSetPlaybackRate");
        if (NULL == procAddress) LogImportFailed(L"waveOutSetPlaybackRate");
        importTable.waveOutSetPlaybackRate = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutSetVolume");
        if (NULL == procAddress) LogImportFailed(L"waveOutSetVolume");
        importTable.waveOutSetVolume = (MMRESULT(WINAPI*)(HWAVEOUT, DWORD))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutUnprepareHeader");
        if (NULL == procAddress) LogImportFailed(L"waveOutUnprepareHeader");
        importTable.waveOutUnprepareHeader = (MMRESULT(WINAPI*)(HWAVEOUT, LPWAVEHDR, UINT))procAddress;

        procAddress = GetProcAddress(loadedLibrary, "waveOutWrite");
        if (NULL == procAddress) LogImportFailed(L"waveOutWrite");
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
        LogMissingFunctionCalled(L"CloseDriver");

    return importTable.CloseDriver(hdrvr, lParam1, lParam2);
}

// ---------

LRESULT ImportApiWinMM::DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
{
    Initialize();

    if (NULL == importTable.DefDriverProc)
        LogMissingFunctionCalled(L"DefDriverProc");

    return importTable.DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
}

// ---------

BOOL ImportApiWinMM::DriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
    Initialize();

    if (NULL == importTable.DriverCallback)
        LogMissingFunctionCalled(L"DriverCallback");

    return importTable.DriverCallback(dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
}

// ---------

HMODULE ImportApiWinMM::DrvGetModuleHandle(HDRVR hDriver)
{
    Initialize();

    if (NULL == importTable.DrvGetModuleHandle)
        LogMissingFunctionCalled(L"DrvGetModuleHandle");

    return importTable.DrvGetModuleHandle(hDriver);
}

// ---------

HMODULE ImportApiWinMM::GetDriverModuleHandle(HDRVR hdrvr)
{
    Initialize();

    if (NULL == importTable.GetDriverModuleHandle)
        LogMissingFunctionCalled(L"GetDriverModuleHandle");

    return importTable.GetDriverModuleHandle(hdrvr);
}

// ---------

HDRVR ImportApiWinMM::OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
{
    Initialize();

    if (NULL == importTable.OpenDriver)
        LogMissingFunctionCalled(L"OpenDriver");

    return importTable.OpenDriver(lpDriverName, lpSectionName, lParam);
}

// ---------

BOOL ImportApiWinMM::PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    Initialize();

    if (NULL == importTable.PlaySoundA)
        LogMissingFunctionCalled(L"PlaySoundA");

    return importTable.PlaySoundA(pszSound, hmod, fdwSound);
}

// ---------

BOOL ImportApiWinMM::PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    Initialize();

    if (NULL == importTable.PlaySoundW)
        LogMissingFunctionCalled(L"PlaySoundW");

    return importTable.PlaySoundW(pszSound, hmod, fdwSound);
}

// ---------

LRESULT ImportApiWinMM::SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    Initialize();

    if (NULL == importTable.SendDriverMessage)
        LogMissingFunctionCalled(L"SendDriverMessage");

    return importTable.SendDriverMessage(hdrvr, msg, lParam1, lParam2);
}

// ---------

MMRESULT ImportApiWinMM::auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
{
    Initialize();

    if (NULL == importTable.auxGetDevCapsA)
        LogMissingFunctionCalled(L"auxGetDevCapsA");

    return importTable.auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
}

// ---------

MMRESULT ImportApiWinMM::auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
{
    Initialize();

    if (NULL == importTable.auxGetDevCapsW)
        LogMissingFunctionCalled(L"auxGetDevCapsW");

    return importTable.auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
}

// ---------

UINT ImportApiWinMM::auxGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.auxGetNumDevs)
        LogMissingFunctionCalled(L"auxGetNumDevs");

    return importTable.auxGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
{
    Initialize();

    if (NULL == importTable.auxGetVolume)
        LogMissingFunctionCalled(L"auxGetVolume");

    return importTable.auxGetVolume(uDeviceID, lpdwVolume);
}

// ---------

MMRESULT ImportApiWinMM::auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.auxOutMessage)
        LogMissingFunctionCalled(L"auxOutMessage");

    return importTable.auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::auxSetVolume(UINT uDeviceID, DWORD dwVolume)
{
    Initialize();

    if (NULL == importTable.auxSetVolume)
        LogMissingFunctionCalled(L"auxSetVolume");

    return importTable.auxSetVolume(uDeviceID, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::joyConfigChanged(DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.joyConfigChanged)
        LogMissingFunctionCalled(L"joyConfigChanged");

    return importTable.joyConfigChanged(dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    Initialize();

    if (NULL == importTable.joyGetDevCapsA)
        LogMissingFunctionCalled(L"joyGetDevCapsA");

    return importTable.joyGetDevCapsA(uJoyID, pjc, cbjc);
}

// ---------

MMRESULT ImportApiWinMM::joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    Initialize();

    if (NULL == importTable.joyGetDevCapsW)
        LogMissingFunctionCalled(L"joyGetDevCapsW");

    return importTable.joyGetDevCapsW(uJoyID, pjc, cbjc);
}

// ---------

UINT ImportApiWinMM::joyGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.joyGetNumDevs)
        LogMissingFunctionCalled(L"joyGetNumDevs");

    return importTable.joyGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::joyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    Initialize();

    if (NULL == importTable.joyGetPos)
        LogMissingFunctionCalled(L"joyGetPos");

    return importTable.joyGetPos(uJoyID, pji);
}

// ---------

MMRESULT ImportApiWinMM::joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    Initialize();

    if (NULL == importTable.joyGetPosEx)
        LogMissingFunctionCalled(L"joyGetPosEx");

    return importTable.joyGetPosEx(uJoyID, pji);
}

// ---------

MMRESULT ImportApiWinMM::joyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    Initialize();

    if (NULL == importTable.joyGetThreshold)
        LogMissingFunctionCalled(L"joyGetThreshold");

    return importTable.joyGetThreshold(uJoyID, puThreshold);
}

// ---------

MMRESULT ImportApiWinMM::joyReleaseCapture(UINT uJoyID)
{
    Initialize();

    if (NULL == importTable.joyReleaseCapture)
        LogMissingFunctionCalled(L"joyReleaseCapture");

    return importTable.joyReleaseCapture(uJoyID);
}

// ---------

MMRESULT ImportApiWinMM::joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    Initialize();

    if (NULL == importTable.joySetCapture)
        LogMissingFunctionCalled(L"joySetCapture");

    return importTable.joySetCapture(hwnd, uJoyID, uPeriod, fChanged);
}

// ---------

MMRESULT ImportApiWinMM::joySetThreshold(UINT uJoyID, UINT uThreshold)
{
    Initialize();

    if (NULL == importTable.joySetThreshold)
        LogMissingFunctionCalled(L"joySetThreshold");

    return importTable.joySetThreshold(uJoyID, uThreshold);
}

// ---------

BOOL ImportApiWinMM::mciDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
{
    Initialize();

    if (NULL == importTable.mciDriverNotify)
        LogMissingFunctionCalled(L"mciDriverNotify");

    return importTable.mciDriverNotify(hwndCallback, IDDevice, uStatus);
}

// ---------

UINT ImportApiWinMM::mciDriverYield(MCIDEVICEID IDDevice)
{
    Initialize();

    if (NULL == importTable.mciDriverYield)
        LogMissingFunctionCalled(L"mciDriverYield");

    return importTable.mciDriverYield(IDDevice);
}

// ---------

BOOL ImportApiWinMM::mciExecute(LPCSTR pszCommand)
{
    Initialize();

    if (NULL == importTable.mciExecute)
        LogMissingFunctionCalled(L"mciExecute");

    return importTable.mciExecute(pszCommand);
}

// ---------

BOOL ImportApiWinMM::mciFreeCommandResource(UINT uResource)
{
    Initialize();

    if (NULL == importTable.mciFreeCommandResource)
        LogMissingFunctionCalled(L"mciFreeCommandResource");

    return importTable.mciFreeCommandResource(uResource);
}

// ---------

HANDLE ImportApiWinMM::mciGetCreatorTask(MCIDEVICEID IDDevice)
{
    Initialize();

    if (NULL == importTable.mciGetCreatorTask)
        LogMissingFunctionCalled(L"mciGetCreatorTask");

    return importTable.mciGetCreatorTask(IDDevice);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDA(LPCSTR lpszDevice)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDA)
        LogMissingFunctionCalled(L"mciGetDeviceIDA");

    return importTable.mciGetDeviceIDA(lpszDevice);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDW(LPCWSTR lpszDevice)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDW)
        LogMissingFunctionCalled(L"mciGetDeviceIDW");

    return importTable.mciGetDeviceIDW(lpszDevice);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDFromElementIDA)
        LogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDA");

    return importTable.mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
}

// ---------

MCIDEVICEID ImportApiWinMM::mciGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType)
{
    Initialize();

    if (NULL == importTable.mciGetDeviceIDFromElementIDW)
        LogMissingFunctionCalled(L"mciGetDeviceIDFromElementIDW");

    return importTable.mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
}

// ---------

DWORD_PTR ImportApiWinMM::mciGetDriverData(MCIDEVICEID IDDevice)
{
    Initialize();

    if (NULL == importTable.mciGetDriverData)
        LogMissingFunctionCalled(L"mciGetDriverData");

    return importTable.mciGetDriverData(IDDevice);
}

// ---------

BOOL ImportApiWinMM::mciGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
{
    Initialize();

    if (NULL == importTable.mciGetErrorStringA)
        LogMissingFunctionCalled(L"mciGetErrorStringA");

    return importTable.mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
}

// ---------

BOOL ImportApiWinMM::mciGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
{
    Initialize();

    if (NULL == importTable.mciGetErrorStringW)
        LogMissingFunctionCalled(L"mciGetErrorStringW");

    return importTable.mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
}

// ---------

YIELDPROC ImportApiWinMM::mciGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
{
    Initialize();

    if (NULL == importTable.mciGetYieldProc)
        LogMissingFunctionCalled(L"mciGetYieldProc");

    return importTable.mciGetYieldProc(IDDevice, lpdwYieldData);
}

// ---------

UINT ImportApiWinMM::mciLoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
{
    Initialize();

    if (NULL == importTable.mciLoadCommandResource)
        LogMissingFunctionCalled(L"mciLoadCommandResource");

    return importTable.mciLoadCommandResource(hInst, lpwstrResourceName, uType);
}

// ---------

MCIERROR ImportApiWinMM::mciSendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
    Initialize();

    if (NULL == importTable.mciSendCommandA)
        LogMissingFunctionCalled(L"mciSendCommandA");

    return importTable.mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
}

// ---------

MCIERROR ImportApiWinMM::mciSendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
    Initialize();

    if (NULL == importTable.mciSendCommandW)
        LogMissingFunctionCalled(L"mciSendCommandW");

    return importTable.mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
}

// ---------

MCIERROR ImportApiWinMM::mciSendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    Initialize();

    if (NULL == importTable.mciSendStringA)
        LogMissingFunctionCalled(L"mciSendStringA");

    return importTable.mciSendStringA(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
}

// ---------

MCIERROR ImportApiWinMM::mciSendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    Initialize();

    if (NULL == importTable.mciSendStringW)
        LogMissingFunctionCalled(L"mciSendStringW");

    return importTable.mciSendStringW(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
}

// ---------

BOOL ImportApiWinMM::mciSetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
{
    Initialize();

    if (NULL == importTable.mciSetDriverData)
        LogMissingFunctionCalled(L"mciSetDriverData");

    return importTable.mciSetDriverData(IDDevice, data);
}

// ---------

UINT ImportApiWinMM::mciSetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
{
    Initialize();

    if (NULL == importTable.mciSetYieldProc)
        LogMissingFunctionCalled(L"mciSetYieldProc");

    return importTable.mciSetYieldProc(IDDevice, yp, dwYieldData);
}

// ---------

MMRESULT ImportApiWinMM::midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
{
    Initialize();

    if (NULL == importTable.midiConnect)
        LogMissingFunctionCalled(L"midiConnect");

    return importTable.midiConnect(hMidi, hmo, pReserved);
}

// ---------

MMRESULT ImportApiWinMM::midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
{
    Initialize();

    if (NULL == importTable.midiDisconnect)
        LogMissingFunctionCalled(L"midiDisconnect");

    return importTable.midiDisconnect(hMidi, hmo, pReserved);
}

// ---------

MMRESULT ImportApiWinMM::midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    Initialize();

    if (NULL == importTable.midiInAddBuffer)
        LogMissingFunctionCalled(L"midiInAddBuffer");

    return importTable.midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiInClose(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInClose)
        LogMissingFunctionCalled(L"midiInClose");

    return importTable.midiInClose(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
{
    Initialize();

    if (NULL == importTable.midiInGetDevCapsA)
        LogMissingFunctionCalled(L"midiInGetDevCapsA");

    return importTable.midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
{
    Initialize();

    if (NULL == importTable.midiInGetDevCapsW)
        LogMissingFunctionCalled(L"midiInGetDevCapsW");

    return importTable.midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiInGetErrorTextA)
        LogMissingFunctionCalled(L"midiInGetErrorTextA");

    return importTable.midiInGetErrorTextA(wError, lpText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiInGetErrorTextW)
        LogMissingFunctionCalled(L"midiInGetErrorTextW");

    return importTable.midiInGetErrorTextW(wError, lpText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::midiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.midiInGetID)
        LogMissingFunctionCalled(L"midiInGetID");

    return importTable.midiInGetID(hmi, puDeviceID);
}

// ---------

UINT ImportApiWinMM::midiInGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.midiInGetNumDevs)
        LogMissingFunctionCalled(L"midiInGetNumDevs");

    return importTable.midiInGetNumDevs();
}

// ---------

DWORD ImportApiWinMM::midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    Initialize();

    if (NULL == importTable.midiInMessage)
        LogMissingFunctionCalled(L"midiInMessage");

    return importTable.midiInMessage(deviceID, msg, dw1, dw2);
}

// ---------

MMRESULT ImportApiWinMM::midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.midiInOpen)
        LogMissingFunctionCalled(L"midiInOpen");

    return importTable.midiInOpen(lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    Initialize();

    if (NULL == importTable.midiInPrepareHeader)
        LogMissingFunctionCalled(L"midiInPrepareHeader");

    return importTable.midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiInReset(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInReset)
        LogMissingFunctionCalled(L"midiInReset");

    return importTable.midiInReset(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInStart(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInStart)
        LogMissingFunctionCalled(L"midiInStart");

    return importTable.midiInStart(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInStop(HMIDIIN hMidiIn)
{
    Initialize();

    if (NULL == importTable.midiInStop)
        LogMissingFunctionCalled(L"midiInStop");

    return importTable.midiInStop(hMidiIn);
}

// ---------

MMRESULT ImportApiWinMM::midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    Initialize();

    if (NULL == importTable.midiInUnprepareHeader)
        LogMissingFunctionCalled(L"midiInUnprepareHeader");

    return importTable.midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.midiOutCacheDrumPatches)
        LogMissingFunctionCalled(L"midiOutCacheDrumPatches");

    return importTable.midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.midiOutCachePatches)
        LogMissingFunctionCalled(L"midiOutCachePatches");

    return importTable.midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiOutClose(HMIDIOUT hmo)
{
    Initialize();

    if (NULL == importTable.midiOutClose)
        LogMissingFunctionCalled(L"midiOutClose");

    return importTable.midiOutClose(hmo);
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
{
    Initialize();

    if (NULL == importTable.midiOutGetDevCapsA)
        LogMissingFunctionCalled(L"midiOutGetDevCapsA");

    return importTable.midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
{
    Initialize();

    if (NULL == importTable.midiOutGetDevCapsW)
        LogMissingFunctionCalled(L"midiOutGetDevCapsW");

    return importTable.midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
}

// ---------

UINT ImportApiWinMM::midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiOutGetErrorTextA)
        LogMissingFunctionCalled(L"midiOutGetErrorTextA");

    return importTable.midiOutGetErrorTextA(mmrError, lpText, cchText);
}

// ---------

UINT ImportApiWinMM::midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.midiOutGetErrorTextW)
        LogMissingFunctionCalled(L"midiOutGetErrorTextW");

    return importTable.midiOutGetErrorTextW(mmrError, lpText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.midiOutGetID)
        LogMissingFunctionCalled(L"midiOutGetID");

    return importTable.midiOutGetID(hmo, puDeviceID);
}

// ---------

UINT ImportApiWinMM::midiOutGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.midiOutGetNumDevs)
        LogMissingFunctionCalled(L"midiOutGetNumDevs");

    return importTable.midiOutGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
{
    Initialize();

    if (NULL == importTable.midiOutGetVolume)
        LogMissingFunctionCalled(L"midiOutGetVolume");

    return importTable.midiOutGetVolume(hmo, lpdwVolume);
}

// ---------

MMRESULT ImportApiWinMM::midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    Initialize();

    if (NULL == importTable.midiOutLongMsg)
        LogMissingFunctionCalled(L"midiOutLongMsg");

    return importTable.midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

DWORD ImportApiWinMM::midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    Initialize();

    if (NULL == importTable.midiOutMessage)
        LogMissingFunctionCalled(L"midiOutMessage");

    return importTable.midiOutMessage(deviceID, msg, dw1, dw2);
}

// ---------

MMRESULT ImportApiWinMM::midiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.midiOutOpen)
        LogMissingFunctionCalled(L"midiOutOpen");

    return importTable.midiOutOpen(lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
}

// ---------

MMRESULT ImportApiWinMM::midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    Initialize();

    if (NULL == importTable.midiOutPrepareHeader)
        LogMissingFunctionCalled(L"midiOutPrepareHeader");

    return importTable.midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiOutReset(HMIDIOUT hmo)
{
    Initialize();

    if (NULL == importTable.midiOutReset)
        LogMissingFunctionCalled(L"midiOutReset");

    return importTable.midiOutReset(hmo);
}

// ---------

MMRESULT ImportApiWinMM::midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
{
    Initialize();

    if (NULL == importTable.midiOutSetVolume)
        LogMissingFunctionCalled(L"midiOutSetVolume");

    return importTable.midiOutSetVolume(hmo, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
{
    Initialize();

    if (NULL == importTable.midiOutShortMsg)
        LogMissingFunctionCalled(L"midiOutShortMsg");

    return importTable.midiOutShortMsg(hmo, dwMsg);
}

// ---------

MMRESULT ImportApiWinMM::midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    Initialize();

    if (NULL == importTable.midiOutUnprepareHeader)
        LogMissingFunctionCalled(L"midiOutUnprepareHeader");

    return importTable.midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamClose(HMIDISTRM hStream)
{
    Initialize();

    if (NULL == importTable.midiStreamClose)
        LogMissingFunctionCalled(L"midiStreamClose");

    return importTable.midiStreamClose(hStream);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.midiStreamOpen)
        LogMissingFunctionCalled(L"midiStreamOpen");

    return importTable.midiStreamOpen(lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
{
    Initialize();

    if (NULL == importTable.midiStreamOut)
        LogMissingFunctionCalled(L"midiStreamOut");

    return importTable.midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamPause(HMIDISTRM hms)
{
    Initialize();

    if (NULL == importTable.midiStreamPause)
        LogMissingFunctionCalled(L"midiStreamPause");

    return importTable.midiStreamPause(hms);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.midiStreamPosition)
        LogMissingFunctionCalled(L"midiStreamPosition");

    return importTable.midiStreamPosition(hms, pmmt, cbmmt);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
{
    Initialize();

    if (NULL == importTable.midiStreamProperty)
        LogMissingFunctionCalled(L"midiStreamProperty");

    return importTable.midiStreamProperty(hm, lppropdata, dwProperty);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamRestart(HMIDISTRM hms)
{
    Initialize();

    if (NULL == importTable.midiStreamRestart)
        LogMissingFunctionCalled(L"midiStreamRestart");

    return importTable.midiStreamRestart(hms);
}

// ---------

MMRESULT ImportApiWinMM::midiStreamStop(HMIDISTRM hms)
{
    Initialize();

    if (NULL == importTable.midiStreamStop)
        LogMissingFunctionCalled(L"midiStreamStop");

    return importTable.midiStreamStop(hms);
}

// ---------

MMRESULT ImportApiWinMM::mixerClose(HMIXER hmx)
{
    Initialize();

    if (NULL == importTable.mixerClose)
        LogMissingFunctionCalled(L"mixerClose");

    return importTable.mixerClose(hmx);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    Initialize();

    if (NULL == importTable.mixerGetControlDetailsA)
        LogMissingFunctionCalled(L"mixerGetControlDetailsA");

    return importTable.mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    Initialize();

    if (NULL == importTable.mixerGetControlDetailsW)
        LogMissingFunctionCalled(L"mixerGetControlDetailsW");

    return importTable.mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
{
    Initialize();

    if (NULL == importTable.mixerGetDevCapsA)
        LogMissingFunctionCalled(L"mixerGetDevCapsA");

    return importTable.mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
{
    Initialize();

    if (NULL == importTable.mixerGetDevCapsW)
        LogMissingFunctionCalled(L"mixerGetDevCapsW");

    return importTable.mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
{
    Initialize();

    if (NULL == importTable.mixerGetID)
        LogMissingFunctionCalled(L"mixerGetID");

    return importTable.mixerGetID(hmxobj, puMxId, fdwId);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
{
    Initialize();

    if (NULL == importTable.mixerGetLineControlsA)
        LogMissingFunctionCalled(L"mixerGetLineControlsA");

    return importTable.mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
{
    Initialize();

    if (NULL == importTable.mixerGetLineControlsW)
        LogMissingFunctionCalled(L"mixerGetLineControlsW");

    return importTable.mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
{
    Initialize();

    if (NULL == importTable.mixerGetLineInfoA)
        LogMissingFunctionCalled(L"mixerGetLineInfoA");

    return importTable.mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
}

// ---------

MMRESULT ImportApiWinMM::mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
{
    Initialize();

    if (NULL == importTable.mixerGetLineInfoW)
        LogMissingFunctionCalled(L"mixerGetLineInfoW");

    return importTable.mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
}

// ---------

UINT ImportApiWinMM::mixerGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.mixerGetNumDevs)
        LogMissingFunctionCalled(L"mixerGetNumDevs");

    return importTable.mixerGetNumDevs();
}

// ---------

DWORD ImportApiWinMM::mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.mixerMessage)
        LogMissingFunctionCalled(L"mixerMessage");

    return importTable.mixerMessage(driverID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.mixerOpen)
        LogMissingFunctionCalled(L"mixerOpen");

    return importTable.mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    Initialize();

    if (NULL == importTable.mixerSetControlDetails)
        LogMissingFunctionCalled(L"mixerSetControlDetails");

    return importTable.mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT ImportApiWinMM::mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioAdvance)
        LogMissingFunctionCalled(L"mmioAdvance");

    return importTable.mmioAdvance(hmmio, lpmmioinfo, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioAscend)
        LogMissingFunctionCalled(L"mmioAscend");

    return importTable.mmioAscend(hmmio, lpck, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioClose(HMMIO hmmio, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioClose)
        LogMissingFunctionCalled(L"mmioClose");

    return importTable.mmioClose(hmmio, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioCreateChunk)
        LogMissingFunctionCalled(L"mmioCreateChunk");

    return importTable.mmioCreateChunk(hmmio, lpck, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioDescend)
        LogMissingFunctionCalled(L"mmioDescend");

    return importTable.mmioDescend(hmmio, lpck, lpckParent, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioFlush(HMMIO hmmio, UINT fuFlush)
{
    Initialize();

    if (NULL == importTable.mmioFlush)
        LogMissingFunctionCalled(L"mmioFlush");

    return importTable.mmioFlush(hmmio, fuFlush);
}

// ---------


MMRESULT ImportApiWinMM::mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioGetInfo)
        LogMissingFunctionCalled(L"mmioGetInfo");

    return importTable.mmioGetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


LPMMIOPROC ImportApiWinMM::mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.mmioInstallIOProcA)
        LogMissingFunctionCalled(L"mmioInstallIOProcA");

    return importTable.mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
}

// ---------


LPMMIOPROC ImportApiWinMM::mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    Initialize();

    if (NULL == importTable.mmioInstallIOProcW)
        LogMissingFunctionCalled(L"mmioInstallIOProcW");

    return importTable.mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
}

// ---------


HMMIO ImportApiWinMM::mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    Initialize();

    if (NULL == importTable.mmioOpenA)
        LogMissingFunctionCalled(L"mmioOpenA");

    return importTable.mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


HMMIO ImportApiWinMM::mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    Initialize();

    if (NULL == importTable.mmioOpenW)
        LogMissingFunctionCalled(L"mmioOpenW");

    return importTable.mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


LONG ImportApiWinMM::mmioRead(HMMIO hmmio, HPSTR pch, LONG cch)
{
    Initialize();

    if (NULL == importTable.mmioRead)
        LogMissingFunctionCalled(L"mmioRead");

    return importTable.mmioRead(hmmio, pch, cch);
}

// ---------


MMRESULT ImportApiWinMM::mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    Initialize();

    if (NULL == importTable.mmioRenameA)
        LogMissingFunctionCalled(L"mmioRenameA");

    return importTable.mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    Initialize();

    if (NULL == importTable.mmioRenameW)
        LogMissingFunctionCalled(L"mmioRenameW");

    return importTable.mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


LONG ImportApiWinMM::mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
{
    Initialize();

    if (NULL == importTable.mmioSeek)
        LogMissingFunctionCalled(L"mmioSeek");

    return importTable.mmioSeek(hmmio, lOffset, iOrigin);
}

// ---------


LRESULT ImportApiWinMM::mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
    Initialize();

    if (NULL == importTable.mmioSendMessage)
        LogMissingFunctionCalled(L"mmioSendMessage");

    return importTable.mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
}

// ---------


MMRESULT ImportApiWinMM::mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioSetBuffer)
        LogMissingFunctionCalled(L"mmioSetBuffer");

    return importTable.mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
}

// ---------


MMRESULT ImportApiWinMM::mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioSetInfo)
        LogMissingFunctionCalled(L"mmioSetInfo");

    return importTable.mmioSetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


FOURCC ImportApiWinMM::mmioStringToFOURCCA(LPCSTR sz, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioStringToFOURCCA)
        LogMissingFunctionCalled(L"mmioStringToFOURCCA");

    return importTable.mmioStringToFOURCCA(sz, wFlags);
}

// ---------


FOURCC ImportApiWinMM::mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags)
{
    Initialize();

    if (NULL == importTable.mmioStringToFOURCCW)
        LogMissingFunctionCalled(L"mmioStringToFOURCCW");

    return importTable.mmioStringToFOURCCW(sz, wFlags);
}

// ---------


LONG ImportApiWinMM::mmioWrite(HMMIO hmmio, const char* pch, LONG cch)
{
    Initialize();

    if (NULL == importTable.mmioWrite)
        LogMissingFunctionCalled(L"mmioWrite");

    return importTable.mmioWrite(hmmio, pch, cch);
}

// ---------

BOOL ImportApiWinMM::sndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
{
    Initialize();

    if (NULL == importTable.sndPlaySoundA)
        LogMissingFunctionCalled(L"sndPlaySoundA");

    return importTable.sndPlaySoundA(lpszSound, fuSound);
}

// ---------

BOOL ImportApiWinMM::sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
{
    Initialize();

    if (NULL == importTable.sndPlaySoundW)
        LogMissingFunctionCalled(L"sndPlaySoundW");

    return importTable.sndPlaySoundW(lpszSound, fuSound);
}

// ---------

MMRESULT ImportApiWinMM::timeBeginPeriod(UINT uPeriod)
{
    Initialize();

    if (NULL == importTable.timeBeginPeriod)
        LogMissingFunctionCalled(L"timeBeginPeriod");

    return importTable.timeBeginPeriod(uPeriod);
}

// ---------

MMRESULT ImportApiWinMM::timeEndPeriod(UINT uPeriod)
{
    Initialize();

    if (NULL == importTable.timeEndPeriod)
        LogMissingFunctionCalled(L"timeEndPeriod");

    return importTable.timeEndPeriod(uPeriod);
}

// ---------

MMRESULT ImportApiWinMM::timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    Initialize();

    if (NULL == importTable.timeGetDevCaps)
        LogMissingFunctionCalled(L"timeGetDevCaps");

    return importTable.timeGetDevCaps(ptc, cbtc);
}

// ---------

MMRESULT ImportApiWinMM::timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.timeGetSystemTime)
        LogMissingFunctionCalled(L"timeGetSystemTime");

    return importTable.timeGetSystemTime(pmmt, cbmmt);
}

// ---------

DWORD ImportApiWinMM::timeGetTime(void)
{
    Initialize();

    if (NULL == importTable.timeGetTime)
        LogMissingFunctionCalled(L"timeGetTime");

    return importTable.timeGetTime();
}

// ---------

MMRESULT ImportApiWinMM::timeKillEvent(UINT uTimerID)
{
    Initialize();

    if (NULL == importTable.timeKillEvent)
        LogMissingFunctionCalled(L"timeKillEvent");

    return importTable.timeKillEvent(uTimerID);
}

// ---------

MMRESULT ImportApiWinMM::timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
    Initialize();

    if (NULL == importTable.timeSetEvent)
        LogMissingFunctionCalled(L"timeSetEvent");

    return importTable.timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
}

// ---------

MMRESULT ImportApiWinMM::waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveInAddBuffer)
        LogMissingFunctionCalled(L"waveInAddBuffer");

    return importTable.waveInAddBuffer(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveInClose(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInClose)
        LogMissingFunctionCalled(L"waveInClose");

    return importTable.waveInClose(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
{
    Initialize();

    if (NULL == importTable.waveInGetDevCapsA)
        LogMissingFunctionCalled(L"waveInGetDevCapsA");

    return importTable.waveInGetDevCapsA(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
{
    Initialize();

    if (NULL == importTable.waveInGetDevCapsW)
        LogMissingFunctionCalled(L"waveInGetDevCapsW");

    return importTable.waveInGetDevCapsW(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveInGetErrorTextA)
        LogMissingFunctionCalled(L"waveInGetErrorTextA");

    return importTable.waveInGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveInGetErrorTextW)
        LogMissingFunctionCalled(L"waveInGetErrorTextW");

    return importTable.waveInGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.waveInGetID)
        LogMissingFunctionCalled(L"waveInGetID");

    return importTable.waveInGetID(hwi, puDeviceID);
}

// ---------

UINT ImportApiWinMM::waveInGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.waveInGetNumDevs)
        LogMissingFunctionCalled(L"waveInGetNumDevs");

    return importTable.waveInGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.waveInGetPosition)
        LogMissingFunctionCalled(L"waveInGetPosition");

    return importTable.waveInGetPosition(hwi, pmmt, cbmmt);
}

// ---------

DWORD ImportApiWinMM::waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.waveInMessage)
        LogMissingFunctionCalled(L"waveInMessage");

    return importTable.waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.waveInOpen)
        LogMissingFunctionCalled(L"waveInOpen");

    return importTable.waveInOpen(phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveInPrepareHeader)
        LogMissingFunctionCalled(L"waveInPrepareHeader");

    return importTable.waveInPrepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveInReset(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInReset)
        LogMissingFunctionCalled(L"waveInReset");

    return importTable.waveInReset(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInStart(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInStart)
        LogMissingFunctionCalled(L"waveInStart");

    return importTable.waveInStart(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInStop(HWAVEIN hwi)
{
    Initialize();

    if (NULL == importTable.waveInStop)
        LogMissingFunctionCalled(L"waveInStop");

    return importTable.waveInStop(hwi);
}

// ---------

MMRESULT ImportApiWinMM::waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveInUnprepareHeader)
        LogMissingFunctionCalled(L"waveInUnprepareHeader");

    return importTable.waveInUnprepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutBreakLoop(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutBreakLoop)
        LogMissingFunctionCalled(L"waveOutBreakLoop");

    return importTable.waveOutBreakLoop(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutClose(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutClose)
        LogMissingFunctionCalled(L"waveOutClose");

    return importTable.waveOutClose(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
{
    Initialize();

    if (NULL == importTable.waveOutGetDevCapsA)
        LogMissingFunctionCalled(L"waveOutGetDevCapsA");

    return importTable.waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
{
    Initialize();

    if (NULL == importTable.waveOutGetDevCapsW)
        LogMissingFunctionCalled(L"waveOutGetDevCapsW");

    return importTable.waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveOutGetErrorTextA)
        LogMissingFunctionCalled(L"waveOutGetErrorTextA");

    return importTable.waveOutGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    Initialize();

    if (NULL == importTable.waveOutGetErrorTextW)
        LogMissingFunctionCalled(L"waveOutGetErrorTextW");

    return importTable.waveOutGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
{
    Initialize();

    if (NULL == importTable.waveOutGetID)
        LogMissingFunctionCalled(L"waveOutGetID");

    return importTable.waveOutGetID(hwo, puDeviceID);
}

// ---------

UINT ImportApiWinMM::waveOutGetNumDevs(void)
{
    Initialize();

    if (NULL == importTable.waveOutGetNumDevs)
        LogMissingFunctionCalled(L"waveOutGetNumDevs");

    return importTable.waveOutGetNumDevs();
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
{
    Initialize();

    if (NULL == importTable.waveOutGetPitch)
        LogMissingFunctionCalled(L"waveOutGetPitch");

    return importTable.waveOutGetPitch(hwo, pdwPitch);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
{
    Initialize();

    if (NULL == importTable.waveOutGetPlaybackRate)
        LogMissingFunctionCalled(L"waveOutGetPlaybackRate");

    return importTable.waveOutGetPlaybackRate(hwo, pdwRate);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
{
    Initialize();

    if (NULL == importTable.waveOutGetPosition)
        LogMissingFunctionCalled(L"waveOutGetPosition");

    return importTable.waveOutGetPosition(hwo, pmmt, cbmmt);
}

// ---------

MMRESULT ImportApiWinMM::waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
{
    Initialize();

    if (NULL == importTable.waveOutGetVolume)
        LogMissingFunctionCalled(L"waveOutGetVolume");

    return importTable.waveOutGetVolume(hwo, pdwVolume);
}

// ---------

DWORD ImportApiWinMM::waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    Initialize();

    if (NULL == importTable.waveOutMessage)
        LogMissingFunctionCalled(L"waveOutMessage");

    return importTable.waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT ImportApiWinMM::waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    Initialize();

    if (NULL == importTable.waveOutOpen)
        LogMissingFunctionCalled(L"waveOutOpen");

    return importTable.waveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT ImportApiWinMM::waveOutPause(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutPause)
        LogMissingFunctionCalled(L"waveOutPause");

    return importTable.waveOutPause(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveOutPrepareHeader)
        LogMissingFunctionCalled(L"waveOutPrepareHeader");

    return importTable.waveOutPrepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutReset(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutReset)
        LogMissingFunctionCalled(L"waveOutReset");

    return importTable.waveOutReset(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutRestart(HWAVEOUT hwo)
{
    Initialize();

    if (NULL == importTable.waveOutRestart)
        LogMissingFunctionCalled(L"waveOutRestart");

    return importTable.waveOutRestart(hwo);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
{
    Initialize();

    if (NULL == importTable.waveOutSetPitch)
        LogMissingFunctionCalled(L"waveOutSetPitch");

    return importTable.waveOutSetPitch(hwo, dwPitch);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
{
    Initialize();

    if (NULL == importTable.waveOutSetPlaybackRate)
        LogMissingFunctionCalled(L"waveOutSetPlaybackRate");

    return importTable.waveOutSetPlaybackRate(hwo, dwRate);
}

// ---------

MMRESULT ImportApiWinMM::waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
{
    Initialize();

    if (NULL == importTable.waveOutSetVolume)
        LogMissingFunctionCalled(L"waveOutSetVolume");

    return importTable.waveOutSetVolume(hwo, dwVolume);
}

// ---------

MMRESULT ImportApiWinMM::waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveOutUnprepareHeader)
        LogMissingFunctionCalled(L"waveOutUnprepareHeader");

    return importTable.waveOutUnprepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT ImportApiWinMM::waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Initialize();

    if (NULL == importTable.waveOutWrite)
        LogMissingFunctionCalled(L"waveOutWrite");

    return importTable.waveOutWrite(hwo, pwh, cbwh);
}


// -------- HELPERS -------------------------------------------------------- //
// See "ImportApiWinMM.h" for documentation.

void ImportApiWinMM::LogImportFailed(LPCWSTR functionName)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelWarning, L"Import library is missing WinMM function \"%s\". Attempts to call it will fail.", functionName);
}

// --------

void ImportApiWinMM::LogInitializeLibraryPath(LPCWSTR libraryPath)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelDebug, L"Attempting to import WinMM functions from \"%s\".", libraryPath);
}

// --------

void ImportApiWinMM::LogInitializeFailed(void)
{
    Log::WriteLogMessage(ELogLevel::LogLevelError, L"Failed to initialize imported WinMM functions.");
}

// --------

void ImportApiWinMM::LogInitializeSucceeded(void)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelInfo, L"Successfully initialized imported WinMM functions.");
}

// --------

void ImportApiWinMM::LogMissingFunctionCalled(LPCWSTR functionName)
{
    Log::WriteFormattedLogMessage(ELogLevel::LogLevelError, L"Application has attempted to call missing WinMM import function \"%s\".", functionName);
}
