/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ExportApiWinMM.h
 *   Declaration of primary exported functions for the WinMM library.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See WinMM documentation for more information.

LRESULT     WINAPI ExportApiWinMMCloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2);
LRESULT     WINAPI ExportApiWinMMDefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2);
BOOL        WINAPI ExportApiWinMMDriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
HMODULE     WINAPI ExportApiWinMMDrvGetModuleHandle(HDRVR hDriver);
HMODULE     WINAPI ExportApiWinMMGetDriverModuleHandle(HDRVR hdrvr);
HDRVR       WINAPI ExportApiWinMMOpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam);
BOOL        WINAPI ExportApiWinMMPlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
BOOL        WINAPI ExportApiWinMMPlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
LRESULT     WINAPI ExportApiWinMMSendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2);

MMRESULT    WINAPI ExportApiWinMMAuxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);
MMRESULT    WINAPI ExportApiWinMMAuxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);
UINT        WINAPI ExportApiWinMMAuxGetNumDevs(void);
MMRESULT    WINAPI ExportApiWinMMAuxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);
MMRESULT    WINAPI ExportApiWinMMAuxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT    WINAPI ExportApiWinMMAuxSetVolume(UINT uDeviceID, DWORD dwVolume);

MMRESULT    WINAPI ExportApiWinMMJoyConfigChanged(DWORD dwFlags);
MMRESULT    WINAPI ExportApiWinMMJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
MMRESULT    WINAPI ExportApiWinMMJoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
UINT        WINAPI ExportApiWinMMJoyGetNumDevs(void);
MMRESULT    WINAPI ExportApiWinMMJoyGetPos(UINT uJoyID, LPJOYINFO pji);
MMRESULT    WINAPI ExportApiWinMMJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
MMRESULT    WINAPI ExportApiWinMMJoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
MMRESULT    WINAPI ExportApiWinMMJoyReleaseCapture(UINT uJoyID);
MMRESULT    WINAPI ExportApiWinMMJoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
MMRESULT    WINAPI ExportApiWinMMJoySetThreshold(UINT uJoyID, UINT uThreshold);

MMRESULT    WINAPI ExportApiWinMMMidiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);
MMRESULT    WINAPI ExportApiWinMMMidiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);

MMRESULT    WINAPI ExportApiWinMMMidiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
MMRESULT    WINAPI ExportApiWinMMMidiInClose(HMIDIIN hMidiIn);
MMRESULT    WINAPI ExportApiWinMMMidiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps);
MMRESULT    WINAPI ExportApiWinMMMidiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps);
MMRESULT    WINAPI ExportApiWinMMMidiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMMidiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMMidiInGetID(HMIDIIN hmi, LPUINT puDeviceID);
UINT        WINAPI ExportApiWinMMMidiInGetNumDevs(void);
DWORD       WINAPI ExportApiWinMMMidiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
MMRESULT    WINAPI ExportApiWinMMMidiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
MMRESULT    WINAPI ExportApiWinMMMidiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
MMRESULT    WINAPI ExportApiWinMMMidiInReset(HMIDIIN hMidiIn);
MMRESULT    WINAPI ExportApiWinMMMidiInStart(HMIDIIN hMidiIn);
MMRESULT    WINAPI ExportApiWinMMMidiInStop(HMIDIIN hMidiIn);
MMRESULT    WINAPI ExportApiWinMMMidiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);

MMRESULT    WINAPI ExportApiWinMMMidiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMidiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMidiOutClose(HMIDIOUT hmo);
MMRESULT    WINAPI ExportApiWinMMMidiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps);
MMRESULT    WINAPI ExportApiWinMMMidiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps);
UINT        WINAPI ExportApiWinMMMidiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText);
UINT        WINAPI ExportApiWinMMMidiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMMidiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID);
UINT        WINAPI ExportApiWinMMMidiOutGetNumDevs(void);
MMRESULT    WINAPI ExportApiWinMMMidiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume);
MMRESULT    WINAPI ExportApiWinMMMidiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
DWORD       WINAPI ExportApiWinMMMidiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
MMRESULT    WINAPI ExportApiWinMMMidiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
MMRESULT    WINAPI ExportApiWinMMMidiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
MMRESULT    WINAPI ExportApiWinMMMidiOutReset(HMIDIOUT hmo);
MMRESULT    WINAPI ExportApiWinMMMidiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume);
MMRESULT    WINAPI ExportApiWinMMMidiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg);
MMRESULT    WINAPI ExportApiWinMMMidiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);

MMRESULT    WINAPI ExportApiWinMMMidiStreamClose(HMIDISTRM hStream);
MMRESULT    WINAPI ExportApiWinMMMidiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
MMRESULT    WINAPI ExportApiWinMMMidiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr);
MMRESULT    WINAPI ExportApiWinMMMidiStreamPause(HMIDISTRM hms);
MMRESULT    WINAPI ExportApiWinMMMidiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt);
MMRESULT    WINAPI ExportApiWinMMMidiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty);
MMRESULT    WINAPI ExportApiWinMMMidiStreamRestart(HMIDISTRM hms);
MMRESULT    WINAPI ExportApiWinMMMidiStreamStop(HMIDISTRM hms);

