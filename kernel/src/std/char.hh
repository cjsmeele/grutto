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

#include "types.hh"

template<typename N>
constexpr bool inrange(N mi, N ma, N x) { return x >= mi && x <= ma; }

constexpr char tolower (char c) { return inrange('A', 'Z', c) ? c - 'A' + 'a' : c; }
constexpr char toupper (char c) { return inrange('a', 'z', c) ? c - 'a' + 'A' : c; }

constexpr bool islower (char c) { return inrange('a', 'z', c); }
constexpr bool isupper (char c) { return inrange('A', 'Z', c); }
constexpr bool isalpha (char c) { return islower(c) || isupper(c); }
constexpr bool isnum   (char c) { return inrange('0', '9', c); }
constexpr bool ishexnum(char c) { return isnum(c) || inrange('a', 'z', tolower(c)); }
constexpr bool isalnum (char c) { return isalpha(c) || isnum(c); }


template<typename T>
constexpr bool is_oneof(const T v) {
    return false;
}

template<typename T, typename... Us>
constexpr bool is_oneof(const T v, const Us&... rest) {
    return ((v == rest) || ...);
}
//template<typename T, typename... Us>
//constexpr bool is_oneof(const T, const Us&... rest) {
//}

constexpr bool isspace (char c) { return is_oneof(c, ' ', '\t', '\n', '\r'); }
