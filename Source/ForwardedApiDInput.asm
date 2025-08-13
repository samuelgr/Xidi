;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Xidi
;   DirectInput interface for XInput controllers.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Authored by Samuel Grossman
; Copyright (c) 2016-2025
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ForwardedApiDInput.asm
;   Partial implementation of exported function entry points for DInput that should be forwarded
;   perfectly to either the system version of the same or to the main Xidi library.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INCLUDE DllExportForward.inc


_TEXT                                       SEGMENT


DllExportForward Xidi, dinput_DirectInputCreateA
DllExportForward Xidi, dinput_DirectInputCreateW
DllExportForward Xidi, dinput_DirectInputCreateEx
DllExportForward Xidi, dinput_DllRegisterServer
DllExportForward Xidi, dinput_DllUnregisterServer
DllExportForward Xidi, dinput_DllCanUnloadNow
DllExportForward Xidi, dinput_DllGetClassObject


_TEXT                                       ENDS


END