MMRESULT    WINAPI ExportApiWinMMMixerClose(HMIXER hmx);
MMRESULT    WINAPI ExportApiWinMMMixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
MMRESULT    WINAPI ExportApiWinMMMixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
MMRESULT    WINAPI ExportApiWinMMMixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
MMRESULT    WINAPI ExportApiWinMMMixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
MMRESULT    WINAPI ExportApiWinMMMixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId);
MMRESULT    WINAPI ExportApiWinMMMixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
MMRESULT    WINAPI ExportApiWinMMMixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
MMRESULT    WINAPI ExportApiWinMMMixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
MMRESULT    WINAPI ExportApiWinMMMixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
UINT        WINAPI ExportApiWinMMMixerGetNumDevs(void);
DWORD       WINAPI ExportApiWinMMMixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT    WINAPI ExportApiWinMMMixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
MMRESULT    WINAPI ExportApiWinMMMixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);

MMRESULT    WINAPI ExportApiWinMMMMIOAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMMIOAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMMIOClose(HMMIO hmmio, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMMIOCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMMIODescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMMIOFlush(HMMIO hmmio, UINT fuFlush);
MMRESULT    WINAPI ExportApiWinMMMMIOGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
LPMMIOPROC  WINAPI ExportApiWinMMMMIOInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
LPMMIOPROC  WINAPI ExportApiWinMMMMIOInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
HMMIO       WINAPI ExportApiWinMMMMIOOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
HMMIO       WINAPI ExportApiWinMMMMIOOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
LONG        WINAPI ExportApiWinMMMMIORead(HMMIO hmmio, HPSTR pch, LONG cch);
MMRESULT    WINAPI ExportApiWinMMMMIORenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
MMRESULT    WINAPI ExportApiWinMMMMIORenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
LONG        WINAPI ExportApiWinMMMMIOSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
LRESULT     WINAPI ExportApiWinMMMMIOSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2);
MMRESULT    WINAPI ExportApiWinMMMMIOSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags);
MMRESULT    WINAPI ExportApiWinMMMMIOSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags);
FOURCC      WINAPI ExportApiWinMMMMIOStringToFOURCCA(LPCSTR sz, UINT wFlags);
FOURCC      WINAPI ExportApiWinMMMMIOStringToFOURCCW(LPCWSTR sz, UINT wFlags);
LONG        WINAPI ExportApiWinMMMMIOWrite(HMMIO hmmio, const char* pch, LONG cch);

BOOL        WINAPI ExportApiWinMMSndPlaySoundA(LPCSTR lpszSound, UINT fuSound);
BOOL        WINAPI ExportApiWinMMSndPlaySoundW(LPCWSTR lpszSound, UINT fuSound);

MMRESULT    WINAPI ExportApiWinMMTimeBeginPeriod(UINT uPeriod);
MMRESULT    WINAPI ExportApiWinMMTimeEndPeriod(UINT uPeriod);
MMRESULT    WINAPI ExportApiWinMMTimeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
MMRESULT    WINAPI ExportApiWinMMTimeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
DWORD       WINAPI ExportApiWinMMTimeGetTime(void);
MMRESULT    WINAPI ExportApiWinMMTimeKillEvent(UINT uTimerID);
MMRESULT    WINAPI ExportApiWinMMTimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);

MMRESULT    WINAPI ExportApiWinMMWaveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
MMRESULT    WINAPI ExportApiWinMMWaveInClose(HWAVEIN hwi);
MMRESULT    WINAPI ExportApiWinMMWaveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);
MMRESULT    WINAPI ExportApiWinMMWaveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);
MMRESULT    WINAPI ExportApiWinMMWaveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMWaveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMWaveInGetID(HWAVEIN hwi, LPUINT puDeviceID);
UINT        WINAPI ExportApiWinMMWaveInGetNumDevs(void);
MMRESULT    WINAPI ExportApiWinMMWaveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);
DWORD       WINAPI ExportApiWinMMWaveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT    WINAPI ExportApiWinMMWaveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
MMRESULT    WINAPI ExportApiWinMMWaveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
MMRESULT    WINAPI ExportApiWinMMWaveInReset(HWAVEIN hwi);
MMRESULT    WINAPI ExportApiWinMMWaveInStart(HWAVEIN hwi);
MMRESULT    WINAPI ExportApiWinMMWaveInStop(HWAVEIN hwi);
MMRESULT    WINAPI ExportApiWinMMWaveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

MMRESULT    WINAPI ExportApiWinMMWaveOutBreakLoop(HWAVEOUT hwo);
MMRESULT    WINAPI ExportApiWinMMWaveOutClose(HWAVEOUT hwo);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);
UINT        WINAPI ExportApiWinMMWaveOutGetNumDevs(void);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
MMRESULT    WINAPI ExportApiWinMMWaveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);
DWORD       WINAPI ExportApiWinMMWaveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT    WINAPI ExportApiWinMMWaveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
MMRESULT    WINAPI ExportApiWinMMWaveOutPause(HWAVEOUT hwo);
MMRESULT    WINAPI ExportApiWinMMWaveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
MMRESULT    WINAPI ExportApiWinMMWaveOutReset(HWAVEOUT hwo);
MMRESULT    WINAPI ExportApiWinMMWaveOutRestart(HWAVEOUT hwo);
MMRESULT    WINAPI ExportApiWinMMWaveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);
MMRESULT    WINAPI ExportApiWinMMWaveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);
MMRESULT    WINAPI ExportApiWinMMWaveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);
MMRESULT    WINAPI ExportApiWinMMWaveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
MMRESULT    WINAPI ExportApiWinMMWaveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
