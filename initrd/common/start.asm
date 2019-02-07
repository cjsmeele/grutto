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

global start_
global threadpoline_

extern main
extern CTORS_START
extern CTORS_END

start_:
    ;xchg bx, bx ; magic break
    call call_constructors
    call main
    ;xchg bx, bx ; magic break
.hang:
    ;; FIXME: Provide syscall definitions for asm.
    mov eax, 7 ;; SYS_PROCESS_EXIT
    int 0xca
    jmp .hang

call_constructors:
    mov ebx, dword CTORS_START

.loop:
    mov eax, dword CTORS_END
    cmp ebx, eax
    jge .end
    call [ebx]
    add ebx, 4
    jmp .loop
.end:
    ret

;; Entrypoint for all new threads.
threadpoline_:
    ;; Context arg is in ebx, actual entrypoint is in eax.
    push ebx
    call eax
    push eax
.hang:
    pop  ebx
    push ebx
    mov eax, 6 ;; SYS_THREAD_REJOIN
    int 0xca
    jmp .hang
