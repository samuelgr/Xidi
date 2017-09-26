/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2017
 *************************************************************************//**
 * @file ImportApiWinMM.h
 *   Declarations related to importing the API from the WinMM library.
 *****************************************************************************/

#pragma once

#include "ApiWindows.h"


namespace Xidi
{
    // Enables access to the underlying system's WinMM API.
    // Dynamically loads the library and holds pointers to its methods.
    // Methods are intended to be called directly rather than through an instance.
    class ImportApiWinMM
    {
    public:
        // -------- TYPE DEFINITIONS --------------------------------------- //

        // Fields specify the addresses of the imported DirectInput API functions.
        struct SImportTable
        {
            LRESULT     (WINAPI* CloseDriver)(HDRVR, LPARAM, LPARAM);
            LRESULT     (WINAPI* DefDriverProc)(DWORD_PTR, HDRVR, UINT, LONG, LONG);
            BOOL        (WINAPI* DriverCallback)(DWORD, DWORD, HDRVR, DWORD, DWORD, DWORD, DWORD);
            HMODULE     (WINAPI* DrvGetModuleHandle)(HDRVR);
            HMODULE     (WINAPI* GetDriverModuleHandle)(HDRVR);
            HDRVR       (WINAPI* OpenDriver)(LPCWSTR, LPCWSTR, LPARAM);
            BOOL        (WINAPI* PlaySoundA)(LPCSTR, HMODULE, DWORD);
            BOOL        (WINAPI* PlaySoundW)(LPCWSTR, HMODULE, DWORD);
            LRESULT     (WINAPI* SendDriverMessage)(HDRVR, UINT, LPARAM, LPARAM);
            
            MMRESULT    (WINAPI* auxGetDevCapsA)(UINT_PTR, LPAUXCAPSA, UINT);
            MMRESULT    (WINAPI* auxGetDevCapsW)(UINT_PTR, LPAUXCAPSW, UINT);
            UINT        (WINAPI* auxGetNumDevs)(void);
            MMRESULT    (WINAPI* auxGetVolume)(UINT, LPDWORD);
            MMRESULT    (WINAPI* auxOutMessage)(UINT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* auxSetVolume)(UINT, DWORD);

            MMRESULT    (WINAPI* joyConfigChanged)(DWORD);
            MMRESULT    (WINAPI* joyGetDevCapsA)(UINT_PTR, LPJOYCAPSA, UINT);
            MMRESULT    (WINAPI* joyGetDevCapsW)(UINT_PTR, LPJOYCAPSW, UINT);
            UINT        (WINAPI* joyGetNumDevs)(void);
            MMRESULT    (WINAPI* joyGetPos)(UINT, LPJOYINFO);
            MMRESULT    (WINAPI* joyGetPosEx)(UINT, LPJOYINFOEX);
            MMRESULT    (WINAPI* joyGetThreshold)(UINT, LPUINT);
            MMRESULT    (WINAPI* joyReleaseCapture)(UINT);
            MMRESULT    (WINAPI* joySetCapture)(HWND, UINT, UINT, BOOL);
            MMRESULT    (WINAPI* joySetThreshold)(UINT, UINT);
            
            MMRESULT    (WINAPI* midiConnect)(HMIDI, HMIDIOUT, LPVOID);
            MMRESULT    (WINAPI* midiDisconnect)(HMIDI, HMIDIOUT, LPVOID);
            
            MMRESULT    (WINAPI* midiInAddBuffer)(HMIDIIN, LPMIDIHDR, UINT);
            MMRESULT    (WINAPI* midiInClose)(HMIDIIN);
            MMRESULT    (WINAPI* midiInGetDevCapsA)(UINT_PTR, LPMIDIINCAPSA, UINT);
            MMRESULT    (WINAPI* midiInGetDevCapsW)(UINT_PTR, LPMIDIINCAPSW, UINT);
            MMRESULT    (WINAPI* midiInGetErrorTextA)(MMRESULT, LPSTR, UINT);
            MMRESULT    (WINAPI* midiInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT    (WINAPI* midiInGetID)(HMIDIIN, LPUINT);
            UINT        (WINAPI* midiInGetNumDevs)(void);
            DWORD       (WINAPI* midiInMessage)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* midiInOpen)(LPHMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* midiInPrepareHeader)(HMIDIIN, LPMIDIHDR, UINT);
            MMRESULT    (WINAPI* midiInReset)(HMIDIIN);
            MMRESULT    (WINAPI* midiInStart)(HMIDIIN);
            MMRESULT    (WINAPI* midiInStop)(HMIDIIN);
            MMRESULT    (WINAPI* midiInUnprepareHeader)(HMIDIIN, LPMIDIHDR, UINT);
            
