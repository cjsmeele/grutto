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
#include "syscall.hh"
#include "sched.hh"
#include <syscalls.h>

namespace Syscall {

    using handler = void (*)(Int::interrupt_frame&);

    // Don't want to redefine λ, so let's pick a lookalike ;-)
#define ト(...) [] (Int::interrupt_frame &f) -> void { __VA_ARGS__; }

    // TODO: Figure out a good error reporting system for this.
    //       (maybe start defining error numbers?)

    void (*handlers[SYS_COUNT_])(Int::interrupt_frame&)
    {/* SYS_NOP           */ ト()
                             // XXX
                             // Reading and printing random strings from userspace-supplied
                             // addresses seems like a safe thing to do ;^)
    ,/* SYS_DBG_PRINT     */ ト(koi.fmt("{}", (const char*)f.ebx))
    ,/* SYS_DBG_PRINT_NUM */ ト(koi.fmt("{}", (u32)f.ebx))
    ,/* SYS_DBG_HALT      */ ト(koi.fmt("\nkernel: halting on user request.\n"); hang())
                             // End the current task's timeslice.
    ,/* SYS_YIELD         */ ト(Sched::maybe_switch_task(f))
    ,/* SYS_THREAD_CREATE */ ト(
        auto pc         = f.ebx;
        auto stack      = f.ecx;
        auto stack_size = f.edx;
        auto context1   = f.esi;
        auto context2   = f.edi;

        Sched::current_task()->make_thread(vaddr_t{pc},
                                           vaddr_t{stack},
                                           stack_size,
                                           context1,
                                           context2)
            .then_do(λx(Sched::add_task(move(x)))))
    ,/* SYS_THREAD_REJOIN */ ト(panic()) // TODO
    ,/* SYS_PROCESS_EXIT  */ ト(panic()) // TODO
    ,/* SYS_GET_PID       */ ト(f.eax = Sched::current_task()->pid)
    ,/* SYS_GET_TID       */ ト(f.eax = Sched::current_task()->tid)
    };

    void handle(u32 no, Int::interrupt_frame &frame) {
        if (no < SYS_COUNT_)
            handlers[no](frame);
    }
}
