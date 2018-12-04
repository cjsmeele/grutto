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
#include "common/time.hh"
#include "pit.hh"

namespace Time {

    //u64 tick_length = 838_ns;
    u64 tick_length = (double)1/(Pit::hz/Pit::divider) * time_granularity;

    volatile u64 systick_counter;
}

void ksleep(time_t t) {
    auto t_ = uptime();
    while (uptime() < t_ + t)
        asm_hlt();
}
