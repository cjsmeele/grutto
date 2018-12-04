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

#include "common.hh"
#include "multiboot-info.hh"

namespace Cmdline {

    bool get_flag(string_view key) {
        const char *line = Multiboot::cmdline();
        while (*line) {
            if (str_prefix_of(key.data(), line)
                && (!line[key.length()] || isspace(line[key.length()])))
                return true;
            while (*line && !isspace(*line)) line++;
            while (*line &&  isspace(*line)) line++;
        }
        return false;
    }
    //char *get(const char *key);
}
