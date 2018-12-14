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

    constexpr auto kernel_stack_size = 16_K;

    // 4M-aligned va of the kernel's page tables.
    constexpr auto va_kernel_pts   = vaddr_t {1022ULL << 22}; // 0xff800000
    // page-aligned va of the kernel's stack.
    constexpr auto va_kernel_stack = vaddr_t {0xffff1000ULL};

    // A 2560*1600*4 framebuffer would be slightly less than 16M,
    // so rounds up to 0x01000000.
    // Reserve that much vas in advance.
    constexpr auto   va_framebuffer          = vaddr_t {0xef000000ULL};
    constexpr size_t va_framebuffer_max_size =          0x01000000ULL;

    // constexpr bool is_page_aligned(vaddr_t va) { return va.is_aligned(page_size); }

    paddr_t kva_to_pa(vaddr_t va);

    constexpr size_t va_to_ptn(vaddr_t va) { return vpage_t{va}.u() >> 10; }

    void map_page (vpage_t vn, ppage_t pn);
    void map_pages(vpage_t vn, ppage_t pn, size_t count);

    void alloc_at(vpage_t vn, size_t count);

    void alloc(size_t count);

    void unmap_page(vpage_t vn);
    void free_page (vpage_t vn);

    void init();
}
