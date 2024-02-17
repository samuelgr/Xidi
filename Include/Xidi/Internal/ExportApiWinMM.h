/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2024
 ***********************************************************************************************//**
 * @file ExportApiWinMM.h
 *   Declaration of primary exported functions for the WinMM library.
 **************************************************************************************************/

#pragma once

#include "ApiWindows.h"

extern "C"
{
  // clang-format off

  LRESULT     __stdcall ExportApiWinMMCloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2);
  LRESULT     __stdcall ExportApiWinMMDefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2);
  BOOL        __stdcall ExportApiWinMMDriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
  HMODULE     __stdcall ExportApiWinMMDrvGetModuleHandle(HDRVR hDriver);
  HMODULE     __stdcall ExportApiWinMMGetDriverModuleHandle(HDRVR hdrvr);
  HDRVR       __stdcall ExportApiWinMMOpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam);
  BOOL        __stdcall ExportApiWinMMPlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
  BOOL        __stdcall ExportApiWinMMPlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
  LRESULT     __stdcall ExportApiWinMMSendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2);

  MMRESULT    __stdcall ExportApiWinMMAuxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);
  MMRESULT    __stdcall ExportApiWinMMAuxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);
  UINT        __stdcall ExportApiWinMMAuxGetNumDevs(void);
  MMRESULT    __stdcall ExportApiWinMMAuxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);
  MMRESULT    __stdcall ExportApiWinMMAuxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  MMRESULT    __stdcall ExportApiWinMMAuxSetVolume(UINT uDeviceID, DWORD dwVolume);

  MMRESULT    __stdcall ExportApiWinMMJoyConfigChanged(DWORD dwFlags);
  MMRESULT    __stdcall ExportApiWinMMJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
  MMRESULT    __stdcall ExportApiWinMMJoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
  UINT        __stdcall ExportApiWinMMJoyGetNumDevs(void);
  MMRESULT    __stdcall ExportApiWinMMJoyGetPos(UINT uJoyID, LPJOYINFO pji);
  MMRESULT    __stdcall ExportApiWinMMJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
  MMRESULT    __stdcall ExportApiWinMMJoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
  MMRESULT    __stdcall ExportApiWinMMJoyReleaseCapture(UINT uJoyID);
  MMRESULT    __stdcall ExportApiWinMMJoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
  MMRESULT    __stdcall ExportApiWinMMJoySetThreshold(UINT uJoyID, UINT uThreshold);

  BOOL        __stdcall ExportApiWinMMMCIDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus);
  UINT        __stdcall ExportApiWinMMMCIDriverYield(MCIDEVICEID IDDevice);
  BOOL        __stdcall ExportApiWinMMMCIExecute(LPCSTR pszCommand);
  BOOL        __stdcall ExportApiWinMMMCIFreeCommandResource(UINT uResource);
  HANDLE      __stdcall ExportApiWinMMMCIGetCreatorTask(MCIDEVICEID IDDevice);
  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDA(LPCSTR lpszDevice);
  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDW(LPCWSTR lpszDevice);
  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType);
  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType);
  DWORD_PTR   __stdcall ExportApiWinMMMCIGetDriverData(MCIDEVICEID IDDevice);
  BOOL        __stdcall ExportApiWinMMMCIGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText);
  BOOL        __stdcall ExportApiWinMMMCIGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText);
  YIELDPROC   __stdcall ExportApiWinMMMCIGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData);
  UINT        __stdcall ExportApiWinMMMCILoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType);
  MCIERROR    __stdcall ExportApiWinMMMCISendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam);
  MCIERROR    __stdcall ExportApiWinMMMCISendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam);
  MCIERROR    __stdcall ExportApiWinMMMCISendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback);
  MCIERROR    __stdcall ExportApiWinMMMCISendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback);
  BOOL        __stdcall ExportApiWinMMMCISetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data);
  UINT        __stdcall ExportApiWinMMMCISetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData);

  MMRESULT    __stdcall ExportApiWinMMMidiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);
  MMRESULT    __stdcall ExportApiWinMMMidiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);

  MMRESULT    __stdcall ExportApiWinMMMidiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
  MMRESULT    __stdcall ExportApiWinMMMidiInClose(HMIDIIN hMidiIn);
  MMRESULT    __stdcall ExportApiWinMMMidiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps);
  MMRESULT    __stdcall ExportApiWinMMMidiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps);
  MMRESULT    __stdcall ExportApiWinMMMidiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMMidiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMMidiInGetID(HMIDIIN hmi, LPUINT puDeviceID);
  UINT        __stdcall ExportApiWinMMMidiInGetNumDevs(void);
  DWORD       __stdcall ExportApiWinMMMidiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
  MMRESULT    __stdcall ExportApiWinMMMidiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
  MMRESULT    __stdcall ExportApiWinMMMidiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
  MMRESULT    __stdcall ExportApiWinMMMidiInReset(HMIDIIN hMidiIn);
  MMRESULT    __stdcall ExportApiWinMMMidiInStart(HMIDIIN hMidiIn);
  MMRESULT    __stdcall ExportApiWinMMMidiInStop(HMIDIIN hMidiIn);
  MMRESULT    __stdcall ExportApiWinMMMidiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);

  MMRESULT    __stdcall ExportApiWinMMMidiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMidiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMidiOutClose(HMIDIOUT hmo);
  MMRESULT    __stdcall ExportApiWinMMMidiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps);
  MMRESULT    __stdcall ExportApiWinMMMidiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps);
  UINT        __stdcall ExportApiWinMMMidiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText);
  UINT        __stdcall ExportApiWinMMMidiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMMidiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID);
  UINT        __stdcall ExportApiWinMMMidiOutGetNumDevs(void);
  MMRESULT    __stdcall ExportApiWinMMMidiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume);
  MMRESULT    __stdcall ExportApiWinMMMidiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
  DWORD       __stdcall ExportApiWinMMMidiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
  MMRESULT    __stdcall ExportApiWinMMMidiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
  MMRESULT    __stdcall ExportApiWinMMMidiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
  MMRESULT    __stdcall ExportApiWinMMMidiOutReset(HMIDIOUT hmo);
  MMRESULT    __stdcall ExportApiWinMMMidiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume);
  MMRESULT    __stdcall ExportApiWinMMMidiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg);
  MMRESULT    __stdcall ExportApiWinMMMidiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);

  MMRESULT    __stdcall ExportApiWinMMMidiStreamClose(HMIDISTRM hStream);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamPause(HMIDISTRM hms);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamRestart(HMIDISTRM hms);
  MMRESULT    __stdcall ExportApiWinMMMidiStreamStop(HMIDISTRM hms);

  MMRESULT    __stdcall ExportApiWinMMMixerClose(HMIXER hmx);
  MMRESULT    __stdcall ExportApiWinMMMixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
  MMRESULT    __stdcall ExportApiWinMMMixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
  MMRESULT    __stdcall ExportApiWinMMMixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
  MMRESULT    __stdcall ExportApiWinMMMixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
  MMRESULT    __stdcall ExportApiWinMMMixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId);
  MMRESULT    __stdcall ExportApiWinMMMixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
  MMRESULT    __stdcall ExportApiWinMMMixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
  MMRESULT    __stdcall ExportApiWinMMMixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
  MMRESULT    __stdcall ExportApiWinMMMixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
  UINT        __stdcall ExportApiWinMMMixerGetNumDevs(void);
  DWORD       __stdcall ExportApiWinMMMixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  MMRESULT    __stdcall ExportApiWinMMMixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
  MMRESULT    __stdcall ExportApiWinMMMixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);

  MMRESULT    __stdcall ExportApiWinMMMMIOAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIOAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIOClose(HMMIO hmmio, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIOCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIODescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIOFlush(HMMIO hmmio, UINT fuFlush);
  MMRESULT    __stdcall ExportApiWinMMMMIOGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
  LPMMIOPROC  __stdcall ExportApiWinMMMMIOInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
  LPMMIOPROC  __stdcall ExportApiWinMMMMIOInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
  HMMIO       __stdcall ExportApiWinMMMMIOOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
  HMMIO       __stdcall ExportApiWinMMMMIOOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
  LONG        __stdcall ExportApiWinMMMMIORead(HMMIO hmmio, HPSTR pch, LONG cch);
  MMRESULT    __stdcall ExportApiWinMMMMIORenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIORenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
  LONG        __stdcall ExportApiWinMMMMIOSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
  LRESULT     __stdcall ExportApiWinMMMMIOSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2);
  MMRESULT    __stdcall ExportApiWinMMMMIOSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags);
  MMRESULT    __stdcall ExportApiWinMMMMIOSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags);
  FOURCC      __stdcall ExportApiWinMMMMIOStringToFOURCCA(LPCSTR sz, UINT wFlags);
  FOURCC      __stdcall ExportApiWinMMMMIOStringToFOURCCW(LPCWSTR sz, UINT wFlags);
  LONG        __stdcall ExportApiWinMMMMIOWrite(HMMIO hmmio, const char* pch, LONG cch);

  BOOL        __stdcall ExportApiWinMMSndPlaySoundA(LPCSTR lpszSound, UINT fuSound);
  BOOL        __stdcall ExportApiWinMMSndPlaySoundW(LPCWSTR lpszSound, UINT fuSound);

  MMRESULT    __stdcall ExportApiWinMMTimeBeginPeriod(UINT uPeriod);
  MMRESULT    __stdcall ExportApiWinMMTimeEndPeriod(UINT uPeriod);
  MMRESULT    __stdcall ExportApiWinMMTimeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
  MMRESULT    __stdcall ExportApiWinMMTimeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
  DWORD       __stdcall ExportApiWinMMTimeGetTime(void);
  MMRESULT    __stdcall ExportApiWinMMTimeKillEvent(UINT uTimerID);
  MMRESULT    __stdcall ExportApiWinMMTimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);

  MMRESULT    __stdcall ExportApiWinMMWaveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
  MMRESULT    __stdcall ExportApiWinMMWaveInClose(HWAVEIN hwi);
  MMRESULT    __stdcall ExportApiWinMMWaveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);
  MMRESULT    __stdcall ExportApiWinMMWaveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);
  MMRESULT    __stdcall ExportApiWinMMWaveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMWaveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMWaveInGetID(HWAVEIN hwi, LPUINT puDeviceID);
  UINT        __stdcall ExportApiWinMMWaveInGetNumDevs(void);
  MMRESULT    __stdcall ExportApiWinMMWaveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);
  DWORD       __stdcall ExportApiWinMMWaveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  MMRESULT    __stdcall ExportApiWinMMWaveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
  MMRESULT    __stdcall ExportApiWinMMWaveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
  MMRESULT    __stdcall ExportApiWinMMWaveInReset(HWAVEIN hwi);
  MMRESULT    __stdcall ExportApiWinMMWaveInStart(HWAVEIN hwi);
  MMRESULT    __stdcall ExportApiWinMMWaveInStop(HWAVEIN hwi);
  MMRESULT    __stdcall ExportApiWinMMWaveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

  MMRESULT    __stdcall ExportApiWinMMWaveOutBreakLoop(HWAVEOUT hwo);
  MMRESULT    __stdcall ExportApiWinMMWaveOutClose(HWAVEOUT hwo);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);
  UINT        __stdcall ExportApiWinMMWaveOutGetNumDevs(void);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
  MMRESULT    __stdcall ExportApiWinMMWaveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);
  DWORD       __stdcall ExportApiWinMMWaveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  MMRESULT    __stdcall ExportApiWinMMWaveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
  MMRESULT    __stdcall ExportApiWinMMWaveOutPause(HWAVEOUT hwo);
  MMRESULT    __stdcall ExportApiWinMMWaveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
  MMRESULT    __stdcall ExportApiWinMMWaveOutReset(HWAVEOUT hwo);
  MMRESULT    __stdcall ExportApiWinMMWaveOutRestart(HWAVEOUT hwo);
  MMRESULT    __stdcall ExportApiWinMMWaveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);
  MMRESULT    __stdcall ExportApiWinMMWaveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);
  MMRESULT    __stdcall ExportApiWinMMWaveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);
  MMRESULT    __stdcall ExportApiWinMMWaveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
  MMRESULT    __stdcall ExportApiWinMMWaveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);

  // clang-format on
}
