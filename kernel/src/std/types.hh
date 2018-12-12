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

// NULL macro is bad practice, use nullptr instead.
#ifdef NULL
#undef NULL
#endif

using u8  = unsigned char;
using s8  =   signed char;
using u16 = unsigned short;
using s16 =   signed short;
using u32 = unsigned int;
using s32 =   signed int;
using u64 = unsigned long long;
using s64 =   signed long long;

using size_t    = u32;
using ptrdiff_t = s32;

// Compile-time integer type.
template<size_t I> struct int_ { static constexpr size_t value = I; };

#include "literals.hh"

/**
 * Address type.
 *
 * This type, which is guaranteed to be of the same size as any pointer, is
 * used for storing memory addresses.
 *
 * addr_base_t is not implicitly convertable from/to integer types, which helps
 * prevent programming errors.
 *
 * Additionally we have separated paddr_t and vaddr_t to explicitly separate the
 * virtual address type from the physical address type, making it very hard to
 * mix up the two erroneously.
 *
 * The Tag template argument annotates the address with virtual/physical-ness.
 * (see the type aliases below this struct)
 */
template<typename Tag>
struct addr_base_t {
    using type = size_t;
private:
    type x;
public:
    // Conversions.
    // FIXME: Remove implicit conversion to/from T* for phy addresses.
    template<typename T> inline operator  T*()  const { return (T*)x; }
    constexpr   explicit inline operator type() const { return x; }
    constexpr   explicit inline operator bool() const { return x != 0; }

    [[nodiscard]] inline constexpr type u() const { return x; }

    [[nodiscard]] inline constexpr addr_base_t offset(type y) const { return addr_base_t{x + y}; }

    inline constexpr bool is_aligned(size_t alignment) const { return x % alignment == 0; }

    [[nodiscard]] inline addr_base_t align_down(size_t alignment) { return addr_base_t {x - x % alignment}; }
    [[nodiscard]] inline addr_base_t align_up  (size_t alignment) {
        auto rest = x % alignment;
        if (rest) return addr_base_t{x + alignment - rest};
        else      return *this;
    }

    // Constructors.
    inline           addr_base_t () = default;
    template<typename T>
    inline           addr_base_t (const T    *p)   : x(reinterpret_cast<type>(p)) { }
    template<typename T, typename... As>
    inline           addr_base_t (T(p)(As...))     : x(reinterpret_cast<type>(p)) { }

    explicit inline constexpr addr_base_t (type n)      : x(n) { }
    inline constexpr addr_base_t (const addr_base_t &o) : x(o.x) { }
    inline addr_base_t &operator=(const addr_base_t &o) { x = o.x; return *this; }
};

// Our physical page size.
constexpr auto page_size = 4_K;

/**
 * Page number type.
 *
 * Just like addresses, page numbers are annotated with virtual/physical-ness,
 * using the Tag template parameter.
 *
 * This type is convertable to/from addresses of the same Tag.
 * As part of the conversion, a shift is performed based on the page size.
 */
template<typename Tag>
struct page_base_t {
    using type = size_t;
private:
    type x;
public:
    // Conversions.
    explicit inline constexpr operator type() const { return x; }

    inline constexpr type u() const { return x; }
    explicit inline constexpr operator addr_base_t<Tag>() const {
        return addr_base_t<Tag> {static_cast<type>(x * page_size)};
    }

    inline constexpr page_base_t operator+(type y) const { return page_base_t { x + y }; }
    inline constexpr page_base_t operator-(type y) const { return page_base_t { x - y }; }

    // Constructors.
    inline           page_base_t () = default;
    inline constexpr page_base_t (addr_base_t<Tag> a) : x(a.u() / page_size) { }

    explicit inline constexpr page_base_t (type n)          : x(n)    { }
             inline constexpr page_base_t (const page_base_t &o) : x(o.x)  { }
             inline           page_base_t &operator=(const page_base_t &o) { x = o.x; return *this; }
};

using tag_physical_t = int_<1>;
using tag_virtual_t  = int_<2>;

using paddr_t = addr_base_t<tag_physical_t>;
using vaddr_t = addr_base_t<tag_virtual_t>;
using  addr_t = vaddr_t;

using ppage_t = page_base_t<tag_physical_t>;
using vpage_t = page_base_t<tag_virtual_t>;

static_assert(sizeof(size_t) == sizeof(addr_t) && sizeof(addr_t) == sizeof(u8*),
              "Incorrect size type");

struct unusable_t {
    unusable_t(unusable_t&&) = delete;
    unusable_t(unusable_t&)  = delete;
    unusable_t()             = delete;
   ~unusable_t()             = delete;
};
