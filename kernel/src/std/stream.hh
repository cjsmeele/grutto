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

#include "std/types.hh"
#include "std/fmt.hh"

class OStream;


class OStream {

    struct FmtCallback {
        OStream &os;
        void operator()(char c)                  { os.put_char(c);   }
        void operator()(const char *s)           { os.put_string(s); }
        void operator()(const char *s, size_t n) { os.put(s, n);     }
    };

public:
    virtual void put_char(char c) = 0;
    virtual void put_string(const char *c);
    virtual void put(const char *s, size_t count);

    template<typename... As>
    void fmt(const char *s, const As&... as) {
        ::fmt(FmtCallback{*this}, s, as...);
    }
};

class IStream {
public:
    virtual char get_char() = 0;
};

class IoStream : public IStream, public OStream { };

class DummyStream : public IoStream {
public:
    void put_char(char c) { }
    void put_string(const char *c) { }
    void put(const char *s, size_t count) { }
    char get_char() { stdfail(); }
};

extern DummyStream stream_dummy;
