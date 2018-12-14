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

template<typename T, size_t Sz>
class Array {
    T elems[Sz];

public:
    //constexpr const T* data() const { return elems; }
    //constexpr       T* data()       { return elems; }

    constexpr const T* operator*() const { return elems; }
    constexpr       T* operator*()       { return elems; }

    static constexpr size_t size() { return Sz; }

    void clear(const T &x) { for (size_t i = 0; i < Sz; ++i) elems[i] = x;    }
    void clear()           { for (size_t i = 0; i < Sz; ++i) elems[i] = T {}; }

    constexpr const T &operator[](size_t i) const { return elems[i]; }
    constexpr       T &operator[](size_t i)       { return elems[i]; }

    constexpr const T *begin() const { return elems; }
    constexpr       T *begin()       { return elems; }
    constexpr const T *end()   const { return elems+Sz; }
    constexpr       T *end()         { return elems+Sz; }

    template<typename... As>
    constexpr Array(const As&... as) : elems { as... } { }
    constexpr Array() {}
};

static_assert(sizeof(Array<u8 , 1>) ==  1);
static_assert(sizeof(Array<u16,10>) == 20);

template<typename A, typename... As>
constexpr auto make_array(const A& a, const As&... as) {
    return Array<A, sizeof...(as)+1> { a, as... };
}
