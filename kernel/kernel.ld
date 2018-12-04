/* Copyright (c) 2018, Chris Smeele
 *
 * This file is part of Grutto.
 *
 * Grutto is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Grutto is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Grutto.  If not, see <https://www.gnu.org/licenses/>.
 */

OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS {
    BOOTSTRAP_LMA = 0x00100000;
    BOOTSTRAP_VMA = BOOTSTRAP_LMA;

    . = BOOTSTRAP_VMA;

    .bootstrap : AT(BOOTSTRAP_LMA) {

        *(.multiboot_header)
        obj/bootstrap.o (*)
        obj/target-bootstrap.o (*)

    } BOOTSTRAP_SIZE = SIZEOF(.bootstrap);

    KERNEL_LMA = ALIGN(0x1000);
    KERNEL_VMA = 0xc0000000;

    /* NB: Bootstrap code/data are not mapped in high kernel memory */
    . = KERNEL_VMA;

    .text : AT(KERNEL_LMA) {
        KERNEL_TEXT_VMA = .;
        *(.text*)
        *(.gnu.linkonce.t*)

    } KERNEL_TEXT_SIZE = SIZEOF(.text);

    .rodata ALIGN(0x1000) : {
        KERNEL_RODATA_VMA = .;

        CTORS_START = .;
        *(SORT(.ctor*))
        CTORS_END = .;

        DTORS_START = .;
        *(SORT(.dtor*))
        DTORS_END = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)

    } KERNEL_RODATA_SIZE = SIZEOF(.rodata);

    .data ALIGN(0x1000) : {
        KERNEL_DATA_VMA = .;
        *(.data)
        *(.gnu.linkonce.d*)

    } KERNEL_DATA_SIZE = SIZEOF(.data);

    .bss ALIGN(0x1000) : {
        KERNEL_BSS_VMA = .;
        *(COMMON)
        *(.gnu.linkonce.b*)
        *(.bss)

    } KERNEL_BSS_SIZE = SIZEOF(.bss);

    KERNEL_SIZE = . - KERNEL_VMA;

    KERNEL_END = .;
    HEAP_START = ALIGN(0x1000);

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
