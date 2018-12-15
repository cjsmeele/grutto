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
// |------------------| 0xe000'0000
// |------------------| 0xef00'0000
// | Framebuffer      |
// |------------------| 0xff80'0000
// | Cur. page tables |
// |------------------| 0xffff'1000
// | Kernel stack     |
// |------------------| 0xffff'6000
// | Cur. page dir.   |
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
    using pdir_t __attribute__((aligned(page_size))) = Array<pde_t,1_K>;
    using ptab_t __attribute__((aligned(page_size))) = Array<pte_t,1_K>;

    static_assert(alignof(pdir_t) == 4_K, "Page directory type is not aligned");

    // va_pdir always points to the current page directory.
    constexpr auto va_pdir         = vaddr_t {0xffff6000UL};

    // va_pts always points to the 4M va-region containing page tables for the current task.
    // the upper ¼th is the same for all processes (kernel memory).
    // the lower ¾th differs per process.
    constexpr auto va_pts          = vaddr_t {0xff800000UL}; // (1022ULL << 22)

    // page-aligned va of the kernel's stack.
    constexpr auto va_kernel_stack = vaddr_t {0xffff1000UL};

    // Beyond this address, the kernel may allocate virtual memory
    // for e.g. memory mapped IO.
    constexpr auto va_kernel_heap_end = vaddr_t {0xe0000000UL};

    // A 2560*1600*4 framebuffer would be slightly less than 16M,
    // so rounds up to 0x01000000 and reserve that much vas in advance.
    // TODO: Remove static allocation.
    constexpr auto   va_framebuffer          = vaddr_t {0xef000000UL};
    constexpr size_t va_framebuffer_max_size =          0x01000000UL;

    constexpr size_t va_to_ptn(vaddr_t va) { return vpage_t{va}.u() >> 10; }

    void init();

    namespace Kernel {
        void   map(vpage_t vp, ppage_t pp, size_t count);
        void unmap(vpage_t vp, size_t count);
        Optional<vpage_t> map_alloc(vpage_t vp, size_t count);
    }

    namespace User {
        // Safe user-mode API.

        //void map(vpage_t vn, size_t count);
    }
}
