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

#define asm_hlt()  asm volatile ("hlt")
#define asm_cli()  asm volatile ("cli")
#define asm_sti()  asm volatile ("sti")
#define asm_nop()  asm volatile ("nop")

#define asm_read_esp() ([](){ u32 x; asm volatile ("mov %%esp, %0": "=a"(x)); return x; })()
#define asm_read_cr0() ([](){ u32 x; asm volatile ("mov %%cr0, %0": "=a"(x)); return x; })()
#define asm_read_cr2() ([](){ u32 x; asm volatile ("mov %%cr2, %0": "=a"(x)); return x; })()
#define asm_read_cr3() ([](){ u32 x; asm volatile ("mov %%cr3, %0": "=a"(x)); return x; })()

[[noreturn]]
inline void hang() { while (true) { asm_cli(); asm_hlt(); } }
