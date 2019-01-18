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
#include "mem-ops.hh"
#include "std/memory.hh"

extern "C" {
    void dlfree(void*);
    [[nodiscard]] void *dlcalloc  (size_t count, size_t size);
    [[nodiscard]] void *dlmalloc  (size_t size);
    [[nodiscard]] void* dlmemalign(size_t align, size_t size);
}

namespace Mem {

    void kfree(void *p) {
        dlfree(p);
    }

    void *kmalloc(size_t size, size_t align) { return dlmemalign(align, size); }

    void *kmalloc(size_t size) { return dlmalloc(size); }
    void *kcalloc(size_t size) { return dlcalloc(size, 1); }
}

void *operator new(size_t size)               { return Mem::kmalloc(size); }
void *operator new[](size_t size)             { return Mem::kmalloc(size); }
void  operator delete(void *ptr)              { Mem::kfree(ptr); }
void  operator delete[](void *ptr)            { Mem::kfree(ptr); }

namespace std { enum class align_val_t : size_t {}; }
void *operator new(size_t size, std::align_val_t align) { return Mem::kmalloc(size, static_cast<size_t>(align)); }