            MMRESULT    (WINAPI* midiOutCacheDrumPatches)(HMIDIOUT, UINT, WORD*, UINT);
            MMRESULT    (WINAPI* midiOutCachePatches)(HMIDIOUT, UINT, WORD*, UINT);
            MMRESULT    (WINAPI* midiOutClose)(HMIDIOUT);
            MMRESULT    (WINAPI* midiOutGetDevCapsA)(UINT_PTR, LPMIDIOUTCAPSA, UINT);
            MMRESULT    (WINAPI* midiOutGetDevCapsW)(UINT_PTR, LPMIDIOUTCAPSW, UINT);
            UINT        (WINAPI* midiOutGetErrorTextA)(MMRESULT, LPSTR, UINT);
            UINT        (WINAPI* midiOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT    (WINAPI* midiOutGetID)(HMIDIOUT, LPUINT);
            UINT        (WINAPI* midiOutGetNumDevs)(void);
            MMRESULT    (WINAPI* midiOutGetVolume)(HMIDIOUT, LPDWORD);
            MMRESULT    (WINAPI* midiOutLongMsg)(HMIDIOUT, LPMIDIHDR, UINT);
            DWORD       (WINAPI* midiOutMessage)(HMIDIOUT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* midiOutOpen)(LPHMIDIOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* midiOutPrepareHeader)(HMIDIOUT, LPMIDIHDR, UINT);
            MMRESULT    (WINAPI* midiOutReset)(HMIDIOUT);
            MMRESULT    (WINAPI* midiOutSetVolume)(HMIDIOUT, DWORD);
            MMRESULT    (WINAPI* midiOutShortMsg)(HMIDIOUT, DWORD);
            MMRESULT    (WINAPI* midiOutUnprepareHeader)(HMIDIOUT, LPMIDIHDR, UINT);
            
            MMRESULT    (WINAPI* midiStreamClose)(HMIDISTRM);
            MMRESULT    (WINAPI* midiStreamOpen)(LPHMIDISTRM, LPUINT, DWORD, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* midiStreamOut)(HMIDISTRM, LPMIDIHDR, UINT);
            MMRESULT    (WINAPI* midiStreamPause)(HMIDISTRM);
            MMRESULT    (WINAPI* midiStreamPosition)(HMIDISTRM, LPMMTIME, UINT);
            MMRESULT    (WINAPI* midiStreamProperty)(HMIDISTRM, LPBYTE, DWORD);
            MMRESULT    (WINAPI* midiStreamRestart)(HMIDISTRM);
            MMRESULT    (WINAPI* midiStreamStop)(HMIDISTRM);

            MMRESULT    (WINAPI* mixerClose)(HMIXER);
            MMRESULT    (WINAPI* mixerGetControlDetailsA)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);
            MMRESULT    (WINAPI* mixerGetControlDetailsW)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);
            MMRESULT    (WINAPI* mixerGetDevCapsA)(UINT_PTR, LPMIXERCAPS, UINT);
            MMRESULT    (WINAPI* mixerGetDevCapsW)(UINT_PTR, LPMIXERCAPS, UINT);
            MMRESULT    (WINAPI* mixerGetID)(HMIXEROBJ, UINT*, DWORD);
            MMRESULT    (WINAPI* mixerGetLineControlsA)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD);
            MMRESULT    (WINAPI* mixerGetLineControlsW)(HMIXEROBJ, LPMIXERLINECONTROLS, DWORD);
            MMRESULT    (WINAPI* mixerGetLineInfoA)(HMIXEROBJ, LPMIXERLINE, DWORD);
            MMRESULT    (WINAPI* mixerGetLineInfoW)(HMIXEROBJ, LPMIXERLINE, DWORD);
            UINT        (WINAPI* mixerGetNumDevs)(void);
            DWORD       (WINAPI* mixerMessage)(HMIXER, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* mixerOpen)(LPHMIXER, UINT, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* mixerSetControlDetails)(HMIXEROBJ, LPMIXERCONTROLDETAILS, DWORD);

