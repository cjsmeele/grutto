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
#include "gdt.hh"

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

    using desc_t = u64;

                                      // Don't question these values.
    constexpr desc_t make_desc_null () { return 0x0000'0000'0000'0000ULL; }
    constexpr desc_t make_desc_kcode() { return 0x00df'9a00'0000'ffffULL; }
    constexpr desc_t make_desc_kdata() { return 0x00df'9200'0000'ffffULL; }
    constexpr desc_t make_desc_ucode() { return 0x00df'fa00'0000'ffffULL; }
    constexpr desc_t make_desc_udata() { return 0x00df'f200'0000'ffffULL; }

    constexpr u64 make_ptr(addr_t a, u16 n_entries) {
        // NB: n_entries must include the null descriptor.
        return u64{a.u()} << 16 | (n_entries * 8);
    }

    u64 gdt_ptr;

    inline void asm_lgdt(u16 n_code, u16 n_data) {
        asm volatile ("lgdtl %[ptr] \n"
                      "mov %[nd], %%ax \n"
                      "mov %%ax,  %%ds \n"
                      "mov %%ax,  %%es \n"
                      "mov %%ax,  %%fs \n"
                      "mov %%ax,  %%gs \n"
                      "mov %%ax,  %%ss \n"
                      "mov %%ax,  %%ax \n"
                      "ljmpl %[nc], $new_cs___ \n"
                      "new_cs___: nop\n"
                      :: [ptr] "m" (gdt_ptr)
                       , [nc]  "i" (n_code*8)
                       , [nd]  "i" (n_data*8)
                       );
    }


    alignas(8) desc_t gdt[5];

    void init() {
        gdt[0]  = make_desc_null();
        gdt[1]  = make_desc_kcode();
        gdt[2]  = make_desc_kdata();
        gdt[3]  = make_desc_ucode();
        gdt[4]  = make_desc_udata();
        gdt_ptr = make_ptr(gdt, 5);
        asm_lgdt(1, 2);
    }

}
