/* Copyright (c) 2018, 2019, Chris Smeele
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
// #include "../sched.hh"

extern unusable_t kernel_stack;

namespace Vmm {

    // Resolve a va without looking it up in paging structures.
    // This is only possible (and necessary) during initial paging setup.
    // It also only works for data that is within (ro)data or bss sections,
    // that is, it cannot be used for dynamically allocated and
    // automatic-storage variables.
    paddr_t kstatic_va_to_pa(vaddr_t va) {
        return paddr_t{ va.u() - kernel_vma().u() + kernel_lma().u() };
    }

    void flush_tlb() { asm_cr3(asm_cr3()); }

    pdir_t kernel_pd;

    // Provided for those moments where the current page directory is not yet
    // mapped at va_pdir, and you need to map it.
    // This does not affect any mappings that may be made in userspace memory,
    // so be careful to only use it during page directory switches.
    // This is marked const because we must guarantee that no changes to the
    // pdir are made when override_current_pdir is in effect (but it cannot be
    // enforced by C++) - see also the ugly const_cast below.
    //
    // We would not need this if we had a "remap" function to replace a pdir mapping.
    // Something to consider...
    // (or in general: if we keep track of the type of mapping using context bits,
    // we can safely remap the pdir at va_pdir without needing this override)
    const pdir_t *override_current_pdir = nullptr;

    pdir_t &current_pd() {
        if (UNLIKELY(override_current_pdir != nullptr))
             return *const_cast<pdir_t*>(override_current_pdir);
        else return *(pdir_t*)va_pdir;
        // if (Sched::current_task())
        //      return *Sched::current_task()->pdir;
        // else return kernel_pd;
    }

    static_assert(alignof(decltype(kernel_pd)) == page_size,
                  "Kernel page directory is misaligned");

    ppage_t pte_page(pte_t pte) { return ppage_t{pte >> 12}; }
    //addr_t pde_ctx    (pde_t pde)             { return (pde >> 9) & 0b111; }
    //void   pde_ctx    (pde_t pde, u8 val)     { pde = (pde & 0xfffff1fff) | ((val&0b111) << 9); }

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
        auto &pd  = current_pd();
        auto &pde = pd[va_to_ptn(va)];
        if (UNLIKELY(!pde_present(pde))) return nullopt;

        auto &pt = *(ptab_t*)va_pts;
        auto &pte = pt[vpage_t{va}.u()];
        if (UNLIKELY(!pte_present(pte))) return nullopt;

        return paddr_t{pte_page(pte)}.offset(va.u() % page_size);
    }

    // An alias. The name you use to get this functionality specifies your intent.
    Optional<paddr_t> va_to_pa(vaddr_t va) { return resolve_va(va); }

    void invalidate(vpage_t vp) {
        asm volatile ("invlpg (%0)" :: "a" (vp.u()) : "memory");
    }

    // Get the page table for a virtual address in kernel memory area.
    // This cannot fail - all kernel pagetables are always present.
    inline ptab_t &get_kptab(vpage_t vp) {
        return *(ptab_t*)va_pts.offset(vp_to_ptn(vp) * 4_K);
    }

    ptab_t *get_ptab(vpage_t vp) {

        // Short-circuit for page tabels that are guaranteed to be present.
        if (vaddr_t{vp}.u() >= 0xc0000000) return &get_kptab(vp);

        // Page table number.
        auto ptn = vp_to_ptn(vp);

        ptab_t *ptabp = (ptab_t*)va_pts.offset(ptn * 4_K);

        auto &pde = current_pd()[ptn];

        // Pagetable exists, we are done.
        if (LIKELY(pde_present(pde)))
            return ptabp;

        // Otherwise, the pagetable must be allocated and mapped.

        // koi(LL::debug).fmt("alloc ptab @{} for vp {} (ptn {})\n",
        //                    ptabp, vaddr_t{vp}, vp_to_ptn(vp));

        // Allocate a phy page (pp) - this will be the pagetable.
        auto pp = Pmm::alloc(1);
        if (UNLIKELY(!pp.ok())) {
            koi(LL::critical).fmt("failed to allocate phy page for new page table");
            return nullptr;
        }

        // In va_pts, map the ptn'th page to point to pp - so we can access the page.
        map(vaddr_t{**ptabp}, *pp, 1, P_Supervisor | P_Writable);

        // Clear the pagetable using its va_pts relative address.
        ptabp->clear();

        // In the pdir, insert pp at ptn.
        pde = make_pde_pt(*pp, P_User | P_Writable);

        flush_tlb();

        return ptabp;
    }

    // Map a page.
    void map_one(vpage_t vp, ppage_t pp, PageFlags flags) {

        auto *pt_ = get_ptab(vp);
        // TODO: Kill a task instead of panicking.
        assert(pt_, "could not allocate pagetable");
        auto &pt  = *pt_;
        auto &pte = pt[vp.u() % 1_K];

        // koi(LL::debug).fmt("map page {} -> {}\n", vaddr_t{vp}, paddr_t{pp});

        // FIXME: This isn't a hard error. Allow for error reporting.
        // If we allowed this, we would need to know whether the existing page
        // "owns" the associated phy page - so that we may free that.
        assert(!pte_present(pte), "tried to remap a page - it needs to be unmapped first");
        pte = make_pte(pp, flags);

        invalidate(vp);
    }

    void map(vpage_t vp, ppage_t pp, size_t count, PageFlags flags) {
        for (size_t i = 0; i < count; ++i)
            map_one(vp + i, pp + i, flags);
    }

    void unmap(vpage_t vp) {
        auto *pt  = get_ptab(vp); // XXX: This may allocate!
        if (!pt) return;
        auto &pte = (*pt)[vp.u() % 1_K];
        if (!pte_present(pte)) return;

        pte = 0;

        invalidate(vp);

        // Note: In the future we may clean up userspace pagetables when they
        // are no longer needed - that is, when all their ptes are absent.
        // The benefits may not be worth it however, as user paging structs
        // take up a max of only 3M.
    }
    void unmap(vpage_t vp, size_t count) {
        for (size_t i = 0; i < count; ++i)
            unmap(vp + i);
    }

    Optional<vpage_t> map_alloc(vpage_t vp, size_t count, PageFlags flags) {
        // Map memory backed by physical memory.

        // XXX: Ugly pretend-succes case.
        if (UNLIKELY(count == 0)) return vpage_t{0};

        if (vaddr_t{vp + count}.u() > va_kernel_heap_end.u())
            return nullopt; // Out of memory.

        // Allocate physical pages.
        auto pps = Pmm::alloc(count);
        if (pps.ok()) {
            map(vp, *pps, count, flags);
        } else {
            // No contiguous physical memory region available.
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
        auto *pt  = get_ptab(vp); // XXX: This may allocate.
        if (!pt) return;
        auto &pte = (*pt)[vp.u() % 1_K];
        if (!pte_present(pte)) return;

        Pmm::free(pte_page(pte));
        unmap(vp);
    }
    void unmap_free(vpage_t vp, size_t count) {
        // NB: Freeing blocks of physical memory is currently expensive.
        //     It might be cheaper if we kept track of virt&phy regions that
        //     can be freed at once - avoiding separate calls to the Pmm.
        for (size_t i = 0; i < count; ++i)
            unmap_free(vp + i);
    }

    // Map the page directory at a known location.
    void map_pdir(const pdir_t &pdir, paddr_t pa) {
        override_current_pdir = &pdir;
        if (resolve_va(va_pdir).ok())
            unmap(va_pdir);
        map_one(va_pdir, pa, P_Writable | P_Supervisor);
        override_current_pdir = nullptr;
    }
    void map_pdir(const pdir_t &pdir) {
        // Note - passing pdir by ref/va requires a lookup - this is NOT
        // possible while we are still on the bootstrap page dir, as that dir
        // doesn't have itself mapped recursively.
        auto pa = *resolve_va(*pdir);
        map_pdir(pdir, pa);
    }

    void switch_pd(paddr_t pa) { asm_cr3(pa.u()); }
    void switch_pd(const pdir_t &pd) {
        // Note - passing pdir by ref/va requires a lookup - this is NOT
        // possible while we are still on the bootstrap page dir, as that dir
        // doesn't have itself mapped recursively.

        CRITICAL_SCOPE();
        //koi(LL::debug).fmt("switching pd? va {}\n", *pd);
        auto x = *resolve_va(*pd);
        //koi(LL::debug).fmt("switching pd to phy {}\n", x);
        switch_pd(x);
    }

    pdir_t* clone_pd() {
        auto *pdir_ = new pdir_t;
        if (!pdir_) {
            koi(LL::critical).fmt("failed to allocate page directory for clone_pdir()");
            return nullptr;
        }
        pdir_t &pdir = *pdir_;
        auto pdir_pa = *resolve_va(*pdir);

        pdir_t const &current_pdir = current_pd();

        for (size_t i = 0; i < 1_K; ++i) {
            if (i < 768) {
                if (pde_present(current_pdir[i]))
                    koi.fmt("present userspace pde: {08x} @{}\n", current_pdir[i], &current_pdir[i]);
                assert(!pde_present(current_pdir[i]),
                       "TODO: Copy pagetables and phy pages for userspace stuff?");
                pdir[i] = 0;
            } else {
                pdir[i] = current_pdir[i];
                // kernel ptabs need not be cloned, they are the same for all tasks.
            }
        }

        // Map the page directory in itself.
        // (this may not be necessary - we can keep track of the current va
        // of the pdir in other ways, e.g. in task structs)
        CRITICAL_SCOPE();
        switch_pd(pdir);
        map_pdir(pdir);
        switch_pd(current_pdir);

        return pdir_;
    }

    // 4M-aligned pa of the kernel's page tables (to be allocated).
    auto pa_kernel_pts = paddr_t{0};

    void init() {
        // Paging is already enabled by bootstrap code.
        // We must recreate kernel mappings.

        CRITICAL_SCOPE();

        // First off, some sanity checks.
        assert(kernel_lma().is_aligned(page_size),
               "kernel is not loaded on a page boundary");
        assert(paddr_t{(size_t)&kernel_stack}.is_aligned(page_size),
               "kernel stack is not page-aligned");

        // Enable page size extensions for 4M pages.
        // TODO: Should probably check (CPUID?) whether this is actually available.
        //asm_cr4(asm_cr4() | 0x10);
        // (not currently used)

        // Fetch current page directory, as created by bootstrap code.
        // Note: This pd lives in the (currently) identity-mapped bootstrap area,
        // so we can safely address it using its LMA.
        auto &bootstrap_pd = *(pdir_t*)asm_cr3();

        // Start building a new page directory.
        kernel_pd.clear();

        // We want to create a 4M virtual region for recursive page mappings, but we
        // run into a chicken-egg problem there. How do we create a page-table
        // for our page table mappings if we need those page tables to be
        // accessible (mapped) in order to access our page table?
        // (ok that text wasn't clear at all)
        // The point is, we need a temporary pagetable that is *already* mapped.
        // so we use init_pt, which is mapped in our data region.
        static ptab_t init_pt;
        init_pt.clear();

        // We make init_pt describe the recursive page mapping region.
        bootstrap_pd[va_to_ptn(va_pts)]
            = make_pde_pt(kstatic_va_to_pa(*init_pt), P_Supervisor | P_Writable);

        flush_tlb();

        // A utility function for looping over kernel pagetables.
        auto for_kpts = [](auto fn) {
            for (size_t kpt_no = 0; kpt_no < 256; ++kpt_no) {
                auto ptn = 768 + kpt_no;
                auto pt_va = vaddr_t {*(((ptab_t*)va_pts)[ptn])};
                fn(ptn, kpt_no, pt_va);
            }
        };

        // Allocate 1M of physical memory for 256 page tables.
        // These tables will be used to describe all kernel memory (>=0xc0000000)
        { auto kernel_pt_phy_ = Pmm::alloc(256);
          assert(kernel_pt_phy_.ok(), "could not allocate kernel pt phy pages");
          pa_kernel_pts = paddr_t{*kernel_pt_phy_}; }

        // Map our freshly allocated kernel pagetables into the bootstrap pdir
        // via init_pt, so that we can use them for mapping immediately.
        for_kpts([&] (auto ptn, auto kpt_no, auto pt_va) {

            init_pt[ptn] = make_pte(pa_kernel_pts.offset(kpt_no * page_size), P_Writable);
            invalidate(pt_va);
        });

        // init_pt is complete, so all kernel pts are now accessible from the bootstrap pdir.

        // Now that they are mapped, clear the allocated pagetables.
        for_kpts([&] (auto ptn, auto kpt_no, auto pt_va) {
            (*(ptab_t*)pt_va).clear();
        });

        // Insert the pagetables into the new page directory.
        for_kpts([&] (auto ptn, auto kpt_no, auto pt_va) {
            kernel_pd[ptn] = make_pde_pt(pa_kernel_pts.offset(kpt_no * page_size), P_Writable);
        });

        // Next, we need to copy init_pt to the correct allocated page table.
        // (we could keep using init_pt's phy address and allocate one less phy
        // page, but this makes it more uniform).

        (*(ptab_t*)va_pts.offset(va_to_ptn(va_pts) * page_size)) = init_pt;

        // Map the page directory at a known location.
        // (thereby we pretend that we are already on the new pdir)
        map_pdir(kernel_pd, kstatic_va_to_pa(*kernel_pd));

        // With all that in place, we can now map the kernel in the new page directory.

        map(kernel_text_vma()
           ,kstatic_va_to_pa(kernel_text_vma())
           ,div_ceil(kernel_text_size(), page_size)
           ,P_ReadOnly | P_Supervisor);

        map(kernel_rodata_vma()
           ,kstatic_va_to_pa(kernel_rodata_vma())
           ,div_ceil(kernel_rodata_size(), page_size)
           ,P_ReadOnly | P_Supervisor);

        map(kernel_data_vma()
           ,kstatic_va_to_pa(kernel_data_vma())
           // Include bss.
           ,div_ceil(kernel_end_vma().align_up(page_size).u()
                     - kernel_data_vma().align_down(page_size).u()
                    ,page_size)
           ,P_Writable | P_Supervisor);

        // Kernel stack is located in bootstrap code.
        // As such, the kernel_stack sym is a LMA, not a VMA (!)
        map(va_kernel_stack
           ,paddr_t{(size_t)&kernel_stack}
           ,div_ceil(kernel_stack_size, page_size)
           ,P_Writable | P_Supervisor);

        // Load the new page directory.
        // Firstly, switch by specifying the pa explicitly.
        switch_pd(kstatic_va_to_pa(*kernel_pd));

        // Secondly, switch specifying the va. The lookup should succeed now
        // that we have switched off the bootstrap pagedir.
        // (we do this as a sanity check - it shouldn't have any effect if we did everything right)
        switch_pd(kernel_pd);
    }
}
