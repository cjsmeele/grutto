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

    static int count = 0;

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
        task->id    = ++count;

        return Right(move(task));
    }

}
