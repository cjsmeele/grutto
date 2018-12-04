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
#include "pit.hh"
#include "int/int.hh"

namespace Pit {

    void init() {
        // This assumes interrupts are turned OFF.
        Io::outb(0x43, 0x34);
        Io::inb(0x80);
        Io::outb(0x40, divider & 0xff);
        Io::inb(0x80);
        Io::outb(0x40, divider >> 8);
    }

}
