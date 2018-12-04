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

namespace Io {

    /*
    inline void outb(u16 port, u8 val) {
        asm volatile ("out dx, al"::"a"(val), "d" (port));
    }

    inline void outw(u16 port, u16 val) {
        asm volatile ("out dx, ax"::"a"(val), "d" (port));
    }

    inline void outl(u16 port, u32 val) {
        asm volatile ("out dx, eax"::"a"(val), "d" (port));
    }

    inline u8 inb(u16 port) {
        u8 ret;
        asm volatile ("in al, dx":"=a"(ret):"d"(port));
        return ret;
    }

    inline u16 inw(u16 port) {
        u16 ret;
        asm volatile ("in ax, dx":"=a"(ret):"d"(port));
        return ret;
    }

    inline u32 inl(u16 port) {
        u32 ret;
        asm volatile ("in eax, dx":"=a"(ret):"d"(port));
        return ret;
    }
    */



    inline void outb(u16 port, u8 val) {
        asm volatile ("outb %0, %1"::"a"(val), "Nd" (port));
    }

    inline void outw(u16 port, u16 val) {
        asm volatile ("outw %0,%1"::"a"(val), "Nd" (port));
    }

    inline void outl(u16 port, u32 val) {
        asm volatile ("outl %0,%1"::"a"(val), "Nd" (port));
    }

    inline u8 inb(u16 port) {
        u8 ret;
        asm volatile ("inb %1,%0":"=a"(ret):"Nd"(port));
        return ret;
    }

    inline u16 inw(u16 port) {
        u16 ret;
        asm volatile ("inw %1,%0":"=a"(ret):"Nd"(port));
        return ret;
    }

    inline u32 inl(u16 port) {
        u32 ret;
        asm volatile ("inl %1,%0":"=a"(ret):"Nd"(port));
        return ret;
    }
    

    /*
    inline void outb(u16 port, u8 val) {
        asm volatile ("out byte [%1], %0"::"a"(val), "Nd" (port));
    }

    inline void outw(u16 port, u16 val) {
        asm volatile ("out word %0,%1"::"a"(val), "Nd" (port));
    }

    inline void outl(u16 port, u32 val) {
        asm volatile ("out dword %0,%1"::"a"(val), "Nd" (port));
    }

    inline u8 inb(u16 port) {
        u8 ret;
        asm volatile ("in byte %1,%0":"=a"(ret):"Nd"(port));
        return ret;
    }

    inline u16 inw(u16 port) {
        u16 ret;
        asm volatile ("in word %1,%0":"=a"(ret):"Nd"(port));
        return ret;
    }

    inline u32 inl(u16 port) {
        u32 ret;
        asm volatile ("in dword %1,%0":"=a"(ret):"Nd"(port));
        return ret;
    }
    */
}
