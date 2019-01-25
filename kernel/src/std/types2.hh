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

#include "types1.hh"
#include "type-traits1.hh"
#include "math.hh"

#include "draken.hh"
#include "literals.hh"

using tag_physical_t = tt::uint<1>;
using tag_virtual_t  = tt::uint<2>;

#include "optional.hh"

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
    // TODO: Remove implicit conversion to/from T* for phy addresses.
    template<typename T> inline operator  T*()  const { return (T*)x; }
    constexpr   explicit inline operator type() const { return x; }
    constexpr   explicit inline operator bool() const { return x != 0; }

    [[nodiscard]] inline constexpr type u() const { return x; }

    [[nodiscard]] inline constexpr addr_base_t offset(type y) const {
        return addr_base_t{x + y};
    }
    [[nodiscard]] inline constexpr Optional<addr_base_t> safe_offset(type y) const {
        if (UNLIKELY(add_overflows(x, y))) return nullopt;
        return addr_base_t{x + y};
    }

    inline constexpr bool is_aligned(size_t alignment) const { return x % alignment == 0; }

    [[nodiscard]] inline addr_base_t align_down(size_t alignment) {
        return addr_base_t {x - x % alignment};
    }
    [[nodiscard]] inline addr_base_t align_up  (size_t alignment) {
        auto rest = x % alignment;
        if (rest) return addr_base_t{x + alignment - rest};
        else      return *this;
    }

    inline constexpr bool operator==(addr_base_t o) const { return x == o.x; }

    // Constructors.
    inline addr_base_t () = default;
    template<typename T>
    inline addr_base_t (const T *p)  : x(reinterpret_cast<type>(p)) {
        // Constructing a paddr_t from a pointer is almost always a bad idea.
        // This is ugly - we should be able to use enable_if or something.
        //static_assert(is_same<T,tag_virtual_t>::value,
        static_assert(tt::run<tt::equal<>,Tag,tag_virtual_t>::value,
                      "tried to construct non-virtual addr-type from a pointer");
        // Calling in the dragons may be a bit overkill, but tt::equal is the only
        // type-traity thing we have that does not depend on *this* header.
        // ¯\_(ツ)_/¯
    }
    template<typename T, typename... As>
    inline addr_base_t (T(p)(As...)) : x(reinterpret_cast<type>(p)) { }

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

    explicit inline constexpr page_base_t (type n)               : x(n)    { }
             inline constexpr page_base_t (const page_base_t &o) : x(o.x)  { }
             inline           page_base_t &operator=(const page_base_t &o) { x = o.x; return *this; }
};

using paddr_t = addr_base_t<tag_physical_t>;
using vaddr_t = addr_base_t<tag_virtual_t>;
using  addr_t = vaddr_t; // We usually talk only about addresses in va space.

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

/**
 * Span type.
 *
 * A span is a region represented by a starting address and a size.
 */
template<typename T>
struct span_t {

    static_assert(is_unsigned<T>::value,
                  "span_t is only defined for unsigned types");

    T start_ = 0;
    T size_  = 0;

    constexpr T start()    const { return start_;     }
    constexpr T start(T v)       { return start_ = v; }
    constexpr T size()     const { return size_;      }
    constexpr T size(T v)        { return size_  = v; }

    constexpr bool empty() const { return size_ == 0; }
    constexpr bool valid() const { return empty() || !add_overflows(start_, size_-1); }

    constexpr bool overlaps(const span_t &o) const {
        return !  empty()
            && !o.empty()
            &&    valid()
            &&  o.valid()
            && (start_ <= o.start_
                ?   start_ + (  size_-1) >= o.start_
                : o.start_ + (o.size_-1) >=   start_);
    }

    constexpr bool contains(const span_t &o) const {
        return !  empty()
            && !o.empty()
            &&    valid()
            &&  o.valid()
            && start_ <= o.start_
            && o.start_ + (o.size_-1) <= start_ + (size_-1);
    }

    constexpr bool contains(T o) const {
        return !  empty()
            &&    valid()
            && start_ <= o
            && o <= start_ + (size_-1);
    }

    constexpr span_t(T start, T size) : start_{start}, size_{size} { }

    template<typename Tag>
    constexpr span_t(addr_base_t<Tag> start, size_t size) : start_{start.u()}, size_{size} { }
};

// Deduction guide for addr_t-ish address types.
// (will resolve to the underlying integer type)
template<typename Tag>
span_t(addr_base_t<Tag>, size_t) -> span_t<typename addr_base_t<Tag>::type>;
