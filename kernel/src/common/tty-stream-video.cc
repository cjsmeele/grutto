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
#include "tty-stream.hh"
// XXX
#include "../mem/vmm.hh"

constexpr u32 color(u32 rgb)          { return rgb & 0xffffff; }
constexpr u32 color(u8 r, u8 g, u8 b) { return (u32)r << 16 | (u32)g << 8 | b; }

constexpr u32 adjust(u32 c, s32 x) {
    auto f = [=](auto &y) { y = clamp(0,255, y * x / 100); };
    u8 a = c >> 24; f(a);
    u8 r = c >> 16; f(r);
    u8 g = c >>  8; f(g);
    u8 b = c;       f(b);
    return color(r, g, b);
}

static constexpr u32 bg  = color(0x242418);
static constexpr u32 fg  = color(0xffffff);
static constexpr u32 bg2 = adjust(bg, 120);

//constexpr u32 maxw = 120;
constexpr u32 maxh = 40;
constexpr u32 maxw = 120;
//constexpr u32 maxh = intmax<u32>::value;

u32 TtyVideoOutput::cols() const {
    return min(maxw, width  / cw);
}
u32 TtyVideoOutput::rows() const {
    return min(maxh, height / ch);
}

void TtyVideoOutput::put_char(char c) {
    auto cursor_it = [&] () {
        for (u32 cy = 0; cy < ch; ++cy) {
            for (u32 cx = 0; cx < cw/4; ++cx) {
                //auto &x = fb[(fby*ch+cy)*width + fbx*cw + cx];
                //x = ~x;
            }
        }
    };
    cursor_it();

    if (c == '\r') {
        fbx = 0;
    } else if (c == '\b') {
        if (fbx) --fbx;
    } else if (c == '\n') {
        fbx = 0;
        fby++;
    } else {
        for (u32 cy = 0; cy < ch; ++cy) {
            for (u32 cx = 0; cx < cw; ++cx) {
                bool bit  = bit_get(font[(u8)c*ch+cy], cw-1-cx);
                bool bit_ = cx&&cy && bit_get(font[(u8)c*ch+cy-1], cw-1-cx+1);
                u32 color = bit ? fg : bit_ ? 0 : bg;
                if (color == fg)
                    color = adjust(fg, 100 - (cy*100 / ch / 2));
                fb[choff(fbx, fby) + cy*width + cx] = color;
            }
        }
        ++fbx;
    }
    if (fbx >= cols()) {
        fby++; fbx = 0;
    }
    if (fby >= rows())
        scroll(1);

    cursor_it();
}
void TtyVideoOutput::put_string(const char *s) {
    while (*s)
        put_char(*(s++));
}

void TtyVideoOutput::clear() {
    Mem::set(fb, bg2, width*height);
    for (u32 y = 0; y < rows()*ch; ++y)
        Mem::set(fb+choff(0, y/ch)+y%ch*width, bg, cols()*cw);
}
void TtyVideoOutput::scroll(int n) {
    while (n > 0) {
        for (u32 y = 0; y < rows()-1; ++y) {
            for (u32 cy = 0; cy < ch; ++cy) {
                Mem::copy(fb+(choff(0,y  )+cy*width),
                          fb+(choff(0,y+1)+cy*width),
                          cols()*cw);
            }
        }
        for (u32 cy = 0; cy < ch; ++cy)
            Mem::set(fb+(choff(0,rows()-1)+cy*width), bg, cols()*cw);

        --fby;
        --n;
    }
}
void TtyVideoOutput::load_font(u8 *f, u32 fw, u32 fh) {
    assert(fw == 8 && fh == 16, "other font dimensions are not yet supported");
    font = f;
    cw = fw;
    ch = fh;
}

void TtyVideoOutput::init(u32 w, u32 h, u32 bpp, u32 pitch, addr_t pa_framebuffer) {

    koi(LL::debug).fmt("video parameters: {}x{}x{}, pitch {}, fb @phy {08x}\n",
                       w, h, bpp, pitch,
                       pa_framebuffer);

    if (bpp   == 32
     && pitch == w*4
     && pa_framebuffer.is_aligned(4)) {


        // XXX
        assert(w*h*(bpp/8) < Vmm::va_framebuffer_max_size, "framebuffer too large");

        // FIXME: Actually write something that allocates virtual memory.
        Vmm::map_pages(Vmm::va_framebuffer
                      ,pa_framebuffer
                      ,div_ceil(w*h*(bpp/8), Vmm::page_size));

        fb = (u32*)Vmm::va_framebuffer;

        width  = w;
        height = h;

        RESDECL_(font, f);
        load_font(f, 8, 16);

        clear();

        // Draw a grutto.
        {
            RESDECLT_(grutto_small, pic, u32); u32 pw = 244; u32 ph = 144;
            for (size_t y = 0; y < ph; ++y) {
                for (size_t x = 0; x < pw; ++x) {
                    auto c = pic[y*pw+x];
                    if (c & 0xff000000) {
                        fb[choff(0,0) + (rows()*ch-(ph+1)+y  )*width + (cols()*cw-(pw+1)+x  )] = adjust(bg,  80);
                        fb[choff(0,0) + (rows()*ch-(ph+1)+y+1)*width + (cols()*cw-(pw+1)+x+1)] = adjust(bg, 150);
                        fb[choff(0,0) + (rows()*ch-(ph+1)+y+2)*width + (cols()*cw-(pw+1)+x+2)] = adjust(bg, 130);
                    }
                }
            }
        }

        available_ = true;

    } else {
        // Unsupported video mode.
    }
}
