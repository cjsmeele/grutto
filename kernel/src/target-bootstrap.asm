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

extern bootstrap_multiboot_info_addr
extern KERNEL_LMA
extern CTORS_START
extern CTORS_END
extern cstart

global bootstrap_target_boot
global kernel_stack

[bits 32]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  KERNEL STACK
;;  ------------
;;
;;  This is the stack that will be used by the main kernel task.

section .bss

KERNEL_STACK_SIZE equ 16 * 1024

;; Align stack on page boundary.
align 0x1000
kernel_stack:
    resb KERNEL_STACK_SIZE

align 0x1000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  GLOBAL DESCRIPTOR TABLE
;;  -----------------------
;;
;;  We don't plan to make much use of the features of the GDT and LDT,
;;  so we can keep the contents to a minimum.
;;  Our segment setup shouldn't be all that different from the one the
;;  bootloader leaves us with.
;;
;;  We need one code and one data segment descriptor that cover the
;;  entire 32-bit address space.

section .data

global gdt_
gdt_:
.null:
    ;; Null descriptor.
    dq 0
.kcode:
    ;; 32-bit kernel code segment.
    dw 0xffff                  ; Limit 0-15.
    dw 0x0000                  ; Base address  0-15.
    db 0x00                    ; Base address 16-23.
    db 10011010b               ; Present, dpl0, code (rx).
    db 11011111b               ; Granularity, 32-bit op, available, limit 16-19.
    db 0x00                    ; Base address 24-31.
.kdata:
    ;; Kernel data segment.
    dw 0xffff                  ; Limit 0-15.
    dw 0x0000                  ; Base address  0-15.
    db 0x00                    ; Base address 16-23.
    db 10010010b               ; Present, dpl0, data (rw).
    db 11011111b               ; Granularity, 32-bit op, available, limit 16-19.
    db 0x00                    ; Base address 24-31.
.ucode:
    ;; 32-bit user code segment.
    dw 0xffff                  ; Limit 0-15.
    dw 0x0000                  ; Base address  0-15.
    db 0x00                    ; Base address 16-23.
    db 11111010b               ; Present, dpl3, code (rx).
    db 11011111b               ; Granularity, 32-bit op, available, limit 16-19.
    db 0x00                    ; Base address 24-31.
.udata:
    ;; User data segment.
    dw 0xffff                  ; Limit 0-15.
    dw 0x0000                  ; Base address  0-15.
    db 0x00                    ; Base address 16-23.
    db 11110010b               ; Present, dpl3, data (rw).
    db 11011111b               ; Granularity, 32-bit op, available, limit 16-19.
    db 0x00                    ; Base address 24-31.
.tss:
    ;; Task state segment.
    dw 0x0067                  ; Limit 0-15.
    dw 0x0000                  ; Base address  0-15 (to be patched by setup_gdt).
    db 0x00                    ; Base address 16-23.
    db 10001001b               ; Present, dpl0, tss.
    db 01010000b               ; Granularity, 32-bit op, available, limit 16-19.
    db 0x00                    ; Base address 24-31.
    ;; GDT pointer.
.ptr:
    dw 6 * 8                   ; GDT size.
    dd gdt_

global tss_
tss_:
    times 26 dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  PAGING STRUCTURES
;;  -----------------
;;
;;  Initial paging structures take up 16K.

section .bss

align 0x1000

global page_directory
page_directory:
    ;; The PD covers the entire 4G address space.
    ;; PDEs cover 4M each.
    ;; dd ((0x000000 << 12) | 000000000011b)
    times 1024 dd 0

page_table_0:                   ; 0x0000.0000 - 0x0040.0000
    ;; PTEs cover 4K each.
    times 1024 dd 0

page_table_768:                 ; 0xc000.0000 - 0xc040.0000
    times 1024 dd 0

page_table_1012:                ; 0xfd00.0000
    times 1024 dd 0
page_table_1013:                ; 0xfd40.0000
    times 1024 dd 0

page_table_1023:                ; 0xffc0.0000 - 0x0000.0000
    times 1024 dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  TARGET-SPECIFIC BOOT CODE
;;  -------------------------
;;
;;  This contains the startup routine for i686.

section .bootstrap32

bootstrap_target_boot:
    call setup_gdt
    call setup_paging

    ;; Set up stack at the virtual address we mapped for it just now.
    ;; We leave a single unmapped page below it, which should help catch stack overflows.
    mov esp, 0xffff1000 + KERNEL_STACK_SIZE
    ;; ffff5000

    call call_constructors

    mov eax, [bootstrap_multiboot_info_addr]
    push eax
    call cstart

.hang:
    cli
    hlt
    jmp .hang

setup_gdt:
    ;; Patch GDT TSS base address.
    lea eax, [tss_]
    mov word [gdt_.tss + 2], ax
    shr eax, 16
    mov byte [gdt_.tss + 4], al
    mov byte [gdt_.tss + 7], ah

    lgdt [gdt_.ptr]
    mov ax, 2*8
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 1*8:.new_cs
.new_cs:
    mov ax, 5*8 + 3
    ltr ax ; Load task register.

    ret

setup_paging:
    ;;  Insert initial page tables into page directory.
    mov eax, page_table_0
    mov ebx, 0
    call set_pde
    mov eax, page_table_768
    mov ebx, 768
    call set_pde
    mov eax, page_table_1023
    mov ebx, 1023
    call set_pde

    ;; Insert 4M of identity mapped PTEs (1024) into PT 0.
    ;; This identity maps both the first MB of memory and the kernel
    ;; (which is loaded at LMA >1M).
    mov edi, page_table_0
    mov ebx, 0
    mov eax, 0
.loop0:
    call set_pte
    inc ebx
    add eax, 0x1000
    cmp ebx, 1024
    jl .loop0

    ;; Insert 4M of PTEs (1024) into PT 768.
    ;; This maps the kernel from VMA 0xc000.0000 to LMA KERNEL_LMA.
    mov edi, page_table_768
    mov ebx, 0
    ;; mov eax, 0x00100000
    mov eax, KERNEL_LMA         ; A little higher than 1M.
.loop1:
    call set_pte
    inc ebx
    add eax, 0x1000
    cmp ebx, 1024
    jl .loop1

    ;; Insert 16K of PTEs (4) into PT 1023.
    ;; This maps the kernel stack from VMA 0xffff.0000 to LMA kernel_stack.
    mov edi, page_table_1023
    mov ebx, 1009               ; -> 0xffff.1000
    mov eax, kernel_stack
.loop2:
    call set_pte
    inc ebx
    add eax, 0x1000
    cmp ebx, 1013               ; -> 0xffff.5000
    jl .loop2

    ;; Load page directory.
    mov eax, page_directory
    and eax, 0xfffff000
    mov cr3, eax

    ;; Enable paging.
    mov eax, cr0
    bts eax, 31
    bts eax, 16 ; Enable write-protect.
    mov cr0, eax

    ret

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

set_pde:
    and eax, 0xfffff000
    or eax, 11b
    mov [page_directory+ebx*4], eax
    ret

set_pte:
    and eax, 0xfffff000
    or eax, 11b
    mov [edi+ebx*4], eax
    ret
