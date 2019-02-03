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
#include "pci.hh"

namespace Pci {

    Io::Port<u32> port_addr { 0x0cf8 };
    Io::Port<u32> port_data { 0x0cfc };

    u32 make_address(u8 bus, u8 slot, u8 func, u8 offset) {
        return        1 << 31
            | u32{bus}  << 16
            | u32{slot} << 11
            | u32{func} <<  8
            | (offset & 0xfc);
    }

    u32 read_config(u8 bus, u8 slot, u8 func, u8 offset) {

        port_addr.write(make_address(bus, slot, func, offset));
        return port_data.read();
    }
    void write_config(u8 bus, u8 slot, u8 func, u8 offset, u32 value) {

        port_addr.write(make_address(bus, slot, func, offset));
        port_data.write(value);
    }

    // bool device_exists(u8 bus, u8 slot) {
    //     return (read_config(bus, slot, 0, 0) & 0xffff) != 0xffff;
    // }

    void init() {
        // TODO: This is all temporary.
        //       Split it up properly and remove all traces of magic.

        // Enumerate PCI devices.

        auto ll = LL::null;

        for (int bus = 0; bus < 256; ++bus) {
            for (int slot = 0; slot < 32; ++slot) {
                auto x = read_config(bus,slot,0,0);
                if (x == intmax<u32>::value)
                    continue;

                auto f = [ll,bus,slot](u8 g) {
                    auto x = read_config(bus,slot,g,2*4);
                    koi(ll).fmt("class {04x}\n", x >> 16);
                    for (int b = 0; b < 6; ++b) {
                        x = read_config(bus,slot,g,(4+b)*4);

                        if (x) {
                            bool io = x&1;
                            u8 flags = x&0xf;
                            write_config(bus,slot,g,(4+b)*4, ~0xfUL | flags);
                            auto y = (read_config(bus,slot,g,(4+b)*4)&~0xf);
                            write_config(bus,slot,g,(4+b)*4, x);
                            koi(ll)
                               .fmt("   BAR{}: {08x}, {6S} {}\n",
                                    b, x&~0xf, ~y+1,
                                    io ? "IO space" : "memory space");
                        }
                    }
                };

                koi(ll).fmt("* device at bus {} slot {}\n", bus, slot);
                x = read_config(bus,slot,0,3*4);

                if (x >> 16 & 0x80) {
                    // Multi-function, check all functions.
                    for (int function = 0; function < 8; ++function) {
                        x = read_config(bus,slot,function,0);
                        if ((x & 0xffff) != 0xffff) {
                            koi(ll).fmt(" - fn{}:  {08x}, ", function, x);
                            f(function);
                        }
                    }
                } else {
                    koi(ll).fmt(" - fn{}:  {08x}, ", 0, x);
                    f(0);
                }
            }
        }
    }
}
