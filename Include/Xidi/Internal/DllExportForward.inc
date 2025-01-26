;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Xidi
;   DirectInput interface for XInput controllers.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Authored by Samuel Grossman
; Copyright (c) 2016-2025
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DllExportForward.inc
;   Declaration of assembly macros for creating exported entry points in a DLL that are perfectly
;   forwarded to another one.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IFNDEF __XIDI_DLLEXPORTFORWARD_INC
__XIDI_DLLEXPORTFORWARD_INC EQU 1


INCLUDE Infra/Core/Preamble.inc
INCLUDE Infra/Core/Registers.inc


EXTRN _DllForwardedFunctionsInitialize:PROC


DllExportForward MACRO libraryName, funcName

    LOCAL $initialized
    EXTRN @catstr(__ptr_export_, libraryName, _, funcName):PROC

    funcName PROC PUBLIC
        cmp SIZE_T PTR [@catstr(__ptr_export_, libraryName, _, funcName)], 0
        jne $initialized
        call _DllForwardedFunctionsInitialize
    $initialized:
        jmp SIZE_T PTR [@catstr(__ptr_export_, libraryName, _, funcName)]
    funcName ENDP
        
ENDM


ENDIF ; __XIDI_DLLEXPORTFORWARD_INC
