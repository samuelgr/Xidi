/*****************************************************************************
 * Xidi
 *   DirectInput interface for XInput controllers.
 *****************************************************************************
 * Authored by Samuel Grossman
 * Copyright (c) 2016-2020
 *************************************************************************//**
 * @file WrapperJoyWinMM.h
 *   Declaration of the wrapper for all WinMM joystick functions.
 *****************************************************************************/

#pragma once

#include "ApiDirectInput.h"
#include "ApiWindows.h"
#include "VirtualController.h"

#include <string>
#include <utility>
#include <vector>


namespace Xidi
{
    /// Wraps the WinMM joystick interface.
    /// All methods are class methods, because the wrapped interface is not object-oriented.
    class WrapperJoyWinMM
    {
    private:
        // -------- CLASS VARIABLES ------------------------------------------------ //

        /// Fixed set of four virtual controllers.
        static Controller::VirtualController* controllers[4];

        /// Specifies if the class is initialized.
        static BOOL isInitialized;

        /// Maps from application-specified joystick index to the actual indices to present to WinMM or use internally.
        /// Negative values indicate XInput controllers, others indicate values to be passed to WinMM as is.
        static std::vector<int> joyIndexMap;

        /// Holds information about all devices WinMM makes available.
        /// String specifies the device identifier (vendor ID and product ID string), bool value specifies whether the device supports XInput.
        static std::vector<std::pair<std::wstring, bool>> joySystemDeviceInfo;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        WrapperJoyWinMM(void) = delete;


        // -------- CLASS METHODS -------------------------------------------------- //

        /// Initializes this class.
        static void Initialize(void);


        // -------- HELPERS -------------------------------------------------------- //

        /// Creates the joystick index map.
        /// Requires that the system device information data structure already be filled.
        /// If the user's preferred controller is absent or supports XInput, virtual devices are presented first, otherwise they are presented last.
        /// Any controllers that support XInput are removed from the mapping.
        static void CreateJoyIndexMap(void);

        /// Fills in the system device info data structure with information from the registry and from DirectInput.
        static void CreateSystemDeviceInfo(void);

        /// Callback during DirectInput device enumeration.
        /// Used internally to detect which WinMM devices support XInput.
        static BOOL STDMETHODCALLTYPE CreateSystemDeviceInfoEnumCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

        /// Fills in the specified buffer with the name of the registry key to use for referencing controller names.
        /// @tparam StringType Either LPSTR or LPWSTR depending on whether ASCII or Unicode is desired.
        /// @param [out] buf Buffer to be filled.
        /// @param [in] bufcount Number of characters that the buffer can hold.
        /// @return Number of characters written, or negative in the event of an error.
        template <typename StringType> static int FillRegistryKeyString(StringType buf, const size_t bufcount);

        /// Places the required keys and values into the registry so that WinMM-based applications can find the correct controller names.
        /// Consumes the system device information data structure.
        static void SetControllerNameRegistryInfo(void);

        /// Translates an application-supplied joystick index to an internal joystick index using the map.
        /// @param [in] uJoyID WinMM joystick ID supplied by the application.
        /// @return Internal joystick index to either handle or pass to WinMM.
        static int TranslateApplicationJoyIndex(UINT uJoyID);


    public:
        // -------- METHODS: WinMM JOYSTICK ---------------------------------------- //
        static MMRESULT JoyConfigChanged(DWORD dwFlags);
        template <typename JoyCapsType> static MMRESULT JoyGetDevCaps(UINT_PTR uJoyID, JoyCapsType* pjc, UINT cbjc);
        static UINT JoyGetNumDevs(void);
        static MMRESULT JoyGetPos(UINT uJoyID, LPJOYINFO pji);
        static MMRESULT JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        static MMRESULT JoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        static MMRESULT JoyReleaseCapture(UINT uJoyID);
        static MMRESULT JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        static MMRESULT JoySetThreshold(UINT uJoyID, UINT uThreshold);
    };
}
