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

namespace Mem {

    struct alignas(16) max_align_t { };

    // Provided elsewhere.
    /// Alloc memory
    [[nodiscard]]
    void *kalloc(size_t size);
    void  kfree(void *ptr);

    template<typename T>
    inline void set(T *dest,
                    const typename remove_volatile<T>::type &value,
                    size_t count) {

        for (size_t i = 0; i < count; ++i)
            dest[i] = value;
    }

    template<typename T>
    inline typename enable_if<is_integral<T>::value, void>::type
    zero(T *dest,
         size_t count) {

        for (size_t i = 0; i < count; i++)
            dest[i] = 0;
    }

    template<typename T>
    inline void copy(T *__restrict dest,
                     const T  *__restrict src,
                     size_t count = 1) {

        for (size_t i = 0; i < count; ++i)
            dest[i] = src[i];
    }

    template<typename T, typename F>
    inline void for_each(T *dest,
                         size_t count,
                         F f) {

        for (size_t i = 0; i < count; ++i)
            f(dest[i]);
    }
}

extern "C" void *memcpy(void *__restrict dst, const void *__restrict src, size_t sz);
extern "C" void *memset(void *dst, char c, size_t sz);
