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

template<typename L, typename R>
class Either;

// Hints to avoid ambiguity during Either construction.
// (needed below for Optional<T> -> Either<L,T> conversion)
template<typename T> struct Left_  { T v; };
template<typename T> struct Right_ { T v; };
template<typename T> constexpr Left_<T>  Left (const T &v) { return Left_<T>{v}; }
template<typename T> constexpr Right_<T> Right(const T &v) { return Right_<T>{v}; }
constexpr Left_<const char*>  Left (const char *v) { return Left_<const char*>{v}; }

template<typename T>
class Optional {

    template<typename Z>
    struct is_optional              { static constexpr bool value = false; };
    template<typename Z>
    struct is_optional<Optional<Z>> { static constexpr bool value = true; };

    static_assert(!is_optional<T>::value,
                  "nested optionals are not allowed");

    static_assert(!is_pointy<T>::value,
                  "for optional references, use regular pointers with nullptr as nullopt_t instead.");

    static_assert(!is_same<T,nullopt_t>::value, "really?");

    alignas(T) u8 buf[sizeof(T)];
    bool tag;

    T *ptr() const { return (T*)buf; }

public:
    constexpr explicit operator bool   () const { return tag; }
    constexpr                   bool ok() const { return tag; }

    T &operator*()   const { return *ptr(); }
    T *operator->()  const { return  ptr(); }

    Optional &operator=(const T &x)        { *ptr() = x; tag = true;  return *this; }
    Optional &operator=(const nullopt_t &) {             tag = false; return *this; }

    template<typename U> struct optionalify              { using type = Optional<U>; };
    template<typename U> struct optionalify<Optional<U>> { using type = Optional<U>; };

    // Either operations, when encountered in then() context, are downgraded to Optional.
    // To prevent this, upgrade this Optional to an Either using note() before
    // chaining an Either operation.
    template<typename L, typename R> struct optionalify<Either<L,R>> { using type = Optional<R>; };
    // template<typename L, typename R> struct optionalify<Either<L,R>> { using type = Either<L,R>; };

    template<typename F>
    constexpr auto then(F f) -> typename optionalify<typename result_of<F, T>::type>::type {
        if (tag) return f(**this);
        else     return nullopt;
    }

    constexpr auto orelse(const T &v) -> T {
        if (tag) return **this;
        else     return v;
    }

    template<typename F>
    constexpr auto orelse(F f) -> T {
        if (tag) return **this;
        else     return f();
    }

    constexpr auto require(bool b) -> Optional {
        if (tag) return b ? *this : nullopt;
        else     return nullopt;
    }

    template<typename F>
    constexpr auto require(F f)
        -> typename enable_if<is_callable<F, const T&>::value, Optional>::type {

        if (tag) return f(const_cast<const T&>(**this)) ? *this : nullopt;
        else     return nullopt;
    }

    // Create an Either by providing an annotation for the nullopt case.
    template<typename L>
    constexpr auto note(L l) -> Either<L,T> {
        if (tag) return Right(**this);
        else     return Left(l);
    }

    // // Same, but get the annotation from a function.
    // template<typename F>
    // constexpr auto annotate(enable_if<is_callable<F>::value, F> f)
    //     -> Either<typename result_of<F>::type,T> {
    //     if (tag) return **this;
    //     else     return f();
    // }

    constexpr Optional()          : tag(false) { }
    constexpr Optional(nullopt_t) : tag(false) { }
    constexpr Optional(const T &x) { (*this) = x; }
};
