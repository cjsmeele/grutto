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
#include "sched.hh"


namespace Sched {

    task_t *current_task_ = nullptr;

    //task_t *current_task() { return current_task_; }
    task_t *current_task() { return nullptr; }

    Queue<own_ptr<task_t>, 64> ready_queue;

    void add_task(own_ptr<task_t> task) {

        ready_queue.enqueue(move(task));
    }

    void switch_task() {
        if (LIKELY(current_task)) {
        }
    }
}
