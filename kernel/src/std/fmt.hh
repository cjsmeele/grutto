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

#include "std/types.hh"

//template<typename F, typename A>
//void fmt2(F fn, const A &a);

//koi.fmt("{  -8} | {  -8} | {  -8} | { -8} | {-5}\n", "hex", "oct", "bin", "dec", "even?");
//koi.fmt("{  ~8} | {  ~8} | {  ~8} | { ~8} | {~5}\n", '-', '-', '-', '-', '-');
//for (int i = 0; i < 30; ++i)
//koi.fmt("{#x08} | {#o08} | {#b08} | {  8} | {-5}\n", i,i,i,i, (i&1) == 0);
//koi.fmt("\n");

struct Fmtflags {
    int  width             =  0;
    union {
        u8 radix           = 10;
        u8 scale;
    };
    bool explicit_sign : 1 = false; ///< (only for numbers)
    bool unsign        : 1 = false; ///< Remove sign bit.
    bool prefix_radix  : 1 = false; ///< Prefix number with 0b, 0x, 0o, or 0d.
    bool align_left    : 1 = false;
    bool prefix_zero   : 1 = false; ///< (only for numbers)
    bool repeat        : 1 = false; ///< (only for chars) fill width by repetition
    bool size          : 1 = false; ///< (only for numbers) format as human-readable K/M/G
};

template<typename F>
constexpr void fmtpad(F fn, char c, int count) {
    for (int i = 0; i < count; ++i) {
        fn(c);
    }
}

template<typename F>
constexpr void fmt2(F fn, Fmtflags &f, const char *a) {
    if (!a) a = "<null>";

    size_t len = strlen(a);
    int pad    = f.width ? f.width - (int)len : 0;

    if (!f.align_left && pad > 0)
        fmtpad(fn, ' ', pad);

    fn(a);

    if (f.align_left && pad > 0)
        fmtpad(fn, ' ', pad);
}
template<typename F> constexpr void fmt2(F fn, Fmtflags &f, char c) {
    if (f.repeat) {
        for (int i = 0; i < f.width; ++i)
            fn(c);
    } else {
        int pad = f.width ? f.width - 1 : 0;
        if (!f.align_left) fmtpad(fn, ' ', pad);
        fn(c);
        if (f.align_left)  fmtpad(fn, ' ', pad);
    }
}

template<typename F> constexpr void fmt2(F fn, Fmtflags &f, bool b) {
    fmt2(fn, f, b ? "true" : "false");
}

template<typename F>
constexpr void fmt2(F fn, Fmtflags &f, u64 n, bool sign = false) {
    char buf[2 * 64 + 3] = {}; // max length (binary full 64-bit number + sign and radix).
    int  i          = 0;
    int  prefix_len = 0;

    // For printing K/M/G size units.
    u8 scale = f.scale;
    char size_char = 'B';

    if (f.size) { // Reset radix.
        f.radix     = 10;
        prefix_len +=  2; // not actually a prefix, but who cares.
        if (scale == 0)
            scale = min((int)log10(n) / 3, (int)6);
        if (scale == 0) size_char = 'B';
        if (scale == 1) size_char = 'K';
        if (scale == 2) size_char = 'M';
        if (scale == 3) size_char = 'G';
        if (scale == 4) size_char = 'T';
        if (scale == 5) size_char = 'P';
        if (scale == 6) size_char = 'E';
        n /= pow((u32)10, (u32)(scale*3));
    }

    if (sign || f.explicit_sign) prefix_len += 1;
    if (f.prefix_radix)          prefix_len += 2;

    auto emit_radix = [&fn,f] () {
        fn('0');
        fn( f.radix == 10 ? 'd'
          : f.radix == 16 ? 'x'
          : f.radix ==  8 ? 'o'
          : f.radix ==  2 ? 'b'
          : '?');
    };

    auto fmtdigit = [] (int n) {
        if (n < 10) return n + '0';
        else        return n - 10 + 'a';
    };

    if (n) {
        while (n) {
            u64 x = n % f.radix;
            u64 y = n / f.radix;
            buf[i++] = fmtdigit(x);
            n = y;
        }
    } else {
        buf[i++] = '0';
    }

    int pad = f.width ? f.width - (prefix_len + (int)i) : 0;

    if (pad > 0 && !f.align_left && !f.prefix_zero)
        fmtpad(fn, ' ', pad);

    if (sign || f.explicit_sign) fn(sign ? '-' : '+');
    if (f.prefix_radix)          emit_radix();

    if (pad > 0 && !f.align_left && f.prefix_zero)
        fmtpad(fn, '0', pad);

    for (--i; i >= 0; --i)
        fn(buf[i]);

    if (f.size) {
        fn(' ');
        fn(size_char);
    }

    if (pad > 0 && f.align_left)
        fmtpad(fn, ' ', pad);
}

