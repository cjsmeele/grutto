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

namespace Mem::Kmm {

    addr_t heap_start = ::heap_start();
    size_t heap_size  = 0;

    void *more_core(ptrdiff_t diff) {
        koi.fmt("sbrk, heap_sz {08x}, diff {}\n", heap_size, diff);

        if (diff < 0) {
            if ((size_t)-diff > heap_size)
                diff = -heap_size;
        } else if (diff > 0) {
            if (heap_size % Mem::Vmm::granularity == 0) {
                Mem::Vmm::alloc_at((heap_start+heap_size) >> 12,
                                   div_ceil(diff, Mem::Vmm::granularity));

                koi.fmt("alloced {} pages\n", div_ceil(diff, Mem::Vmm::granularity));
            } else {
                size_t extra = (heap_size + diff) / Mem::Vmm::granularity
                             - heap_size / Mem::Vmm::granularity;

                Mem::Vmm::alloc_at(div_ceil(heap_start+heap_size, Mem::Vmm::granularity),
                                   extra);
                koi.fmt("alloced 1\n");
            }
        }

        void *brk = (void*)(heap_start + heap_size);
        heap_size += diff;

        return brk;
    }

}
