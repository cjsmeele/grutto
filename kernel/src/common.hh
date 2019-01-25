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

#include "std/stdcommon.hh"
#include "std/draken.hh"
#include "std/types.hh"
#include "std/type-traits.hh"
#include "std/compiler.hh"
#include "std/literals.hh"
#include "std/types2.hh"
#include "std/memory.hh"
#include "std/pointer.hh"
#include "std/math.hh"
#include "std/tuple.hh"
#include "std/optional.hh"
#include "std/char.hh"
#include "std/string.hh"
#include "std/array.hh"
#include "std/svector.hh"
#include "std/list.hh"
#include "std/queue.hh"
#include "std/bitmap.hh"

#include "common/asm.hh"
#include "common/kernel-sections.hh"
#include "common/mem-ops.hh"
#include "common/ioport.hh"
#include "common/panic.hh"
#include "common/koi-stream.hh"
#include "common/debug.hh"
#include "common/res.hh"
#include "common/time.hh"
// XXX
#include "int/int.hh"

#endif
