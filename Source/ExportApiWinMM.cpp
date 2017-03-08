/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *****************************************************************************
 * ExportApiWinMM.cpp
 *      Implementation of primary exported functions for the WinMM library.
 *****************************************************************************/

#include "ImportApiWinMM.h"
#include "WrapperJoyWinMM.h"

using namespace Xidi;


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See WinMM documentation for more information.

MMRESULT WINAPI ExportApiWinMMAuxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
{
    return ImportApiWinMM::auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
}

// ---------

MMRESULT WINAPI ExportApiWinMMAuxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
{
    return ImportApiWinMM::auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
}

// ---------

UINT WINAPI ExportApiWinMMAuxGetNumDevs(void)
{
    return ImportApiWinMM::auxGetNumDevs();
}

// ---------

MMRESULT WINAPI ExportApiWinMMAuxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
{
    return ImportApiWinMM::auxGetVolume(uDeviceID, lpdwVolume);
}

// ---------

MMRESULT WINAPI ExportApiWinMMAuxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    return ImportApiWinMM::auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT WINAPI ExportApiWinMMAuxSetVolume(UINT uDeviceID, DWORD dwVolume)
{
    return ImportApiWinMM::auxSetVolume(uDeviceID, dwVolume);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyConfigChanged(DWORD dwFlags)
{
    return WrapperJoyWinMM::JoyConfigChanged(dwFlags);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
{
    return WrapperJoyWinMM::JoyGetDevCapsA(uJoyID, pjc, cbjc);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
{
    return WrapperJoyWinMM::JoyGetDevCapsW(uJoyID, pjc, cbjc);
}

// ---------

UINT WINAPI ExportApiWinMMJoyGetNumDevs(void)
{
    return WrapperJoyWinMM::JoyGetNumDevs();
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyGetPos(UINT uJoyID, LPJOYINFO pji)
{
    return WrapperJoyWinMM::JoyGetPos(uJoyID, pji);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
    return WrapperJoyWinMM::JoyGetPosEx(uJoyID, pji);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyGetThreshold(UINT uJoyID, LPUINT puThreshold)
{
    return WrapperJoyWinMM::JoyGetThreshold(uJoyID, puThreshold);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoyReleaseCapture(UINT uJoyID)
{
    return WrapperJoyWinMM::JoyReleaseCapture(uJoyID);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
{
    return WrapperJoyWinMM::JoySetCapture(hwnd, uJoyID, uPeriod, fChanged);
}

// ---------

MMRESULT WINAPI ExportApiWinMMJoySetThreshold(UINT uJoyID, UINT uThreshold)
{
    return WrapperJoyWinMM::JoySetThreshold(uJoyID, uThreshold);
}

// ---------

MMRESULT WINAPI ExportApiWinMMTimeBeginPeriod(UINT uPeriod)
{
    return ImportApiWinMM::timeBeginPeriod(uPeriod);
}

// ---------

MMRESULT WINAPI ExportApiWinMMTimeEndPeriod(UINT uPeriod)
{
    return ImportApiWinMM::timeEndPeriod(uPeriod);
}

// ---------

MMRESULT WINAPI ExportApiWinMMTimeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
{
    return ImportApiWinMM::timeGetDevCaps(ptc, cbtc);
}

// ---------

MMRESULT WINAPI ExportApiWinMMTimeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
{
    return ImportApiWinMM::timeGetSystemTime(pmmt, cbmmt);
}

// ---------

DWORD WINAPI ExportApiWinMMTimeGetTime(void)
{
    return ImportApiWinMM::timeGetTime();
}

// ---------

MMRESULT WINAPI ExportApiWinMMTimeKillEvent(UINT uTimerID)
{
    return ImportApiWinMM::timeKillEvent(uTimerID);
}

// ---------

MMRESULT WINAPI ExportApiWinMMTimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
{
    return ImportApiWinMM::timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    return ImportApiWinMM::waveInAddBuffer(hwi, pwh, cbwh);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInClose(HWAVEIN hwi)
{
    return ImportApiWinMM::waveInClose(hwi);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
{
    return ImportApiWinMM::waveInGetDevCapsA(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
{
    return ImportApiWinMM::waveInGetDevCapsW(uDeviceID, pwic, cbwic);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    return ImportApiWinMM::waveInGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    return ImportApiWinMM::waveInGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
{
    return ImportApiWinMM::waveInGetID(hwi, puDeviceID);
}

// ---------

UINT WINAPI ExportApiWinMMWaveInGetNumDevs(void)
{
    return ImportApiWinMM::waveInGetNumDevs();
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
{
    return ImportApiWinMM::waveInGetPosition(hwi, pmmt, cbmmt);
}

// ---------

DWORD WINAPI ExportApiWinMMWaveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    return ImportApiWinMM::waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    return ImportApiWinMM::waveInOpen(phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    return ImportApiWinMM::waveInPrepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInReset(HWAVEIN hwi)
{
    return ImportApiWinMM::waveInReset(hwi);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInStart(HWAVEIN hwi)
{
    return ImportApiWinMM::waveInStart(hwi);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInStop(HWAVEIN hwi)
{
    return ImportApiWinMM::waveInStop(hwi);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
{
    return ImportApiWinMM::waveInUnprepareHeader(hwi, pwh, cbwh);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutBreakLoop(HWAVEOUT hwo)
{
    return ImportApiWinMM::waveOutBreakLoop(hwo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutClose(HWAVEOUT hwo)
{
    return ImportApiWinMM::waveOutClose(hwo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
{
    return ImportApiWinMM::waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
{
    return ImportApiWinMM::waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText)
{
    return ImportApiWinMM::waveOutGetErrorTextA(mmrError, pszText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText)
{
    return ImportApiWinMM::waveOutGetErrorTextW(mmrError, pszText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
{
    return ImportApiWinMM::waveOutGetID(hwo, puDeviceID);
}

// ---------

UINT WINAPI ExportApiWinMMWaveOutGetNumDevs(void)
{
    return ImportApiWinMM::waveOutGetNumDevs();
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
{
    return ImportApiWinMM::waveOutGetPitch(hwo, pdwPitch);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
{
    return ImportApiWinMM::waveOutGetPlaybackRate(hwo, pdwRate);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
{
    return ImportApiWinMM::waveOutGetPosition(hwo, pmmt, cbmmt);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
{
    return ImportApiWinMM::waveOutGetVolume(hwo, pdwVolume);
}

// ---------

DWORD WINAPI ExportApiWinMMWaveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    return ImportApiWinMM::waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
    return ImportApiWinMM::waveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutPause(HWAVEOUT hwo)
{
    return ImportApiWinMM::waveOutPause(hwo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    return ImportApiWinMM::waveOutPrepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutReset(HWAVEOUT hwo)
{
    return ImportApiWinMM::waveOutReset(hwo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutRestart(HWAVEOUT hwo)
{
    return ImportApiWinMM::waveOutRestart(hwo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
{
    return ImportApiWinMM::waveOutSetPitch(hwo, dwPitch);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
{
    return ImportApiWinMM::waveOutSetPlaybackRate(hwo, dwRate);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
{
    return ImportApiWinMM::waveOutSetVolume(hwo, dwVolume);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    return ImportApiWinMM::waveOutUnprepareHeader(hwo, pwh, cbwh);
}

// ---------

MMRESULT WINAPI ExportApiWinMMWaveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    return ImportApiWinMM::waveOutWrite(hwo, pwh, cbwh);
}
