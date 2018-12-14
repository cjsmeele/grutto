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

    //alignas(page_size) pde_t kernel_pd[1_K] { }; // A page directory holds 1024 pdes.
    //alignas(page_size) Array<pde_t,1_K> kernel_pd; // A page directory holds 1024 pdes.

    pdir_t *current_pd_ = nullptr;

    pdir_t &current_pd()           { return * current_pd_; }
    pdir_t &current_pd(pdir_t &pd) {
        asm_cr3(kva_to_pa(vaddr_t{*pd}).u());
        return *(current_pd_ = &pd);
    }

    pdir_t kernel_pd;

    static_assert(alignof(decltype(kernel_pd)) == 4_K,
                  "Kernel page directory is misaligned");

    //addr_t pde_pte_addr   (pde_t pde)             { return addr_t{ (pde >> 12) << 12 }; }
    //void   pde_pte_addr   (pde_t pde, addr_t val) { pde = (pde & 0xfffff000) | (pde_t{val.u()} << 12); }
    //addr_t pde_pte_ctx    (pde_t pde)             { return (pde >> 9) & 0b111; }
    //void   pde_pte_ctx    (pde_t pde, u8 val)     { pde = (pde & 0xfffff1fff) | ((val&0b111) << 9); }
    //bool   pde_pte_present(pde_t pde)             { return (pde & 1); }
    //void   pde_pte_present(pde_t pde, bool val)   { pde = (pde & ~1) | !!val; }

    paddr_t kva_to_pa(vaddr_t va) { return paddr_t{ va.u() - kernel_vma().u() + kernel_lma().u() }; }

    pte_t make_pde_4M(ppage_t pn)    { return pte_t{   pn.u()} << 12 | 0x80 | 3; }
    pte_t make_pde_pt(ppage_t pt_pn) { return pte_t{pt_pn.u()} << 12 | 3; }
    pte_t make_pte(ppage_t pn)       { return pte_t{   pn.u()} << 12 | 3; }

    // 4M-aligned pa of the kernel's page tables (to be allocated).
    auto pa_kernel_pts = paddr_t{0};

    // Optional<addr_t> va_to_pa(addr_t va)  { return page_t{va}.u() >> 10; }
    // Optional<page_t> vp_to_pp(page_t vp)  {
    //     // TODO
    //     return {get_pte(vp) >> 12} page_t{va}.u() >> 10;
    // }

    pte_t &get_pte(vpage_t vn) {
        // Obtain page table entry for the given virtual address.
        u32 ptn   = vn.u() >> 10;   // pde / page table number.
        u32 pten  = vn.u() & 0x3ff; // pte / page number.
        pte_t *pt = (pte_t*)va_kernel_pts + ptn*1_K;
        if (kernel_pd[ptn] & 1) { // pde present?
            assert((kernel_pd[ptn] & 0x80) == 0,
                   "Cannot split up 4M mapping to get a PTE");
            // To handle above case, we should split the 4M mapping into 4K mappings.
        } else {
            // pde not present, create it.
            kernel_pd[ptn] = make_pde_pt(pa_kernel_pts.offset(ptn*4_K));
        }
        return pt[pten];
    }

    void map_page(vpage_t vn, ppage_t pn) {
        get_pte(vn) = make_pte(pn);
        // koi.fmt("map {08x} -> {08x} [{08x}]\n",
        //         addr_t{vn},
        //         addr_t{pn},
        //         get_pte(vn));
    }

    void map_pages(vpage_t vn, ppage_t pn, size_t count) {
        if (UNLIKELY(count == 0)) return;

        if (   is_divisible(vn.u(), 1_K)
            && is_divisible(pn.u(), 1_K)
            && is_divisible(count,  1_K)) {
            // TODO: 4M page optimization?
        }
        //koi.fmt("alloc {} pages\n", count);
        for (size_t i = 0; i < count; ++i)
            map_page(vn+i, pn+i);
    }

    void alloc_at(vpage_t vn, size_t count) {

        if (UNLIKELY(count == 0)) return;

        auto pns = Pmm::alloc(count);
        if (pns) {
            for (size_t i = 0; i < count; ++i)
                get_pte(vn+i) = make_pte(*pns + i);
        } else {
            // No contiguous block available.
            // (should probably divide by two and try again...)
            for (size_t i = 0; i < count; ++i) {
                auto pn = Pmm::alloc(1);
                assert(pn.ok(), "could not allocate phy page");
                map_page(vn+i, *pn);
            }
        }
    }

    void unmap_page(vpage_t vn) {
        get_pte(vn) = 0;
    }

    void free_page(vpage_t vn) {
        Pmm::free(ppage_t {get_pte(vn) >> 12});
        unmap_page(vn);
    }

    pte_t *get_pt(size_t tn) {
        return ((pte_t*)va_kernel_pts) + 1_K * tn;
    }

    void init() {
        // Paging is already enabled by bootstrap code.
        // We must recreate kernel mappings.

        // First off, some sanity checks.
        assert(kernel_lma().is_aligned(page_size),
               "kernel is not loaded on a page boundary");
        assert(paddr_t{&kernel_stack}.is_aligned(page_size),
               "kernel stack is not page-aligned");

        // Enable page size extensions for 4M pages.
        // TODO: Should probably check (CPUID?) whether this is actually available.
        asm_cr4(asm_cr4() | 0x10);

        // Fetch current page directory, created by bootstrap code.
        current_pd_ = (pdir_t*)asm_cr3(); // eww.

        // Start building a new page directory.
        kernel_pd.clear();

        // Allocate 4M of physical memory for 1024 page tables.
        // 1024 4K pages, aka 4M bytes.
        auto kernel_pt_phy_ = Pmm::alloc(1_K, 32 /* XXX alignment is a hack */);
        assert(kernel_pt_phy_.ok(), "could not allocate kernel pt phy pages");

        // These are not mapped yet.
        pa_kernel_pts = paddr_t{*kernel_pt_phy_};

        assert(pa_kernel_pts.is_aligned(4_M),
               "could not 4M-align kernel pt phy pages");

        // Map them in the current page directory.
        current_pd()[va_to_ptn(va_kernel_pts)] = make_pde_4M(pa_kernel_pts);
        //map_pages(va_to_ptn(va_kernel_pts)
        //         ,pa_kernel_pts
        //         ,1_K);

        // Insert them into the new page directory as well.
        kernel_pd[va_to_ptn(va_kernel_pts)] = make_pde_4M(pa_kernel_pts);

        Mem::set((pte_t*)va_kernel_pts, (pde_t)0, 1_M);

        // Map the kernel.
        { map_pages(kernel_vma()
                   ,kernel_lma()
                   ,div_ceil(kernel_size(), page_size));

          // XXX: Kernel stack is located in bootstrap code.
          //      As such, the kernel_stack sym is a LMA, not a VMA (!)
          map_pages(va_kernel_stack
                   ,paddr_t{&kernel_stack}
                   ,div_ceil(kernel_stack_size, page_size));
        }

        // Load the new page directory.
        //asm_cr3(kva_to_pa(vaddr_t{*kernel_pd}).u());
        current_pd(kernel_pd);
    }
}
