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
#include "panic.hh"

void stdfail() {
    asm_magic();
    panic();
}

void mallocfail() {
    asm_magic();
    panic();
}
void mallocfail_corrupt(void *m) {
    stdtrace("kernel malloc corruption.\n");
    asm_magic();
    mallocfail();
}

void mallocfail_pebcak(void *m, void *p) {
    stdtrace("kernel malloc usage error.\n");
    mallocfail();
}

void panic() {
    asm_cli();

    while (true)
        asm_hlt();
}
