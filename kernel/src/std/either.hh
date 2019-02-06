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

#include "draken.hh"
#include "optional.hh"

template<typename L, typename R>
class Either {

    template<typename Z>
    struct is_either              { static constexpr bool value = false; };
    template<typename Y, typename Z>
    struct is_either<Either<Y,Z>> { static constexpr bool value = true; };

    static_assert(!is_either<R>::value && !is_either<L>::value,
                  "nested eithers are not allowed");

    using alignment = tt::run<tt::max_align<>, L, R>;
    using size      = tt::run<tt::max_size<>,  L, R>;

    // Sanity checks.
    static_assert(alignment::value >= alignof(L)
               && alignment::value >= alignof(R)
               &&      size::value >=  sizeof(L)
               &&      size::value >=  sizeof(R));

    alignas(alignment::value) u8 buf[size::value];
    bool tag;

public:
    constexpr const L &left()  const { return *reinterpret_cast<const L*>(buf); }
    constexpr const R &right() const { return *reinterpret_cast<const R*>(buf); }
    constexpr       L &left()        { return *reinterpret_cast<      L*>(buf); }
    constexpr       R &right()       { return *reinterpret_cast<      R*>(buf); }

    constexpr explicit operator const L*() const { return &left();  }
    constexpr explicit operator const R*() const { return &right(); }
    constexpr explicit operator       L*()       { return &left();  }
    constexpr explicit operator       R*()       { return &right(); }

    constexpr explicit operator bool   () const { return tag; }
    constexpr                   bool ok() const { return tag; }

    constexpr operator Optional<R>() {
        if (tag) return right();
        else     return nullopt;
    }

    // To allow for reducing to an Optional within a then().then()... chain.
    constexpr Optional<R> as_opt() { return static_cast<Optional<R>>(*this); }

    constexpr const R &operator*() const { return right(); }
    constexpr       R &operator*()       { return right(); }

    constexpr Either &operator=(const Right_<R>  &x) { right() = x.v;       tag = true;  return *this; }
    constexpr Either &operator=(const Left_<L>   &x) { left()  = x.v;       tag = false; return *this; }
    constexpr Either &operator=(      Right_<R> &&x) { right() = move(x.v); tag = true;  return *this; }
    constexpr Either &operator=(      Left_<L>  &&x) { left()  = move(x.v); tag = false; return *this; }
    constexpr Either &operator=(const R  &x)         { right() = x;         tag = true;  return *this; }
    constexpr Either &operator=(const L  &x)         { left()  = x;         tag = false; return *this; }
    constexpr Either &operator=(      R &&x)         { right() = move(x);   tag = true;  return *this; }
    constexpr Either &operator=(      L &&x)         { left()  = move(x);   tag = false; return *this; }

    // This should perhaps assert U /= Either<M,T>, so that we do not
    // accidentally nest Eithers with different error types.
    template<typename U> struct eitherify              { using type = Either<L,U>; };
    template<typename U> struct eitherify<Either<L,U>> { using type = Either<L,U>; };
    template<typename U> struct eitherify<Optional<U>> { using type = Either<L,U>; };

    // Do impure stuff with the value.
    // (ignores any return value of the given function)
    template<typename F>
    constexpr auto then_do(F f) -> Either& {

        if (tag) f(right());
        return *this;
    }

    // Chain either-operations.
    template<typename F>
    constexpr auto then(F f)
        -> typename eitherify<typename result_of<F, R&>::type>::type {

        if (tag) return f(right());
        else     return left();
    }

    // Chain either-operations with optional operations.
    // (lifts the Optional into Either context)
    template<typename F>
    constexpr auto then(F f, L l)
        -> typename eitherify<typename result_of<F, R&>::type>::type {

        if (tag) return f(right()).note(l);
        else     return left();
    }

    // Substitute Left.
    constexpr auto orelse(const R &v) -> R {
        if (tag) return right();
        else     return v;
    }

    // "Catch" Left.
    template<typename F>
    constexpr auto orelse(F f) -> typename enable_if<is_callable<F,L&>::value, R>::type {
        if (tag) return right();
        else     return f(const_cast<const L&>(left()));
    }

    // "Catch" Left, but ignore its value.
    template<typename F>
    constexpr auto orelse(F f) -> typename enable_if<is_callable<F>::value, R&>::type  {
        if (tag) return right();
        else     return f();
    }

    constexpr auto require(bool b, L err) -> Either {
        if (tag) return b ? *this : err;
        else     return *this;
    }

    template<typename F>
    constexpr auto require(F f, L err)
        -> typename enable_if<is_callable<F, const R&>::value, Either>::type {

        if (tag) return f(const_cast<const R&>(right())) ? *this : err;
        else     return *this;
    }

    constexpr Either() { (*this) = L { }; }

    constexpr Either(const Either &o) = delete;

    constexpr Either(Either &&o) {
        if (o.tag) (*this) = move(o.right());
        else       (*this) = move(o.left());
    }

    constexpr Either(const L  &x) { (*this) = x; }
    constexpr Either(const R  &x) { (*this) = x; }
    constexpr Either(      L &&x) { (*this) = move(x); }
    constexpr Either(      R &&x) { (*this) = move(x); }

    constexpr Either(const  Left_<L>  &x) { (*this) = x; }
    constexpr Either(const Right_<R>  &x) { (*this) = x; }
    constexpr Either(       Left_<L> &&x) { (*this) = move(x); }
    constexpr Either(      Right_<R> &&x) { (*this) = move(x); }
};

// alias name?
//template<typename T> using EitherS = Either<const char*, T>;
