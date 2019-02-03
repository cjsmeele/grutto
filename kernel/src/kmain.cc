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
#include "common.hh"

#include "multiboot-info.hh"
#include "int/pic.hh"
#include "gdt.hh"
#include "pit.hh"
#include "mem/pmm.hh"
#include "mem/vmm.hh"
#include "mem/kmm.hh"
#include "pci.hh"
#include "initrd.hh"
#include "elf.hh"
#include "task.hh"
#include "sched.hh"

// nnoremap <F5> :w \| split \| terminal make -Bj8 && make -C.. run NOVGA=1<CR>
//register int sp asm ("sp");

static void kmain() {

    // Second stage IO initialization, now that we can access multiboot info.
    koi.init_after_multiboot();

    // Initialize interrupts and timers.
    Pit::init();
    Int::Pic::init();

    // Reinitialize GDT using high-half table.
    Gdt::init();

    // Initialize memory management.
    Pmm::init();
    Vmm::init();

    // Enable interrupts.
    Int::sti();

    // Third and final stage IO initialization, now that we can manage memory.
    koi.init_after_mem_init();

    //koi(LL::notice) << "\e[2J\e[0;0f";
    koi(LL::notice).fmt("\n * {} is taking flight *\n\n", KERNEL_NAME);

    if (Multiboot::bootloader_name())
        koi(LL::notice).fmt("{-20} {}\n", "bootloader:",         Multiboot::bootloader_name());

        koi(LL::notice).fmt("{-20} {}\n", "kernel version:",     KERNEL_VNAME);

    if (Multiboot::cmdline())
        koi(LL::notice).fmt("{-20} {}\n", "kernel commandline:", Multiboot::cmdline());

    koi(LL::notice).fmt("\nmemory: {S} available\n\n", Pmm::mem_available());

    Pci::init();

    RESDECLT_(initrd, initrd, u8);
    Initrd::init(initrd, initrd_size);
    Initrd::dump();

    const char *task_name = "hello.elf";
    auto task_ = Initrd::get_file(task_name)
                        .note("initial task not found in initrd")
                        .then(λx(Task::from_elf(x.data, x.size)));

    if (task_.ok()) {
        koi.fmt("\nRunning '{}' task in user-mode...\n\n", task_name);
        Sched::init();
        Sched::add_task(move(*task_));
        Sched::switch_task();

        // XXX: (switch_task does not return)
    } else {
        koi.fmt("\nCould not load {} user task: {}\n", task_name, task_.left());
    }

    auto start_time = uptime();
    for (u64 i = 0;; ++i) {
        auto up = uptime();
        koi.fmt("\r[{6}.{02}] {} ",
                time_s(up), time_ms(up)/10 % 100,
                "/-\\|"[i/2%4]);
        ksleep(50_ms);

        if (uptime() - start_time > 10_s)
            trap(0xe0fe0f88);
    }
}

/// Kernel entrypoint.
extern "C" void cstart(multiboot_info *info) {

    // Initialize rudimentary IO options first, so we can print errors early on.
    koi.init();

    Multiboot::init(info);

    kmain();
    panic();
}
