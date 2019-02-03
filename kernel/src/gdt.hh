/* Copyright (c) 2018, 2019, Chris Smeele
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
#pragma once

#include "common.hh"

namespace Gdt {

    struct tss_t {
        u16 prev_task; u16 pad_0;
        u32 esp0;
        u16 ss0;   u16 pad_1;
        u32 esp1;
        u16 ss1;   u16 pad_2;
        u32 esp2;
        u16 ss2;   u16 pad_3;
        u32 cr3;
        u32 eip;
        u32 eflags;
        u32 eax;
        u32 ecx;
        u32 edx;
        u32 ebx;
        u32 esp;
        u32 ebp;
        u32 esi;
        u32 edi;
        u16 es;    u16 pad_4;
        u16 cs;    u16 pad_5;
        u16 ss;    u16 pad_6;
        u16 ds;    u16 pad_7;
        u16 fs;    u16 pad_8;
        u16 gs;    u16 pad_9;
        u16 ldtss; u16 pad_10;
        u16 trap;  u16 iomap_offset;
    } __attribute__((packed));

    void set_tss_sp(vaddr_t sp);

    void init();
}

