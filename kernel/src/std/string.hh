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

constexpr size_t strlen(const char *str) {
    size_t i = 0;
    while (str[i]) ++i;
    return i;
}

class string_view {
    size_t length_;
    const char *data_;
public:
    constexpr size_t length() const { return length_; }
    constexpr const char *data() const { return data_; }
    constexpr const char &operator[](size_t i) const { return data_[i]; }

    constexpr bool operator==(string_view o) const {
        if (length() != o.length()) return false;
        for (size_t i = 0; i < length(); ++i)
            if ((*this)[i] != o[i])
                return false;
        return true;
    }

    constexpr bool operator!=(string_view o) const {
        return !((*this) == o);
    }

    constexpr string_view(const char *str, size_t length)
        : length_(length),
          data_(str)
        { }
    constexpr string_view(const char *str)
        : length_(strlen(str)),
          data_(str)
        { }
};

constexpr size_t strlen(string_view str) {
    return str.length();
}

constexpr bool streq(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1++ == *s2++);
    return *s1 == *s2;
}
constexpr bool str_prefix_of(const char *prefix, const char *s2) {
    while (*prefix && *s2 && *prefix++ == *s2++);
    return !*prefix || *prefix == *s2;
}

inline char *strdup(string_view str) {
    char *dst = (char*)stdmalloc(str.length()+1);
    // this would make it incompatible (req delete[]).
    //char *dst = new char[str.length()+1];
    Mem::copy(dst, str.data(), str.length());
    dst[str.length()] = 0;
    return dst;
}

inline void strncpy(char *dst, const char *src, size_t mx) {
    if (!mx) return;
    size_t i = 0;
    for (; src[i] && i+1 < mx; ++i)
        dst[i] = src[i];
    dst[i] = 0;
}
