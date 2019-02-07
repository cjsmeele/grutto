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
#pragma once

#include "mem/vmm.hh"
#include "int/handlers.hh"

struct task_t {
    using stack_word_t = int;
    constexpr static size_t per_task_kernel_stack_words = 256;

    bool started = false;

    int pid;
    int tid;
    Vmm::pdir_t *pdir;
    vaddr_t stack;
    vaddr_t pc;

    Int::interrupt_frame frame;

    // NOTE: Per-task kernel stack should be mostly useless until we allow the
    //       kernel to be interrupted during syscalls.
    stack_word_t  kstack_[per_task_kernel_stack_words];
    stack_word_t *kstack_top = &kstack_[per_task_kernel_stack_words];

    // A new thread re-uses the address space of the existing task.
    // Threads only need their own stack and registers.
    Either<const char*, own_ptr<task_t>>
    make_thread(vaddr_t entry,
                vaddr_t sp,
                size_t stack_size,
                size_t context1,
                size_t context2);
};

namespace Task {
    Either<const char*, own_ptr<task_t>>
    from_elf(vaddr_t elf_start, size_t elf_size);
}
