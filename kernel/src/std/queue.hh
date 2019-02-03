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
#pragma once

#include "types.hh"

// Fixed capacity queue (ring buffer)

template<typename T, size_t Cap>
class FixedQueue {

    struct null_item_t { char _; };
    union Item {
        null_item_t _;
        T thing;

         Item() : _{} {}
        ~Item() { }
    };

    size_t length_ = 0;

    Item items[Cap];

    size_t i_writer = 0; // -> where the next item is written.
    size_t i_reader = 0; // -> where the next item is read.

    void increment(size_t &i) {
        if (UNLIKELY(i == Cap - 1))
             i = i + 1;
        else i = 0;
    };

public:

    size_t length() const { return length_; }
    size_t size()   const { return Cap; }

    void enqueue(const T &item) {
        if (UNLIKELY(length_ == Cap)) {
            stdtrace("Queue overflow");
            stdfail();
        }
        items[i_writer].thing = item;
        increment(i_writer);

        ++length_;
    }
    void enqueue(T &&item) {
        if (UNLIKELY(length_ == Cap)) {
            stdtrace("Queue overflow");
            stdfail();
        }
        items[i_writer].thing = move(item);
        increment(i_writer);

        ++length_;
    }


    T dequeue() {
        if (UNLIKELY(length_ == 0)) {
            stdtrace("Queue underflow");
            stdfail();
        }

        T item = move(items[i_reader].thing);

        items[i_reader].thing.~T();
        items[i_reader]._ = null_item_t { }; // For good measure.

        increment(i_reader);
        --length_;

        return move(item);
    }

    ~FixedQueue() {
        while (length_) dequeue();
    }
};


template<typename T, size_t Cap>
using Queue = FixedQueue<T, Cap>;
