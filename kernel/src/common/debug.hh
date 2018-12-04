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

#include "common/koi-stream.hh"

inline void dink(char c = '#', u8 attr = 0x0a) {
    *(volatile u16*)0xb8002 = ((u16)attr << 8) | c;
}

inline void do_assert(bool assertion, const char *err = "WAAAAGH!") {
    if (UNLIKELY(!assertion)) {
        koi << "Assertion failed: " << err << "\n";
        panic();
    }
}

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define assert(expr, err) \
    do_assert((expr), __FILE__ ":" STRINGIFY(__LINE__) ": !(" #expr ") -> " err)
