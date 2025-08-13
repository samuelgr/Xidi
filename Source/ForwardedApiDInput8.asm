;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Xidi
;   DirectInput interface for XInput controllers.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Authored by Samuel Grossman
; Copyright (c) 2016-2025
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ForwardedApiDInput8.asm
;   Partial implementation of exported function entry points for DInput8 that should be forwarded
;   perfectly to either the system version of the same or to the main Xidi library.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

INCLUDE DllExportForward.inc


_TEXT                                       SEGMENT


DllExportForward Xidi, dinput8_DirectInput8Create
DllExportForward Xidi, dinput8_DllRegisterServer
DllExportForward Xidi, dinput8_DllUnregisterServer
DllExportForward Xidi, dinput8_DllCanUnloadNow
DllExportForward Xidi, dinput8_DllGetClassObject


_TEXT                                       ENDS


END
