/*****************************************************************************
 * Xidi
 *      DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016
 *****************************************************************************
 * ImportApiWinMM.h
 *      Declarations related to importing the API from the WinMM library.
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
        // -------- TYPE DEFINITIONS ----------------------------------------------- //

        // Fields specify the addresses of the imported DirectInput API functions.
        struct SImportTable
        {
            MMRESULT (WINAPI* auxGetDevCapsA)(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);
            MMRESULT (WINAPI* auxGetDevCapsW)(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);
            UINT     (WINAPI* auxGetNumDevs)(void);
            MMRESULT (WINAPI* auxGetVolume)(UINT uDeviceID, LPDWORD lpdwVolume);
            MMRESULT (WINAPI* auxOutMessage)(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
            MMRESULT (WINAPI* auxSetVolume)(UINT uDeviceID, DWORD dwVolume);

            MMRESULT (WINAPI* joyConfigChanged)(DWORD dwFlags);
            MMRESULT (WINAPI* joyGetDevCapsA)(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
            MMRESULT (WINAPI* joyGetDevCapsW)(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
            UINT     (WINAPI* joyGetNumDevs)(void);
            MMRESULT (WINAPI* joyGetPos)(UINT uJoyID, LPJOYINFO pji);
            MMRESULT (WINAPI* joyGetPosEx)(UINT uJoyID, LPJOYINFOEX pji);
            MMRESULT (WINAPI* joyGetThreshold)(UINT uJoyID, LPUINT puThreshold);
            MMRESULT (WINAPI* joyReleaseCapture)(UINT uJoyID);
            MMRESULT (WINAPI* joySetCapture)(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
            MMRESULT (WINAPI* joySetThreshold)(UINT uJoyID, UINT uThreshold);
            
            MMRESULT (WINAPI* timeBeginPeriod)(UINT uPeriod);
            MMRESULT (WINAPI* timeEndPeriod)(UINT uPeriod);
            MMRESULT (WINAPI* timeGetDevCaps)(LPTIMECAPS ptc, UINT cbtc);
            MMRESULT (WINAPI* timeGetSystemTime)(LPMMTIME pmmt, UINT cbmmt);
            DWORD    (WINAPI* timeGetTime)(void);
            MMRESULT (WINAPI* timeKillEvent)(UINT uTimerID);
            MMRESULT (WINAPI* timeSetEvent)(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);
        };


        // -------- CONSTANTS ------------------------------------------------------ //

        // Holds the name of the library to load from the system directory.
        static const TCHAR* const kWinMMLibraryName;

        // Holds the length, in characters, of the name of the library.
        static const DWORD kWinMMLibraryLength;


    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        // Holds the imported DirectInput API function addresses.
        static SImportTable importTable;

        // Specifies whether or not the import table has been initialized.
        static BOOL importTableIsInitialized;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        // Default constructor. Should never be invoked.
        ImportApiWinMM();


    public:
        // -------- CLASS METHODS -------------------------------------------------- //

        // Dynamically loads the WinMM library and sets up all imported function calls.
        // Returns MMSYSERR_NOERROR on success and MMSYSERR_ERROR on failure.
        static MMRESULT Initialize(void);

        // Calls the imported function auxDevGetCapsA.
        static MMRESULT auxGetDevCapsA(UINT_PTR uDeviceID, LPAUXCAPSA lpCaps, UINT cbCaps);

        // Calls the imported function auxDevGetCapsW.
        static MMRESULT auxGetDevCapsW(UINT_PTR uDeviceID, LPAUXCAPSW lpCaps, UINT cbCaps);

        // Calls the imported function auxGetNumDevs.
        static UINT auxGetNumDevs(void);

        // Calls the imported function auxGetVolume.
        static MMRESULT auxGetVolume(UINT uDeviceID, LPDWORD lpdwVolume);

        // Calls the imported function auxOutMessage.
        static MMRESULT auxOutMessage(UINT uDeviceID, UINT uMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        // Calls the imported function auxSetVolume.
        static MMRESULT auxSetVolume(UINT uDeviceID, DWORD dwVolume);
        
        // Calls the imported function joyConfigChanged.
        static MMRESULT joyConfigChanged(DWORD dwFlags);

        // Calls the imported function joyGetDevCapsA.
        static MMRESULT joyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);

        // Calls the imported function joyGetDevCapsW.
        static MMRESULT joyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);

        // Calls the imported function joyGetNumDevs.
        static UINT joyGetNumDevs(void);

        // Calls the imported function joyGetPos.
        static MMRESULT joyGetPos(UINT uJoyID, LPJOYINFO pji);

        // Calls the imported function joyGetPosEx.
        static MMRESULT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);

        // Calls the imported function joyGetThreshold.
        static MMRESULT joyGetThreshold(UINT uJoyID, LPUINT puThreshold);

        // Calls the imported function joyReleaseCapture.
        static MMRESULT joyReleaseCapture(UINT uJoyID);

        // Calls the imported function joySetCapture.
        static MMRESULT joySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);

        // Calls the imported function joySetThreshold.
        static MMRESULT joySetThreshold(UINT uJoyID, UINT uThreshold);
        
        // Calls the imported function timeBeginPeriod.
        static MMRESULT timeBeginPeriod(UINT uPeriod);

        // Calls the imported function timeEndPeriod.
        static MMRESULT timeEndPeriod(UINT uPeriod);

        // Calls the imported function timeGetDevCaps.
        static MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc);

        // Calls the imported function timeGetSystemTime.
        static MMRESULT timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);

        // Calls the imported function timeGetTime.
        static DWORD timeGetTime(void);

        // Calls the imported function timeKillEvent.
        static MMRESULT timeKillEvent(UINT uTimerID);

        // Calls the imported function timeSetEvent.
        static MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser, UINT fuEvent);
    };
}
