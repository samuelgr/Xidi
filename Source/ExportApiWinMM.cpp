/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ExportApiWinMM.cpp
 *   Implementation of primary exported functions for the WinMM library.
 **************************************************************************************************/

#include "ImportApiWinMM.h"
#include "WrapperJoyWinMM.h"

using namespace Xidi;

extern "C"
{
  LRESULT __stdcall ExportApiWinMMCloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2)
  {
    return ImportApiWinMM::CloseDriver(hdrvr, lParam1, lParam2);
  }

  LRESULT __stdcall ExportApiWinMMDefDriverProc(
      DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2)
  {
    return ImportApiWinMM::DefDriverProc(dwDriverId, hdrvr, msg, lParam1, lParam2);
  }

  BOOL __stdcall ExportApiWinMMDriverCallback(
      DWORD dwCallBack,
      DWORD dwFlags,
      HDRVR hdrvr,
      DWORD msg,
      DWORD dwUser,
      DWORD dwParam1,
      DWORD dwParam2)
  {
    return ImportApiWinMM::DriverCallback(
        dwCallBack, dwFlags, hdrvr, msg, dwUser, dwParam1, dwParam2);
  }

  HMODULE __stdcall ExportApiWinMMDrvGetModuleHandle(HDRVR hDriver)
  {
    return ImportApiWinMM::DrvGetModuleHandle(hDriver);
  }

  HMODULE __stdcall ExportApiWinMMGetDriverModuleHandle(HDRVR hdrvr)
  {
    return ImportApiWinMM::GetDriverModuleHandle(hdrvr);
  }

  HDRVR __stdcall ExportApiWinMMOpenDriver(
      LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam)
  {
    return ImportApiWinMM::OpenDriver(lpDriverName, lpSectionName, lParam);
  }

  BOOL __stdcall ExportApiWinMMPlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
  {
    return ImportApiWinMM::PlaySoundA(pszSound, hmod, fdwSound);
  }

  BOOL __stdcall ExportApiWinMMPlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound)
  {
    return ImportApiWinMM::PlaySoundW(pszSound, hmod, fdwSound);
  }

  LRESULT __stdcall ExportApiWinMMSendDriverMessage(
      HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2)
  {
    return ImportApiWinMM::SendDriverMessage(hdrvr, msg, lParam1, lParam2);
  }

  MMRESULT __stdcall ExportApiWinMMAuxGetDevCapsA(
      UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps)
  {
    return ImportApiWinMM::auxGetDevCapsA(uDeviceID, lpCaps, cbCaps);
  }

  MMRESULT __stdcall ExportApiWinMMAuxGetDevCapsW(
      UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps)
  {
    return ImportApiWinMM::auxGetDevCapsW(uDeviceID, lpCaps, cbCaps);
  }

  UINT __stdcall ExportApiWinMMAuxGetNumDevs(void)
  {
    return ImportApiWinMM::auxGetNumDevs();
  }

  MMRESULT __stdcall ExportApiWinMMAuxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume)
  {
    return ImportApiWinMM::auxGetVolume(uDeviceID, lpdwVolume);
  }

  MMRESULT __stdcall ExportApiWinMMAuxOutMessage(
      UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
  {
    return ImportApiWinMM::auxOutMessage(uDeviceID, uMsg, dwParam1, dwParam2);
  }

  MMRESULT __stdcall ExportApiWinMMAuxSetVolume(UINT uDeviceID, DWORD dwVolume)
  {
    return ImportApiWinMM::auxSetVolume(uDeviceID, dwVolume);
  }

  MMRESULT __stdcall ExportApiWinMMJoyConfigChanged(DWORD dwFlags)
  {
    return WrapperJoyWinMM::JoyConfigChanged(dwFlags);
  }

  MMRESULT __stdcall ExportApiWinMMJoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc)
  {
    return WrapperJoyWinMM::JoyGetDevCaps(uJoyID, pjc, cbjc);
  }

  MMRESULT __stdcall ExportApiWinMMJoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc)
  {
    return WrapperJoyWinMM::JoyGetDevCaps(uJoyID, pjc, cbjc);
  }

  UINT __stdcall ExportApiWinMMJoyGetNumDevs(void)
  {
    return WrapperJoyWinMM::JoyGetNumDevs();
  }

  MMRESULT __stdcall ExportApiWinMMJoyGetPos(UINT uJoyID, LPJOYINFO pji)
  {
    return WrapperJoyWinMM::JoyGetPos(uJoyID, pji);
  }

  MMRESULT __stdcall ExportApiWinMMJoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
  {
    return WrapperJoyWinMM::JoyGetPosEx(uJoyID, pji);
  }

  MMRESULT __stdcall ExportApiWinMMJoyGetThreshold(UINT uJoyID, LPUINT puThreshold)
  {
    return WrapperJoyWinMM::JoyGetThreshold(uJoyID, puThreshold);
  }

  MMRESULT __stdcall ExportApiWinMMJoyReleaseCapture(UINT uJoyID)
  {
    return WrapperJoyWinMM::JoyReleaseCapture(uJoyID);
  }

  MMRESULT __stdcall ExportApiWinMMJoySetCapture(
      HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged)
  {
    return WrapperJoyWinMM::JoySetCapture(hwnd, uJoyID, uPeriod, fChanged);
  }

  MMRESULT __stdcall ExportApiWinMMJoySetThreshold(UINT uJoyID, UINT uThreshold)
  {
    return WrapperJoyWinMM::JoySetThreshold(uJoyID, uThreshold);
  }

  BOOL __stdcall ExportApiWinMMMCIDriverNotify(
      HWND hwndCallback, MCIDEVICEID IDDevice, UINT uStatus)
  {
    return ImportApiWinMM::mciDriverNotify(hwndCallback, IDDevice, uStatus);
  }

  UINT __stdcall ExportApiWinMMMCIDriverYield(MCIDEVICEID IDDevice)
  {
    return ImportApiWinMM::mciDriverYield(IDDevice);
  }

  BOOL __stdcall ExportApiWinMMMCIExecute(LPCSTR pszCommand)
  {
    return ImportApiWinMM::mciExecute(pszCommand);
  }

  BOOL __stdcall ExportApiWinMMMCIFreeCommandResource(UINT uResource)
  {
    return ImportApiWinMM::mciFreeCommandResource(uResource);
  }

  HANDLE __stdcall ExportApiWinMMMCIGetCreatorTask(MCIDEVICEID IDDevice)
  {
    return ImportApiWinMM::mciGetCreatorTask(IDDevice);
  }

  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDA(LPCSTR lpszDevice)
  {
    return ImportApiWinMM::mciGetDeviceIDA(lpszDevice);
  }

  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDW(LPCWSTR lpszDevice)
  {
    return ImportApiWinMM::mciGetDeviceIDW(lpszDevice);
  }

  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDFromElementIDA(
      DWORD dwElementID, LPCSTR lpstrType)
  {
    return ImportApiWinMM::mciGetDeviceIDFromElementIDA(dwElementID, lpstrType);
  }

  MCIDEVICEID __stdcall ExportApiWinMMMCIGetDeviceIDFromElementIDW(
      DWORD dwElementID, LPCWSTR lpstrType)
  {
    return ImportApiWinMM::mciGetDeviceIDFromElementIDW(dwElementID, lpstrType);
  }

  DWORD_PTR __stdcall ExportApiWinMMMCIGetDriverData(MCIDEVICEID IDDevice)
  {
    return ImportApiWinMM::mciGetDriverData(IDDevice);
  }

  BOOL __stdcall ExportApiWinMMMCIGetErrorStringA(
      DWORD fdwError, LPCSTR lpszErrorText, UINT cchErrorText)
  {
    return ImportApiWinMM::mciGetErrorStringA(fdwError, lpszErrorText, cchErrorText);
  }

  BOOL __stdcall ExportApiWinMMMCIGetErrorStringW(
      DWORD fdwError, LPWSTR lpszErrorText, UINT cchErrorText)
  {
    return ImportApiWinMM::mciGetErrorStringW(fdwError, lpszErrorText, cchErrorText);
  }

  YIELDPROC __stdcall ExportApiWinMMMCIGetYieldProc(MCIDEVICEID IDDevice, LPDWORD lpdwYieldData)
  {
    return ImportApiWinMM::mciGetYieldProc(IDDevice, lpdwYieldData);
  }

  UINT __stdcall ExportApiWinMMMCILoadCommandResource(
      HINSTANCE hInst, LPCWSTR lpwstrResourceName, UINT uType)
  {
    return ImportApiWinMM::mciLoadCommandResource(hInst, lpwstrResourceName, uType);
  }

  MCIERROR __stdcall ExportApiWinMMMCISendCommandA(
      MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
  {
    return ImportApiWinMM::mciSendCommandA(IDDevice, uMsg, fdwCommand, dwParam);
  }

  MCIERROR __stdcall ExportApiWinMMMCISendCommandW(
      MCIDEVICEID IDDevice, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam)
  {
    return ImportApiWinMM::mciSendCommandW(IDDevice, uMsg, fdwCommand, dwParam);
  }

  MCIERROR __stdcall ExportApiWinMMMCISendStringA(
      LPCSTR lpszCommand, LPSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
  {
    return ImportApiWinMM::mciSendStringA(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
  }

  MCIERROR __stdcall ExportApiWinMMMCISendStringW(
      LPCWSTR lpszCommand, LPWSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
  {
    return ImportApiWinMM::mciSendStringW(lpszCommand, lpszReturnString, cchReturn, hwndCallback);
  }

  BOOL __stdcall ExportApiWinMMMCISetDriverData(MCIDEVICEID IDDevice, DWORD_PTR data)
  {
    return ImportApiWinMM::mciSetDriverData(IDDevice, data);
  }

  UINT __stdcall ExportApiWinMMMCISetYieldProc(
      MCIDEVICEID IDDevice, YIELDPROC yp, DWORD dwYieldData)
  {
    return ImportApiWinMM::mciSetYieldProc(IDDevice, yp, dwYieldData);
  }

  MMRESULT __stdcall ExportApiWinMMMidiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
  {
    return ImportApiWinMM::midiConnect(hMidi, hmo, pReserved);
  }

  MMRESULT __stdcall ExportApiWinMMMidiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved)
  {
    return ImportApiWinMM::midiDisconnect(hMidi, hmo, pReserved);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInAddBuffer(
      HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
  {
    return ImportApiWinMM::midiInAddBuffer(hMidiIn, lpMidiInHdr, cbMidiInHdr);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInClose(HMIDIIN hMidiIn)
  {
    return ImportApiWinMM::midiInClose(hMidiIn);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInGetDevCapsA(
      UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps)
  {
    return ImportApiWinMM::midiInGetDevCapsA(uDeviceID, lpMidiInCaps, cbMidiInCaps);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInGetDevCapsW(
      UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps)
  {
    return ImportApiWinMM::midiInGetDevCapsW(uDeviceID, lpMidiInCaps, cbMidiInCaps);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText)
  {
    return ImportApiWinMM::midiInGetErrorTextA(wError, lpText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText)
  {
    return ImportApiWinMM::midiInGetErrorTextW(wError, lpText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInGetID(HMIDIIN hmi, LPUINT puDeviceID)
  {
    return ImportApiWinMM::midiInGetID(hmi, puDeviceID);
  }

  UINT __stdcall ExportApiWinMMMidiInGetNumDevs(void)
  {
    return ImportApiWinMM::midiInGetNumDevs();
  }

  DWORD __stdcall ExportApiWinMMMidiInMessage(
      HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
  {
    return ImportApiWinMM::midiInMessage(deviceID, msg, dw1, dw2);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInOpen(
      LPHMIDIIN lphMidiIn,
      UINT uDeviceID,
      DWORD_PTR dwCallback,
      DWORD_PTR dwCallbackInstance,
      DWORD dwFlags)
  {
    return ImportApiWinMM::midiInOpen(
        lphMidiIn, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInPrepareHeader(
      HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
  {
    return ImportApiWinMM::midiInPrepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInReset(HMIDIIN hMidiIn)
  {
    return ImportApiWinMM::midiInReset(hMidiIn);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInStart(HMIDIIN hMidiIn)
  {
    return ImportApiWinMM::midiInStart(hMidiIn);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInStop(HMIDIIN hMidiIn)
  {
    return ImportApiWinMM::midiInStop(hMidiIn);
  }

  MMRESULT __stdcall ExportApiWinMMMidiInUnprepareHeader(
      HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr)
  {
    return ImportApiWinMM::midiInUnprepareHeader(hMidiIn, lpMidiInHdr, cbMidiInHdr);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutCacheDrumPatches(
      HMIDIOUT hmo, UINT wPatch, WORD* lpKeyArray, UINT wFlags)
  {
    return ImportApiWinMM::midiOutCacheDrumPatches(hmo, wPatch, lpKeyArray, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutCachePatches(
      HMIDIOUT hmo, UINT wBank, WORD* lpPatchArray, UINT wFlags)
  {
    return ImportApiWinMM::midiOutCachePatches(hmo, wBank, lpPatchArray, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutClose(HMIDIOUT hmo)
  {
    return ImportApiWinMM::midiOutClose(hmo);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutGetDevCapsA(
      UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps)
  {
    return ImportApiWinMM::midiOutGetDevCapsA(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutGetDevCapsW(
      UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps)
  {
    return ImportApiWinMM::midiOutGetDevCapsW(uDeviceID, lpMidiOutCaps, cbMidiOutCaps);
  }

  UINT __stdcall ExportApiWinMMMidiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText)
  {
    return ImportApiWinMM::midiOutGetErrorTextA(mmrError, lpText, cchText);
  }

  UINT __stdcall ExportApiWinMMMidiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText)
  {
    return ImportApiWinMM::midiOutGetErrorTextW(mmrError, lpText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID)
  {
    return ImportApiWinMM::midiOutGetID(hmo, puDeviceID);
  }

  UINT __stdcall ExportApiWinMMMidiOutGetNumDevs(void)
  {
    return ImportApiWinMM::midiOutGetNumDevs();
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume)
  {
    return ImportApiWinMM::midiOutGetVolume(hmo, lpdwVolume);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutLongMsg(
      HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
  {
    return ImportApiWinMM::midiOutLongMsg(hmo, lpMidiOutHdr, cbMidiOutHdr);
  }

  DWORD __stdcall ExportApiWinMMMidiOutMessage(
      HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
  {
    return ImportApiWinMM::midiOutMessage(deviceID, msg, dw1, dw2);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutOpen(
      LPHMIDIOUT lphmo,
      UINT uDeviceID,
      DWORD_PTR dwCallback,
      DWORD_PTR dwCallbackInstance,
      DWORD dwFlags)
  {
    return ImportApiWinMM::midiOutOpen(lphmo, uDeviceID, dwCallback, dwCallbackInstance, dwFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutPrepareHeader(
      HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
  {
    return ImportApiWinMM::midiOutPrepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutReset(HMIDIOUT hmo)
  {
    return ImportApiWinMM::midiOutReset(hmo);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume)
  {
    return ImportApiWinMM::midiOutSetVolume(hmo, dwVolume);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg)
  {
    return ImportApiWinMM::midiOutShortMsg(hmo, dwMsg);
  }

  MMRESULT __stdcall ExportApiWinMMMidiOutUnprepareHeader(
      HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr)
  {
    return ImportApiWinMM::midiOutUnprepareHeader(hmo, lpMidiOutHdr, cbMidiOutHdr);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamClose(HMIDISTRM hStream)
  {
    return ImportApiWinMM::midiStreamClose(hStream);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamOpen(
      LPHMIDISTRM lphStream,
      LPUINT puDeviceID,
      DWORD cMidi,
      DWORD_PTR dwCallback,
      DWORD_PTR dwInstance,
      DWORD fdwOpen)
  {
    return ImportApiWinMM::midiStreamOpen(
        lphStream, puDeviceID, cMidi, dwCallback, dwInstance, fdwOpen);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamOut(
      HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
  {
    return ImportApiWinMM::midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamPause(HMIDISTRM hms)
  {
    return ImportApiWinMM::midiStreamPause(hms);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt)
  {
    return ImportApiWinMM::midiStreamPosition(hms, pmmt, cbmmt);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamProperty(
      HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty)
  {
    return ImportApiWinMM::midiStreamProperty(hm, lppropdata, dwProperty);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamRestart(HMIDISTRM hms)
  {
    return ImportApiWinMM::midiStreamRestart(hms);
  }

  MMRESULT __stdcall ExportApiWinMMMidiStreamStop(HMIDISTRM hms)
  {
    return ImportApiWinMM::midiStreamStop(hms);
  }

  MMRESULT __stdcall ExportApiWinMMMixerClose(HMIXER hmx)
  {
    return ImportApiWinMM::mixerClose(hmx);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetControlDetailsA(
      HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
  {
    return ImportApiWinMM::mixerGetControlDetailsA(hmxobj, pmxcd, fdwDetails);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetControlDetailsW(
      HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
  {
    return ImportApiWinMM::mixerGetControlDetailsW(hmxobj, pmxcd, fdwDetails);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetDevCapsA(
      UINT_PTR uMxId, LPMIXERCAPSA pmxcaps, UINT cbmxcaps)
  {
    return ImportApiWinMM::mixerGetDevCapsA(uMxId, pmxcaps, cbmxcaps);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetDevCapsW(
      UINT_PTR uMxId, LPMIXERCAPSW pmxcaps, UINT cbmxcaps)
  {
    return ImportApiWinMM::mixerGetDevCapsW(uMxId, pmxcaps, cbmxcaps);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId)
  {
    return ImportApiWinMM::mixerGetID(hmxobj, puMxId, fdwId);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetLineControlsA(
      HMIXEROBJ hmxobj, LPMIXERLINECONTROLSA pmxlc, DWORD fdwControls)
  {
    return ImportApiWinMM::mixerGetLineControlsA(hmxobj, pmxlc, fdwControls);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetLineControlsW(
      HMIXEROBJ hmxobj, LPMIXERLINECONTROLSW pmxlc, DWORD fdwControls)
  {
    return ImportApiWinMM::mixerGetLineControlsW(hmxobj, pmxlc, fdwControls);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetLineInfoA(
      HMIXEROBJ hmxobj, LPMIXERLINEA pmxl, DWORD fdwInfo)
  {
    return ImportApiWinMM::mixerGetLineInfoA(hmxobj, pmxl, fdwInfo);
  }

  MMRESULT __stdcall ExportApiWinMMMixerGetLineInfoW(
      HMIXEROBJ hmxobj, LPMIXERLINEW pmxl, DWORD fdwInfo)
  {
    return ImportApiWinMM::mixerGetLineInfoW(hmxobj, pmxl, fdwInfo);
  }

  UINT __stdcall ExportApiWinMMMixerGetNumDevs(void)
  {
    return ImportApiWinMM::mixerGetNumDevs();
  }

  DWORD __stdcall ExportApiWinMMMixerMessage(
      HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
  {
    return ImportApiWinMM::mixerMessage(driverID, uMsg, dwParam1, dwParam2);
  }

  MMRESULT __stdcall ExportApiWinMMMixerOpen(
      LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
  {
    return ImportApiWinMM::mixerOpen(phmx, uMxId, dwCallback, dwInstance, fdwOpen);
  }

  MMRESULT __stdcall ExportApiWinMMMixerSetControlDetails(
      HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
  {
    return ImportApiWinMM::mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
  {
    return ImportApiWinMM::mmioAdvance(hmmio, lpmmioinfo, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
  {
    return ImportApiWinMM::mmioAscend(hmmio, lpck, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOClose(HMMIO hmmio, UINT wFlags)
  {
    return ImportApiWinMM::mmioClose(hmmio, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags)
  {
    return ImportApiWinMM::mmioCreateChunk(hmmio, lpck, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIODescend(
      HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags)
  {
    return ImportApiWinMM::mmioDescend(hmmio, lpck, lpckParent, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOFlush(HMMIO hmmio, UINT fuFlush)
  {
    return ImportApiWinMM::mmioFlush(hmmio, fuFlush);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags)
  {
    return ImportApiWinMM::mmioGetInfo(hmmio, lpmmioinfo, wFlags);
  }

  LPMMIOPROC __stdcall ExportApiWinMMMMIOInstallIOProcA(
      FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
  {
    return ImportApiWinMM::mmioInstallIOProcA(fccIOProc, pIOProc, dwFlags);
  }

  LPMMIOPROC __stdcall ExportApiWinMMMMIOInstallIOProcW(
      FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags)
  {
    return ImportApiWinMM::mmioInstallIOProcW(fccIOProc, pIOProc, dwFlags);
  }

  HMMIO __stdcall ExportApiWinMMMMIOOpenA(
      LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
  {
    return ImportApiWinMM::mmioOpenA(szFilename, lpmmioinfo, dwOpenFlags);
  }

  HMMIO __stdcall ExportApiWinMMMMIOOpenW(
      LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
  {
    return ImportApiWinMM::mmioOpenW(szFilename, lpmmioinfo, dwOpenFlags);
  }

  LONG __stdcall ExportApiWinMMMMIORead(HMMIO hmmio, HPSTR pch, LONG cch)
  {
    return ImportApiWinMM::mmioRead(hmmio, pch, cch);
  }

  MMRESULT __stdcall ExportApiWinMMMMIORenameA(
      LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
  {
    return ImportApiWinMM::mmioRenameA(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIORenameW(
      LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags)
  {
    return ImportApiWinMM::mmioRenameW(szFilename, szNewFilename, lpmmioinfo, dwRenameFlags);
  }

  LONG __stdcall ExportApiWinMMMMIOSeek(HMMIO hmmio, LONG lOffset, int iOrigin)
  {
    return ImportApiWinMM::mmioSeek(hmmio, lOffset, iOrigin);
  }

  LRESULT __stdcall ExportApiWinMMMMIOSendMessage(
      HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2)
  {
    return ImportApiWinMM::mmioSendMessage(hmmio, wMsg, lParam1, lParam2);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOSetBuffer(
      HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags)
  {
    return ImportApiWinMM::mmioSetBuffer(hmmio, pchBuffer, cchBuffer, wFlags);
  }

  MMRESULT __stdcall ExportApiWinMMMMIOSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags)
  {
    return ImportApiWinMM::mmioSetInfo(hmmio, lpmmioinfo, wFlags);
  }

  FOURCC __stdcall ExportApiWinMMMMIOStringToFOURCCA(LPCSTR sz, UINT wFlags)
  {
    return ImportApiWinMM::mmioStringToFOURCCA(sz, wFlags);
  }

  FOURCC __stdcall ExportApiWinMMMMIOStringToFOURCCW(LPCWSTR sz, UINT wFlags)
  {
    return ImportApiWinMM::mmioStringToFOURCCW(sz, wFlags);
  }

  LONG __stdcall ExportApiWinMMMMIOWrite(HMMIO hmmio, const char* pch, LONG cch)
  {
    return ImportApiWinMM::mmioWrite(hmmio, pch, cch);
  }

  BOOL __stdcall ExportApiWinMMSndPlaySoundA(LPCSTR lpszSound, UINT fuSound)
  {
    return ImportApiWinMM::sndPlaySoundA(lpszSound, fuSound);
  }

  BOOL __stdcall ExportApiWinMMSndPlaySoundW(LPCWSTR lpszSound, UINT fuSound)
  {
    return ImportApiWinMM::sndPlaySoundW(lpszSound, fuSound);
  }

  MMRESULT __stdcall ExportApiWinMMTimeBeginPeriod(UINT uPeriod)
  {
    return ImportApiWinMM::timeBeginPeriod(uPeriod);
  }

  MMRESULT __stdcall ExportApiWinMMTimeEndPeriod(UINT uPeriod)
  {
    return ImportApiWinMM::timeEndPeriod(uPeriod);
  }

  MMRESULT __stdcall ExportApiWinMMTimeGetDevCaps(LPTIMECAPS ptc, UINT cbtc)
  {
    return ImportApiWinMM::timeGetDevCaps(ptc, cbtc);
  }

  MMRESULT __stdcall ExportApiWinMMTimeGetSystemTime(LPMMTIME pmmt, UINT cbmmt)
  {
    return ImportApiWinMM::timeGetSystemTime(pmmt, cbmmt);
  }

  DWORD __stdcall ExportApiWinMMTimeGetTime(void)
  {
    return ImportApiWinMM::timeGetTime();
  }

  MMRESULT __stdcall ExportApiWinMMTimeKillEvent(UINT uTimerID)
  {
    return ImportApiWinMM::timeKillEvent(uTimerID);
  }

  MMRESULT __stdcall ExportApiWinMMTimeSetEvent(
      UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent)
  {
    return ImportApiWinMM::timeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
  {
    return ImportApiWinMM::waveInAddBuffer(hwi, pwh, cbwh);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInClose(HWAVEIN hwi)
  {
    return ImportApiWinMM::waveInClose(hwi);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInGetDevCapsA(
      UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic)
  {
    return ImportApiWinMM::waveInGetDevCapsA(uDeviceID, pwic, cbwic);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInGetDevCapsW(
      UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic)
  {
    return ImportApiWinMM::waveInGetDevCapsW(uDeviceID, pwic, cbwic);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInGetErrorTextA(
      MMRESULT mmrError, LPCSTR pszText, UINT cchText)
  {
    return ImportApiWinMM::waveInGetErrorTextA(mmrError, pszText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInGetErrorTextW(
      MMRESULT mmrError, LPWSTR pszText, UINT cchText)
  {
    return ImportApiWinMM::waveInGetErrorTextW(mmrError, pszText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInGetID(HWAVEIN hwi, LPUINT puDeviceID)
  {
    return ImportApiWinMM::waveInGetID(hwi, puDeviceID);
  }

  UINT __stdcall ExportApiWinMMWaveInGetNumDevs(void)
  {
    return ImportApiWinMM::waveInGetNumDevs();
  }

  MMRESULT __stdcall ExportApiWinMMWaveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt)
  {
    return ImportApiWinMM::waveInGetPosition(hwi, pmmt, cbmmt);
  }

  DWORD __stdcall ExportApiWinMMWaveInMessage(
      HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
  {
    return ImportApiWinMM::waveInMessage(deviceID, uMsg, dwParam1, dwParam2);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInOpen(
      LPHWAVEIN phwi,
      UINT uDeviceID,
      LPCWAVEFORMATEX pwfx,
      DWORD_PTR dwCallback,
      DWORD_PTR dwCallbackInstance,
      DWORD fdwOpen)
  {
    return ImportApiWinMM::waveInOpen(
        phwi, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
  {
    return ImportApiWinMM::waveInPrepareHeader(hwi, pwh, cbwh);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInReset(HWAVEIN hwi)
  {
    return ImportApiWinMM::waveInReset(hwi);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInStart(HWAVEIN hwi)
  {
    return ImportApiWinMM::waveInStart(hwi);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInStop(HWAVEIN hwi)
  {
    return ImportApiWinMM::waveInStop(hwi);
  }

  MMRESULT __stdcall ExportApiWinMMWaveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh)
  {
    return ImportApiWinMM::waveInUnprepareHeader(hwi, pwh, cbwh);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutBreakLoop(HWAVEOUT hwo)
  {
    return ImportApiWinMM::waveOutBreakLoop(hwo);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutClose(HWAVEOUT hwo)
  {
    return ImportApiWinMM::waveOutClose(hwo);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetDevCapsA(
      UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc)
  {
    return ImportApiWinMM::waveOutGetDevCapsA(uDeviceID, pwoc, cbwoc);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetDevCapsW(
      UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc)
  {
    return ImportApiWinMM::waveOutGetDevCapsW(uDeviceID, pwoc, cbwoc);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetErrorTextA(
      MMRESULT mmrError, LPCSTR pszText, UINT cchText)
  {
    return ImportApiWinMM::waveOutGetErrorTextA(mmrError, pszText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetErrorTextW(
      MMRESULT mmrError, LPWSTR pszText, UINT cchText)
  {
    return ImportApiWinMM::waveOutGetErrorTextW(mmrError, pszText, cchText);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID)
  {
    return ImportApiWinMM::waveOutGetID(hwo, puDeviceID);
  }

  UINT __stdcall ExportApiWinMMWaveOutGetNumDevs(void)
  {
    return ImportApiWinMM::waveOutGetNumDevs();
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch)
  {
    return ImportApiWinMM::waveOutGetPitch(hwo, pdwPitch);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate)
  {
    return ImportApiWinMM::waveOutGetPlaybackRate(hwo, pdwRate);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
  {
    return ImportApiWinMM::waveOutGetPosition(hwo, pmmt, cbmmt);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume)
  {
    return ImportApiWinMM::waveOutGetVolume(hwo, pdwVolume);
  }

  DWORD __stdcall ExportApiWinMMWaveOutMessage(
      HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
  {
    return ImportApiWinMM::waveOutMessage(deviceID, uMsg, dwParam1, dwParam2);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutOpen(
      LPHWAVEOUT phwo,
      UINT_PTR uDeviceID,
      LPWAVEFORMATEX pwfx,
      DWORD_PTR dwCallback,
      DWORD_PTR dwCallbackInstance,
      DWORD fdwOpen)
  {
    return ImportApiWinMM::waveOutOpen(
        phwo, uDeviceID, pwfx, dwCallback, dwCallbackInstance, fdwOpen);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutPause(HWAVEOUT hwo)
  {
    return ImportApiWinMM::waveOutPause(hwo);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
  {
    return ImportApiWinMM::waveOutPrepareHeader(hwo, pwh, cbwh);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutReset(HWAVEOUT hwo)
  {
    return ImportApiWinMM::waveOutReset(hwo);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutRestart(HWAVEOUT hwo)
  {
    return ImportApiWinMM::waveOutRestart(hwo);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch)
  {
    return ImportApiWinMM::waveOutSetPitch(hwo, dwPitch);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate)
  {
    return ImportApiWinMM::waveOutSetPlaybackRate(hwo, dwRate);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume)
  {
    return ImportApiWinMM::waveOutSetVolume(hwo, dwVolume);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
  {
    return ImportApiWinMM::waveOutUnprepareHeader(hwo, pwh, cbwh);
  }

  MMRESULT __stdcall ExportApiWinMMWaveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
  {
    return ImportApiWinMM::waveOutWrite(hwo, pwh, cbwh);
  }
}
