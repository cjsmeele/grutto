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
using addr_t    = u32;
using ptrdiff_t = s32;

// NULL macro is bad practice, use nullptr instead.
#ifdef NULL
#undef NULL
#endif

static_assert(sizeof(size_t) == sizeof(addr_t) && sizeof(addr_t) == sizeof(u8*),
              "Incorrect size type");

struct unusable_t {
    unusable_t(unusable_t&&) = delete;
    unusable_t(unusable_t&)  = delete;
    unusable_t()             = delete;
    ~unusable_t()            = delete;
};
