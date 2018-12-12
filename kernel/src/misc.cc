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
#include "common.hh"

extern "C" void __cxa_pure_virtual();
extern "C" void __cxa_pure_virtual() {
    // This shouldn't happen.
    panic();
}

extern "C" void __cxa_atexit(void (*)(void*), void*, void*);
extern "C" void __cxa_atexit(void (*)(void*), void*, void*) {
    // Don't care, it's not like kmain will ever return :-)
}
