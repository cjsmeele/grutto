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

    paddr_t kva_to_pa(vaddr_t va) { return paddr_t{ va.u() - kernel_vma().u() + kernel_lma().u() }; }

    pdir_t &current_pd() { return *(pdir_t*)va_pdir; }

    void switch_pd(paddr_t pa) { asm_cr3(pa.u()); }
    void switch_pd(pdir_t &pd) { switch_pd(kva_to_pa(vaddr_t{*pd})); }

    //pdir_t &current_pd(pdir_t &pd) {
    //    asm_cr3(kva_to_pa(vaddr_t{*pd}).u());
    //    return *(current_pd_ = &pd);
    //}

    pdir_t kernel_pd;
    ptab_t kernel_pt_ptab; // A pagetable for pages containing pagetables.

    static_assert(alignof(decltype(kernel_pd)) == page_size,
                  "Kernel page directory is misaligned");

    ppage_t pte_page(pde_t pde) { return ppage_t{pde >> 12}; }
    //addr_t pde_ctx    (pde_t pde)             { return (pde >> 9) & 0b111; }
    //void   pde_ctx    (pde_t pde, u8 val)     { pde = (pde & 0xfffff1fff) | ((val&0b111) << 9); }

    namespace Impl {

        pte_t make_pte(ppage_t pn, PageFlags flags) {
            return pte_t{   pn.u()} << 12
                   | (flags & P_Writable)
                   | (flags & P_User)
                   | 1; // present.
        }
        pde_t make_pde4M(ppage_t pn, PageFlags flags) {
            return pte_t{   pn.u()} << 12
                   | 0x80 // big page.
                   | (flags & P_Writable)
                   | (flags & P_User)
                   | 1;   // present.
        }
        pde_t make_pde_pt(ppage_t pt_pn, PageFlags flags) {
            return pde_t{pt_pn.u()} << 12
                   | (flags & P_Writable)
                   | (flags & P_User)
                   | 1; // present.
        }

        Optional<paddr_t> resolve_va(vaddr_t va) {
            auto &pd = *(pdir_t*)va_pdir;
            auto &pde = pd[va_to_ptn(va)];
            if (!(pde & 1)) return nullopt;
            auto &pt = *(ptab_t*)va_pts;
            auto &pte = pt[vpage_t{va}.u() % 1_K];
            if (!(pte & 1)) return nullopt;
            return paddr_t{pte_page(pte)}.offset(va.u() % page_size);
        }

        void flush() { switch_pd(*resolve_va(va_pdir)); }
        void invalidate(vpage_t vp) {
            asm volatile ("invlpg (%0)" :: "a" (vp.u()) : "memory");
        }
    }

    namespace User {
        // Manage user memory mappings.

        // TODO.
    }

    namespace Kernel {
        // Manage kernel memory mappings.
        // These functions operate regardless of the currently selected page
        // directory, as they modify only the kernel pagetables, which are
        // shared between all processes.

        // Get the page table for a virtual address in kernel memory area.
        // This cannot fail - all kernel pagetables are always present.
        ptab_t &get_ptab(vpage_t vp) {
            assert(vaddr_t{vp}.u() >= 0xc0000000, "bad kptab");
            return *(ptab_t*)va_pts.offset((vp.u() >> 10) * 4_K);
        }

        // Map a page in kernel memory.
        void map_one(vpage_t vp, ppage_t pp, PageFlags flags) {
            // NB: Not currently exposed.
            auto &pt  = get_ptab(vp);
            auto &pte = pt[vp.u() % 1_K];
            // koi.fmt("map page {} -> {} .. {08x} (@{}, -> {})\n",
            //         vaddr_t{vp}, paddr_t{pp}, pte, &pte,
            //         paddr_t{pte_page(pte)});
            assert(!(pte & 1), "tried to remap page in kernel memory - needs to be unmapped first");
            pte = Impl::make_pte(pp, flags);

            Impl::invalidate(vp);
        }

        void map(vpage_t vp, ppage_t pp, size_t count, PageFlags flags) {
            for (size_t i = 0; i < count; ++i)
                map_one(vp + i, pp + i, flags);
        }

        void unmap(vpage_t vp) {
            auto &pt  = get_ptab(vp);
            auto &pte = pt[vp.u() % 1_K];
            assert(pte & 1, "tried to unmap an already unmapped page in kernel memory");
            pte = 0;

            Impl::invalidate(vp);
        }
        void unmap(vpage_t vp, size_t count) {
            for (size_t i = 0; i < count; ++i)
                unmap(vp + i);
        }

        Optional<vpage_t> map_alloc(vpage_t vp, size_t count, PageFlags flags) {
            // Map memory backed by physical memory.
            if (UNLIKELY(count == 0)) return vpage_t{0};

            if (vaddr_t{vp + count}.u() > va_kernel_heap_end.u()) {
                return nullopt; // Out of memory.
            }

            auto pps = Pmm::alloc(count);
            if (pps.ok()) {
                map(vp, *pps, count, flags);
            } else {
                // No contiguous block available.
                // (should probably divide by two and try again...)
                for (size_t i = 0; i < count; ++i) {
                    auto pp = Pmm::alloc(1);
                    //assert(pp.ok(), "could not allocate phy page");
                    if (!pp.ok())
                        return nullopt;
                    map_one(vp+i, *pp, flags);
                }
            }
            return vp;
        }
        void unmap_free(vpage_t vp) {
            auto &pt  = get_ptab(vp);
            auto &pte = pt[vp.u() % 1_K];
            assert(pte & 1, "tried to free an already unmapped page in kernel memory");
            Pmm::free(pte_page(pte));
            unmap(vp);
        }
        void unmap_free(vpage_t vp, size_t count) {
            for (size_t i = 0; i < count; ++i)
                unmap_free(vp + i);
        }
    }

    // 4M-aligned pa of the kernel's page tables (to be allocated).
    auto pa_kernel_pts = paddr_t{0};

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

        // Fetch current page directory, as created by bootstrap code.
        // Note: This pd lives in the (currently) identity-mapped bootstrap area,
        // so we can safely address it using its LMA.
        auto &bootstrap_pd = *(pdir_t*)asm_cr3();

        // Start building a new page directory.
        kernel_pd.clear();
        kernel_pt_ptab.clear();

        // Insert the pagetables pagetable into the page directory.
        // Also store it in the bootstrap pd, so that we can use the pt
        // before switching over to the new page directory.
          kernel_pd   [va_to_ptn(va_pts)]
        = bootstrap_pd[va_to_ptn(va_pts)]
        = Impl::make_pde_pt(kva_to_pa(vaddr_t{*kernel_pt_ptab})
                           ,P_Writable);

        // Flush.
        switch_pd(paddr_t{*bootstrap_pd});

        // Allocate 1M of physical memory for 256 page tables.
        // These tables will be used to describe all kernel memory (>=0xc0000000)
        { auto kernel_pt_phy_ = Pmm::alloc(256);
          assert(kernel_pt_phy_.ok(), "could not allocate kernel pt phy pages");
          pa_kernel_pts = paddr_t{*kernel_pt_phy_}; }

        // Insert phy addresses of kernel pagetables into the pagetable,
        // and mark all kernel pagetables as present in the page directory.
        for (size_t i = 0; i < 256; ++i) {
            auto j = 768 + i;
            // Do not overwrite existing mappings.
            // (specifically, do not overwrite the va_pts mapping)
            if (!(kernel_pd[j] & 1))
                kernel_pd[j] = Impl::make_pde_pt(pa_kernel_pts.offset(i * page_size)
                                                ,P_Writable);
            kernel_pt_ptab[j] = Impl::make_pte(pa_kernel_pts.offset(i * page_size)
                                              ,P_Writable);
            ((ptab_t*)va_pts)[j].clear();
        }

        // Now the entire kernel memory region is supplied with pagetables,
        // making kernel memory allocation quite a bit simpler for us.
        // Regular Vmm functions to manage kernel memory mappings are now
        // available to us.

        // Map the page directory at a known location.
        Kernel::map_one(va_pdir, kva_to_pa(*kernel_pd), P_Writable | P_Supervisor);

        // Map the kernel.

        Kernel::map(kernel_text_vma()
                   ,kva_to_pa(kernel_text_vma())
                   ,div_ceil(kernel_text_size(), page_size)
                   ,P_ReadOnly | P_Supervisor);

        Kernel::map(kernel_rodata_vma()
                   ,kva_to_pa(kernel_rodata_vma())
                   ,div_ceil(kernel_rodata_size(), page_size)
                   ,P_ReadOnly | P_Supervisor);

        Kernel::map(kernel_data_vma()
                   ,kva_to_pa(kernel_data_vma())
                   // Include bss.
                   ,div_ceil(kernel_end_vma().align_up(page_size).u()
                             - kernel_data_vma().align_down(page_size).u()
                            ,page_size)
                   ,P_Writable | P_Supervisor);

        // XXX: Kernel stack is located in bootstrap code.
        //      As such, the kernel_stack sym is a LMA, not a VMA (!)
        Kernel::map(va_kernel_stack
                   ,paddr_t{&kernel_stack}
                   ,div_ceil(kernel_stack_size, page_size)
                   ,P_Writable | P_Supervisor);

        // Load the new page directory.
        switch_pd(kernel_pd);
    }
}
