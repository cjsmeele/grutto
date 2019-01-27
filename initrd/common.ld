/* Copyright (c) 2019, Chris Smeele
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
ENTRY(start_)

SECTIONS {

    . = 0x00100000;

    .text : {
        *(.text*)
        *(.gnu.linkonce.t*)
    }

    .rodata ALIGN(0x1000) : {

        CTORS_START = .;
        *(SORT(.ctor*))
        CTORS_END = .;

        DTORS_START = .;
        *(SORT(.dtor*))
        DTORS_END = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)

    }

    .data ALIGN(0x1000) : {
        *(.data)
        *(.gnu.linkonce.d*)

    }

    .bss ALIGN(0x1000) : {
        BSS_START = .;
        *(COMMON)
        *(.gnu.linkonce.b*)
        *(.bss)
        BSS_END = .;
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
