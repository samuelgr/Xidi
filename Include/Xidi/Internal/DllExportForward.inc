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
ENDIF


EXTRN DllForwardedFunctionsInitialize:PROC


DllExportForward MACRO libraryName, funcName

    LOCAL $ptr_export, $initialized

IFDEF _WIN32
    $ptr_export TEXTEQU @catstr(__ptr_export, _, libraryName, _, funcName)
ELSE
    $ptr_export TEXTEQU @catstr(_ptr_export, _, libraryName, _, funcName)
ENDIF

    EXTRN $ptr_export:PROC

    funcName PROC PUBLIC
        cmp SIZE_T PTR [$ptr_export], 0
        jne $initialized

        push sbp
IFDEF _WIN64
        ; 64-bit function arguments are passed in these registers.
        ; The initialization function might overwrite them because it intercepts what is otherwise expected to be a function call.
        push rcx
        push rdx
        push r8
        push r9
ENDIF

        call DllForwardedFunctionsInitialize

IFDEF _WIN64
        pop r9
        pop r8
        pop rdx
        pop rcx
ENDIF
        pop sbp

    $initialized:
        jmp SIZE_T PTR [$ptr_export]
    funcName ENDP
        
ENDM


ENDIF ; __XIDI_DLLEXPORTFORWARD_INC
