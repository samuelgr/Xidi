/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2023
 *************************************************************************//**
 * @file ImportApiWinMM.h
 *   Declarations of functions for accessing the WinMM API imported from the
 *   native WinMM library.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


namespace Xidi
{
    namespace ImportApiWinMM
    {
        // -------- FUNCTIONS ---------------------------------------------- //

        /// Dynamically loads the WinMM library and sets up all imported function calls.
        void Initialize(void);


        // -------- IMPORTED FUNCTIONS ------------------------------------- //
        // See WinMM documentation for more information.

        LRESULT      CloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2);
        LRESULT      DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2);
        BOOL         DriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
        HMODULE      DrvGetModuleHandle(HDRVR hDriver);
        HMODULE      GetDriverModuleHandle(HDRVR hdrvr);
        HDRVR        OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam);
        BOOL         PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
        BOOL         PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
        LRESULT      SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2);

        MMRESULT     auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);
        MMRESULT     auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);
        UINT         auxGetNumDevs(void);
        MMRESULT     auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);
        MMRESULT     auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        MMRESULT     auxSetVolume(UINT uDeviceID, DWORD dwVolume);

        MMRESULT     joyConfigChanged(DWORD dwFlags);
        MMRESULT     joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
        MMRESULT     joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
        UINT         joyGetNumDevs(void);
        MMRESULT     joyGetPos(UINT uJoyID, LPJOYINFO pji);
        MMRESULT     joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        MMRESULT     joyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        MMRESULT     joyReleaseCapture(UINT uJoyID);
        MMRESULT     joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        MMRESULT     joySetThreshold(UINT uJoyID, UINT uThreshold);

        BOOL         mciDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus);
        UINT         mciDriverYield(MCIDEVICEID IDDevice);
        BOOL         mciExecute(LPCSTR pszCommand);
        BOOL         mciFreeCommandResource(UINT uResource);
        HANDLE       mciGetCreatorTask(MCIDEVICEID IDDevice);
        MCIDEVICEID  mciGetDeviceIDA(LPCSTR lpszDevice);
        MCIDEVICEID  mciGetDeviceIDW(LPCWSTR lpszDevice);
        MCIDEVICEID  mciGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType);
        MCIDEVICEID  mciGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType);
        DWORD_PTR    mciGetDriverData(MCIDEVICEID IDDevice);
        BOOL         mciGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText);
        BOOL         mciGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText);
        YIELDPROC    mciGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData);
        UINT         mciLoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType);
        MCIERROR     mciSendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam);
        MCIERROR     mciSendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam);
        MCIERROR     mciSendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback);
        MCIERROR     mciSendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback);
        BOOL         mciSetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data);
        UINT         mciSetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData);

        MMRESULT     midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);
        MMRESULT     midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);

        MMRESULT     midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
        MMRESULT     midiInClose(HMIDIIN hMidiIn);
        MMRESULT     midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps);
        MMRESULT     midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps);
        MMRESULT     midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText);
        MMRESULT     midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText);
        MMRESULT     midiInGetID(HMIDIIN hmi, LPUINT puDeviceID);
        UINT         midiInGetNumDevs(void);
        DWORD        midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
        MMRESULT     midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
        MMRESULT     midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
        MMRESULT     midiInReset(HMIDIIN hMidiIn);
        MMRESULT     midiInStart(HMIDIIN hMidiIn);
        MMRESULT     midiInStop(HMIDIIN hMidiIn);
        MMRESULT     midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);

        MMRESULT     midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD *lpKeyArray, UINT wFlags);
        MMRESULT     midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD *lpPatchArray, UINT wFlags);
        MMRESULT     midiOutClose(HMIDIOUT hmo);
        MMRESULT     midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps);
        MMRESULT     midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps);
        UINT         midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText);
        UINT         midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText);
        MMRESULT     midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID);
        UINT         midiOutGetNumDevs(void);
        MMRESULT     midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume);
        MMRESULT     midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
        DWORD        midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
        MMRESULT     midiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
        MMRESULT     midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
        MMRESULT     midiOutReset(HMIDIOUT hmo);
        MMRESULT     midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume);
        MMRESULT     midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg);
        MMRESULT     midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);

        MMRESULT     midiStreamClose(HMIDISTRM hStream);
        MMRESULT     midiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
        MMRESULT     midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr);
        MMRESULT     midiStreamPause(HMIDISTRM hms);
        MMRESULT     midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt);
        MMRESULT     midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty);
        MMRESULT     midiStreamRestart(HMIDISTRM hms);
        MMRESULT     midiStreamStop(HMIDISTRM hms);

        MMRESULT     mixerClose(HMIXER hmx);
        MMRESULT     mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
        MMRESULT     mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
        MMRESULT     mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
        MMRESULT     mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
        MMRESULT     mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId);
        MMRESULT     mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
        MMRESULT     mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
        MMRESULT     mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
        MMRESULT     mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
        UINT         mixerGetNumDevs(void);
        DWORD        mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        MMRESULT     mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
        MMRESULT     mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);

        MMRESULT     mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
        MMRESULT     mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
        MMRESULT     mmioClose(HMMIO hmmio, UINT wFlags);
        MMRESULT     mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
        MMRESULT     mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags);
        MMRESULT     mmioFlush(HMMIO hmmio, UINT fuFlush);
        MMRESULT     mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
        LPMMIOPROC   mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
        LPMMIOPROC   mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
        HMMIO        mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
        HMMIO        mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
        LONG         mmioRead(HMMIO hmmio, HPSTR pch, LONG cch);
        MMRESULT     mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
        MMRESULT     mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
        LONG         mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
        LRESULT      mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2);
        MMRESULT     mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags);
        MMRESULT     mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags);
        FOURCC       mmioStringToFOURCCA(LPCSTR sz, UINT wFlags);
        FOURCC       mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags);
        LONG         mmioWrite(HMMIO hmmio, const char* pch, LONG cch);

        BOOL         sndPlaySoundA(LPCSTR lpszSound, UINT fuSound);
        BOOL         sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound);

        MMRESULT     timeBeginPeriod(UINT uPeriod);
        MMRESULT     timeEndPeriod(UINT uPeriod);
        MMRESULT     timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
        MMRESULT     timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
        DWORD        timeGetTime(void);
        MMRESULT     timeKillEvent(UINT uTimerID);
        MMRESULT     timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);

        MMRESULT     waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
        MMRESULT     waveInClose(HWAVEIN hwi);
        MMRESULT     waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);
        MMRESULT     waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);
        MMRESULT     waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
        MMRESULT     waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
        MMRESULT     waveInGetID(HWAVEIN hwi, LPUINT puDeviceID);
        UINT         waveInGetNumDevs(void);
        MMRESULT     waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);
        DWORD        waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        MMRESULT     waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
        MMRESULT     waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
        MMRESULT     waveInReset(HWAVEIN hwi);
        MMRESULT     waveInStart(HWAVEIN hwi);
        MMRESULT     waveInStop(HWAVEIN hwi);
        MMRESULT     waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);

        MMRESULT     waveOutBreakLoop(HWAVEOUT hwo);
        MMRESULT     waveOutClose(HWAVEOUT hwo);
        MMRESULT     waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);
        MMRESULT     waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);
        MMRESULT     waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
        MMRESULT     waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
        MMRESULT     waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);
        UINT         waveOutGetNumDevs(void);
        MMRESULT     waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);
        MMRESULT     waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);
        MMRESULT     waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
        MMRESULT     waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);
        DWORD        waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        MMRESULT     waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
        MMRESULT     waveOutPause(HWAVEOUT hwo);
        MMRESULT     waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
        MMRESULT     waveOutReset(HWAVEOUT hwo);
        MMRESULT     waveOutRestart(HWAVEOUT hwo);
        MMRESULT     waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);
        MMRESULT     waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);
        MMRESULT     waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);
        MMRESULT     waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
        MMRESULT     waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
    }
}
