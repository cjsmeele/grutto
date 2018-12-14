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

void TtyTextOutput::scroll(int n) {
    while (n > 0) {
        for (u8 y = 0; y < height-1; ++y) {
            Mem::copy(&fb[y     * width],
                      &fb[(y+1) * width],
                      width);
        }
        Mem::set(&fb[(height-1) * width], u16{0}, width);
        --fby;
        --n;
    }
}

void TtyTextOutput::put_char(char c) {
    if (c == '\r') {
        fbx = 0;
    } else if (c == '\n') {
        fbx = 0;
        fby++;
    } else {
        fb[fby*width + fbx++] = 0x0700 | c;
    }
    if (fbx >= width) {
        fby++; fbx = 0;
    }
    if (fby >= height)
        scroll(1);
}
void TtyTextOutput::put_string(const char *s) {
    while (*s)
        put_char(*(s++));
}

void TtyTextOutput::init_after_mem_init() {
    // The first 1M is no longer identity-mapped, we need to remap it at the right spot.
    Vmm::map_pages(Vmm::va_framebuffer
                  ,pa_fb
                  ,div_ceil(width*height*2, page_size));

    fb = (u16*)Vmm::va_framebuffer;
}

void TtyTextOutput::init() {
}
