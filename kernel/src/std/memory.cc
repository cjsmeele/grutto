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
#include "memory.hh"

extern "C" void *memcpy(void *__restrict dst, const void *__restrict src, size_t sz) {
    Mem::copy((u8*)dst, (u8*)src, sz);
    return dst;
}

extern "C" void *memset(void *dst, char c, size_t sz) {
    Mem::set((char*)dst, c, sz);
    return dst;
}
