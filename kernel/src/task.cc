/* Copyright (c) 2019, Chris Smeele
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
#include "task.hh"
#include "elf.hh"
#include "mem/vmm.hh"

namespace Task {

    // FIXME: These should not be simple incrementing counters...
    static int pcount = 0;
    static int tcount = 0;

    Either<const char*, own_ptr<task_t>>
    from_elf(vaddr_t elf_start, size_t elf_size) {

        auto *pd = Vmm::clone_pd();
        if (!pd) return Left("could not clone pdir");

        auto &current_pd = Vmm::current_pd();
        Vmm::switch_pd(*pd);

        auto entry_ = Elf::load(elf_start, elf_size);

        Vmm::switch_pd(current_pd);

        if (!entry_) return Left(entry_.left());

        auto task = own_ptr<task_t>(new task_t);

        // TODO: Remove magic numbers.
        task->stack = vaddr_t{0xc000'0000UL - 4_K};
        task->pc    = *entry_;
        task->pdir  = pd;
        task->pid   = ++pcount;
        task->tid   = ++tcount;

        return Right(move(task));
    }
}

Either<const char*, own_ptr<task_t>>
task_t::make_thread(vaddr_t entry, vaddr_t sp, size_t stack_size, size_t context1, size_t context2) {
    auto *t = new task_t();
    if (!t) { return Left("Could not allocate new task"); }
    t->pid       = pid;
    t->tid       = ++Task::tcount;
    t->pdir      = pdir;
    t->stack     = sp.offset(stack_size); // Invalid inputs will cause GPF/page faults in user-mode code.
    t->pc        = entry;
    t->frame     = frame;
    t->frame.eax = context1;
    t->frame.ebx = context2;
    t->started   = false;
    return Right(own_ptr<task_t>(t));
}