            MMRESULT    (WINAPI* mmioAdvance)(HMMIO, LPMMIOINFO, UINT);
            MMRESULT    (WINAPI* mmioAscend)(HMMIO, LPMMCKINFO, UINT);
            MMRESULT    (WINAPI* mmioClose)(HMMIO, UINT);
            MMRESULT    (WINAPI* mmioCreateChunk)(HMMIO, LPMMCKINFO, UINT);
            MMRESULT    (WINAPI* mmioDescend)(HMMIO, LPMMCKINFO, LPCMMCKINFO, UINT);
            MMRESULT    (WINAPI* mmioFlush)(HMMIO, UINT);
            MMRESULT    (WINAPI* mmioGetInfo)(HMMIO, LPMMIOINFO, UINT);
            LPMMIOPROC  (WINAPI* mmioInstallIOProcA)(FOURCC, LPMMIOPROC, DWORD);
            LPMMIOPROC  (WINAPI* mmioInstallIOProcW)(FOURCC, LPMMIOPROC, DWORD);
            HMMIO       (WINAPI* mmioOpenA)(LPSTR, LPMMIOINFO, DWORD);
            HMMIO       (WINAPI* mmioOpenW)(LPWSTR, LPMMIOINFO, DWORD);
            LONG        (WINAPI* mmioRead)(HMMIO, HPSTR, LONG);
            MMRESULT    (WINAPI* mmioRenameA)(LPCSTR, LPCSTR, LPCMMIOINFO, DWORD);
            MMRESULT    (WINAPI* mmioRenameW)(LPCWSTR, LPCWSTR, LPCMMIOINFO, DWORD);
            LONG        (WINAPI* mmioSeek)(HMMIO, LONG, int);
            LRESULT     (WINAPI* mmioSendMessage)(HMMIO, UINT, LPARAM, LPARAM);
            MMRESULT    (WINAPI* mmioSetBuffer)(HMMIO, LPSTR, LONG, UINT);
            MMRESULT    (WINAPI* mmioSetInfo)(HMMIO, LPCMMIOINFO, UINT);
            FOURCC      (WINAPI* mmioStringToFOURCCA)(LPCSTR, UINT);
            FOURCC      (WINAPI* mmioStringToFOURCCW)(LPCWSTR, UINT);
            LONG        (WINAPI* mmioWrite)(HMMIO, const char*, LONG);
            
            BOOL        (WINAPI* sndPlaySoundA)(LPCSTR lpszSound, UINT fuSound);
            BOOL        (WINAPI* sndPlaySoundW)(LPCWSTR lpszSound, UINT fuSound);
            
            MMRESULT    (WINAPI* timeBeginPeriod)(UINT);
            MMRESULT    (WINAPI* timeEndPeriod)(UINT);
            MMRESULT    (WINAPI* timeGetDevCaps)(LPTIMECAPS, UINT);
            MMRESULT    (WINAPI* timeGetSystemTime)(LPMMTIME, UINT);
            DWORD       (WINAPI* timeGetTime)(void);
            MMRESULT    (WINAPI* timeKillEvent)(UINT);
            MMRESULT    (WINAPI* timeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);

            MMRESULT    (WINAPI* waveInAddBuffer)(HWAVEIN, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveInClose)(HWAVEIN);
            MMRESULT    (WINAPI* waveInGetDevCapsA)(UINT_PTR, LPWAVEINCAPSA, UINT);
            MMRESULT    (WINAPI* waveInGetDevCapsW)(UINT_PTR, LPWAVEINCAPSW, UINT);
            MMRESULT    (WINAPI* waveInGetErrorTextA)(MMRESULT, LPCSTR, UINT);
            MMRESULT    (WINAPI* waveInGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT    (WINAPI* waveInGetID)(HWAVEIN, LPUINT);
            UINT        (WINAPI* waveInGetNumDevs)(void);
            MMRESULT    (WINAPI* waveInGetPosition)(HWAVEIN, LPMMTIME, UINT);
            DWORD       (WINAPI* waveInMessage)(HWAVEIN, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* waveInOpen)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* waveInPrepareHeader)(HWAVEIN, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveInReset)(HWAVEIN);
            MMRESULT    (WINAPI* waveInStart)(HWAVEIN);
            MMRESULT    (WINAPI* waveInStop)(HWAVEIN);
            MMRESULT    (WINAPI* waveInUnprepareHeader)(HWAVEIN, LPWAVEHDR, UINT);

