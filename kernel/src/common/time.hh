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

#include "std/types.hh"

namespace Time {
    constexpr u64 time_granularity = 1'000'000'000; // 1ns
    //constexpr u64 time_granularity = 1'000'000; // 1us
}

using time_t = u64;

constexpr u64 operator ""_ns(u64 n) { return n * (Time::time_granularity / 1'000'000'000); }
constexpr u64 operator ""_us(u64 n) { return n * (Time::time_granularity / 1'000'000); }
constexpr u64 operator ""_ms(u64 n) { return n * (Time::time_granularity / 1'000); }
constexpr u64 operator ""_s (u64 n) { return n * Time::time_granularity; }

namespace Time {

    extern          u64 tick_length;
    extern volatile u64 systick_counter;

}

inline time_t uptime() { return Time::systick_counter * Time::tick_length; }

//inline u64 time_ns(u64 t) { return t / (Time::time_granularity / 1'000'000'000); }
inline time_t time_us(u64 t) { return t / (Time::time_granularity / 1'000'000); }
inline time_t time_ms(u64 t) { return t / (Time::time_granularity / 1'000); }
inline time_t time_s(u64 t)  { return t / Time::time_granularity; }

void ksleep(time_t t);
