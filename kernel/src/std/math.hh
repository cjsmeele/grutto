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

#include "types.hh"

// Naive, unoptimized math functions.

template<typename N>
constexpr N pow(N x, N y) {
    static_assert(is_unsigned<N>::value);
    N res = 1;
    for (N i = 0; i < y; ++i)
        res *= x;
    return res;
}

template<typename N>
constexpr N abs(N x) {
    return x < 0 ? -x : x;
}
template<typename N>
constexpr int sgn(N x) {
    static_assert(is_signed<N>::value);
    return x < 0 ? -1 : 1;
}

template<typename N>
constexpr N logn(u32 n, N x) {
    x = abs(x);
    N y = 0;

    while (x >= n) {
        x /= n;
        y++;
    }
    return y;
}

template<typename N> constexpr N log10(N x) { return logn(10, x); }
template<typename N> constexpr N log2 (N x) { return logn( 2, x); }

template<typename N> constexpr N max(N x, N y) { return x >= y ? x : y; }
template<typename N> constexpr N min(N x, N y) { return x <= y ? x : y; }
template<typename N> constexpr N clamp(N mi, N ma, N x) { return min(ma, max(mi, x)); }

template<typename N>
constexpr u8 count_1s(N x) {
    constexpr auto nbits = sizeof(N) * 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconstant-conversion"
    static_assert(__builtin_popcount(u32{0xffff0001}) == 17
                  , "builtin popcount does not work for 32-bit numbers");

    if constexpr (nbits == 64) {
        return __builtin_popcount(x >> 32)
             + __builtin_popcount((u32)x);
    } else {
        return __builtin_popcount(x);
    }
#pragma GCC diagnostic pop
}
template<typename N>
constexpr u8 count_0s(N x) {
    constexpr auto nbits = sizeof(N) * 8;
    return nbits - count_1s(x);
}

template<typename N>
constexpr u8 leading_0s(N x) {
    constexpr auto nbits = sizeof(N) * 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconstant-conversion"
    if (!x) return nbits;
    static_assert(__builtin_clz(u32{         1}) == 31, "builtin clz does not work for 32-bit numbers");
    static_assert(__builtin_clz(u32{0xffffffff}) ==  0, "builtin clz does not work for 32-bit numbers");
    if constexpr (nbits == 64) {
        if (x >> 32) return __builtin_clz((u32)(x >> 32));
        else         return __builtin_clz((u32)x) + 32;
    } else if constexpr (nbits < 32) {
        return __builtin_clz((u32)x) - (32 - nbits);
    } else {
        return __builtin_clz(x);
    }
#pragma GCC diagnostic pop
}

template<typename N>
constexpr u8 leading_1s(N x) {
    return leading_0s(static_cast<N>(~x));
}

template<typename N>
constexpr u8 trailing_0s(N x) {
    constexpr auto nbits = sizeof(N) * 8;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconstant-conversion"
    if (!x) return nbits;
    static_assert(__builtin_ctz(u32{         1}) ==  0, "builtin ctz does not work for 32-bit numbers");
    static_assert(__builtin_ctz(u32{0x80000000}) == 31, "builtin ctz does not work for 32-bit numbers");
    if constexpr (nbits == 64) {
        if ((u32)x) return __builtin_ctz((u32)x);
        else        return __builtin_ctz((u32)(x >> 32)) + 32;
    } else {
        return __builtin_ctz(x);
    }
#pragma GCC diagnostic pop
}
template<typename N>
constexpr u8 trailing_1s(N x) {
    return trailing_0s(static_cast<N>(~x));
}


/*
    auto tst1 = [](auto n) {
        koi.fmt("u{-2d} {064b}, 1s {2d} 0s {2d} lead0 {2d} trail0 {2d} lead1 {2d} trail1 {2d}\n"
                , sizeof(n)*8
                ,n
                ,count_1s(n)
                ,count_0s(n)
                ,leading_0s(n)
                ,trailing_0s(n)
                ,leading_1s(n)
                ,trailing_1s(n)
                );
    };
    auto tst2 = [](auto n) { koi.fmt("{064b}, 0s {2d}\n", n, popcount(n)); };

    tst1(u32 {0});
    tst1(u32 {1});
    tst1(u32 {0x8000});
    tst1(u32 {0x80000000});
    tst1(u32 {0xfffeffff});
    tst1(u32 {0xffffffff});
    tst1(u16 {0xffff});
    tst1(u16 {0xfeff});
    tst1(u16 {0x0000});
    tst1(u64 {0xffffffff});
    tst1(u64 {0xf0ffffffff});
    tst1(u64 {0xf00000ff'ffffffff});
    tst1(u64 {0xf0000100'00000000});
    tst1(u64 {0x00000000'00000010});
    tst1(u8  {0x0f});
    tst1(u8  {0xf0});
    */

