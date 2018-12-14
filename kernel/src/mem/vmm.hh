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

// Virtual address map:
//
// +------------------+ 0x0000'0000
// | Unusable 1M      |
// |------------------| 0x0010'0000
// | User program     |
// |------------------|
// | User heap        |
// |------------------|
// | free             |
// |                  |
// |------------------| 0xc000'0000
// | Kernel code      |
// |------------------|
// | Kernel data      |
// |------------------|
// | Kernel heap      |
// |------------------| 0xef00'0000
// | Framebuffer      |
// |------------------| 0xffff'1000
// | Kernel stack     |
// |------------------|
// | free             |
// +------------------+

// Page tables for kernel memory (>=c0000000) are pre-allocated and mapped in
// all address spaces.
// pa_kernel_pts contains enough (1024) pagetables to map all virtual memory.
//
// We will not bother with ASLR and PTI, so stuff should be mostly
// straightforward from here.

namespace Vmm {

    using pde_t = u32;
    using pte_t = u32;

    // Note: alignas() is illegal on type-aliases (why?),
    // so we resort to using compiler-specific attributes instead.
    //using pdir_t alignas(page_size) = Array<pde_t,1_K>;
    using pdir_t __attribute__((aligned(page_size))) = Array<pde_t,1_K>;

    static_assert(alignof(pdir_t) == 4_K, "Page directory type is not aligned");

    // 4M-aligned va of the kernel's page tables.
    constexpr auto va_kernel_pts   = vaddr_t {0xff800000ULL}; // (1022ULL << 22)
    // page-aligned va of the kernel's stack.
    constexpr auto va_kernel_stack = vaddr_t {0xffff1000ULL};

    // A 2560*1600*4 framebuffer would be slightly less than 16M,
    // so rounds up to 0x01000000 and reserve that much vas in advance.
    constexpr auto   va_framebuffer          = vaddr_t {0xef000000ULL};
    constexpr size_t va_framebuffer_max_size =          0x01000000ULL;

    paddr_t kva_to_pa(vaddr_t va);

    constexpr size_t va_to_ptn(vaddr_t va) { return vpage_t{va}.u() >> 10; }

    void map_page (vpage_t vn, ppage_t pn);
    void map_pages(vpage_t vn, ppage_t pn, size_t count);

    void alloc_at(vpage_t vn, size_t count);

    void alloc(size_t count);

    void unmap_page(vpage_t vn);
    void free_page (vpage_t vn);

    void init();

    namespace User {
        // Safe user-mode API.

        void map(vpage_t vn, size_t count);
    }
}
