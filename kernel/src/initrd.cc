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
#include "initrd.hh"

namespace Initrd {

    addr_t initrd_start = addr_t{0};
    size_t initrd_size  = 0;
    size_t entry_count  = 0;

    struct header_t {
        char magic[24];
        u32  entry_count;
        u32  entry_size;
    } __attribute__((packed));

    struct file_entry_t {
        char name[max_file_name_length+1];
        u32  offset;
        u32  size;

        explicit operator file_t() const;
    } __attribute__((packed));

    constexpr size_t file_entry_offset(size_t n) {
        return sizeof(header_t) + n * sizeof(file_entry_t);
    }
    size_t data_offset() { return file_entry_offset(entry_count); }

    file_entry_t::operator file_t() const {
        file_t file;

        strncpy(file.name, name, max_file_name_length);
        file.data = initrd_start.offset(data_offset() + offset);
        file.size = size;

        return file;
    }

    file_entry_t *get_entry(size_t i) {
        if (i >= entry_count)
            return nullptr;

        return (file_entry_t*)initrd_start.offset(file_entry_offset(i));
    }

    file_entry_t *get_entry(const char *name) {
        for (size_t i = 0; i < entry_count; ++i) {
            auto &entry = *get_entry(i);
            if (strneq(name, entry.name, max_file_name_length))
                return &entry;
        }
        return nullptr;
    }

    Optional<file_t> get_file(const char *name) {
        auto *e = get_entry(name);
        if (e) return static_cast<file_t>(*e);
        else   return nullopt;
    }

    template<typename F>
    void iterate(F f) {
        for (size_t i = 0; i < entry_count; ++i)
            f(static_cast<file_t>(*get_entry(i)));
    }

    void dump() {
        koi.fmt("initrd contents:\n");
        iterate([](const file_t &f) {
            koi.fmt("- @{} {6S}, {-24}\n", f.data, f.size, f.name);
        });
    }

    void init(addr_t addr, size_t size) {

        initrd_start = addr;
        initrd_size  = size;

        // Sanity checks {{{

        if (initrd_size < sizeof(header_t)) {
            koi(LL::error).fmt("invalid initrd format ({S} is too small to fit header)\n", initrd_size);
            return;
        }

        auto &header = *(header_t*)initrd_start;
        if (!str_prefix_of("INITRD", header.magic)) {
            koi(LL::error).fmt("invalid initrd format (magic does not match)\n");
            return;
        }

        if (header.entry_size != sizeof(file_entry_t)) {
            koi(LL::error).fmt("initrd entry size does not match {S} /= {S}\n",
                               header.entry_size, sizeof(file_entry_t));
            return;
        }

        if ((u64)header.entry_count * (u64)header.entry_size > (u64)initrd_size) {
            koi(LL::error).fmt("initrd header invalid\n");
            return;
        }

        for (size_t i = 0; i < header.entry_count; ++i) {
            auto &entry = *(file_entry_t*)initrd_start.offset(file_entry_offset(i));

            if (   (u64)data_offset() + entry.offset              > initrd_size
                || (u64)data_offset() + entry.offset + entry.size > initrd_size
                || entry.name[max_file_name_length] != 0) {

                koi(LL::error).fmt("initrd table invalid\n");
                return;
            }
        }

        // }}}

        entry_count = header.entry_count;
    }
}
