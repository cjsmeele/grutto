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
#include "gdt.hh"

namespace Gdt {

    using desc_t = u64;

    static_assert(sizeof(void*) == 4, "this needs to be changed for long mode");

                                      // Don't question these values.
    constexpr desc_t make_desc_null () { return 0x0000'0000'0000'0000ULL; }
    constexpr desc_t make_desc_kcode() { return 0x00df'9a00'0000'ffffULL; }
    constexpr desc_t make_desc_kdata() { return 0x00df'9200'0000'ffffULL; }
    constexpr desc_t make_desc_ucode() { return 0x00df'fa00'0000'ffffULL; }
    constexpr desc_t make_desc_udata() { return 0x00df'f200'0000'ffffULL; }
    constexpr desc_t make_desc_tss(vaddr_t a) {
        return 0x0050'8900'0000'0067ULL
             | ((u64{a.u()} & 0x00ffffff) << 16)
             | ((u64{a.u()} & 0xff000000) << 32);
    }

    constexpr u64 make_ptr(vaddr_t a, u16 n_entries) {
        // NB: n_entries must include the null descriptor.
        return u64{a.u()} << 16 | (n_entries * 8);
    }

    u64 gdt_ptr;

    inline void asm_lgdt(u16 n_code, u16 n_data, u16 n_tss) {
        asm volatile ("lgdtl %[ptr] \n"
                      "mov %[nd], %%ax \n"
                      "mov %%ax,  %%ds \n"
                      "mov %%ax,  %%es \n"
                      "mov %%ax,  %%fs \n"
                      "mov %%ax,  %%gs \n"
                      "mov %%ax,  %%ss \n"
                      "ljmpl %[nc], $new_cs___ \n"
                      "new_cs___: \n"
                      "mov %[nt], %%ax \n"
                      "ltr %%ax \n"
                      :: [ptr] "m" (gdt_ptr)
                       , [nc]  "i" (n_code*8)
                       , [nd]  "i" (n_data*8)
                       , [nt]  "i" (n_tss *8)
                       );
    }

    alignas(8) Array<desc_t, 6> gdt;

    tss_t tss { };

    void set_tss_sp(vaddr_t sp) {
        tss.esp0 = sp.u();
        tss.ss0  = 2*8; // Keep in sync with segment order specified in init().
                        // This must always be kdata.
    }

    void init() {
        tss.iomap_offset = sizeof(tss_t);

        gdt[0]  = make_desc_null();
        gdt[1]  = make_desc_kcode();
        gdt[2]  = make_desc_kdata();
        gdt[3]  = make_desc_ucode();
        gdt[4]  = make_desc_udata();
        gdt[5]  = make_desc_tss(&tss);
        gdt_ptr = make_ptr(*gdt, gdt.length());
        asm_lgdt(1, 2, 5);
    }
}
