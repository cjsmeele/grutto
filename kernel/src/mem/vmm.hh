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

#include "common.hh"
#include "pmm.hh"

namespace Vmm {

    constexpr auto granularity = Mem::Pmm::granularity;
    constexpr auto page_size   = 4_K;
    static_assert(page_size == granularity);

    constexpr bool is_page_aligned(addr_t va) { return is_aligned(va, granularity); }

    constexpr auto pt_size           =  4_M;
    constexpr auto kernel_stack_size = 16_K;

    addr_t kva_to_pa(addr_t va);
    addr_t kva_to_pa(void *va);

    void map_page (addr_t vn, addr_t pn);
    void map_pages(addr_t vn, addr_t pn, size_t count);

    void alloc_at(addr_t vn, size_t count);

    void alloc(size_t count);

    void unmap_page(addr_t vn);
    void free_page(addr_t vn);

    void init();
}
