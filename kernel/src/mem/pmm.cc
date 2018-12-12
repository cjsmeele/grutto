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
#include "pmm.hh"

#include "multiboot-info.hh"

namespace Pmm {

    Bitmap<bitmap_elems, 1> bmp;

    constexpr size_t max_regions = 32;

    struct region_t {
        paddr_t start;
        size_t length;
    };

    SVector<region_t, max_regions> regions;

    bool is_allocated(size_t elem, size_t size) {
        return bmp.elem(elem);
    }

    Optional<ppage_t> alloc(size_t count, size_t align) {
         auto start = bmp.find_contiguous(div_ceil(lowest_usable_address, page_size)
                                         ,count
                                         ,align);
         if (start)
             bmp.set_range(*start, count, true);
         return ppage_t {*start};
    }

    void free(ppage_t elem, size_t count) {
        // NB: This assumes the given range is actually allocated.
        //     Otherwise, counters will lose accuracy.

        bmp.set_range(elem.u(), count, false);
    }

    size_t mem_available() {
        return bmp.available * page_size;
    }

    void init() {
        auto &mbinfo = Multiboot::info();

        assert(mbinfo.flags & MULTIBOOT_INFO_MEM_MAP, "No memory map available");

        koi(LL::debug).fmt("bios-supplied memory map:\n");

        // Mark all memory as unavailable.
        bmp.set_range(0, bmp.length, true);

        // These regions are assumed NOT to overlap.
        // Overlapping regions will break memory availability counters.
        for (auto p = paddr_t{mbinfo.mmap_addr}
             ; p.u() + 4 < mbinfo.mmap_addr + mbinfo.mmap_length
             ; p = p.offset(((multiboot_mmap_entry*)p)->size + 4)) {

            auto *entry = (multiboot_mmap_entry*)p;

            auto addr = entry->addr;
            auto len  = entry->len;

            {
                // Make sure the address is aligned.
                auto x = div_ceil(addr, page_size) * page_size;
                if (x != addr) {
                    if (x - addr >= len)
                         len = 0; // Bogus.
                    else len -= x - addr;

                    addr = x;
                }
            }

            len -= len % page_size;

            if (entry->len) {
                koi(LL::debug).fmt("  {016x} - {016x} type:{}, {6S}"
                       ,addr
                       ,addr + len - 1
                       ,entry->type
                       ,len);

                u64 usable = entry->type == MULTIBOOT_MEMORY_AVAILABLE
                           ? (addr         <= intmax<paddr_t>::value
                             // A region crossing the 4G barrier seems unlikely,
                             // but we will account for it anyway.
                             ? (addr + len <= intmax<paddr_t>::value
                               ? len
                               : intmax<paddr_t>::value - addr + 1
                               )
                             : 0)
                           : 0;

                bmp.set_range(addr   / page_size
                             ,usable / page_size, false);

                if (usable && usable != len) {
                    koi(LL::debug).fmt(" of which {S} usable", usable);
                } else if (!usable) {
                    koi(LL::debug).fmt(" (unusable)");
                }
                koi(LL::debug).put_char('\n');

                if (usable)
                    regions.push(region_t{ static_cast<paddr_t>(addr), (size_t)usable });
            }
        }

        // Mark kernel sections as allocated.
        bmp.set_range(bootstrap_lma().u() / page_size
                      ,div_ceil(kernel_size()
                                + (kernel_lma().u() - bootstrap_lma().u())
                               , page_size)
                      ,true);

        koi(LL::notice).fmt("total {S} usable phy pages ({S})\n",
                            bmp.available, bmp.available * page_size);
    }

    void dump_pmm() {
        for (size_t i = 0; i < bmp.size; i+=8) {
            koi.fmt("{08x}: ", i*bmp.bits_per_entry*page_size);
            for (size_t j = 0; j < 8; ++j)
                koi.fmt("{08x}", bmp.entries[i+j]);
            koi.put_char('\n');
        }
        koi.fmt("bootstrap lma: {08x}\n",    bootstrap_lma());
        koi.fmt("kernel lma:    {08x}\n",    kernel_lma());
        koi.fmt("kernel vma:    {08x}\n",    kernel_vma());
        koi.fmt("kernel size:   { 8S}\n",    kernel_size());
        koi.fmt("total {S} usable memory\n", bmp.available * page_size);
    }
}
