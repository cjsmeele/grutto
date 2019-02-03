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

#ifdef __cplusplus
extern "C" {
#endif

//using u32 = unsigned int;
typedef unsigned int u32;

inline int syscallish(u32 a = 0,
                      u32 b = 0,
                      u32 c = 0,
                      u32 d = 0) {
    int r;
    asm volatile ("int $0xca\n"
                  "mov %%eax, %0\n"
                 :"=r" (r)
                 :"a"  (a),
                  "b"  (b),
                  "c"  (c),
                  "d"  (d));

    return r;
}

inline void print_string(const char *s) { syscallish(0xbeeeeeef, (u32)s); }
inline void print_num(int x)            { syscallish(0xbeeeeef, x); }

#ifdef __cplusplus
}

inline void print(const char *s) { print_string(s); }
inline void print(int x)         { print_num(x); }

#endif /* __cplusplus */
