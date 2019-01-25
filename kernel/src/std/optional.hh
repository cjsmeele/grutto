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
#pragma once

#include "type-traits1.hh"

struct nullopt_t {};

constexpr inline nullopt_t nullopt;

template<typename T>
class Optional {

    static_assert(!is_pointy<T>::value,
                  "for optional references, use regular pointers with nullptr as nullopt_t instead.");

    alignas(T) u8 buf[sizeof(T)];
    bool tag;

    T *ptr() const { return (T*)buf; }

public:
    constexpr explicit operator bool   () const { return tag; }
    constexpr                   bool ok() const { return tag; }

    T &operator*()   const { return *ptr(); }
    T *operator->()  const { return  ptr(); }

    Optional &operator=(const T &x) {
        *ptr() = x;
        tag = true;
        return *this;
    }

    Optional &operator=(const nullopt_t &) {
        tag = false;
        return *this;
    }

    template<typename U> struct optionalify              { using type = Optional<U>; };
    template<typename U> struct optionalify<Optional<U>> { using type = Optional<U>; };

    template<typename F>
    auto then(F f) -> typename optionalify<typename result_of<F, T>::type>::type {
        if (tag) return f(**this);
        else     return nullopt;
    }

    Optional()          : tag(false) { }
    Optional(nullopt_t) : tag(false) { }
    Optional(const T &x) { (*this) = x; }
};
