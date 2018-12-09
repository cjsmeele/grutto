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
#pragma once

#include "common.hh"

namespace Pmm {

    // Our physical "page" size.
    constexpr auto page_size   = 4_K;

    // It takes 4G/4K/8 = 128K to have an allocation bitmap for the entire
    // 32-bit address space. Well worth it to avoid allocating new bitmaps
    // at runtime.
    constexpr auto page_count   = 4_G / page_size;
    constexpr auto bitmap_elems = page_count;

    // Reserve low memory.
    constexpr auto lowest_usable_address = 1_M;

}

struct page_t {
    using type = size_t;
private:
    type x;
public:
    // Conversions.
    //template<typename T> inline operator  T*()  const { return (T*)x; }
    explicit inline constexpr operator type() const { return x; }

    inline constexpr type u() const { return x; }
    explicit inline constexpr operator addr_t() const { return addr_t {static_cast<type>(x * Pmm::page_size)}; }

    inline constexpr page_t operator+(type y) const { return page_t { x + y }; }
    inline constexpr page_t operator-(type y) const { return page_t { x - y }; }

    // Constructors.
    //constexpr page_t()      : x(0) { }
    inline           page_t () = default;
    inline constexpr page_t (addr_t a) : x(a.u() / Pmm::page_size) { }

    explicit inline constexpr page_t (type n)          : x(n)   { }
             inline constexpr page_t (const page_t &o) : x(o.x) { }
             inline           page_t &operator=(const page_t &o) { x = o.x; return *this; }
};

namespace Pmm {

    bool is_allocated(size_t elem, size_t size = 1);

    // XXX: Strategy:
    //      When allocating N pages, try to alloc(N).
    //      if that fails, halve N and alloc twice.
    //      if that fails, repeat until N = 1 and panic.
    [[nodiscard]]
    Optional<page_t> alloc(size_t count, size_t align = 0);

    void free(page_t elem, size_t count = 1);

    size_t mem_available();

    void dump_pmm();

    void init();
}
