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
#include "tty-stream.hh"
#include "../multiboot-info.hh"

void TtyStream::put_char(char c) {
    if (tty) tty->put_char(c);
}
void TtyStream::put_string(const char *s) {
    if (tty) tty->put_string(s);
}
char TtyStream::get_char() {
    // We should be talking to some keyboard driver.
    hang();
}

void TtyStream::init_after_mem_init() {

    auto &info = Multiboot::info();

    if (info.flags | MULTIBOOT_INFO_FRAMEBUFFER_INFO
        && info.framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {

        tty_video.init(info.framebuffer_width
                      ,info.framebuffer_height
                      ,info.framebuffer_bpp
                      ,info.framebuffer_pitch
                      ,paddr_t{(size_t)info.framebuffer_addr});

        if (tty_video.available()) {
             tty = &tty_video;
        } else {
            // We are in some video mode, but we are unable to use it.
            // Trying to use it as text mode will not be possible.
            tty = nullptr;
        }
    } else {
        // Keep using text mode.

        tty_text.init_after_mem_init();
    }
}

void TtyStream::init() {
    tty_text.init();
    tty = &tty_text;
}
