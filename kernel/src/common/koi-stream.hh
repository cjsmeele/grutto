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

#include "common/serial-stream.hh"
#include "common/tty-stream.hh"

enum class LL {
    critical = 0,
    error,
    warning,
    notice,
    debug,
    null,
};

class KoiStream : public IoStream {
    SerialStream<0> serial;
    TtyStream       tty;

    LL ll = LL::notice;

    IoStream *default_stream = nullptr;

    bool initialized = false;

public:
    void put_char(char c);
    void put_string(const char *s);
    void put(const char *s, size_t count);
    char get_char();

    IoStream &operator()(LL loglevel);

    void init();
    void init_after_multiboot();
    void init_after_mem_init();
};

extern KoiStream koi;
