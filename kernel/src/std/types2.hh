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

#include "types.hh"
#include "type-traits.hh"
#include "math.hh"

/**
 * Span type.
 *
 * A span is a region represented by a starting address and a size.
 */
template<typename T>
struct span_t {

    static_assert(is_unsigned<T>::value,
                  "span_t is only defined for unsigned types");

    T start_ = 0;
    T size_  = 0;

    constexpr T start()    const { return start_;     }
    constexpr T start(T v)       { return start_ = v; }
    constexpr T size()     const { return size_;      }
    constexpr T size(T v)        { return size_  = v; }

    constexpr bool empty() const { return size_ == 0; }
    constexpr bool valid() const { return empty() || !add_overflows(start_, size_-1); }

    constexpr bool overlaps(const span_t &o) const {
        return !  empty()
            && !o.empty()
            &&    valid()
            &&  o.valid()
            && (start_ <= o.start_
                ?   start_ + (  size_-1) >= o.start_
                : o.start_ + (o.size_-1) >=   start_);
    }

    constexpr span_t(T start, T size) : start_{start}, size_{size} { }
};
