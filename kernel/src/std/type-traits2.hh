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

#include "types2.hh"

template<typename T>
struct intmax {
    constexpr static size_t value = is_signed<T>::value
        ? ~T(1 << (sizeof(T)*8-1)) : static_cast<T>(-1);
};
template<typename Tag>
struct intmax<addr_base_t<Tag>> {
    constexpr static size_t value = intmax<typename addr_base_t<Tag>::type>::value;
};
