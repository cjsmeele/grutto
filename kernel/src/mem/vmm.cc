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
#include "vmm.hh"

extern unusable_t kernel_stack;

namespace Vmm {

    using pde_t = u32;
    using pte_t = u32;

    alignas(granularity) pde_t pd[1_K] { }; // A page directory holds 1024 entries.

    // TODO: Remove magic numbers, create helpers for building PDEs and PTEs.

    // TODO: Create addr_t, va_t, pa_t, pn_t types that are not implicitly
    //       convertable to u32 in order to make this interface safer.

    addr_t pde_pte_addr   (pde_t pde)             { return (pde >> 12) << 12; }
    void   pde_pte_addr   (pde_t pde, addr_t val) { pde = (pde & 0xfffff000) | (val << 12); }
    addr_t pde_pte_ctx    (pde_t pde)             { return (pde >> 9) & 0b111; }
    void   pde_pte_ctx    (pde_t pde, u8 val)     { pde = (pde & 0xfffff1fff) | ((val&0b111) << 9); }
    bool   pde_pte_present(pde_t pde)             { return (pde & 1); }
    void   pde_pte_present(pde_t pde, bool val)   { pde = (pde & ~1) | !!val; }

    addr_t kva_to_pa(addr_t va) { return va - kernel_vma() + kernel_lma(); }
    addr_t kva_to_pa(void *va)  { return kva_to_pa((addr_t)va); }

    pte_t make_pde_4M(addr_t pn)    { return    pn << 12 | 0x80 | 3; }
    pte_t make_pde_pt(addr_t pt_pn) { return pt_pn << 12 | 3; }
    pte_t make_pte(addr_t pn)       { return    pn << 12 | 3; }

    // 4M-aligned pa of the kernel's page tables (to be allocated).
    addr_t kernel_pt_phy = 0;

    pte_t &get_pte(addr_t vn) {
        // Obtain page table entry for the given virtual address.
        u32 ptn  = vn >> 10;   // pde / page table number.
        u32 pten = vn & 0x3ff; // pte / page number.
        pte_t *pt = (pte_t*)va_kernel_pts + ptn*1_K;
        if (pd[ptn] & 1) { // pde present?
            assert((pd[ptn] & 0x80) == 0, "Cannot split up 4M mapping to get a PTE");
            // To handle above case, we should split the 4M mapping into 4K mappings.
        } else {
            // pde not present, create it.
            pd[ptn] = make_pde_pt(kernel_pt_phy + ptn);
        }
        return pt[pten];
    }

    void map_page(addr_t vn, addr_t pn) {
        get_pte(vn) = make_pte(pn);
        //koi.fmt("{} {} {08x}\n", ptn, pten, pt[pten]);
        //koi.fmt("map {08x} -> {08x}\n", vn<<12, pn<<12);
    }

    void map_pages(addr_t vn, addr_t pn, size_t count) {
        if (UNLIKELY(count == 0)) return;

        if (   is_divisible(vn,    1_K)
            && is_divisible(pn,    1_K)
            && is_divisible(count, 1_K)) {
            // TODO: 4M page optimization?
        }
        //koi.fmt("alloc {} pages\n", count);
        for (size_t i = 0; i < count; ++i)
            map_page(vn+i, pn+i);
    }

    void alloc_at(addr_t vn, size_t count) {
        if (UNLIKELY(count == 0)) return;

        auto pns = Mem::Pmm::alloc(count);
        if (pns) {
            for (size_t i = 0; i < count; ++i)
                get_pte(vn+i) = make_pte(*pns + i);
        } else {
            // No contiguous block available.
            // (should probably divide by two and try again...)
            for (size_t i = 0; i < count; ++i) {
                auto pn = Mem::Pmm::alloc(1);
                assert(pn, "could not allocate phy page");
                map_page(vn+i, *pn);
            }
        }
    }

    void unmap_page(addr_t vn) {
        get_pte(vn) = 0;
    }

    void free_page(addr_t vn) {
        Mem::Pmm::free(get_pte(vn) >> 12);
        unmap_page(vn);
    }

    pte_t *get_pt(addr_t tn) {
        return ((pte_t*)va_kernel_pts) + 1_K * tn;
    }

    void init() {
        // Paging is already enabled by bootstrap code.
        // We must recreate kernel mappings.

        // First off, some sanity checks.
        assert(is_divisible(kernel_lma(), granularity),
               "kernel is not loaded on a page boundary");
        assert(is_divisible((addr_t)&kernel_stack, granularity),
               "kernel stack is not page-aligned");

        // Enable page size extensions for 4M pages.
        // TODO: Should probably check (CPUID?) whether this is actually available.
        asm_cr4(asm_cr4() | 0x10);

        // Start building a new page directory.
        Mem::set(pd, (pde_t)0, 1_K);

        // Allocate 4M of physical memory for 1024 page tables.
        // 1024 4K pages, aka 4M bytes..
        auto kernel_pt_phy_ = Mem::Pmm::alloc(1_K, 32 /* XXX alignment is a hack */);
        assert(kernel_pt_phy_, "could not allocate kernel pt phy pages");

        // These are not mapped yet.
        kernel_pt_phy = *kernel_pt_phy_;

        // Monkey-patch current page directory so we can address these WIP tables.
        // ~~ If it's hacky and you know it, clap your hands. 👏 👏 ~~
        { pde_t *old_dir = (pde_t*)asm_cr3();
          old_dir[va_kernel_pts >> 22] = make_pde_4M(kernel_pt_phy);
        }
        // (we could instead have added 4M+alignment to .bss, but that stinks)

        // Insert them into the new page directory as well.
        pd[va_kernel_pts >> 22] = make_pde_4M(kernel_pt_phy);

        Mem::set((pte_t*)va_kernel_pts, (pde_t)0, 1_M);

        // Map the kernel.
        { map_pages(kernel_vma() / granularity
                   ,kernel_lma() / granularity
                   ,div_ceil(kernel_size(), granularity));

          // XXX: Kernel stack is located in bootstrap code.
          //      As such, the kernel_stack sym is a LMA, not a VMA (!)
          map_pages(va_kernel_stack           / granularity
                   ,(addr_t)&kernel_stack     / granularity
                   ,div_ceil(kernel_stack_size, granularity));
        }

        // Load the new page directory.
        asm_cr3(kva_to_pa((addr_t)pd));
    }
}
