;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Xidi
;   DirectInput interface for XInput controllers.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Authored by Samuel Grossman
; Copyright (c) 2016-2025
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ForwardedApiWinMM.asm
;   Partial implementation of exported function entry points for WinMM that should be forwarded
;   perfectly to the system version of the same.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INCLUDE DllExportForward.inc


_TEXT                                       SEGMENT


DllExportForward winmm, CloseDriver
DllExportForward winmm, DefDriverProc
DllExportForward winmm, DriverCallback
DllExportForward winmm, DrvGetModuleHandle
DllExportForward winmm, GetDriverModuleHandle
DllExportForward winmm, OpenDriver
DllExportForward winmm, PlaySoundA
DllExportForward winmm, PlaySoundW
DllExportForward winmm, SendDriverMessage
DllExportForward winmm, auxGetDevCapsA
DllExportForward winmm, auxGetDevCapsW
DllExportForward winmm, auxGetNumDevs
DllExportForward winmm, auxGetVolume
DllExportForward winmm, auxOutMessage
DllExportForward winmm, auxSetVolume
DllExportForward winmm, mciDriverNotify
DllExportForward winmm, mciDriverYield
DllExportForward winmm, mciExecute
DllExportForward winmm, mciFreeCommandResource
DllExportForward winmm, mciGetCreatorTask
DllExportForward winmm, mciGetDeviceIDA
DllExportForward winmm, mciGetDeviceIDW
DllExportForward winmm, mciGetDeviceIDFromElementIDA
DllExportForward winmm, mciGetDeviceIDFromElementIDW
DllExportForward winmm, mciGetDriverData
DllExportForward winmm, mciGetErrorStringA
DllExportForward winmm, mciGetErrorStringW
DllExportForward winmm, mciGetYieldProc
DllExportForward winmm, mciLoadCommandResource
DllExportForward winmm, mciSendCommandA
DllExportForward winmm, mciSendCommandW
DllExportForward winmm, mciSendStringA
DllExportForward winmm, mciSendStringW
DllExportForward winmm, mciSetDriverData
DllExportForward winmm, mciSetYieldProc
DllExportForward winmm, midiConnect
DllExportForward winmm, midiDisconnect
DllExportForward winmm, midiInAddBuffer
DllExportForward winmm, midiInClose
DllExportForward winmm, midiInGetDevCapsA
DllExportForward winmm, midiInGetDevCapsW
DllExportForward winmm, midiInGetErrorTextA
DllExportForward winmm, midiInGetErrorTextW
DllExportForward winmm, midiInGetID
DllExportForward winmm, midiInGetNumDevs
DllExportForward winmm, midiInMessage
DllExportForward winmm, midiInOpen
DllExportForward winmm, midiInPrepareHeader
DllExportForward winmm, midiInReset
DllExportForward winmm, midiInStart
DllExportForward winmm, midiInStop
DllExportForward winmm, midiInUnprepareHeader
DllExportForward winmm, midiOutCacheDrumPatches
DllExportForward winmm, midiOutCachePatches
DllExportForward winmm, midiOutClose
DllExportForward winmm, midiOutGetDevCapsA
DllExportForward winmm, midiOutGetDevCapsW
DllExportForward winmm, midiOutGetErrorTextA
DllExportForward winmm, midiOutGetErrorTextW
DllExportForward winmm, midiOutGetID
DllExportForward winmm, midiOutGetNumDevs
DllExportForward winmm, midiOutGetVolume
DllExportForward winmm, midiOutLongMsg
DllExportForward winmm, midiOutMessage
DllExportForward winmm, midiOutOpen
DllExportForward winmm, midiOutPrepareHeader
DllExportForward winmm, midiOutReset
DllExportForward winmm, midiOutSetVolume
DllExportForward winmm, midiOutShortMsg
DllExportForward winmm, midiOutUnprepareHeader
DllExportForward winmm, midiStreamClose
DllExportForward winmm, midiStreamOpen
DllExportForward winmm, midiStreamOut
DllExportForward winmm, midiStreamPause
DllExportForward winmm, midiStreamPosition
DllExportForward winmm, midiStreamProperty
DllExportForward winmm, midiStreamRestart
DllExportForward winmm, midiStreamStop
DllExportForward winmm, mixerClose
DllExportForward winmm, mixerGetControlDetailsA
DllExportForward winmm, mixerGetControlDetailsW
DllExportForward winmm, mixerGetDevCapsA
DllExportForward winmm, mixerGetDevCapsW
DllExportForward winmm, mixerGetID
DllExportForward winmm, mixerGetLineControlsA
DllExportForward winmm, mixerGetLineControlsW
DllExportForward winmm, mixerGetLineInfoA
DllExportForward winmm, mixerGetLineInfoW
DllExportForward winmm, mixerGetNumDevs
DllExportForward winmm, mixerMessage
DllExportForward winmm, mixerOpen
DllExportForward winmm, mixerSetControlDetails
DllExportForward winmm, mmioAdvance
DllExportForward winmm, mmioAscend
DllExportForward winmm, mmioClose
DllExportForward winmm, mmioCreateChunk
DllExportForward winmm, mmioDescend
DllExportForward winmm, mmioFlush
DllExportForward winmm, mmioGetInfo
DllExportForward winmm, mmioInstallIOProcA
DllExportForward winmm, mmioInstallIOProcW
DllExportForward winmm, mmioOpenA
DllExportForward winmm, mmioOpenW
DllExportForward winmm, mmioRead
DllExportForward winmm, mmioRenameA
DllExportForward winmm, mmioRenameW
DllExportForward winmm, mmioSeek
DllExportForward winmm, mmioSendMessage
DllExportForward winmm, mmioSetBuffer
DllExportForward winmm, mmioSetInfo
DllExportForward winmm, mmioStringToFOURCCA
DllExportForward winmm, mmioStringToFOURCCW
DllExportForward winmm, mmioWrite
DllExportForward winmm, sndPlaySoundA
DllExportForward winmm, sndPlaySoundW
DllExportForward winmm, timeBeginPeriod
DllExportForward winmm, timeEndPeriod
DllExportForward winmm, timeGetDevCaps
DllExportForward winmm, timeGetSystemTime
DllExportForward winmm, timeGetTime
DllExportForward winmm, timeKillEvent
DllExportForward winmm, timeSetEvent
DllExportForward winmm, waveInAddBuffer
DllExportForward winmm, waveInClose
DllExportForward winmm, waveInGetDevCapsA
DllExportForward winmm, waveInGetDevCapsW
DllExportForward winmm, waveInGetErrorTextA
DllExportForward winmm, waveInGetErrorTextW
DllExportForward winmm, waveInGetID
DllExportForward winmm, waveInGetNumDevs
DllExportForward winmm, waveInGetPosition
DllExportForward winmm, waveInMessage
DllExportForward winmm, waveInOpen
DllExportForward winmm, waveInPrepareHeader
DllExportForward winmm, waveInReset
DllExportForward winmm, waveInStart
DllExportForward winmm, waveInStop
DllExportForward winmm, waveInUnprepareHeader
DllExportForward winmm, waveOutBreakLoop
DllExportForward winmm, waveOutClose
DllExportForward winmm, waveOutGetDevCapsA
DllExportForward winmm, waveOutGetDevCapsW
DllExportForward winmm, waveOutGetErrorTextA
DllExportForward winmm, waveOutGetErrorTextW
DllExportForward winmm, waveOutGetID
DllExportForward winmm, waveOutGetNumDevs
DllExportForward winmm, waveOutGetPitch
DllExportForward winmm, waveOutGetPlaybackRate
DllExportForward winmm, waveOutGetPosition
DllExportForward winmm, waveOutGetVolume
DllExportForward winmm, waveOutMessage
DllExportForward winmm, waveOutOpen
DllExportForward winmm, waveOutPause
DllExportForward winmm, waveOutPrepareHeader
DllExportForward winmm, waveOutReset
DllExportForward winmm, waveOutRestart
DllExportForward winmm, waveOutSetPitch
DllExportForward winmm, waveOutSetPlaybackRate
DllExportForward winmm, waveOutSetVolume
DllExportForward winmm, waveOutUnprepareHeader
DllExportForward winmm, waveOutWrite


_TEXT                                       ENDS


END
