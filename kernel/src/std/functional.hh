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

// This makes me happy.

#define λ(...)   ([&]()                 { return (__VA_ARGS__); })
#define λ_(...)  ([&](auto &)           { return (__VA_ARGS__); })
#define λx(...)  ([&](auto &x)          { return (__VA_ARGS__); })
#define λy(...)  ([&](auto &y)          { return (__VA_ARGS__); })
#define λz(...)  ([&](auto &z)          { return (__VA_ARGS__); })
#define λxy(...) ([&](auto &x, auto &y) { return (__VA_ARGS__); })
