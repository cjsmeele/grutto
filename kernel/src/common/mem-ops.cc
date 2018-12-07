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
#include "mem-ops.hh"
#include "std/memory.hh"

extern "C" {
    void dlfree(void*);
    [[nodiscard]] void *dlcalloc(size_t count, size_t size);
    [[nodiscard]] void *dlmalloc(size_t size);
    [[nodiscard]] void* dlmemalign(size_t align, size_t size);
}

namespace Mem {

    void kfree(void *p) {
        dlfree(p);
    }

    void *kalloc0(size_t size) {
        static addr_t a = (addr_t)KERNEL_END;
        auto alignment = alignof(max_align_t);
        if (a & (alignment-1)) {
            a += alignment - (a & (alignment-1));
        }
        auto addr = a;
        a += size;
        set((u8*)addr, (u8)0x7f, size);
        return (void*)addr;
    }

    void *kmalloc(size_t size, size_t align) { return dlmemalign(align, size); }

    void *kmalloc(size_t size) { return dlmalloc(size); }
    void *kcalloc(size_t size) { return dlcalloc(size, 1); }
}

void *operator new(size_t size)    { return Mem::kmalloc(size); }
void *operator new[](size_t size)  { return Mem::kmalloc(size); }
void  operator delete(void *ptr)   { Mem::kfree(ptr); }
void  operator delete[](void *ptr) { Mem::kfree(ptr); }
