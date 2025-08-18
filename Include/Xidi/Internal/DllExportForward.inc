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


IFDEF _WIN32
DllForwardedFunctionsInitialize TEXTEQU <_DllForwardedFunctionsInitialize>
_ptr_export TEXTEQU <__ptr_export_>
ENDIF


EXTRN DllForwardedFunctionsInitialize:PROC


DllExportForward MACRO libraryName, funcName

    LOCAL $initialized
    EXTRN @catstr(_ptr_export, _, libraryName, _, funcName):PROC

    funcName PROC PUBLIC
        cmp SIZE_T PTR [@catstr(_ptr_export, _, libraryName, _, funcName)], 0
        jne $initialized
        call DllForwardedFunctionsInitialize
    $initialized:
        jmp SIZE_T PTR [@catstr(_ptr_export, _, libraryName, _, funcName)]
    funcName ENDP
        
ENDM


ENDIF ; __XIDI_DLLEXPORTFORWARD_INC
