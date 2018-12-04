;; Copyright (c) 2018, Chris Smeele
;;
;; This file is part of Grutto.
;;
;; Grutto is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; Grutto is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with Grutto.  If not, see <https://www.gnu.org/licenses/>.

[bits 32]

section .text

extern common_exception_handler
extern common_interrupt_handler

%macro DEF_EXCEPTION_HANDLER 1
global interrupt_handler_%1
interrupt_handler_%1:
    cli ;; should not be necessary unless invoked manually.
    push dword 0
    push dword %1
    jmp exception_handler_shell
%endmacro

%macro DEF_EXCEPTION_HANDLER_ERRORCODE 1
global interrupt_handler_%1
interrupt_handler_%1:
    cli ;; should not be necessary unless invoked manually.
    push dword %1
    jmp exception_handler_shell
%endmacro

%macro DEF_INTERRUPT_HANDLER 1
global interrupt_handler_%1
interrupt_handler_%1:
    cli
    push dword 0
    push dword %1
    jmp interrupt_handler_shell
%endmacro


exception_handler_shell:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push eax

    mov eax, common_exception_handler
    call eax

    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8
    iret

interrupt_handler_shell:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax

    mov eax, common_interrupt_handler
    call eax

    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8
    iret

; Exception handlers {{{
;                                      Exception Type | Error Code             | Code | Description
;                                     ----------------+------------------------+------+-------------------------------
DEF_EXCEPTION_HANDLER           0x00 ; Fault          |                        | #DE  | Divide Error Exception
DEF_EXCEPTION_HANDLER           0x01 ; Trap / Fault   |                        | #DB  | Debug Exception
DEF_EXCEPTION_HANDLER           0x02 ;                |                        |      | Non-Maskable Interrupt
DEF_EXCEPTION_HANDLER           0x03 ; Trap           |                        | #BP  | Breakpoint Exception
DEF_EXCEPTION_HANDLER           0x04 ; Trap           |                        | #OF  | Overflow Exception
DEF_EXCEPTION_HANDLER           0x05 ; Fault          |                        | #BR  | BOUND Range Exceeded Exception
DEF_EXCEPTION_HANDLER           0x06 ; Fault          |                        | #UD  | Invalid Opcode Exception
DEF_EXCEPTION_HANDLER           0x07 ; Fault          |                        | #NM  | Device Not Available Exception
DEF_EXCEPTION_HANDLER_ERRORCODE 0x08 ; Abort          | Always Zero            | #DF  | Double Fault Exception
DEF_EXCEPTION_HANDLER           0x09 ; Abort          |                        |      | Coprocessor Segment Overrun
DEF_EXCEPTION_HANDLER_ERRORCODE 0x0a ; Fault          | Segment Selector Index | #TS  | Invalid TSS Exception
DEF_EXCEPTION_HANDLER_ERRORCODE 0x0b ; Fault          | Segment Selector Index | #NP  | Segment Not Present
DEF_EXCEPTION_HANDLER_ERRORCODE 0x0c ; Fault          | Segment Selector / 0   | #SS  | Stack Fault Exception
DEF_EXCEPTION_HANDLER_ERRORCODE 0x0d ; Fault          | Segment Selector / 0   | #GP  | General Protection Exception
DEF_EXCEPTION_HANDLER_ERRORCODE 0x0e ; Fault          | Page-Fault Error Code  | #PF  | Page-Fault Exception
DEF_EXCEPTION_HANDLER           0x0f ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x10 ; Fault          |                        | #MF  | FPU Floating-Point Error
DEF_EXCEPTION_HANDLER_ERRORCODE 0x11 ; Fault          | Always Zero            | #AC  | Alignment Check Exception
DEF_EXCEPTION_HANDLER           0x12 ; Abort          |                        | #MC  | Machine-Check Exception
DEF_EXCEPTION_HANDLER           0x13 ; Fault          |                        | #XM  | SIMD Floating-Point Exception
DEF_EXCEPTION_HANDLER           0x14 ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x15 ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x16 ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x17 ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x18 ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x19 ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x1a ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x1b ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x1c ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x1d ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x1e ;                |                        |      | Reserved
DEF_EXCEPTION_HANDLER           0x1f ;                |                        |      | Reserved
; }}}
; Interrupt handlers {{{
DEF_INTERRUPT_HANDLER           0x20
DEF_INTERRUPT_HANDLER           0x21
DEF_INTERRUPT_HANDLER           0x22
DEF_INTERRUPT_HANDLER           0x23
DEF_INTERRUPT_HANDLER           0x24
DEF_INTERRUPT_HANDLER           0x25
DEF_INTERRUPT_HANDLER           0x26
DEF_INTERRUPT_HANDLER           0x27

DEF_INTERRUPT_HANDLER           0x28
DEF_INTERRUPT_HANDLER           0x29
DEF_INTERRUPT_HANDLER           0x2a
DEF_INTERRUPT_HANDLER           0x2b
DEF_INTERRUPT_HANDLER           0x2c
DEF_INTERRUPT_HANDLER           0x2d
DEF_INTERRUPT_HANDLER           0x2e
DEF_INTERRUPT_HANDLER           0x2f

DEF_INTERRUPT_HANDLER           0x30
DEF_INTERRUPT_HANDLER           0x31
DEF_INTERRUPT_HANDLER           0x32
DEF_INTERRUPT_HANDLER           0x33
DEF_INTERRUPT_HANDLER           0x34
DEF_INTERRUPT_HANDLER           0x35
DEF_INTERRUPT_HANDLER           0x36
DEF_INTERRUPT_HANDLER           0x37
DEF_INTERRUPT_HANDLER           0x38
DEF_INTERRUPT_HANDLER           0x39
DEF_INTERRUPT_HANDLER           0x3a
DEF_INTERRUPT_HANDLER           0x3b
DEF_INTERRUPT_HANDLER           0x3c
DEF_INTERRUPT_HANDLER           0x3d
DEF_INTERRUPT_HANDLER           0x3e
DEF_INTERRUPT_HANDLER           0x3f
DEF_INTERRUPT_HANDLER           0x40
DEF_INTERRUPT_HANDLER           0x41
DEF_INTERRUPT_HANDLER           0x42
DEF_INTERRUPT_HANDLER           0x43
DEF_INTERRUPT_HANDLER           0x44
DEF_INTERRUPT_HANDLER           0x45
DEF_INTERRUPT_HANDLER           0x46
DEF_INTERRUPT_HANDLER           0x47
DEF_INTERRUPT_HANDLER           0x48
DEF_INTERRUPT_HANDLER           0x49
DEF_INTERRUPT_HANDLER           0x4a
DEF_INTERRUPT_HANDLER           0x4b
DEF_INTERRUPT_HANDLER           0x4c
DEF_INTERRUPT_HANDLER           0x4d
DEF_INTERRUPT_HANDLER           0x4e
DEF_INTERRUPT_HANDLER           0x4f
DEF_INTERRUPT_HANDLER           0x50
DEF_INTERRUPT_HANDLER           0x51
DEF_INTERRUPT_HANDLER           0x52
DEF_INTERRUPT_HANDLER           0x53
DEF_INTERRUPT_HANDLER           0x54
DEF_INTERRUPT_HANDLER           0x55
DEF_INTERRUPT_HANDLER           0x56
DEF_INTERRUPT_HANDLER           0x57
DEF_INTERRUPT_HANDLER           0x58
DEF_INTERRUPT_HANDLER           0x59
DEF_INTERRUPT_HANDLER           0x5a
DEF_INTERRUPT_HANDLER           0x5b
DEF_INTERRUPT_HANDLER           0x5c
DEF_INTERRUPT_HANDLER           0x5d
DEF_INTERRUPT_HANDLER           0x5e
DEF_INTERRUPT_HANDLER           0x5f
DEF_INTERRUPT_HANDLER           0x60
DEF_INTERRUPT_HANDLER           0x61
DEF_INTERRUPT_HANDLER           0x62
DEF_INTERRUPT_HANDLER           0x63
DEF_INTERRUPT_HANDLER           0x64
DEF_INTERRUPT_HANDLER           0x65
DEF_INTERRUPT_HANDLER           0x66
DEF_INTERRUPT_HANDLER           0x67
DEF_INTERRUPT_HANDLER           0x68
DEF_INTERRUPT_HANDLER           0x69
DEF_INTERRUPT_HANDLER           0x6a
DEF_INTERRUPT_HANDLER           0x6b
DEF_INTERRUPT_HANDLER           0x6c
DEF_INTERRUPT_HANDLER           0x6d
DEF_INTERRUPT_HANDLER           0x6e
DEF_INTERRUPT_HANDLER           0x6f
DEF_INTERRUPT_HANDLER           0x70
DEF_INTERRUPT_HANDLER           0x71
DEF_INTERRUPT_HANDLER           0x72
DEF_INTERRUPT_HANDLER           0x73
DEF_INTERRUPT_HANDLER           0x74
DEF_INTERRUPT_HANDLER           0x75
DEF_INTERRUPT_HANDLER           0x76
DEF_INTERRUPT_HANDLER           0x77
DEF_INTERRUPT_HANDLER           0x78
DEF_INTERRUPT_HANDLER           0x79
DEF_INTERRUPT_HANDLER           0x7a
DEF_INTERRUPT_HANDLER           0x7b
DEF_INTERRUPT_HANDLER           0x7c
DEF_INTERRUPT_HANDLER           0x7d
DEF_INTERRUPT_HANDLER           0x7e
DEF_INTERRUPT_HANDLER           0x7f
DEF_INTERRUPT_HANDLER           0x80
DEF_INTERRUPT_HANDLER           0x81
DEF_INTERRUPT_HANDLER           0x82
DEF_INTERRUPT_HANDLER           0x83
DEF_INTERRUPT_HANDLER           0x84
DEF_INTERRUPT_HANDLER           0x85
DEF_INTERRUPT_HANDLER           0x86
DEF_INTERRUPT_HANDLER           0x87
DEF_INTERRUPT_HANDLER           0x88
DEF_INTERRUPT_HANDLER           0x89
DEF_INTERRUPT_HANDLER           0x8a
DEF_INTERRUPT_HANDLER           0x8b
DEF_INTERRUPT_HANDLER           0x8c
DEF_INTERRUPT_HANDLER           0x8d
DEF_INTERRUPT_HANDLER           0x8e
DEF_INTERRUPT_HANDLER           0x8f
DEF_INTERRUPT_HANDLER           0x90
DEF_INTERRUPT_HANDLER           0x91
DEF_INTERRUPT_HANDLER           0x92
DEF_INTERRUPT_HANDLER           0x93
DEF_INTERRUPT_HANDLER           0x94
DEF_INTERRUPT_HANDLER           0x95
DEF_INTERRUPT_HANDLER           0x96
DEF_INTERRUPT_HANDLER           0x97
DEF_INTERRUPT_HANDLER           0x98
DEF_INTERRUPT_HANDLER           0x99
DEF_INTERRUPT_HANDLER           0x9a
DEF_INTERRUPT_HANDLER           0x9b
DEF_INTERRUPT_HANDLER           0x9c
DEF_INTERRUPT_HANDLER           0x9d
DEF_INTERRUPT_HANDLER           0x9e
DEF_INTERRUPT_HANDLER           0x9f
DEF_INTERRUPT_HANDLER           0xa0
DEF_INTERRUPT_HANDLER           0xa1
DEF_INTERRUPT_HANDLER           0xa2
DEF_INTERRUPT_HANDLER           0xa3
DEF_INTERRUPT_HANDLER           0xa4
DEF_INTERRUPT_HANDLER           0xa5
DEF_INTERRUPT_HANDLER           0xa6
DEF_INTERRUPT_HANDLER           0xa7
DEF_INTERRUPT_HANDLER           0xa8
DEF_INTERRUPT_HANDLER           0xa9
DEF_INTERRUPT_HANDLER           0xaa
DEF_INTERRUPT_HANDLER           0xab
DEF_INTERRUPT_HANDLER           0xac
DEF_INTERRUPT_HANDLER           0xad
DEF_INTERRUPT_HANDLER           0xae
DEF_INTERRUPT_HANDLER           0xaf
DEF_INTERRUPT_HANDLER           0xb0
DEF_INTERRUPT_HANDLER           0xb1
DEF_INTERRUPT_HANDLER           0xb2
DEF_INTERRUPT_HANDLER           0xb3
DEF_INTERRUPT_HANDLER           0xb4
DEF_INTERRUPT_HANDLER           0xb5
DEF_INTERRUPT_HANDLER           0xb6
DEF_INTERRUPT_HANDLER           0xb7
DEF_INTERRUPT_HANDLER           0xb8
DEF_INTERRUPT_HANDLER           0xb9
DEF_INTERRUPT_HANDLER           0xba
DEF_INTERRUPT_HANDLER           0xbb
DEF_INTERRUPT_HANDLER           0xbc
DEF_INTERRUPT_HANDLER           0xbd
DEF_INTERRUPT_HANDLER           0xbe
DEF_INTERRUPT_HANDLER           0xbf
DEF_INTERRUPT_HANDLER           0xc0
DEF_INTERRUPT_HANDLER           0xc1
DEF_INTERRUPT_HANDLER           0xc2
DEF_INTERRUPT_HANDLER           0xc3
DEF_INTERRUPT_HANDLER           0xc4
DEF_INTERRUPT_HANDLER           0xc5
DEF_INTERRUPT_HANDLER           0xc6
DEF_INTERRUPT_HANDLER           0xc7
DEF_INTERRUPT_HANDLER           0xc8
DEF_INTERRUPT_HANDLER           0xc9
DEF_INTERRUPT_HANDLER           0xca
DEF_INTERRUPT_HANDLER           0xcb
DEF_INTERRUPT_HANDLER           0xcc
DEF_INTERRUPT_HANDLER           0xcd
DEF_INTERRUPT_HANDLER           0xce
DEF_INTERRUPT_HANDLER           0xcf
DEF_INTERRUPT_HANDLER           0xd0
DEF_INTERRUPT_HANDLER           0xd1
DEF_INTERRUPT_HANDLER           0xd2
DEF_INTERRUPT_HANDLER           0xd3
DEF_INTERRUPT_HANDLER           0xd4
DEF_INTERRUPT_HANDLER           0xd5
DEF_INTERRUPT_HANDLER           0xd6
DEF_INTERRUPT_HANDLER           0xd7
DEF_INTERRUPT_HANDLER           0xd8
DEF_INTERRUPT_HANDLER           0xd9
DEF_INTERRUPT_HANDLER           0xda
DEF_INTERRUPT_HANDLER           0xdb
DEF_INTERRUPT_HANDLER           0xdc
DEF_INTERRUPT_HANDLER           0xdd
DEF_INTERRUPT_HANDLER           0xde
DEF_INTERRUPT_HANDLER           0xdf
DEF_INTERRUPT_HANDLER           0xe0
DEF_INTERRUPT_HANDLER           0xe1
DEF_INTERRUPT_HANDLER           0xe2
DEF_INTERRUPT_HANDLER           0xe3
DEF_INTERRUPT_HANDLER           0xe4
DEF_INTERRUPT_HANDLER           0xe5
DEF_INTERRUPT_HANDLER           0xe6
DEF_INTERRUPT_HANDLER           0xe7
DEF_INTERRUPT_HANDLER           0xe8
DEF_INTERRUPT_HANDLER           0xe9
DEF_INTERRUPT_HANDLER           0xea
DEF_INTERRUPT_HANDLER           0xeb
DEF_INTERRUPT_HANDLER           0xec
DEF_INTERRUPT_HANDLER           0xed
DEF_INTERRUPT_HANDLER           0xee
DEF_INTERRUPT_HANDLER           0xef
DEF_INTERRUPT_HANDLER           0xf0
DEF_INTERRUPT_HANDLER           0xf1
DEF_INTERRUPT_HANDLER           0xf2
DEF_INTERRUPT_HANDLER           0xf3
DEF_INTERRUPT_HANDLER           0xf4
DEF_INTERRUPT_HANDLER           0xf5
DEF_INTERRUPT_HANDLER           0xf6
DEF_INTERRUPT_HANDLER           0xf7
DEF_INTERRUPT_HANDLER           0xf8
DEF_INTERRUPT_HANDLER           0xf9
DEF_INTERRUPT_HANDLER           0xfa
DEF_INTERRUPT_HANDLER           0xfb
DEF_INTERRUPT_HANDLER           0xfc
DEF_INTERRUPT_HANDLER           0xfd
DEF_INTERRUPT_HANDLER           0xfe
DEF_INTERRUPT_HANDLER           0xff
; }}}
