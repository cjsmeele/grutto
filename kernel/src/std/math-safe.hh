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

#include "optional.hh"

// Math functions that return nullopt on overflow.

template<typename N, typename M>
constexpr bool add_overflows(N x, M y) {
    N z;
    return __builtin_add_overflow(x, y, &z);
}

template<typename N, typename M>
constexpr auto safe_add(N x, M y) -> Optional<N> {
    N z;
    if (UNLIKELY(__builtin_add_overflow(x, y, &z)))
         return nullopt;
    else return z;
}

template<typename N, typename M>
constexpr auto safe_sub(N x, M y) -> Optional<N> {
    N z;
    if (UNLIKELY(__builtin_sub_overflow(x, y, &z)))
         return nullopt;
    else return z;
}

template<typename N, typename M>
constexpr auto safe_mul(N x, M y) -> Optional<N> {
    N z;
    if (UNLIKELY(__builtin_mul_overflow(x, y, &z)))
         return nullopt;
    else return z;
}
