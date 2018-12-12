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

using u8  = unsigned char;
using s8  =   signed char;
using u16 = unsigned short;
using s16 =   signed short;
using u32 = unsigned int;
using s32 =   signed int;
using u64 = unsigned long long;
using s64 =   signed long long;

using size_t    = u32;
//using addr_t    = u32;
using ptrdiff_t = s32;

/**
 * Address type.
 *
 * This type, which is guaranteed to be of the same size as any pointer, is
 * used for storing memory addresses.
 *
 * addr_base_t is not implicitly convertable from/to integer types, which helps
 * prevent programming errors.
 *
 * Additionally we have separate paddr_t and vaddr_t to explicitly separate the
 * virtual address type from the physical address type, making it very hard to
 * mix up the two erroneously.
 */
template<size_t Tag>
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
    //constexpr addr_base_t()      : x(0) { }
    inline           addr_base_t () = default;
    template<typename T>
    inline           addr_base_t (const T    *p)   : x(reinterpret_cast<type>(p)) { }
    // WTF: Function pointer type syntax.
    template<typename T, typename... As>
    inline           addr_base_t (T(p)(As...))     : x(reinterpret_cast<type>(p)) { }

    explicit inline constexpr addr_base_t (type n)          : x(n) { }
    inline constexpr addr_base_t (const addr_base_t &o) : x(o.x) { }
    inline addr_base_t &operator=(const addr_base_t &o) { x = o.x; return *this; }
};


using  addr_t [[deprecated("use explicit vaddr_t and paddr_t types instead")]]
              = addr_base_t<0>;
using paddr_t = addr_base_t<1>;
using vaddr_t = addr_base_t<2>;

// NULL macro is bad practice, use nullptr instead.
#ifdef NULL
#undef NULL
#endif

static_assert(sizeof(size_t) == sizeof(paddr_t) && sizeof(paddr_t) == sizeof(u8*),
              "Incorrect size type");

struct unusable_t {
    unusable_t(unusable_t&&) = delete;
    unusable_t(unusable_t&)  = delete;
    unusable_t()             = delete;
    ~unusable_t()            = delete;
};
