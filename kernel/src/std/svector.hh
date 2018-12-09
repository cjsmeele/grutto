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

// Fixed capacity vector

template<typename T, size_t Cap>
class SVector {
    struct null_item_t { char _; };
    union Item {
        null_item_t _;
        T thing;
    };

    size_t length_ = 0;

    Item items[Cap];

public:
    // XXX Protect

    constexpr size_t length() const { return length_; }

    constexpr const T *data() const { return (T*)items; }
    constexpr       T *data()       { return (T*)items; }

    constexpr const T &operator[](size_t i) const { return *(T*)&items[i]; }
    constexpr       T &operator[](size_t i)       { return *(T*)&items[i]; }

    constexpr const T *begin() const { return data();            }
    constexpr const T *end()   const { return data() + length(); }
    constexpr       T *begin()       { return data();            }
    constexpr       T *end()         { return data() + length(); }

    // enable if...
    constexpr bool operator==(const SVector<T,Cap> &o) const {
        if (length() != o.length())
            return false;
        for (size_t i = 0; i < length(); ++i) {
            if ((*this)[i] != o[i])
                return false;
        }
    }

    void push(const T &item) {
        items[length_++].thing = item;
    }

    T pop() {
        T item = items[--length_].thing;
        items[length_].thing.~T();
        items[length_]._ = null_item_t { }; // For good measure.
        return item;
    }

    template<typename F>
    constexpr auto map(F f) const {
        //using FT = result_of...
        using FT = size_t;
        SVector<FT, Cap> o;
        for (const auto &x : *this)
            o.push(f(x));
        return o;
    }

    template<typename F>
    auto transform(F f) {
        for (auto &x : *this)
            f(x);
    }

    template<typename F>
    constexpr T foldl(F f) const {
        if (!length())
            return {};

        T x = (*this)[0];

        for (size_t i = 1; i < length(); ++i)
            x = f(x, (*this)[i]);

        return x;
    }
};
