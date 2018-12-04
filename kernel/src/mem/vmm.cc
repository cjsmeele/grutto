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

namespace Mem::Vmm {

    using pde_t = u32;
    using pte_t = u32;

    alignas(4_K) pde_t pd[1_K] { }; // A page directory holds 1024 entries.

    // TODO: Remove magic numbers, create helpers for building PDEs and PTEs.

    addr_t pde_pte_addr   (pde_t pde)             { return (pde >> 12) << 12; }
    void   pde_pte_addr   (pde_t pde, addr_t val) { pde = (pde & 0xfffff000) | (val << 12); }
    addr_t pde_pte_ctx    (pde_t pde)             { return (pde >> 9) & 0b111; }
    void   pde_pte_ctx    (pde_t pde, u8 val)     { pde = (pde & 0xfffff1fff) | ((val&0b111) << 9); }
    bool   pde_pte_present(pde_t pde)             { return (pde & 1); }
    void   pde_pte_present(pde_t pde, bool val)   { pde = (pde & ~1) | !!val; }

    addr_t va_to_klma(addr_t va) { return va - kernel_vma() + kernel_lma(); }

    pte_t make_pde_4M(addr_t pn)    { return    pn << 12 | 0x80 | 3; }
    pte_t make_pde_pt(addr_t pt_pn) { return pt_pn << 12 | 3; }
    pte_t make_pte(addr_t pn)       { return    pn << 12 | 3; }

    addr_t kernel_pt_phy = 0;

    pte_t &get_pte(addr_t vn) {
        u32 ptn  = vn >> 10;
        u32 pten = vn & 0x3ff;
        pte_t *pt = nullptr;
        pt = (pte_t*)kernel_pts + ptn*1_K;
        if (pd[ptn] & 1) {
            assert((pd[ptn] & 0x80) == 0, "already 4M");
        } else {
            pd[ptn] = make_pde_pt(kernel_pt_phy + ptn);
        }
        return pt[pten];
    }

    void map_page(addr_t vn, addr_t pn) {
        get_pte(vn) = make_pte(pn);
        //koi.fmt("{} {} {08x}\n", ptn, pten, pt[pten]);
        //koi.fmt("{08x} -> {08x}\n", vn, pn);
    }
    void map_pages(addr_t vn, addr_t pn, size_t count) {
        if (is_divisible(vn, 1_K) && is_divisible(pn, 1_K) && is_divisible(count, 1_K)) {
            // bla.
        }
        //koi.fmt("alloc {} pages\n", count);
        for (size_t i = 0; i < count; ++i)
            map_page(vn+i, pn+i);
    }
    void alloc_at(addr_t vn, size_t count) {
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
        return ((pte_t*)kernel_pts) + 1_K * tn;
    }

    void init() {
        // Paging is already enabled by bootstrap code.
        // We must recreate kernel mappings.

        // Enable page size extensions for 4M pages.
        u32 cr4;
        asm volatile ("mov %%cr4, %0" : "=b"(cr4));
        cr4 |= 0x10;
        asm volatile ("mov %0, %%cr4" :: "b"(cr4));

        // Reserve 4M physical memory for 1024 page tables.
        auto kernel_pt_phy_ = Mem::Pmm::alloc(1_K, 32 /*XXX HACK*/); // 1024 4K pages, aka 4M bytes..
        assert(kernel_pt_phy_, "could not allocate kernel pt phy pages");

        kernel_pt_phy = *kernel_pt_phy_;

        // Monkey-patch current page directory so we can address these WIP tables.
        // ~~ If it's hacky and you know it, clap your hands. *clap* *clap* ~~
        { u32 cr3 = asm_read_cr3();
          pde_t *dir = (pde_t*)cr3;
          dir[kernel_pts >> 22] = make_pde_4M(kernel_pt_phy);
        }

        Mem::set((pte_t*)kernel_pts, (pde_t)0, 1_M);

        Mem::set(pd, (pde_t)0, 1_K);
        pd[kernel_pts >> 22] = make_pde_4M(kernel_pt_phy);

        auto kernel_pt = get_pt(kernel_vma() >> 22);
        auto stack_pt  = get_pt(0x3ff);

        for (size_t i = 0; i < 1_K; ++i)
            kernel_pt[i] = make_pte((kernel_lma() >> 12) + i);

        stack_pt[1008] = make_pte(((addr_t)&kernel_stack >> 12));
        stack_pt[1009] = make_pte(((addr_t)&kernel_stack >> 12) + 1);
        stack_pt[1010] = make_pte(((addr_t)&kernel_stack >> 12) + 2);
        stack_pt[1011] = make_pte(((addr_t)&kernel_stack >> 12) + 3);

        // Identity-map first 4M.
        pd[   0] = make_pde_4M(0x00000000);
        pd[ 768] = make_pde_pt(kernel_pt_phy + 768);
        pd[1023] = make_pde_pt(kernel_pt_phy + 1023);

        // Load the new page directory.
        asm volatile ("mov %0, %%cr3" :: "b" ((va_to_klma((addr_t)pd) >> 12) << 12));
    }
}
