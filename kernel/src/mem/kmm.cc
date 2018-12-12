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
#include "kmm.hh"
#include "vmm.hh"

namespace Kmm {

    auto   heap_start = ::heap_start();
    size_t heap_size  = 0;

    void *more_core(ptrdiff_t diff) {
        koi.fmt("sbrk, heap_sz {08x}, diff {}\n", heap_size, diff);

        if (LIKELY(diff > 0)) {
            if (is_divisible(heap_size, Vmm::page_size)) {
                Vmm::alloc_at(heap_start.offset(heap_size),
                              div_ceil(diff, Vmm::page_size));

                //koi.fmt("alloced {} pages\n", div_ceil(diff, Vmm::page_size));
            } else {
                size_t extra = (heap_size + diff) / Vmm::page_size
                             - heap_size / Vmm::page_size;

                Vmm::alloc_at(heap_start.offset(heap_size)
                                        .align_up(Vmm::page_size)
                             ,extra);
                //koi.fmt("alloced 1\n");
            }
        } else if (diff < 0) {
            if ((size_t)-diff > heap_size)
                diff = -heap_size;
        }

        void *brk = heap_start.offset(heap_size);
        heap_size += diff;

        return brk;
    }

}
