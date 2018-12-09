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
#include "multiboot-info.hh"

namespace Multiboot {

    multiboot_info info_;

    constexpr auto max_bootloader_name = 32;
    constexpr auto max_cmdline         = 1024;

    char bootloader_name_[max_bootloader_name] { };
    char cmdline_[max_cmdline] { };

    const multiboot_info &info() { return info_; }

    const char *bootloader_name() {
        if (info_.flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
             return bootloader_name_;
        else return nullptr;
    }
    const char *cmdline() {
        if (info_.flags & MULTIBOOT_INFO_CMDLINE)
             return cmdline_;
        else return nullptr;
    }

    void init(const multiboot_info *src) {

        // early fail when no multiboot info available.
        // note: bootstrap code already checked for the multiboot magic number.
        assert(src, "invalid boot_info_ptr");

        // Using a macro instead of a lambda here for clearer assertion error
        // messages at runtime.
        // note: this should actuall use the flags instead of checking for null
        // pointers.
#define CHECKPTR(p,sz)                        \
            assert(!(p) || addr_t{p}.offset(sz).u() <= 1_M, \
                   "multiboot info is required to reside below 1M");

        if (src->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME)
            strncpy(bootloader_name_, (const char*)src->boot_loader_name, max_bootloader_name);
        if (src->flags & MULTIBOOT_INFO_CMDLINE)
            strncpy(cmdline_, (const char*)src->cmdline, max_cmdline);

        CHECKPTR(src,                   sizeof(multiboot_info));
        CHECKPTR(src->mmap_addr,        src->mmap_length);

        memcpy(&info_, src, sizeof(multiboot_info));
    }
}
