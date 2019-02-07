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

#include <syscalls.h>

#ifdef __cplusplus
extern "C" {
#endif

//using u32 = unsigned int;
typedef unsigned int u32;
typedef u32 size_t;

inline int syscallish(u32 a = 0,
                      u32 b = 0,
                      u32 c = 0,
                      u32 d = 0,
                      u32 e = 0,
                      u32 f = 0) {
    asm volatile (//"xchgw %%bx, %%bx\n"
                  "int $0xca\n"
                 :"+a" (a)
                 :"a"  (a),
                  "b"  (b),
                  "c"  (c),
                  "d"  (d),
                  "S"  (e),
                  "D"  (f)
                 :"cc",
                  "memory");

    return a;
}

inline int print_string(const char *s) { return syscallish(SYS_DBG_PRINT, (u32)s); }
inline int print_num(int x)            { return syscallish(SYS_DBG_PRINT_NUM, x); }
inline int yield()                     { return syscallish(SYS_YIELD); }
inline int get_pid()                   { return syscallish(SYS_GET_PID); }
inline int get_tid()                   { return syscallish(SYS_GET_TID); }
inline int make_thread(void(*entry)(size_t),
                       char  *stack,
                       size_t stack_size,
                       size_t context = 0) {
    extern void threadpoline_();
    return syscallish(SYS_THREAD_CREATE,
                      (u32)threadpoline_,
                      (u32)stack,
                           stack_size,
                      (u32)entry,
                           context);
}

#ifdef __cplusplus
}

inline void print(const char *s) { print_string(s); }
inline void print(char c)        { char x[2] { c, 0 }; print_string(x); }
inline void print(int x)         { print_num(x); }

#endif /* __cplusplus */
