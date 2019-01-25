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

u64 constexpr operator ""_K(u64 i) { return i * 1024;                      }
u64 constexpr operator ""_M(u64 i) { return i * 1024 * 1024;               }
u64 constexpr operator ""_G(u64 i) { return i * 1024 * 1024 * 1024;        }
u64 constexpr operator ""_T(u64 i) { return i * 1024 * 1024 * 1024 * 1024; }
