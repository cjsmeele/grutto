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

// Old-style include guard, to aid PCH stuff (maybe unnecessary).
#ifndef COMMON_HH_INCLUDED
#define COMMON_HH_INCLUDED

// Headers without dependencies.
#include "std/stdcommon.hh"
//#include "std/draken.hh"
#include "std/types1.hh"
#include "std/functional.hh"

// Headers depending on basic types and type traits.
#include "std/type-traits1.hh"
#include "std/literals.hh"
#include "std/compiler.hh"
#include "std/optional.hh"
#include "std/math.hh"

// Headers depending on math, compiler builtins, literals and the above.
#include "std/types2.hh"
#include "std/type-traits2.hh"
#include "std/types2.hh"

// Other standard-ish headers.
#include "std/memory.hh"
#include "std/pointer.hh"
#include "std/tuple.hh"
#include "std/char.hh"
#include "std/string.hh"
#include "std/array.hh"
#include "std/svector.hh"
#include "std/list.hh"
#include "std/queue.hh"
#include "std/bitmap.hh"

// Headers specific to the kernel, but used widely enough.
#include "common/asm.hh"
#include "common/kernel-sections.hh"
#include "common/mem-ops.hh"
#include "common/ioport.hh"
#include "common/panic.hh"
#include "common/koi-stream.hh"
#include "common/debug.hh"
#include "common/res.hh"
#include "common/time.hh"

// XXX For critical sections. Should be a common header.
#include "int/int.hh"

#endif
