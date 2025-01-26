/***************************************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 ***************************************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2025
 ***********************************************************************************************//**
 * @file ForwardedApiWinMM.cpp
 *   Partial implementation of exported function entry points for WinMM that should be forwarded
 *   perfectly to the system version of the same.
 **************************************************************************************************/

#include "DllFunctions.h"
#include "Globals.h"
#include "Strings.h"

DLL_EXPORT_FORWARD_DEFINE_DLL_WITH_CUSTOM_PATH(winmm)
{
  using namespace Xidi;
  return Globals::GetConfigurationData()[Strings::kStrConfigurationSectionImport]
                                        [Strings::kStrConfigurationSettingImportWinMM]
                                            .ValueOr(Strings::GetSystemLibraryFilenameWinMM());
}

DLL_EXPORT_FORWARD(winmm, CloseDriver);
DLL_EXPORT_FORWARD(winmm, DefDriverProc);
DLL_EXPORT_FORWARD(winmm, DriverCallback);
DLL_EXPORT_FORWARD(winmm, DrvGetModuleHandle);
DLL_EXPORT_FORWARD(winmm, GetDriverModuleHandle);
DLL_EXPORT_FORWARD(winmm, OpenDriver);
DLL_EXPORT_FORWARD(winmm, PlaySoundA);
DLL_EXPORT_FORWARD(winmm, PlaySoundW);
DLL_EXPORT_FORWARD(winmm, SendDriverMessage);
DLL_EXPORT_FORWARD(winmm, auxGetDevCapsA);
DLL_EXPORT_FORWARD(winmm, auxGetDevCapsW);
DLL_EXPORT_FORWARD(winmm, auxGetNumDevs);
DLL_EXPORT_FORWARD(winmm, auxGetVolume);
DLL_EXPORT_FORWARD(winmm, auxOutMessage);
DLL_EXPORT_FORWARD(winmm, auxSetVolume);
DLL_EXPORT_FORWARD(winmm, mciDriverNotify);
DLL_EXPORT_FORWARD(winmm, mciDriverYield);
DLL_EXPORT_FORWARD(winmm, mciExecute);
DLL_EXPORT_FORWARD(winmm, mciFreeCommandResource);
DLL_EXPORT_FORWARD(winmm, mciGetCreatorTask);
DLL_EXPORT_FORWARD(winmm, mciGetDeviceIDA);
DLL_EXPORT_FORWARD(winmm, mciGetDeviceIDW);
DLL_EXPORT_FORWARD(winmm, mciGetDeviceIDFromElementIDA);
DLL_EXPORT_FORWARD(winmm, mciGetDeviceIDFromElementIDW);
DLL_EXPORT_FORWARD(winmm, mciGetDriverData);
DLL_EXPORT_FORWARD(winmm, mciGetErrorStringA);
DLL_EXPORT_FORWARD(winmm, mciGetErrorStringW);
DLL_EXPORT_FORWARD(winmm, mciGetYieldProc);
DLL_EXPORT_FORWARD(winmm, mciLoadCommandResource);
DLL_EXPORT_FORWARD(winmm, mciSendCommandA);
DLL_EXPORT_FORWARD(winmm, mciSendCommandW);
DLL_EXPORT_FORWARD(winmm, mciSendStringA);
DLL_EXPORT_FORWARD(winmm, mciSendStringW);
DLL_EXPORT_FORWARD(winmm, mciSetDriverData);
DLL_EXPORT_FORWARD(winmm, mciSetYieldProc);
DLL_EXPORT_FORWARD(winmm, midiConnect);
DLL_EXPORT_FORWARD(winmm, midiDisconnect);
DLL_EXPORT_FORWARD(winmm, midiInAddBuffer);
DLL_EXPORT_FORWARD(winmm, midiInClose);
DLL_EXPORT_FORWARD(winmm, midiInGetDevCapsA);
DLL_EXPORT_FORWARD(winmm, midiInGetDevCapsW);
DLL_EXPORT_FORWARD(winmm, midiInGetErrorTextA);
DLL_EXPORT_FORWARD(winmm, midiInGetErrorTextW);
DLL_EXPORT_FORWARD(winmm, midiInGetID);
DLL_EXPORT_FORWARD(winmm, midiInGetNumDevs);
DLL_EXPORT_FORWARD(winmm, midiInMessage);
DLL_EXPORT_FORWARD(winmm, midiInOpen);
DLL_EXPORT_FORWARD(winmm, midiInPrepareHeader);
DLL_EXPORT_FORWARD(winmm, midiInReset);
DLL_EXPORT_FORWARD(winmm, midiInStart);
DLL_EXPORT_FORWARD(winmm, midiInStop);
DLL_EXPORT_FORWARD(winmm, midiInUnprepareHeader);
DLL_EXPORT_FORWARD(winmm, midiOutCacheDrumPatches);
DLL_EXPORT_FORWARD(winmm, midiOutCachePatches);
DLL_EXPORT_FORWARD(winmm, midiOutClose);
DLL_EXPORT_FORWARD(winmm, midiOutGetDevCapsA);
DLL_EXPORT_FORWARD(winmm, midiOutGetDevCapsW);
DLL_EXPORT_FORWARD(winmm, midiOutGetErrorTextA);
DLL_EXPORT_FORWARD(winmm, midiOutGetErrorTextW);
DLL_EXPORT_FORWARD(winmm, midiOutGetID);
DLL_EXPORT_FORWARD(winmm, midiOutGetNumDevs);
DLL_EXPORT_FORWARD(winmm, midiOutGetVolume);
DLL_EXPORT_FORWARD(winmm, midiOutLongMsg);
DLL_EXPORT_FORWARD(winmm, midiOutMessage);
DLL_EXPORT_FORWARD(winmm, midiOutOpen);
DLL_EXPORT_FORWARD(winmm, midiOutPrepareHeader);
DLL_EXPORT_FORWARD(winmm, midiOutReset);
DLL_EXPORT_FORWARD(winmm, midiOutSetVolume);
DLL_EXPORT_FORWARD(winmm, midiOutShortMsg);
DLL_EXPORT_FORWARD(winmm, midiOutUnprepareHeader);
DLL_EXPORT_FORWARD(winmm, midiStreamClose);
DLL_EXPORT_FORWARD(winmm, midiStreamOpen);
DLL_EXPORT_FORWARD(winmm, midiStreamOut);
DLL_EXPORT_FORWARD(winmm, midiStreamPause);
DLL_EXPORT_FORWARD(winmm, midiStreamPosition);
DLL_EXPORT_FORWARD(winmm, midiStreamProperty);
DLL_EXPORT_FORWARD(winmm, midiStreamRestart);
DLL_EXPORT_FORWARD(winmm, midiStreamStop);
DLL_EXPORT_FORWARD(winmm, mixerClose);
DLL_EXPORT_FORWARD(winmm, mixerGetControlDetailsA);
DLL_EXPORT_FORWARD(winmm, mixerGetControlDetailsW);
DLL_EXPORT_FORWARD(winmm, mixerGetDevCapsA);
DLL_EXPORT_FORWARD(winmm, mixerGetDevCapsW);
DLL_EXPORT_FORWARD(winmm, mixerGetID);
DLL_EXPORT_FORWARD(winmm, mixerGetLineControlsA);
DLL_EXPORT_FORWARD(winmm, mixerGetLineControlsW);
DLL_EXPORT_FORWARD(winmm, mixerGetLineInfoA);
DLL_EXPORT_FORWARD(winmm, mixerGetLineInfoW);
DLL_EXPORT_FORWARD(winmm, mixerGetNumDevs);
DLL_EXPORT_FORWARD(winmm, mixerMessage);
DLL_EXPORT_FORWARD(winmm, mixerOpen);
DLL_EXPORT_FORWARD(winmm, mixerSetControlDetails);
DLL_EXPORT_FORWARD(winmm, mmioAdvance);
DLL_EXPORT_FORWARD(winmm, mmioAscend);
DLL_EXPORT_FORWARD(winmm, mmioClose);
DLL_EXPORT_FORWARD(winmm, mmioCreateChunk);
DLL_EXPORT_FORWARD(winmm, mmioDescend);
DLL_EXPORT_FORWARD(winmm, mmioFlush);
DLL_EXPORT_FORWARD(winmm, mmioGetInfo);
DLL_EXPORT_FORWARD(winmm, mmioInstallIOProcA);
DLL_EXPORT_FORWARD(winmm, mmioInstallIOProcW);
DLL_EXPORT_FORWARD(winmm, mmioOpenA);
DLL_EXPORT_FORWARD(winmm, mmioOpenW);
DLL_EXPORT_FORWARD(winmm, mmioRead);
DLL_EXPORT_FORWARD(winmm, mmioRenameA);
DLL_EXPORT_FORWARD(winmm, mmioRenameW);
DLL_EXPORT_FORWARD(winmm, mmioSeek);
DLL_EXPORT_FORWARD(winmm, mmioSendMessage);
DLL_EXPORT_FORWARD(winmm, mmioSetBuffer);
DLL_EXPORT_FORWARD(winmm, mmioSetInfo);
DLL_EXPORT_FORWARD(winmm, mmioStringToFOURCCA);
DLL_EXPORT_FORWARD(winmm, mmioStringToFOURCCW);
DLL_EXPORT_FORWARD(winmm, mmioWrite);
DLL_EXPORT_FORWARD(winmm, sndPlaySoundA);
DLL_EXPORT_FORWARD(winmm, sndPlaySoundW);
DLL_EXPORT_FORWARD(winmm, timeBeginPeriod);
DLL_EXPORT_FORWARD(winmm, timeEndPeriod);
DLL_EXPORT_FORWARD(winmm, timeGetDevCaps);
DLL_EXPORT_FORWARD(winmm, timeGetSystemTime);
DLL_EXPORT_FORWARD(winmm, timeGetTime);
DLL_EXPORT_FORWARD(winmm, timeKillEvent);
DLL_EXPORT_FORWARD(winmm, timeSetEvent);
DLL_EXPORT_FORWARD(winmm, waveInAddBuffer);
DLL_EXPORT_FORWARD(winmm, waveInClose);
DLL_EXPORT_FORWARD(winmm, waveInGetDevCapsA);
DLL_EXPORT_FORWARD(winmm, waveInGetDevCapsW);
DLL_EXPORT_FORWARD(winmm, waveInGetErrorTextA);
DLL_EXPORT_FORWARD(winmm, waveInGetErrorTextW);
DLL_EXPORT_FORWARD(winmm, waveInGetID);
DLL_EXPORT_FORWARD(winmm, waveInGetNumDevs);
DLL_EXPORT_FORWARD(winmm, waveInGetPosition);
DLL_EXPORT_FORWARD(winmm, waveInMessage);
DLL_EXPORT_FORWARD(winmm, waveInOpen);
DLL_EXPORT_FORWARD(winmm, waveInPrepareHeader);
DLL_EXPORT_FORWARD(winmm, waveInReset);
DLL_EXPORT_FORWARD(winmm, waveInStart);
DLL_EXPORT_FORWARD(winmm, waveInStop);
DLL_EXPORT_FORWARD(winmm, waveInUnprepareHeader);
DLL_EXPORT_FORWARD(winmm, waveOutBreakLoop);
DLL_EXPORT_FORWARD(winmm, waveOutClose);
DLL_EXPORT_FORWARD(winmm, waveOutGetDevCapsA);
DLL_EXPORT_FORWARD(winmm, waveOutGetDevCapsW);
DLL_EXPORT_FORWARD(winmm, waveOutGetErrorTextA);
DLL_EXPORT_FORWARD(winmm, waveOutGetErrorTextW);
DLL_EXPORT_FORWARD(winmm, waveOutGetID);
DLL_EXPORT_FORWARD(winmm, waveOutGetNumDevs);
DLL_EXPORT_FORWARD(winmm, waveOutGetPitch);
DLL_EXPORT_FORWARD(winmm, waveOutGetPlaybackRate);
DLL_EXPORT_FORWARD(winmm, waveOutGetPosition);
DLL_EXPORT_FORWARD(winmm, waveOutGetVolume);
DLL_EXPORT_FORWARD(winmm, waveOutMessage);
DLL_EXPORT_FORWARD(winmm, waveOutOpen);
DLL_EXPORT_FORWARD(winmm, waveOutPause);
DLL_EXPORT_FORWARD(winmm, waveOutPrepareHeader);
DLL_EXPORT_FORWARD(winmm, waveOutReset);
DLL_EXPORT_FORWARD(winmm, waveOutRestart);
DLL_EXPORT_FORWARD(winmm, waveOutSetPitch);
DLL_EXPORT_FORWARD(winmm, waveOutSetPlaybackRate);
DLL_EXPORT_FORWARD(winmm, waveOutSetVolume);
DLL_EXPORT_FORWARD(winmm, waveOutUnprepareHeader);
DLL_EXPORT_FORWARD(winmm, waveOutWrite);