template<typename F>
constexpr void fmt2(F fn, Fmtflags &f, s64 n) {
    if (n >= 0)
         fmt2(fn, f, (u64) n);
    else fmt2(fn, f, (u64)-n, !f.unsign);
}

template<typename F> constexpr void fmt2(F fn, Fmtflags &f,  s8 n) { fmt2(fn, f, (s64)n); }
template<typename F> constexpr void fmt2(F fn, Fmtflags &f, s16 n) { fmt2(fn, f, (s64)n); }
template<typename F> constexpr void fmt2(F fn, Fmtflags &f, s32 n) { fmt2(fn, f, (s64)n); }
template<typename F> constexpr void fmt2(F fn, Fmtflags &f,  u8 n) { fmt2(fn, f, (u64)n); }
template<typename F> constexpr void fmt2(F fn, Fmtflags &f, u16 n) { fmt2(fn, f, (u64)n); }
template<typename F> constexpr void fmt2(F fn, Fmtflags &f, u32 n) { fmt2(fn, f, (u64)n); }

template<typename F, size_t Tag>
constexpr void fmt2(F fn, Fmtflags &f, addr_base_t<Tag> a) {
    f.radix        = 16;
    f.prefix_zero  = true;
    f.unsign       = true;

    if constexpr (sizeof(void*) >= 8)
         f.width = 16;
    else f.width =  8;

    if (f.prefix_radix) f.width += 2;

    fmt2(fn, f, u64{a.u()});
}
template<typename F, typename T>
constexpr void fmt2(F fn, Fmtflags &f, const T *a) {
    fmt2(fn, f, vaddr_t{a});
}

template<typename F, typename... As>
constexpr void fmt(F fn, const char *s, const As&... as);

template<typename F, typename A, typename... As>
constexpr void fmt1(F fn, const char *s, const A &a, const As&... as) {
    bool in_brace = false;
    Fmtflags flags;

    auto reset = [&] () { flags    = Fmtflags{};
                          in_brace = false; };

    for (; *s; ++s) {
        auto c = *s;
        if (in_brace) {
            if (c == '}') {
                fmt2(fn, flags, a);
                fmt(fn, s+1, as...);
                return;
            } else if (c == '{') {
                fn(c);
                reset();
            } else if (c == '0' && !flags.width) {
                flags.prefix_zero = true;
            } else if (isnum(c)) {
                flags.width = flags.width * 10 + (c - '0');
            } else if (c == 'u') {
                flags.unsign = true;
            } else if (c == 'x') {
                flags.unsign = true;
                flags.radix  = 16;
            } else if (c == 'o') {
                flags.unsign = true;
                flags.radix  = 8;
            } else if (c == 'b') {
                flags.unsign = true;
                flags.radix  = 2;
            } else if (c == 'S') { flags.size = true; flags.scale = 0;
            } else if (c == 'K') { flags.size = true; flags.scale = 1;
            } else if (c == 'M') { flags.size = true; flags.scale = 2;
            } else if (c == 'G') { flags.size = true; flags.scale = 3;
            } else if (c == 'T') { flags.size = true; flags.scale = 4;
            } else if (c == '-') {
                flags.align_left = true;
            } else if (c == '+') {
                flags.explicit_sign = true;
            } else if (c == '#') {
                flags.prefix_radix = true;
            } else if (c == '~') {
                flags.repeat = true;
            }
        } else {
            if (c == '{') {
                in_brace = true;
            } else {
                fn(c);
            }
        }
    }
}

template<typename F, typename... As>
constexpr void fmt(F fn, const char *s, const As&... as) {
    if constexpr (sizeof...(As) > 0) {
        fmt1(fn, s, as...);
    } else {
        fn(s);
    }
}
