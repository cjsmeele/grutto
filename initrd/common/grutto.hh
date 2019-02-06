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
    asm volatile (//"xchgw %%bx, %%bx\n"
                  "int $0xca\n"
                 :"+a" (a)
                 :"a"  (a),
                  "b"  (b),
                  "c"  (c),
                  "d"  (d)
                 :"cc",
                  "memory");

    return a;
}

inline int print_string(const char *s) { return syscallish(0xbeeeeeef, (u32)s); }
inline int print_num(int x)            { return syscallish(0x0beeeeef, x); }
inline int yield()                     { return syscallish(0x00071e1d); }
inline int getpid()                    { return syscallish(0x0000001d); }

#ifdef __cplusplus
}

inline void print(const char *s) { print_string(s); }
inline void print(char c)        { char x[2] { c, 0 }; print_string(x); }
inline void print(int x)         { print_num(x); }

#endif /* __cplusplus */
