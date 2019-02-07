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
#include "int/handlers.hh" // XXX

namespace Sched {

    own_ptr<task_t> current_task_ = nullptr;
    task_t *current_task()                     { return *current_task_; }
    task_t *current_task(own_ptr<task_t> task) { return *(current_task_ = move(task)); }

    Queue<own_ptr<task_t>, 64> ready_queue;

    void switch_task(own_ptr<task_t> task, Int::interrupt_frame &frame) {

        // This switches address spaces and switches out the interrupt frame,
        // such that when iret is invoked, we continue the given task.

        if (UNLIKELY(current_task() == nullptr)) {
            Vmm::switch_pd(*task->pdir);
            Gdt::set_tss_sp(task->kstack_top);
        }

        //koi.fmt("sw<{}>", task->id);

        current_task_->stack = vaddr_t{frame.esp};
        current_task_->pc    = vaddr_t{frame.eip};
        current_task_->frame = frame;

        auto current_pdir = current_task_->pdir;

        // XXX: This assumption is invalid for blocking syscalls
        ready_queue.enqueue(move(current_task_));

        auto &next = current_task_ = move(task);

        if (next->pdir != current_pdir)
            // Prevent unnecessary TLB flushes when switching to
            // other threads within the same process.
            Vmm::switch_pd(*next->pdir);

        Gdt::set_tss_sp(next->kstack_top);

        if (LIKELY(next->started)) {
            // Restore state of the task we are resuming.
            auto int_no   = frame.int_no;
            frame         = next->frame;
            frame.int_no  = int_no; // Make sure we return from the current interrupt correctly.
        } else {
            // No state to restore - create one based on the task's entrypoint.
            frame.edi     = next->frame.edi;
            frame.ebp     = next->frame.ebp;
            frame.ebx     = next->frame.ebx;
            frame.edx     = next->frame.edx;
            frame.ecx     = next->frame.ecx;
            frame.eax     = next->frame.eax;
            frame.eip     = next->pc.u();
            frame.useresp = next->stack.u();
            next->started = true;
        }
    }

    void exec_task(own_ptr<task_t> task) {
        assert(!current_task(), "exec_task(): current_task not null");

        auto &next = current_task_ = move(task);

        Vmm::switch_pd(*next->pdir);
        Gdt::set_tss_sp(next->kstack_top);

        next->started = true;

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
                      "pop  %%eax \n"
                      // Set IE flag.
                      "orl  $0x200, %%eax  \n"
                      "push %%eax \n"
                      // Push user code sel & task's PC.
                      "push %[ucn]\n"
                      "push %[pc]\n"
                      //"xchgw %%bx,%%bx\n"
                      "iret"
                     : // TODO: Remove magic numbers.
                     : [ucn]    "i" (3*8+3)
                     , [udn]    "i" (4*8+3)
                     , [stack]  "b" (next->stack.u())
                     , [pc]     "d" (next->pc.u()));
    }

    void add_task(own_ptr<task_t> task) {
        ready_queue.enqueue(move(task));
    }

    void maybe_switch_task(Int::interrupt_frame &frame) {
        // No need to switch if there aren't any other ready tasks.
        if (UNLIKELY(ready_queue.length() == 0)) return;

        // TODO: Provide a way for processes to enter a blocking state.
        //       (this would also require some sort of "idle" process.

        switch_task(ready_queue.dequeue(), frame);
    }

    void init() {
    }
}
