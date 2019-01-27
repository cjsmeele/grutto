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
#include "mem/vmm.hh"

namespace Elf {

    constexpr inline size_t max_ph_segments = 16;

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

    enum ElfType : u32 {
        ELF_TYPE_NONE = 0, ///< No type.
        ELF_TYPE_REL  = 1, ///< Relocatable object.
        ELF_TYPE_EXEC = 2, ///< Executable.
        ELF_TYPE_DYN  = 3, ///< Shared object.
        ELF_TYPE_CORE = 4, ///< Core file.
    };

    enum ElfPhEntryType : u8 {
        ELF_PT_LOAD = 1,
    };

    struct Elf32PhEntry {
        elf32_word_t type;
        elf32_off_t  offset;    ///< Offset in this file.
        elf32_addr_t v_addr;    ///< Virtual address destination.
        elf32_addr_t p_addr;    ///< Physical address destination.
        elf32_word_t size_file; ///< Segment size within the ELF binary.
        elf32_word_t size_mem;  ///< Segment size in memory.
        elf32_word_t flags;
        elf32_word_t align;
    } __attribute__((packed));

    Either<const char*, vaddr_t> load(vaddr_t elf_base, size_t elf_size) {

        // * Validate ELF image.

        if (elf_size < sizeof(Elf32Header))              return Left("ELF header invalid");

        const auto elf_span = span_t { elf_base, elf_size };
        const auto &header  = *(Elf32Header*)elf_base;

        // * Validate ELF header.

        if (!strneq(header.ident.magic, "\x7f""ELF", 4)) return Left("ELF magic missing");
        if (header.ident.elf_class   != ELF_32BIT      ) return Left("unsupported ELF bitness");
        if (header.ident.endianness  != ELF_LE         ) return Left("unsupported ELF endianness");
        if (header.type              != ELF_TYPE_EXEC  ) return Left("ELF not executable");

        // * Validate program header location and size.

        auto ph_span_ = safe_add(elf_base.u(), header.ph_off)
                       .then(λx(safe_mul(u32{header.ph_num}, header.ph_ent_size)
                               .then(λy(make_span(x, y)))))
                       .require(header.ph_num > 0)
                       .require(header.ph_num <= max_ph_segments)
                       .require(λx(elf_span.contains(x)));

        if (!ph_span_.ok())                              return Left("invalid program header spec");
        auto ph_span = *ph_span_;

        // * Validate program header entries.

        auto user_span = span_t { size_t{1_M}, size_t{0xc000'0000ULL - 1_M} };

        bool entry_point_mapped = false;

        FixedVector<Elf32PhEntry*, max_ph_segments> segments;

        for (size_t i = 0; i < header.ph_num; ++i) {

            Elf32PhEntry *phe = ((Elf32PhEntry*)ph_span.start()) + i;

            if (phe->type != ELF_PT_LOAD)
                // We are only interested in segments that need loading.
                continue;

            // ** Verify that the claimed memory fully resides in userspace.
            auto mem_span_  = safe_add(phe->size_file, phe->size_mem)
                             .then(λx(make_span(phe->v_addr, x)))
                             .require(λx(user_span.contains(x)));

            // ** Verify that the referenced file data is contained within the ELF image.
            auto file_span_ = safe_add(elf_base.u(), phe->offset)
                             .then(λx(make_span(x, phe->size_file)))
                             .require(λx(x.empty() || elf_span.contains(x)));

            if (!mem_span_.ok() || !file_span_.ok())     return Left("invalid program header entry");

            // ** Verify that the claimed memory does not overlap other segments.

            for (const auto *o : segments) {
                if ((*mem_span_).overlaps(span_t { u32{o->offset},
                                                   u32{o->size_file + o->size_mem} }))
                                                         return Left("invalid program header entry");
            }

            if ((*mem_span_).contains(header.entry))
                entry_point_mapped = true;

            segments.push(phe);
        }

        if (segments.empty())                            return Left("no loadable segments");
        if (!entry_point_mapped)                         return Left("entrypoint not in mapped mem");

        // * Load segments.

        for (const auto *phe : segments) {
            koi.fmt("> PT_LOAD: offset<{08x}> va<{}> file<{6S}> mem<{6S}>\n",
                    phe->offset, vaddr_t{phe->v_addr}, phe->size_file, phe->size_mem);

            auto span = span_t { u32{phe->offset}, u32{phe->size_file + phe->size_mem} };
            if (!Vmm::map_alloc(vaddr_t{phe->v_addr}.align_down(page_size),
                                div_ceil(span.size(), page_size),
                                Vmm::P_User | Vmm::P_Writable).ok()) {

                // XXX: This may leak any previous map/allocs.
                //      (though we should have a good pdir cleanup routine anyway)
                return Left("could not allocate memory for ELF image");
            }

            // Actually load stuff!
            memcpy((char*)phe->v_addr,                elf_base.offset(phe->offset), phe->size_file);
            memset((char*)phe->v_addr+phe->size_file, 0,                            phe->size_mem);
        }

        return Right(vaddr_t{header.entry});
    }
}
