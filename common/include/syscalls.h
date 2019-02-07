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

//#include "types.h"

// XXX: Hardcoded values may be used in initrd/common startup asm.
//      Keep those in sync when inserting new syscalls.

typedef enum Syscalls {
    SYS_NOP           =  0,
    SYS_DBG_PRINT     =  1,
    SYS_DBG_PRINT_NUM =  2,
    SYS_DBG_HALT      =  3,
    SYS_YIELD         =  4,
    SYS_THREAD_CREATE =  5,
    SYS_THREAD_REJOIN =  6,
    SYS_PROCESS_EXIT  =  7,
    SYS_GET_PID       =  8,
    SYS_GET_TID       =  9,
    SYS_COUNT_        = 10,
} Syscalls;
