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

// FIXME: I don't think we have any guarantees concerning alignment here!

#define RESDECLT_(name, alias, t) \
extern const unusable_t _binary_res_##name##_bin_size; \
extern       t          _binary_res_##name##_bin_start[]; \
             size_t     alias##_size[[maybe_unused]] = (size_t)&_binary_res_##name##_bin_size; \
             t         *alias       [[maybe_unused]] = _binary_res_##name##_bin_start;

#define RESDECLT(name, t) RESDECLT_(name, name, t)

#define RESDECL_(name, alias) RESDECLT_(name, alias, u8)

#define RESDECL(name) RESDECL_(name, name)
