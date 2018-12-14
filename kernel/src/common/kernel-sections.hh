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

extern unusable_t BOOTSTRAP_LMA;
//extern unusable_t BOOTSTRAP_VMA; // == lma
extern unusable_t KERNEL_LMA;
extern unusable_t KERNEL_VMA;
extern unusable_t KERNEL_SIZE;
extern unusable_t HEAP_START;
//extern unusable_t page_directory;

inline paddr_t bootstrap_lma() { return paddr_t {&BOOTSTRAP_LMA}; }
inline paddr_t kernel_lma()    { return paddr_t {&KERNEL_LMA}; }
inline vaddr_t kernel_vma()    { return vaddr_t {&KERNEL_VMA}; }
inline size_t kernel_size()    { return (size_t)&KERNEL_SIZE; }
inline vaddr_t heap_start()    { return vaddr_t {&HEAP_START}; }

constexpr auto kernel_stack_size = 16_K;
