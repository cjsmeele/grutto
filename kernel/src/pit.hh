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

#include "common.hh"

namespace Pit {

    constexpr u64 hz      = 1193182;
    //constexpr u16 divider =    1193/16; // ~16 KHz system clock.
    //constexpr u16 divider =    1193/8; // ~8 KHz system clock.
    //constexpr u16 divider =    1193/4; // ~4 KHz system clock.
    //constexpr u16 divider =    1193/2; // ~2 KHz system clock.
    //constexpr u16 divider =  1*1193; // ~1 KHz system clock.
    //constexpr u16 divider =  2*1193; // 500 Hz system clock.
    constexpr u16 divider =  4*1193; // 250 Hz system clock.
    //constexpr u16 divider =  8*1193; // 125 Hz system clock.
    //constexpr u16 divider = 16*1193; // 68 Hz system clock.
    //constexpr u16 divider = 32*1193;  // 34 Hz system clock.

    void init();

}
