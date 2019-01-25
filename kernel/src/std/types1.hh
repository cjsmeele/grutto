/* Copyright (c) 2018, 2019, Chris Smeele
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

// Type definitions that do not depend on anything else.

// NULL macro is bad practice, use nullptr instead.
#ifdef NULL
#undef NULL
#endif

using u8  = unsigned char;      static_assert(sizeof(u8 ) == 1);
using s8  =   signed char;      static_assert(sizeof(s8 ) == 1);
using u16 = unsigned short;     static_assert(sizeof(u16) == 2);
using s16 =   signed short;     static_assert(sizeof(s16) == 2);
using u32 = unsigned int;       static_assert(sizeof(u32) == 4);
using s32 =   signed int;       static_assert(sizeof(s32) == 4);
using u64 = unsigned long long; static_assert(sizeof(u64) == 8);
using s64 =   signed long long; static_assert(sizeof(s64) == 8);

using size_t    = u32;
using ptrdiff_t = s32;
