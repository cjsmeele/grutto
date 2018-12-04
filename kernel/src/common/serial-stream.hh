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

#include "std/stream.hh"
#include "common/ioport.hh"
#include "int/int.hh"

template<u16 PortNumber = 0>
class SerialStream : public IoStream {

    static_assert(PortNumber < 4);

    u16 port = 0;

    void put_char_(char c) {
        // Busy wait till Transmit Holding Register Empty.
        while (!(Io::inb(port + 5) & 0x20))
            Io::inb(0x80);
        Io::outb(port, static_cast<u8>(c));
    }

public:
    void put_char(char c) {

        if (LIKELY(port)) {
            bool ie = Int::enabled();
            asm_cli();
            put_char_(c);
            if (ie) asm_sti();
        }
    }

    void put_string(const char *s) {

        if (LIKELY(port)) {

            bool ie = Int::enabled();
            asm_cli();
            while (*s)
                put_char_(*(s++));
            if (ie) asm_sti();
        }
    }

    char get_char() {

        if (LIKELY(port)) {
            // Busy wait till Receive Buffer Full.
            while (!(Io::inb(port + 5) & 1))
                asm_nop();

            return Io::inb(port);

        } else {
            while (true)
                asm_nop();
        }
    }

    bool available() const {
        return !!port;
    }

    void init() {
        // NOTE: At this point, memory management and interrupts are likely to be unavailable.

        // Look up COM0 io port in BDA.
        port = *((u16*)0x0400 + PortNumber);

        if (port) {
            Io::outb(port + 1, 0x00); // Disable all interrupts
            Io::outb(port + 3, 0x80); // Enable DLAB
            Io::outb(port + 0, 0x01); // Divisor LSB (115200 baud)
            Io::outb(port + 1, 0x00); // Divisor MSB
            Io::outb(port + 3, 0x03); // 8 bits, no parity, one stop bit
            Io::outb(port + 2, 0x07); // Enable FIFO, clear it, with 14-byte threshold
            //Io::outb(port + 4, 0x0b); // IRQs enabled, RTS/DSR set
            Io::outb(port + 4, 0x00); // Disable interrupts.
        }
    }
};
