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

    constexpr auto page_size   = 4_K;

    constexpr bool is_page_aligned(addr_t va) { return va.is_aligned(page_size); }

    constexpr auto kernel_stack_size = 16_K;

    // 4M-aligned va of the kernel's page tables.
    constexpr auto va_kernel_pts   = addr_t {1022ULL << 22}; // 0xff800000
    // page-aligned va of the kernel's stack.
    constexpr auto va_kernel_stack = addr_t {0xffff1000ULL};

    // A 2560*1600*4 framebuffer would be slightly less than 16M, .
    // so rounds up to 0x01000000.
    // Reserve that much vas in advance.
    constexpr addr_t va_framebuffer          = addr_t {0xef000000ULL};
    constexpr size_t va_framebuffer_max_size =         0x01000000ULL;

    addr_t kva_to_pa(addr_t va);

    constexpr size_t va_to_ptn(addr_t va) { return page_t{va}.u() >> 10; }

    void map_page (page_t vn, page_t pn);
    void map_pages(page_t vn, page_t pn, size_t count);

    void alloc_at(page_t vn, size_t count);

    void alloc(size_t count);

    void unmap_page(page_t vn);
    void free_page(page_t vn);

    void init();
}
