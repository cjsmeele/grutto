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
#include "elf.hh"

namespace Elf {

    enum ElfClass : u8 {
        ELF_32BIT = 1,
        ELF_64BIT = 2,
    };
    enum ElfEndianness : u8 {
        ELF_LE = 1,
        ELF_BE = 2,
    };

    // ELF32 types.
    using elf32_addr_t   = u32;
    using elf32_half_t   = u16;
    using elf32_off_t    = u32;
    using elf32_word_t   = u32;
    using elf32_sword_t  = s32;

    struct ElfIdent {
        char magic[4];
        ElfClass      elf_class;  ///< 1 = 32-bit, 2 = 64-bit.
        ElfEndianness endianness; ///< 1 = little-endian, 2 = big-endian.
        u8   version;
        u8   _reserved1[9];
    } __attribute__((packed));

    struct Elf32Header {
        ElfIdent ident;

        elf32_half_t type;
        elf32_half_t machine;
        elf32_word_t version;
        elf32_addr_t entry;
        elf32_off_t  ph_off; ///< Program Header offset.
        elf32_off_t  sh_off; ///< Section Header offset.
        elf32_word_t flags;
        elf32_half_t eh_size;
        elf32_half_t ph_ent_size;
        elf32_half_t ph_num;
        elf32_half_t sh_ent_size;
        elf32_half_t sh_entNum;
        elf32_half_t sh_str_index;
    } __attribute__((packed));

    Optional<vaddr_t> load(vaddr_t elf_base, size_t elf_size) {
        if (elf_size < sizeof(Elf32Header)) return nullopt;

        auto elf_span = span_t { elf_base, elf_size };

        auto &header = *(Elf32Header*)elf_base;

        if (!strneq(header.ident.magic, "\x7f""ELF", 4)) return nullopt;

        if (header.ident.elf_class  != ELF_32BIT) return nullopt;
        if (header.ident.endianness != ELF_LE)    return nullopt;
        if (header.type             != 2)         return nullopt;

        auto ph_span_ = safe_add(elf_base.u(), header.ph_off)
                       .then(λx(safe_mul(u32{header.ph_num}, header.ph_ent_size)
                               .then   (λy(make_span(x, y)))))
                               .require(λx(elf_span.contains(x)));

        if (!ph_span_.ok()) return nullopt; auto ph_span = *ph_span_;


        koi.fmt("header magic: {}\n", header.ident.magic+1);

        //return vaddr_t{header.entry};
        return nullopt;
    }
}
