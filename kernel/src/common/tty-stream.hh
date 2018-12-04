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

#include "std/stream.hh"

class TtyTextOutput : public OStream {

    static constexpr addr_t fb_addr = 0xb8000;
    static constexpr u32    width   = 80;
    static constexpr u32    height  = 25;

    volatile u16 *fb = (u16*)fb_addr;

    u8 fbx = 0;
    u8 fby = 0;

    void scroll(int);

public:
    void put_char(char c);
    void put_string(const char *s);

    void init();
};

class TtyVideoOutput : public OStream {

    bool available_ = false;

    volatile u32 *fb = nullptr;

    u32 width  = 0;
    u32 height = 0;

    u32 cw = 0;
    u32 ch = 0;

    u32 off_x() const { return (width  / cw - cols()) / 2 * cw; }
    u32 off_y() const { return (height / ch - rows()) / 2 * ch; }

    u32 choff(u32 x, u32 y) const { return (off_y() + y*ch)*width + off_x() + x * cw; }

    u32 cols() const;
    u32 rows() const;

    u32 fbx = 0;
    u32 fby = 0;

    u8 *font = nullptr;

public:
    bool available() const { return available_; }

    void clear();
    void scroll(int);
    void load_font(u8 *f, u32 fw, u32 fh);

    void put_char(char c);
    void put_string(const char *s);

    void init(u32 width, u32 height, u32 bpp, u32 pitch, void *framebuffer);
};

class TtyStream : public IoStream {

    TtyTextOutput  tty_text;
    TtyVideoOutput tty_video;

    OStream *tty = nullptr;

public:
    bool available() const { return tty; }

    void put_char(char c);
    void put_string(const char *s);
    char get_char();

    void init_after_mem_init();
    void init();
};
