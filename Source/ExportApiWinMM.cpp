/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ExportApiWinMM.cpp
 *   Implementation of primary exported functions for the WinMM library.
 *****************************************************************************/

#include "ImportApiWinMM.h"
#include "WrapperJoyWinMM.h"

using namespace Xidi;


// -------- DLL EXPORT FUNCTIONS ------------------------------------------- //
// See WinMM documentation for more information.

LRESULT WINAPI ExportApiWinMMCloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2)
{
    return ImportApiWinMM::CloseDriver(hdrvr, lParam1, lParam2);
}

// ---------


LRESULT WINAPI ExportApiWinMMDefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
{
    return ImportApiWinMM::DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
}

// ---------


BOOL WINAPI ExportApiWinMMDriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2)
{
    return ImportApiWinMM::DriverCallback(dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
}

// ---------


HMODULE WINAPI ExportApiWinMMDrvGetModuleHandle(HDRVR hDriver)
{
    return ImportApiWinMM::DrvGetModuleHandle(hDriver);
}

// ---------


HMODULE WINAPI ExportApiWinMMGetDriverModuleHandle(HDRVR hdrvr)
{
    return ImportApiWinMM::GetDriverModuleHandle(hdrvr);
}

// ---------


HDRVR WINAPI ExportApiWinMMOpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
{
    return ImportApiWinMM::OpenDriver(lpDriverName, lpSectionName, lParam);
}

// ---------

BOOL WINAPI ExportApiWinMMPlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    return ImportApiWinMM::PlaySoundA(pszSound, hmod, fdwSound);
}

// ---------

BOOL WINAPI ExportApiWinMMPlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
    return ImportApiWinMM::PlaySoundW(pszSound, hmod, fdwSound);
}

// ---------

LRESULT WINAPI ExportApiWinMMSendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    return ImportApiWinMM::SendDriverMessage(hdrvr, msg, lParam1, lParam2);
}

// ---------

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