//constexpr u8 trailing_0s(N x) { return      __builtin_ctz( x); }

//constexpr auto count_0s(u16 x)    { return      __builtin_popcount((u16)~x); }
//constexpr auto count_1s(u16 x)    { return      __builtin_popcount( x); }
//constexpr auto leading_0s (u16 x) { return      __builtin_clz( x) - 16; }
//constexpr auto trailing_0s(u16 x) { return      __builtin_ctz( x); }
//constexpr auto count_0s(u32 x)    { return      __builtin_popcount((u32)~x); }
//constexpr auto count_1s(u32 x)    { return      __builtin_popcount( x); }
//constexpr auto leading_0s (u32 x) { return      __builtin_clz( x) - 16; }
//constexpr auto trailing_0s(u32 x) { return      __builtin_ctz( x); }
//auto leading_1s (u16 x) { return __builtin_clz(~x); }
//auto trailing_1s(u16 x) { return __builtin_ctz(~x); }

template<typename N> constexpr bool is_even(N x) { return (x & 1) == 0; }
template<typename N> constexpr bool is_odd (N x) { return (x & 1) == 1; }
template<typename N, typename M>
constexpr bool is_divisible(N x, M y) { return x % y == 0; }

template<typename N> constexpr bool bit_get(N x, u8 i) {
    if (i) return !!(x & (1 << i));
    else   return x & 1;
}
template<typename N> constexpr N bit_set(N x, u8 i, bool val = true) {
    if (val) return x |= 1 << i;
    else     return x &= ~(1 << i);
}
template<typename N> constexpr N bit_clear(N x, u8 i) {
    return bit_set(x, i, false);
}
template<typename N> constexpr N bit_toggle(N x, u8 i) {
    return x ^= 1 << i;
}

template<typename N> constexpr N set_bits(N x, u8 i, u8 count, bool val = true) {
    using U = typename add_unsigned<N>::type;
    size_t nbits = sizeof(N) * 8;
    U mask = ((U)(~0) >> (nbits - count)) << i;
    if (val) return x | mask;
    else     return x & (~mask);
}

template<typename N> constexpr N set_rbits(N x, u8 i, u8 count, bool val = true) {
    return set_bits(x, sizeof(N)*8-i-count, count, val);
}

template<typename N, typename M> constexpr N is_aligned(N x, M alignment) {
    return x % alignment == 0;
}
template<typename N, typename M> constexpr N is_aligned(N *x, M alignment) {
    return (addr_t)x % alignment == 0;
}
template<typename N, typename M> constexpr N align_down(N x, M alignment) {
    auto rest = x % alignment;
    if (rest) return x - rest;
    else      return x;
}
template<typename N, typename M> constexpr N align_up(N x, M alignment) {
    auto rest = x % alignment;
    if (rest) return x + (alignment - rest);
    else      return x;
}
template<typename N, typename M> constexpr N div_ceil(N x, M alignment) {
    auto rest = x % alignment;
    if (rest) return x / alignment + 1;
    else      return x / alignment;
}



// FP stuff.
// (not actually usable right now, since we don't initialize the FPU)

#if 0
/// Power.
constexpr double fpow(double x, unsigned y) {
    double n = 1;
    for (unsigned i = 0; i < y; i++)
        n *= x;
    return n;
}

/// Factorial.
constexpr unsigned fac(unsigned x) {
    unsigned n = 1;
    for (unsigned i = x; i > 0; i--)
        n *= i;
    return n;
}

// Approximate sine.
constexpr double csin(double x) {
    return x - fpow(x,  3) / fac( 3)
             + fpow(x,  5) / fac( 5)
             - fpow(x,  7) / fac( 7)
             + fpow(x,  9) / fac( 9)
             - fpow(x, 11) / fac(11)
             + fpow(x, 13) / fac(13);
}

/// Approximate cosine.
constexpr double ccos(double x) {
    return 1 - fpow(x,  2) / fac( 2)
             + fpow(x,  4) / fac( 4)
             - fpow(x,  6) / fac( 6)
             + fpow(x,  8) / fac( 8)
             - fpow(x, 10) / fac(10)
             + fpow(x, 12) / fac(12);
}

/// Rounding.
constexpr int round(double x) { return (int)(x + (double)sgn(x) * 0.5); }
#endif
