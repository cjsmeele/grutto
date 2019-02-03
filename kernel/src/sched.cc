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
#include "sched.hh"
#include "mem/vmm.hh"
#include "gdt.hh"

namespace Sched {

    own_ptr<task_t> current_task_ = nullptr;
    task_t *current_task() { return *current_task_; }

    Queue<own_ptr<task_t>, 64> ready_queue;

    void add_task(own_ptr<task_t> task) {

        ready_queue.enqueue(move(task));
    }

    void switch_task() {
        if (LIKELY(current_task())) {
            // TODO.
        } else {
            assert(ready_queue.length(), "no task to switch to");
            auto &next = current_task_ = ready_queue.dequeue();

            Vmm::switch_pd(*next->pdir);
            Gdt::set_tss_sp(next->kstack_top);

            asm volatile (// Set user-mode segments.
                          "mov %[udn], %%eax \n"
                          "mov %%ax,   %%ds \n"
                          "mov %%ax,   %%es \n"
                          "mov %%ax,   %%fs \n"
                          "mov %%ax,   %%gs \n"
                          "mov %%ax,   %%ax \n"
                          // Push user data sel & task's SP.
                          "push %%eax \n"
                          "push %[stack] \n"
                          "pushf \n"
                          // Push user code sel & task's PC.
                          "push %[ucn]\n"
                          "push %[pc]\n"
                          //"xchgw %%bx,%%bx\n"
                          "iret"
                         : // TODO: Remove magic numbers.
                         : [ucn]    "i" (3*8+3)
                         , [udn]    "i" (4*8+3)
                         , [stack]  "b" (next->stack.u())
                         , [pc]     "d" (next->pc.u())
                         : "memory","eax");
        }
    }

    void init() {
        // TODO.
    }
}