BOOL WINAPI ExportApiWinMMMCIDriverNotify(HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
{
    return ImportApiWinMM::mciDriverNotify(hwndCallback, IDDevice, uStatus);
}

// ---------

UINT WINAPI ExportApiWinMMMCIDriverYield(MCIDEVICEID IDDevice)
{
    return ImportApiWinMM::mciDriverYield(IDDevice);
}

// ---------

BOOL WINAPI ExportApiWinMMMCIExecute(LPCSTR pszCommand)
{
    return ImportApiWinMM::mciExecute(pszCommand);
}

// ---------

BOOL WINAPI ExportApiWinMMMCIFreeCommandResource(UINT uResource)
{
    return ImportApiWinMM::mciFreeCommandResource(uResource);
}

// ---------

HANDLE WINAPI ExportApiWinMMMCIGetCreatorTask(MCIDEVICEID IDDevice)
{
    return ImportApiWinMM::mciGetCreatorTask(IDDevice);
}

// ---------

MCIDEVICEID WINAPI ExportApiWinMMMCIGetDeviceIDA(LPCSTR lpszDevice)
{
    return ImportApiWinMM::mciGetDeviceIDA(lpszDevice);
}

// ---------

MCIDEVICEID WINAPI ExportApiWinMMMCIGetDeviceIDW(LPCWSTR lpszDevice)
{
    return ImportApiWinMM::mciGetDeviceIDW(lpszDevice);
}

// ---------

MCIDEVICEID WINAPI ExportApiWinMMMCIGetDeviceIDFromElementIDA(DWORD dwElementID, LPCSTR lpstrType)
{
    return ImportApiWinMM::mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
}

// ---------

MCIDEVICEID WINAPI ExportApiWinMMMCIGetDeviceIDFromElementIDW(DWORD dwElementID, LPCWSTR lpstrType)
{
    return ImportApiWinMM::mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
}

// ---------

DWORD_PTR WINAPI ExportApiWinMMMCIGetDriverData(MCIDEVICEID IDDevice)
{
    return ImportApiWinMM::mciGetDriverData(IDDevice);
}

// ---------

BOOL WINAPI ExportApiWinMMMCIGetErrorStringA(DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
{
    return ImportApiWinMM::mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
}

// ---------

BOOL WINAPI ExportApiWinMMMCIGetErrorStringW(DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
{
    return ImportApiWinMM::mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
}

// ---------

YIELDPROC WINAPI ExportApiWinMMMCIGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
{
    return ImportApiWinMM::mciGetYieldProc(IDDevice, lpdwYieldData);
}

// ---------

UINT WINAPI ExportApiWinMMMCILoadCommandResource(HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
{
    return ImportApiWinMM::mciLoadCommandResource(hInst, lpwstrResourceName, uType);
}

// ---------

MCIERROR WINAPI ExportApiWinMMMCISendCommandA(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
    return ImportApiWinMM::mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
}

// ---------

MCIERROR WINAPI ExportApiWinMMMCISendCommandW(MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
{
    return ImportApiWinMM::mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
}

// ---------

MCIERROR WINAPI ExportApiWinMMMCISendStringA(LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    return ImportApiWinMM::mciSendStringA(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
}

// ---------

MCIERROR WINAPI ExportApiWinMMMCISendStringW(LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    return ImportApiWinMM::mciSendStringW(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
}

// ---------

BOOL WINAPI ExportApiWinMMMCISetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
{
    return ImportApiWinMM::mciSetDriverData(IDDevice, data);
}

// ---------

UINT WINAPI ExportApiWinMMMCISetYieldProc(MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
{
    return ImportApiWinMM::mciSetYieldProc(IDDevice, yp, dwYieldData);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
{
    return ImportApiWinMM::midiConnect(hMidi, hmo, pReserved);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
{
    return ImportApiWinMM::midiDisconnect(hMidi, hmo, pReserved);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    return ImportApiWinMM::midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInClose(HMIDIIN hMidiIn)
{
    return ImportApiWinMM::midiInClose(hMidiIn);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
{
    return ImportApiWinMM::midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
{
    return ImportApiWinMM::midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
{
    return ImportApiWinMM::midiInGetErrorTextA(wError, lpText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
{
    return ImportApiWinMM::midiInGetErrorTextW(wError, lpText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
{
    return ImportApiWinMM::midiInGetID(hmi, puDeviceID);
}

// ---------

UINT WINAPI ExportApiWinMMMidiInGetNumDevs(void)
{
    return ImportApiWinMM::midiInGetNumDevs();
}

// ---------

DWORD WINAPI ExportApiWinMMMidiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    return ImportApiWinMM::midiInMessage(deviceID, msg, dw1, dw2);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
{
    return ImportApiWinMM::midiInOpen(lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    return ImportApiWinMM::midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInReset(HMIDIIN hMidiIn)
{
    return ImportApiWinMM::midiInReset(hMidiIn);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInStart(HMIDIIN hMidiIn)
{
    return ImportApiWinMM::midiInStart(hMidiIn);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInStop(HMIDIIN hMidiIn)
{
    return ImportApiWinMM::midiInStop(hMidiIn);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
{
    return ImportApiWinMM::midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
{
    return ImportApiWinMM::midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
{
    return ImportApiWinMM::midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutClose(HMIDIOUT hmo)
{
    return ImportApiWinMM::midiOutClose(hmo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
{
    return ImportApiWinMM::midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
{
    return ImportApiWinMM::midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
}

// ---------

UINT WINAPI ExportApiWinMMMidiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
{
    return ImportApiWinMM::midiOutGetErrorTextA(mmrError, lpText, cchText);
}

// ---------

UINT WINAPI ExportApiWinMMMidiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
{
    return ImportApiWinMM::midiOutGetErrorTextW(mmrError, lpText, cchText);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
{
    return ImportApiWinMM::midiOutGetID(hmo, puDeviceID);
}

// ---------

UINT WINAPI ExportApiWinMMMidiOutGetNumDevs(void)
{
    return ImportApiWinMM::midiOutGetNumDevs();
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
{
    return ImportApiWinMM::midiOutGetVolume(hmo, lpdwVolume);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    return ImportApiWinMM::midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

DWORD WINAPI ExportApiWinMMMidiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    return ImportApiWinMM::midiOutMessage(deviceID, msg, dw1, dw2);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags)
{
    return ImportApiWinMM::midiOutOpen(lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    return ImportApiWinMM::midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutReset(HMIDIOUT hmo)
{
    return ImportApiWinMM::midiOutReset(hmo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
{
    return ImportApiWinMM::midiOutSetVolume(hmo, dwVolume);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
{
    return ImportApiWinMM::midiOutShortMsg(hmo, dwMsg);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
{
    return ImportApiWinMM::midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamClose(HMIDISTRM hStream)
{
    return ImportApiWinMM::midiStreamClose(hStream);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    return ImportApiWinMM::midiStreamOpen(lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
{
    return ImportApiWinMM::midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamPause(HMIDISTRM hms)
{
    return ImportApiWinMM::midiStreamPause(hms);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
{
    return ImportApiWinMM::midiStreamPosition(hms, pmmt, cbmmt);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
{
    return ImportApiWinMM::midiStreamProperty(hm, lppropdata, dwProperty);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamRestart(HMIDISTRM hms)
{
    return ImportApiWinMM::midiStreamRestart(hms);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMidiStreamStop(HMIDISTRM hms)
{
    return ImportApiWinMM::midiStreamStop(hms);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerClose(HMIXER hmx)
{
    return ImportApiWinMM::mixerClose(hmx);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    return ImportApiWinMM::mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    return ImportApiWinMM::mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
{
    return ImportApiWinMM::mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps)
{
    return ImportApiWinMM::mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
{
    return ImportApiWinMM::mixerGetID(hmxobj, puMxId, fdwId);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
{
    return ImportApiWinMM::mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls)
{
    return ImportApiWinMM::mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
{
    return ImportApiWinMM::mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo)
{
    return ImportApiWinMM::mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
}

// ---------

UINT WINAPI ExportApiWinMMMixerGetNumDevs(void)
{
    return ImportApiWinMM::mixerGetNumDevs();
}

// ---------

DWORD WINAPI ExportApiWinMMMixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    return ImportApiWinMM::mixerMessage(driverID, uMsg, dwParam1, dwParam2);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    return ImportApiWinMM::mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    return ImportApiWinMM::mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMMIOAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    return ImportApiWinMM::mmioAdvance(hmmio, lpmmioinfo, wFlags);
}

// ---------

MMRESULT WINAPI ExportApiWinMMMMIOAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    return ImportApiWinMM::mmioAscend(hmmio, lpck, wFlags);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIOClose(HMMIO hmmio, UINT wFlags)
{
    return ImportApiWinMM::mmioClose(hmmio, wFlags);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIOCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
{
    return ImportApiWinMM::mmioCreateChunk(hmmio, lpck, wFlags);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIODescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
{
    return ImportApiWinMM::mmioDescend(hmmio, lpck, lpckParent, wFlags);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIOFlush(HMMIO hmmio, UINT fuFlush)
{
    return ImportApiWinMM::mmioFlush(hmmio, fuFlush);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIOGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
{
    return ImportApiWinMM::mmioGetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


LPMMIOPROC WINAPI ExportApiWinMMMMIOInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    return ImportApiWinMM::mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
}

// ---------


LPMMIOPROC WINAPI ExportApiWinMMMMIOInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
{
    return ImportApiWinMM::mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
}

// ---------


HMMIO WINAPI ExportApiWinMMMMIOOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    return ImportApiWinMM::mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


HMMIO WINAPI ExportApiWinMMMMIOOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
    return ImportApiWinMM::mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
}

// ---------


LONG WINAPI ExportApiWinMMMMIORead(HMMIO hmmio, HPSTR pch, LONG cch)
{
    return ImportApiWinMM::mmioRead(hmmio, pch, cch);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIORenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    return ImportApiWinMM::mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIORenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
{
    return ImportApiWinMM::mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
}

// ---------


LONG WINAPI ExportApiWinMMMMIOSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
{
    return ImportApiWinMM::mmioSeek(hmmio, lOffset, iOrigin);
}

// ---------


LRESULT WINAPI ExportApiWinMMMMIOSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
    return ImportApiWinMM::mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIOSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
{
    return ImportApiWinMM::mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
}

// ---------


MMRESULT WINAPI ExportApiWinMMMMIOSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
{
    return ImportApiWinMM::mmioSetInfo(hmmio, lpmmioinfo, wFlags);
}

// ---------


FOURCC WINAPI ExportApiWinMMMMIOStringToFOURCCA(LPCSTR sz, UINT wFlags)
{
    return ImportApiWinMM::mmioStringToFOURCCA(sz, wFlags);
}

// ---------


FOURCC WINAPI ExportApiWinMMMMIOStringToFOURCCW(LPCWSTR sz, UINT wFlags)
{
    return ImportApiWinMM::mmioStringToFOURCCW(sz, wFlags);
}

// ---------


LONG WINAPI ExportApiWinMMMMIOWrite(HMMIO hmmio, const char* pch, LONG cch)
{
    return ImportApiWinMM::mmioWrite(hmmio, pch, cch);
}

// ---------

BOOL WINAPI ExportApiWinMMSndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
{
    return ImportApiWinMM::sndPlaySoundA(lpszSound, fuSound);
}

// ---------

BOOL WINAPI ExportApiWinMMSndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
{
    return ImportApiWinMM::sndPlaySoundW(lpszSound, fuSound);
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
