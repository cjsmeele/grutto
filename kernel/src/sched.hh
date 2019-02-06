/* Copyright (c) 2019, Chris Smeele
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
#include "task.hh"
#include "int/handlers.hh" // XXX

namespace Sched {

    // How many timer ticks each task gets before they must yield.
    constexpr inline size_t jiffies_per_slice = 1;

    task_t *current_task();

    // Enqueue a task.
    void add_task(own_ptr<task_t> task);

    // Start running a task (used only for the first task).
    void exec_task(own_ptr<task_t> task);

    // Switch to another task if there's any ready tasks and the current's
    // timeslice is up.
    void maybe_switch_task(Int::interrupt_frame &frame);

    void init();
}
