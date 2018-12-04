;; Common x86 kernel entrypoint.
;;
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

;; This routine is defined by target-specific code (amd64 or i686).
extern bootstrap_target_boot

;; This is where our bootstrap code starts executing.
global start

global bootstrap_die

[bits 32]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  MULTIBOOT HEADER 
;;  ---------------- 
;;
;;  We can be loaded from any multiboot-compliant bootloader.
;;
;;  The multiboot spec defines the state of the system when the
;;  bootloader passes control to the kernel. Summarized, the state is as
;;  follows:
;;
;;  - EAX contains the magic value 0x2badb002
;;  - EBX contains a 32-bit pointer to a multiboot info struct
;;  - ESP is undefined
;;  - Segment registers:
;;    - CS is 32-bit r/x code covering all 32-bit memory
;;    - Other registers are 32-bit r/w data covering all 32-bit memory
;;  - A20 gate is enabled
;;  - Paging is off
;;  - Interrupts are off

section .multiboot_header

MULTIBOOT_MEMORY_INFO  equ  1 << 1
MULTIBOOT_MAGIC        equ  0x1badb002
MULTIBOOT_FLAGS        equ  MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM     equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  EARLY BOOTSTRAP STACK
;;  ---------------------
;;
;;  This is a temporary stack used only during bootstrap. We cannot rely
;;  on the bootloader's existing stack.
;;
;;  When we've set up paging, target-specific code is used to create a
;;  new stack at an appropriate virtual address.

section .bss

BOOTSTRAP_EARLY_STACK_SIZE equ 64

bootstrap_early_stack:
    resb BOOTSTRAP_EARLY_STACK_SIZE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  INITIAL PROTECTED MODE ENTRYPOINT
;;  ---------------------------------
;;
;;  From the bootloader we jump to the start symbol within this section.
;;
;;  There's still quite a bit of work to be done before we can jump to
;;  the C++ kernel:
;;
;;  - Verify that the bootloader is multiboot-copmliant (check magic)
;;  - Load a GDT (with either 32 or 64-bit code + data segments)
;;  - Set up paging for higher-half kernel execution
;;  - Create a stack
;;  - If target is amd64, switch to long mode
;;  - Call global constructors
;;  - Then finally call main()

section .bootstrap32

;; Show an error code (0..99) and die.
%macro ERROR 1
    mov ebx, ((%1 / 10) << 8) | (%1 % 10)
    jmp bootstrap_die
%endmacro

;; Temporary location to store multiboot information pointer.
global bootstrap_multiboot_info_addr
bootstrap_multiboot_info_addr:
    dd 0

;; Entrypoint.
start:
    ;; Save the mb info struct pointer so we can pass it on later.
    mov [bootstrap_multiboot_info_addr], ebx

    ;; Firstly check whether our environment is sane:
    ;; Verify the multiboot magic number.
    cmp eax, 0x2badb002
    je .sanity

    ERROR(80)

.sanity:
    ;; Ok, now set up a temporary stack.
    mov esp, bootstrap_early_stack + BOOTSTRAP_EARLY_STACK_SIZE

    ;; Pass on control to the target-specific bootstrap code.
    jmp bootstrap_target_boot

    ;; Oops.
    ERROR(99)

bootstrap_die:
    mov edi, 0xb8000+2*72
    mov eax, ebx
    and eax, 0xff
    shr ebx, 8
    or eax, 0x4f00
    or ebx, 0x4f00
    add eax, '0'
    add ebx, '0'
    mov word [edi+ 0], 0x4f00 | ' '
    mov word [edi+ 2], 0x4f00 | 'E'
    mov word [edi+ 4], 0x4f00 | 'R'
    mov word [edi+ 6], 0x4f00 | 'R'
    mov word [edi+ 8], 0x4f00 | ' '
    mov word [edi+10], bx
    mov word [edi+12], ax
    mov word [edi+14], 0x4f00 | ' '
    mov edi, 0xb8000 + 11*80*2

.hang:
    cli
    hlt
    jmp .hang
