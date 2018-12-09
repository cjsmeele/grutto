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
#include "stream.hh"

void OStream::put_string(const char *s) {
    while (s)
        put_char(*(s++));
}

OStream &operator<<(OStream &io, char c) {
    io.put_char(c);
    return io;
}

OStream &operator<<(OStream &io, const char *s) {
    io.put_string(s);
    return io;
}

OStream &operator<<(OStream &io, u64 n) {
    constexpr size_t size = 32;
    char buf[size];
    buf[size - 1] = '\0';
    buf[size - 2] = '0';

    size_t i;
    for (i = 0; n; ++i, n /= 10) {
        buf[size-2 - i] = '0' + (n%10);
    }

    return io << (buf + size - 1 - (i ? i : 1));
}

OStream &operator<<(OStream &io, s64 n) {
    if (n < 0) {
        io << '-';
        n = -n;
    }
    return io << static_cast<u64>(n);
}

OStream &operator<<(OStream &io, s32 n) { return io << (s64)n; }
OStream &operator<<(OStream &io, s16 n) { return io << (s64)n; }
OStream &operator<<(OStream &io, u32 n) { return io << (u64)n; }
OStream &operator<<(OStream &io, u16 n) { return io << (u64)n; }

OStream &operator<<(OStream &io, const void *ptr) {

    constexpr size_t size = sizeof(void*) * 2 + 1;

    char buf[size];
    buf[size - 1] = '\0';
    buf[size - 2] = '0';

    auto n = addr_t{ptr}.u();

    size_t i;
    for (i = 0; n; ++i, n >>= 4) {
        char x = n & 0xf;
        buf[size-2 - i] = (x <= 9 ? '0' + x : 'a' + (x-10));
    }

    for (; i < size-1; ++i)
        buf[size-2 - i] = '0';

    return io << (buf + size - 1 - i);
}

IStream &operator>>(IStream &io, char &c) {
    c = io.get_char();
    return io;
}

DummyStream stream_dummy;
