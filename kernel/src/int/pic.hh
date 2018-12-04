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
#include "int.hh"

namespace Int::Pic {

    constexpr u16 master_port = 0x20;
    constexpr u16  slave_port = 0xa0;

    constexpr u8 cmd_eoi  = 0x20; ///< End Of Interrupt.
    constexpr u8 cmd_init = 0x10;

    inline void ack(bool slave = false) {
        if (slave)
            Io::outb(slave_port, cmd_eoi);
        Io::outb(master_port, cmd_eoi);
    }

    void init();
}
