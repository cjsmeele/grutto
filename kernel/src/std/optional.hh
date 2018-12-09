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

struct nullopt_t {};

constexpr inline nullopt_t nullopt;

template<typename T>
class Optional {
    alignas(T) u8 buf[sizeof(T)];
    bool tag;

    T *ptr() const { return (T*)buf; }

public:
    constexpr explicit operator bool   () const { return tag; }
    constexpr                   bool ok() const { return tag; }
    T &operator*()   const { return *ptr(); }

    T &operator=(const T &x) {
        *ptr() = x;
        tag = true;
        return *ptr();
    }

    Optional()          : tag(false) { }
    Optional(nullopt_t) : tag(false) { }
    Optional(T x) { (*this) = x; }
};
