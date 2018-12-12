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
#include "koi-stream.hh"
#include "../cmdline.hh"

void KoiStream::put_char(char c) {
    if (LIKELY(!!default_stream))
        default_stream->put_char(c);
}
void KoiStream::put_string(const char *s) {
    if (LIKELY(!!default_stream))
        default_stream->put_string(s);
}
char KoiStream::get_char() {
    if (LIKELY(!!default_stream))
        return default_stream->get_char();
    else
        hang();
}

IoStream &KoiStream::operator()(LL loglevel) {
    if (loglevel > ll)
        return stream_dummy;
    //if (loglevel <= LL::critical && serial.available())
    //    return serial;
    if (default_stream)
         return *default_stream;
    else return *this; // Welp.
}

void KoiStream::init_after_mem_init() {

    // Time to switch to a graphical terminal, if available.

    bool force_serial = Cmdline::get_flag("serial");
    bool force_tty    = Cmdline::get_flag("tty");


    if (!force_serial) {
        if (default_stream == &tty)
            // Avoid having the TTY print to itself while it is initializing.
            // (if a serial device is connected, it will be used instead)
            default_stream = nullptr;

        tty.init_after_mem_init();
    }

    if (!force_serial && tty.available()) {
        default_stream = &tty;
    } else if (!force_tty && serial.available()) {
        default_stream = &serial;
    } else {
        // We are blind and deaf, nothing to do about it.
        hang();

        // Actually we should sign up for a braille course...
    }
}

void KoiStream::init_after_multiboot() {
    // We should probably get a proper config system instead of doing ad-hoc
    // commandline flag checks.
    if (Cmdline::get_flag("log-critical")) ll = LL::critical;
    if (Cmdline::get_flag("log-error"))    ll = LL::error;
    if (Cmdline::get_flag("log-warning"))  ll = LL::warning;
    if (Cmdline::get_flag("log-notice"))   ll = LL::notice;
    if (Cmdline::get_flag("log-debug"))    ll = LL::debug;
}

void KoiStream::init() {
    // NOTE: At this point, memory management and interrupts are unavailable.
    serial.init();
    tty.init();

    // We don't yet know whether the TTY is in a text mode or not.
    // Better try to use the serial device instead, if it's available.

    if (serial.available())
         default_stream = &serial;
    else default_stream = &tty;

    initialized = true;
}

KoiStream koi;

void stdtrace(const char *s) {
    koi(LL::debug).put_string(s);
}