            MMRESULT    (WINAPI* waveOutBreakLoop)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutClose)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutGetDevCapsA)(UINT_PTR, LPWAVEOUTCAPSA, UINT);
            MMRESULT    (WINAPI* waveOutGetDevCapsW)(UINT_PTR, LPWAVEOUTCAPSW, UINT);
            MMRESULT    (WINAPI* waveOutGetErrorTextA)(MMRESULT, LPCSTR, UINT);
            MMRESULT    (WINAPI* waveOutGetErrorTextW)(MMRESULT, LPWSTR, UINT);
            MMRESULT    (WINAPI* waveOutGetID)(HWAVEOUT, LPUINT);
            UINT        (WINAPI* waveOutGetNumDevs)(void);
            MMRESULT    (WINAPI* waveOutGetPitch)(HWAVEOUT, LPDWORD);
            MMRESULT    (WINAPI* waveOutGetPlaybackRate)(HWAVEOUT, LPDWORD);
            MMRESULT    (WINAPI* waveOutGetPosition)(HWAVEOUT, LPMMTIME, UINT);
            MMRESULT    (WINAPI* waveOutGetVolume)(HWAVEOUT, LPDWORD);
            DWORD       (WINAPI* waveOutMessage)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR);
            MMRESULT    (WINAPI* waveOutOpen)(LPHWAVEOUT, UINT_PTR, LPWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
            MMRESULT    (WINAPI* waveOutPause)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutPrepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveOutReset)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutRestart)(HWAVEOUT);
            MMRESULT    (WINAPI* waveOutSetPitch)(HWAVEOUT, DWORD);
            MMRESULT    (WINAPI* waveOutSetPlaybackRate)(HWAVEOUT, DWORD);
            MMRESULT    (WINAPI* waveOutSetVolume)(HWAVEOUT, DWORD);
            MMRESULT    (WINAPI* waveOutUnprepareHeader)(HWAVEOUT, LPWAVEHDR, UINT);
            MMRESULT    (WINAPI* waveOutWrite)(HWAVEOUT, LPWAVEHDR, UINT);
        };
        
        
    private:
        // -------- CLASS VARIABLES ---------------------------------------- //

        // Holds the imported DirectInput API function addresses.
        static SImportTable importTable;

        // Specifies whether or not the import table has been initialized.
        static BOOL importTableIsInitialized;


        // -------- CONSTRUCTION AND DESTRUCTION --------------------------- //

        // Default constructor. Should never be invoked.
        ImportApiWinMM();


    public:
        // -------- CLASS METHODS ------------------------------------------ //

        // Dynamically loads the WinMM library and sets up all imported function calls.
        static void Initialize(void);
        
        
        // -------- CLASS METHODS: IMPORTED FUNCTIONS ---------------------- //
        // See WinMM documentation for more information.
        
        static LRESULT      CloseDriver(HDRVR hdrvr, LPARAM lParam1, LPARAM lParam2);
        static LRESULT      DefDriverProc(DWORD_PTR dwDriverId, HDRVR hdrvr, UINT msg, LONG lParam1, LONG lParam2);
        static BOOL         DriverCallback(DWORD dwCallBack, DWORD dwFlags, HDRVR hdrvr, DWORD msg, DWORD dwUser, DWORD dwParam1, DWORD dwParam2);
        static HMODULE      DrvGetModuleHandle(HDRVR hDriver);
        static HMODULE      GetDriverModuleHandle(HDRVR hdrvr);
        static HDRVR        OpenDriver(LPCWSTR lpDriverName, LPCWSTR lpSectionName, LPARAM lParam);
        static BOOL         PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
        static BOOL         PlaySoundW(LPCWSTR pszSound, HMODULE hmod, DWORD fdwSound);
        static LRESULT      SendDriverMessage(HDRVR hdrvr, UINT msg, LPARAM lParam1, LPARAM lParam2);

        static MMRESULT     auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);
        static MMRESULT     auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);
        static UINT         auxGetNumDevs(void);
        static MMRESULT     auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);
        static MMRESULT     auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        static MMRESULT     auxSetVolume(UINT uDeviceID, DWORD dwVolume);
        
        static MMRESULT     joyConfigChanged(DWORD dwFlags);
        static MMRESULT     joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
        static MMRESULT     joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
        static UINT         joyGetNumDevs(void);
        static MMRESULT     joyGetPos(UINT uJoyID, LPJOYINFO pji);
        static MMRESULT     joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        static MMRESULT     joyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        static MMRESULT     joyReleaseCapture(UINT uJoyID);
        static MMRESULT     joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        static MMRESULT     joySetThreshold(UINT uJoyID, UINT uThreshold);
        
        static MMRESULT     midiConnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);
        static MMRESULT     midiDisconnect(HMIDI hMidi, HMIDIOUT hmo, LPVOID pReserved);
        
        static MMRESULT     midiInAddBuffer(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
        static MMRESULT     midiInClose(HMIDIIN hMidiIn);
        static MMRESULT     midiInGetDevCapsA(UINT_PTR uDeviceID, LPMIDIINCAPSA lpMidiInCaps, UINT cbMidiInCaps);
        static MMRESULT     midiInGetDevCapsW(UINT_PTR uDeviceID, LPMIDIINCAPSW lpMidiInCaps, UINT cbMidiInCaps);
        static MMRESULT     midiInGetErrorTextA(MMRESULT wError, LPSTR lpText, UINT cchText);
        static MMRESULT     midiInGetErrorTextW(MMRESULT wError, LPWSTR lpText, UINT cchText);
        static MMRESULT     midiInGetID(HMIDIIN hmi, LPUINT puDeviceID);
        static UINT         midiInGetNumDevs(void);
        static DWORD        midiInMessage(HMIDIIN deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
        static MMRESULT     midiInOpen(LPHMIDIIN lphMidiIn, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
        static MMRESULT     midiInPrepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
        static MMRESULT     midiInReset(HMIDIIN hMidiIn);
        static MMRESULT     midiInStart(HMIDIIN hMidiIn);
        static MMRESULT     midiInStop(HMIDIIN hMidiIn);
        static MMRESULT     midiInUnprepareHeader(HMIDIIN hMidiIn, LPMIDIHDR lpMidiInHdr, UINT cbMidiInHdr);
        
        static MMRESULT     midiOutCacheDrumPatches(HMIDIOUT hmo, UINT wPatch, WORD *lpKeyArray, UINT wFlags);
        static MMRESULT     midiOutCachePatches(HMIDIOUT hmo, UINT wBank, WORD *lpPatchArray, UINT wFlags);
        static MMRESULT     midiOutClose(HMIDIOUT hmo);
        static MMRESULT     midiOutGetDevCapsA(UINT_PTR uDeviceID, LPMIDIOUTCAPSA lpMidiOutCaps, UINT cbMidiOutCaps);
        static MMRESULT     midiOutGetDevCapsW(UINT_PTR uDeviceID, LPMIDIOUTCAPSW lpMidiOutCaps, UINT cbMidiOutCaps);
        static UINT         midiOutGetErrorTextA(MMRESULT mmrError, LPSTR lpText, UINT cchText);
        static UINT         midiOutGetErrorTextW(MMRESULT mmrError, LPWSTR lpText, UINT cchText);
        static MMRESULT     midiOutGetID(HMIDIOUT hmo, LPUINT puDeviceID);
        static UINT         midiOutGetNumDevs(void);
        static MMRESULT     midiOutGetVolume(HMIDIOUT hmo, LPDWORD lpdwVolume);
        static MMRESULT     midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
        static DWORD        midiOutMessage(HMIDIOUT deviceID, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
        static MMRESULT     midiOutOpen(LPHMIDIOUT lphmo, UINT uDeviceID, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD dwFlags);
        static MMRESULT     midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
        static MMRESULT     midiOutReset(HMIDIOUT hmo);
        static MMRESULT     midiOutSetVolume(HMIDIOUT hmo, DWORD dwVolume);
        static MMRESULT     midiOutShortMsg(HMIDIOUT hmo, DWORD dwMsg);
        static MMRESULT     midiOutUnprepareHeader(HMIDIOUT hmo, LPMIDIHDR lpMidiOutHdr, UINT cbMidiOutHdr);
        
        static MMRESULT     midiStreamClose(HMIDISTRM hStream);
        static MMRESULT     midiStreamOpen(LPHMIDISTRM lphStream, LPUINT puDeviceID, DWORD cMidi, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
        static MMRESULT     midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr);
        static MMRESULT     midiStreamPause(HMIDISTRM hms);
        static MMRESULT     midiStreamPosition(HMIDISTRM hms, LPMMTIME pmmt, UINT cbmmt);
        static MMRESULT     midiStreamProperty(HMIDISTRM hm, LPBYTE lppropdata, DWORD dwProperty);
        static MMRESULT     midiStreamRestart(HMIDISTRM hms);
        static MMRESULT     midiStreamStop(HMIDISTRM hms);
        
        static MMRESULT     mixerClose(HMIXER hmx);
        static MMRESULT     mixerGetControlDetailsA(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
        static MMRESULT     mixerGetControlDetailsW(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
        static MMRESULT     mixerGetDevCapsA(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
        static MMRESULT     mixerGetDevCapsW(UINT_PTR uMxId, LPMIXERCAPS pmxcaps, UINT cbmxcaps);
        static MMRESULT     mixerGetID(HMIXEROBJ hmxobj, UINT* puMxId, DWORD fdwId);
        static MMRESULT     mixerGetLineControlsA(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
        static MMRESULT     mixerGetLineControlsW(HMIXEROBJ hmxobj, LPMIXERLINECONTROLS pmxlc, DWORD fdwControls);
        static MMRESULT     mixerGetLineInfoA(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
        static MMRESULT     mixerGetLineInfoW(HMIXEROBJ hmxobj, LPMIXERLINE pmxl, DWORD fdwInfo);
        static UINT         mixerGetNumDevs(void);
        static DWORD        mixerMessage(HMIXER driverID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        static MMRESULT     mixerOpen(LPHMIXER phmx, UINT uMxId, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
        static MMRESULT     mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);
        
        static MMRESULT     mmioAdvance(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
        static MMRESULT     mmioAscend(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
        static MMRESULT     mmioClose(HMMIO hmmio, UINT wFlags);
        static MMRESULT     mmioCreateChunk(HMMIO hmmio, LPMMCKINFO lpck, UINT wFlags);
        static MMRESULT     mmioDescend(HMMIO hmmio, LPMMCKINFO lpck, LPCMMCKINFO lpckParent, UINT wFlags);
        static MMRESULT     mmioFlush(HMMIO hmmio, UINT fuFlush);
        static MMRESULT     mmioGetInfo(HMMIO hmmio, LPMMIOINFO lpmmioinfo, UINT wFlags);
        static LPMMIOPROC   mmioInstallIOProcA(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
        static LPMMIOPROC   mmioInstallIOProcW(FOURCC fccIOProc, LPMMIOPROC pIOProc, DWORD dwFlags);
        static HMMIO        mmioOpenA(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
        static HMMIO        mmioOpenW(LPWSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
        static LONG         mmioRead(HMMIO hmmio, HPSTR pch, LONG cch);
        static MMRESULT     mmioRenameA(LPCSTR szFilename, LPCSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
        static MMRESULT     mmioRenameW(LPCWSTR szFilename, LPCWSTR szNewFilename, LPCMMIOINFO lpmmioinfo, DWORD dwRenameFlags);
        static LONG         mmioSeek(HMMIO hmmio, LONG lOffset, int iOrigin);
        static LRESULT      mmioSendMessage(HMMIO hmmio, UINT wMsg, LPARAM lParam1, LPARAM lParam2);
        static MMRESULT     mmioSetBuffer(HMMIO hmmio, LPSTR pchBuffer, LONG cchBuffer, UINT wFlags);
        static MMRESULT     mmioSetInfo(HMMIO hmmio, LPCMMIOINFO lpmmioinfo, UINT wFlags);
        static FOURCC       mmioStringToFOURCCA(LPCSTR sz, UINT wFlags);
        static FOURCC       mmioStringToFOURCCW(LPCWSTR sz, UINT wFlags);
        static LONG         mmioWrite(HMMIO hmmio, const char* pch, LONG cch);
        
        static BOOL         sndPlaySoundA(LPCSTR lpszSound, UINT fuSound);
        static BOOL         sndPlaySoundW(LPCWSTR lpszSound, UINT fuSound);
        
        static MMRESULT     timeBeginPeriod(UINT uPeriod);
        static MMRESULT     timeEndPeriod(UINT uPeriod);
        static MMRESULT     timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);
        static MMRESULT     timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
        static DWORD        timeGetTime(void);
        static MMRESULT     timeKillEvent(UINT uTimerID);
        static MMRESULT     timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);
        
        static MMRESULT     waveInAddBuffer(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
        static MMRESULT     waveInClose(HWAVEIN hwi);
        static MMRESULT     waveInGetDevCapsA(UINT_PTR uDeviceID, LPWAVEINCAPSA pwic, UINT cbwic);
        static MMRESULT     waveInGetDevCapsW(UINT_PTR uDeviceID, LPWAVEINCAPSW pwic, UINT cbwic);
        static MMRESULT     waveInGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
        static MMRESULT     waveInGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
        static MMRESULT     waveInGetID(HWAVEIN hwi, LPUINT puDeviceID);
        static UINT         waveInGetNumDevs(void);
        static MMRESULT     waveInGetPosition(HWAVEIN hwi, LPMMTIME pmmt, UINT cbmmt);
        static DWORD        waveInMessage(HWAVEIN deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        static MMRESULT     waveInOpen(LPHWAVEIN phwi, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
        static MMRESULT     waveInPrepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
        static MMRESULT     waveInReset(HWAVEIN hwi);
        static MMRESULT     waveInStart(HWAVEIN hwi);
        static MMRESULT     waveInStop(HWAVEIN hwi);
        static MMRESULT     waveInUnprepareHeader(HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh);
        
        static MMRESULT     waveOutBreakLoop(HWAVEOUT hwo);
        static MMRESULT     waveOutClose(HWAVEOUT hwo);
        static MMRESULT     waveOutGetDevCapsA(UINT_PTR uDeviceID, LPWAVEOUTCAPSA pwoc, UINT cbwoc);
        static MMRESULT     waveOutGetDevCapsW(UINT_PTR uDeviceID, LPWAVEOUTCAPSW pwoc, UINT cbwoc);
        static MMRESULT     waveOutGetErrorTextA(MMRESULT mmrError, LPCSTR pszText, UINT cchText);
        static MMRESULT     waveOutGetErrorTextW(MMRESULT mmrError, LPWSTR pszText, UINT cchText);
        static MMRESULT     waveOutGetID(HWAVEOUT hwo, LPUINT puDeviceID);
        static UINT         waveOutGetNumDevs(void);
        static MMRESULT     waveOutGetPitch(HWAVEOUT hwo, LPDWORD pdwPitch);
        static MMRESULT     waveOutGetPlaybackRate(HWAVEOUT hwo, LPDWORD pdwRate);
        static MMRESULT     waveOutGetPosition(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
        static MMRESULT     waveOutGetVolume(HWAVEOUT hwo, LPDWORD pdwVolume);
        static DWORD        waveOutMessage(HWAVEOUT deviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
        static MMRESULT     waveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
        static MMRESULT     waveOutPause(HWAVEOUT hwo);
        static MMRESULT     waveOutPrepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
        static MMRESULT     waveOutReset(HWAVEOUT hwo);
        static MMRESULT     waveOutRestart(HWAVEOUT hwo);
        static MMRESULT     waveOutSetPitch(HWAVEOUT hwo, DWORD dwPitch);
        static MMRESULT     waveOutSetPlaybackRate(HWAVEOUT hwo, DWORD dwRate);
        static MMRESULT     waveOutSetVolume(HWAVEOUT hwo, DWORD dwVolume);
        static MMRESULT     waveOutUnprepareHeader(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
        static MMRESULT     waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
        
        
    private:
        // -------- HELPERS ------------------------------------------------ //

        // Logs a warning event related to failure to import a particular function from the import library.
        static void LogImportFailed(LPCTSTR functionName);
        
        // Logs a debug event related to attempting to load the system-provided library for importing functions.
        static void LogInitializeLibraryPath(LPCTSTR libraryPath);
        
        // Logs an error event related to failure to initialize the import table.
        static void LogInitializeFailed(void);

        // Logs an informational event related to successful initialization of the import table.
        static void LogInitializeSucceeded(void);

        // Logs an error event related to a missing import function that has been invoked.
        static void LogMissingFunctionCalled(LPCTSTR functionName);
    };
}
