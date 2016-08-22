/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ExportApiWinMM.h
 *      Declaration of primary exported functions for the WinMM library.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See WinMM documentation for more information.

MMRESULT WINAPI ExportApiWinMMAuxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);
MMRESULT WINAPI ExportApiWinMMAuxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);
UINT     WINAPI ExportApiWinMMAuxGetNumDevs(void);
MMRESULT WINAPI ExportApiWinMMAuxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);
MMRESULT WINAPI ExportApiWinMMAuxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT WINAPI ExportApiWinMMAuxSetVolume(UINT uDeviceID, DWORD dwVolume);

MMRESULT WINAPI ExportApiWinMMJoyConfigChanged(DWORD dwFlags);
MMRESULT WINAPI ExportApiWinMMJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
MMRESULT WINAPI ExportApiWinMMJoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
UINT     WINAPI ExportApiWinMMJoyGetNumDevs(void);
MMRESULT WINAPI ExportApiWinMMJoyGetPos(UINT uJoyID, LPJOYINFO pji);
MMRESULT WINAPI ExportApiWinMMJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
MMRESULT WINAPI ExportApiWinMMJoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
MMRESULT WINAPI ExportApiWinMMJoyReleaseCapture(UINT uJoyID);
MMRESULT WINAPI ExportApiWinMMJoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
MMRESULT WINAPI ExportApiWinMMJoySetThreshold(UINT uJoyID, UINT uThreshold);

MMRESULT WINAPI ExportApiWinMMTimeBeginPeriod(UINT uPeriod);
MMRESULT WINAPI ExportApiWinMMTimeEndPeriod(UINT uPeriod);
MMRESULT WINAPI ExportApiWinMMTimeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
MMRESULT WINAPI ExportApiWinMMTimeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
DWORD    WINAPI ExportApiWinMMTimeGetTime(void);
MMRESULT WINAPI ExportApiWinMMTimeKillEvent(UINT uTimerID);
MMRESULT WINAPI ExportApiWinMMTimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);

MMRESULT WINAPI ExportApiWinMMWaveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
MMRESULT WINAPI ExportApiWinMMWaveInClose(HWAVEIN hwi);
MMRESULT WINAPI ExportApiWinMMWaveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);
MMRESULT WINAPI ExportApiWinMMWaveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);
MMRESULT WINAPI ExportApiWinMMWaveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
MMRESULT WINAPI ExportApiWinMMWaveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
MMRESULT WINAPI ExportApiWinMMWaveInGetID(HWAVEIN hwi, LPUINT puDeviceID);
UINT     WINAPI ExportApiWinMMWaveInGetNumDevs(void);
MMRESULT WINAPI ExportApiWinMMWaveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);
DWORD    WINAPI ExportApiWinMMWaveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT WINAPI ExportApiWinMMWaveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
MMRESULT WINAPI ExportApiWinMMWaveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
MMRESULT WINAPI ExportApiWinMMWaveInReset(HWAVEIN hwi);
MMRESULT WINAPI ExportApiWinMMWaveInStart(HWAVEIN hwi);
MMRESULT WINAPI ExportApiWinMMWaveInStop(HWAVEIN hwi);
MMRESULT WINAPI ExportApiWinMMWaveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

MMRESULT WINAPI ExportApiWinMMWaveOutBreakLoop(HWAVEOUT hwo);
MMRESULT WINAPI ExportApiWinMMWaveOutClose(HWAVEOUT hwo);
MMRESULT WINAPI ExportApiWinMMWaveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);
MMRESULT WINAPI ExportApiWinMMWaveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);
MMRESULT WINAPI ExportApiWinMMWaveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
MMRESULT WINAPI ExportApiWinMMWaveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
MMRESULT WINAPI ExportApiWinMMWaveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);
UINT     WINAPI ExportApiWinMMWaveOutGetNumDevs(void);
MMRESULT WINAPI ExportApiWinMMWaveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);
MMRESULT WINAPI ExportApiWinMMWaveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);
MMRESULT WINAPI ExportApiWinMMWaveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
MMRESULT WINAPI ExportApiWinMMWaveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);
DWORD    WINAPI ExportApiWinMMWaveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
MMRESULT WINAPI ExportApiWinMMWaveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
MMRESULT WINAPI ExportApiWinMMWaveOutPause(HWAVEOUT hwo);
MMRESULT WINAPI ExportApiWinMMWaveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
MMRESULT WINAPI ExportApiWinMMWaveOutReset(HWAVEOUT hwo);
MMRESULT WINAPI ExportApiWinMMWaveOutRestart(HWAVEOUT hwo);
MMRESULT WINAPI ExportApiWinMMWaveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);
MMRESULT WINAPI ExportApiWinMMWaveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);
MMRESULT WINAPI ExportApiWinMMWaveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);
MMRESULT WINAPI ExportApiWinMMWaveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
MMRESULT WINAPI ExportApiWinMMWaveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
