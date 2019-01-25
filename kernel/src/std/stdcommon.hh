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

#include "types1.hh"

// Declarations for code in std/.
// Needed to keep std from including any other headers.

[[noreturn]]
void stdfail();

namespace Mem {
    void *kmalloc(size_t sz);
    void  kfree(void *p);
}

inline void *stdmalloc(size_t sz) { return Mem::kmalloc(sz); }
inline void  stdfree(void *p)     { return Mem::kfree(p); }

void stdtrace(const char *s);
