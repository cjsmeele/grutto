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
#include "std/fmt.hh"

class OStream {
public:
    virtual void put_char(char c) = 0;
    virtual void put_string(const char *c);

    template<typename... As>
    void fmt(const char *s, const As&... as) {
        ::fmt([this](auto c) { put_char(c); }, s, as...);
    }

};

class IStream {
public:
    virtual char get_char() = 0;
};

class IoStream : public IStream, public OStream {
};

class DummyStream : public IoStream {
public:
    void put_char(char c) { }
    void put_string(const char *c) { }
    char get_char() { stdfail(); }
};

extern DummyStream stream_dummy;


OStream &operator<<(OStream &io, char c);
OStream &operator<<(OStream &io, const char *s);

OStream &operator<<(OStream &io, u64 n);
OStream &operator<<(OStream &io, s64 n);
OStream &operator<<(OStream &io, u32 n);
OStream &operator<<(OStream &io, s32 n);
OStream &operator<<(OStream &io, u16 n);
OStream &operator<<(OStream &io, u16 n);
OStream &operator<<(OStream &io, const void *ptr);

IStream &operator>>(IStream &io, char &c);

