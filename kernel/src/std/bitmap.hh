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

#include "types.hh"

//#include "../common/panic.hh"
//#include "../common/debug.hh"

template<size_t Nelems, typename T>
struct BitmapCommon {
    static_assert(is_unsigned<T>::value);
    static_assert(is_divisible(Nelems, sizeof(T)));

    constexpr static auto bits_per_entry = sizeof(T) * 8;
    constexpr static auto size           = Nelems / bits_per_entry; // Ts

    u64 length    = Nelems; // bits
    u64 available = Nelems;

    T entries[size] = { };

    using index_t = pair<size_t, u8>;

    index_t index(size_t i) const {
        //assert(i < length, "out-of-bounds in pmm bitmap lookup");
        if (UNLIKELY(i >= length)) {
            // whoops.
            stdfail();
        }

        // NB: The addressing of bits matches the ordering of bits in memory.
        //     That is, within 0b0100 (where T=u4), we say the bit at index 1 is set.
        return { i / bits_per_entry
               , (u8)(bits_per_entry - 1 - (i % bits_per_entry)) };
    }

    [[nodiscard]]
    bool elem_(size_t i) const {
        //size_t ei = i / bits_per_entry;
        //size_t ej = i % bits_per_entry;
        auto [ei, ej] = index(i);

        return bit_get(entries[ei], ej);
    }

    bool elem_(size_t i, bool val) {
        auto [ei, ej] = index(i);

        if (bit_get(entries[ei], ej) != val) {
            bit_set(entries[ei], ej, val);
            available += val ? -1 : 1;
        }
        return val;
    }

    Optional<size_t> find_contiguous_(size_t start, size_t count, size_t align = 0) const {
        auto can_continue = [&] (size_t from_entry, size_t count) -> bool {
            size_t ei = from_entry;
            T entry = entries[ei];
            // Consume whole entries.
            for (; count > bits_per_entry && ei < size
                 ; count -= bits_per_entry, entry  = entries[++ei])
                if (entry) return false;

            return !count || leading_0s(entry) >= count;
        };

        auto start_ei  = start / bits_per_entry;
        auto start_off = start % bits_per_entry;

        for (size_t ei = start_ei; ei < size; ++ei) {
            if (align && ei % align)
                continue;

            const T entry = entries[ei];
            if (!align && count <= bits_per_entry && count <= count_0s(entry)) {
                // We can try to fit this into one entry.
                for (size_t shift = ei == start_ei ? start_off : 0
                     ; shift <= bits_per_entry - count
                     ; shift++) {
                    if (count <= leading_0s((T)(entry << shift)))
                        return ei * bits_per_entry + shift;
                }
            }

            size_t trails = trailing_0s(entry);
            if (ei == start_ei)
                trails = min(trails, bits_per_entry - start_off);

            if (trails && ei + 1 < size && (!align || trails == bits_per_entry)) {
                if (can_continue(ei + 1, count - trails))
                    return ei * bits_per_entry + (bits_per_entry - trails);
            }
            //koi.fmt("wtf? {016b}, {}trails (size {})\n", entry, trails, size);
        }

        return nullopt;
    }

    void set_range_(size_t i, size_t count, bool val) {
        if (!count) return;

        auto [ei, ej] = index(i);
        auto *entry   = &entries[ei];
        if (ej != bits_per_entry-1) {
            // Set leading bits.
            auto rej = bits_per_entry - 1 - ej;
            auto n = min(count, bits_per_entry - rej);
            *entry = set_rbits(*entry, bits_per_entry - 1 - ej, n, val);

            // Assumes no overlap (!).
            if (val) available -= n;
            else     available += n;
            count -= n;
            entry++;
        }

        for (; count >= bits_per_entry; count -= bits_per_entry, entry++) {
            *entry = val ? ~0 : 0;
            if (val) available -= bits_per_entry;
            else     available += bits_per_entry;
        }

        // Set trailing.
        if (count) {
            // Set leading bits.
            *entry = set_rbits(*entry, 0, count, val);
            entry++;
            if (val) available -= count;
            else     available += count;
        }
    }
};

template<size_t Nelems, size_t Ndepth = 1, typename T = u32>
struct Bitmap;

template<size_t Nelems, typename T>
struct Bitmap<Nelems, 1, T> : public BitmapCommon<Nelems, T> { 

    using BitmapCommon<Nelems, T>::elem_;
    using BitmapCommon<Nelems, T>::find_contiguous_;
    using BitmapCommon<Nelems, T>::set_range_;

    [[nodiscard]]
    bool elem(size_t i)     const { return elem_(i);      }
    bool elem(size_t i, bool val) { return elem_(i, val); }

    void set_range(size_t i, size_t count, bool val)
    { return set_range_(i, count, val); }

    Optional<size_t> find_contiguous(size_t start, size_t count, size_t align = 0) const
    { return find_contiguous_(start, count, align); }

    //Optional<size_t> alloc(size_t count) {
    //    auto x = find_contiguous(count, align);
    //    if (x) set_range(*x, count, true);
    //    return x;
    //}
};

template<size_t Nelems, size_t Ndepth, typename T>
struct Bitmap : public BitmapCommon<Nelems, T> {

    using BitmapCommon<Nelems, T>::elem_;
    using BitmapCommon<Nelems, T>::bits_per_entry;

    static_assert(Nelems && false, "nested bitmaps unimplemented");
    static_assert(is_divisible(Nelems, bits_per_entry),
                  "amount of bitmap entries must be divisible by bits per elemnt in order to nest deeper");

    Bitmap<Nelems / bits_per_entry, Ndepth - 1> parent;

    [[nodiscard]]
    bool elem(size_t i)           { return elem_(i);      }
    bool elem(size_t i, bool val) { return elem_(i, val); }

    // TODO

    //void set_range(size_t i, size_t count, bool val)

    // vvv Optimize this one vvv
    //Optional<size_t> find_contiguous(size_t count)
};
