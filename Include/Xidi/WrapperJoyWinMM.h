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
#include "Mapper.h"
#include "XInputController.h"

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
        // -------- TYPE DEFINITIONS ----------------------------------------------- //

        /// Holds controller state information retrieved from the mapper.
        struct SJoyStateData
        {
            LONG axisX;                                                     ///< X axis value.
            LONG axisY;                                                     ///< Y axis value.
            LONG axisZ;                                                     ///< Z axis value.
            LONG axisRx;                                                    ///< Rot-X axis value.
            LONG axisRy;                                                    ///< Rot-Y axis value.
            LONG axisRz;                                                    ///< Rot-Z axis value.
            LONG pov;                                                       ///< POV (D-pad) value.
            BYTE buttons[32];                                               ///< Values for up to 32 buttons.
        };


        // -------- CLASS VARIABLES ------------------------------------------------ //

        /// Fixed set of four XInput controllers.
        static XInputController* controllers[XInputController::kMaxNumXInputControllers];

        /// Mapping scheme to be applied to all controllers.
        static Mapper::IMapper* mapper;

        /// Specifies if the class is initialized.
        static BOOL isInitialized;

        /// Specifies the format of each field in SJoyStateData in DirectInput-compatible format.
        static DIOBJECTDATAFORMAT joyStateObjectDataFormat[];

        /// Specifies the overall data format of SJoyStateData in DirectInput-compatible format.
        static const DIDATAFORMAT joyStateDataFormat;

        /// Maps from application-specified joystick index to the actual indices to present to WinMM or use internally.
        /// Negative values indicate XInput controllers, others indicate values to be passed to WinMM as is.
        static std::vector<int> joyIndexMap;

        /// Holds information about all devices WinMM makes available.
        /// String specifies the device identifier (vendor ID and product ID string), bool value specifies whether the device supports XInput.
        static std::vector<std::pair<std::wstring, bool>> joySystemDeviceInfo;


        // -------- CONSTRUCTION AND DESTRUCTION ----------------------------------- //

        /// Default constructor. Should never be invoked.
        WrapperJoyWinMM(void);


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

        /// Communicates with the relevant controller and the mapper to fill the provided structure with device state information.
        /// @param [in] joyID WinMM joystick ID.
        /// @param [out] joyStateData Structure to fill with device state information.
        /// @return `JOYERR_NOERROR` on success, `JOYERR_NOCANDO` if the requested operation cannot be performed.
        static MMRESULT FillDeviceState(UINT joyID, SJoyStateData* joyStateData);

        /// Fills in the specified buffer with the name of the registry key to use for referencing controller names.
        /// This is the non-Unicode version.
        /// @param [out] buf Buffer to be filled.
        /// @param [in] bufcount Number of characters that the buffer can hold.
        /// @return Number of characters written, or negative in the event of an error.
        static int FillRegistryKeyStringA(LPSTR buf, const size_t bufcount);

        /// Fills in the specified buffer with the name of the registry key to use for referencing controller names.
        /// This is the Unicode version.
        /// @param [out] buf Buffer to be filled.
        /// @param [in] bufcount Number of characters that the buffer can hold.
        /// @return Number of characters written, or negative in the event of an error.
        static int FillRegistryKeyStringW(LPWSTR buf, const size_t bufcount);

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
        static MMRESULT JoyGetDevCapsA(UINT_PTR uJoyID, LPJOYCAPSA pjc, UINT cbjc);
        static MMRESULT JoyGetDevCapsW(UINT_PTR uJoyID, LPJOYCAPSW pjc, UINT cbjc);
        static UINT JoyGetNumDevs(void);
        static MMRESULT JoyGetPos(UINT uJoyID, LPJOYINFO pji);
        static MMRESULT JoyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
        static MMRESULT JoyGetThreshold(UINT uJoyID, LPUINT puThreshold);
        static MMRESULT JoyReleaseCapture(UINT uJoyID);
        static MMRESULT JoySetCapture(HWND hwnd, UINT uJoyID, UINT uPeriod, BOOL fChanged);
        static MMRESULT JoySetThreshold(UINT uJoyID, UINT uThreshold);
    };
}
