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
#pragma once

#include "std/types.hh"
#include "std/type-traits.hh"
#include "std/memory.hh"

namespace Mem {

    [[nodiscard]]
    addr_t kmap(size_t size);

    /// Alloc virtual memory space.
    [[nodiscard]]
    addr_t valloc(size_t size);

    [[deprecated]]
    void *kalloc0(size_t count);

    [[nodiscard]] void *kmalloc(size_t size, size_t align);
    [[nodiscard]] void *kmalloc(size_t count);
    [[nodiscard]] void *kcalloc(size_t count);
    void  kfree(void *p);

    //template<typename T>
    //void kfree(T *location) { }

    void map(addr_t vaddr, addr_t paddr, size_t size);
}
