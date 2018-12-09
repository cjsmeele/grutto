# Copyright (c) 2018, Chris Smeele
#
# This file is part of Grutto.
#
# Grutto is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Grutto is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Grutto.  If not, see <https://www.gnu.org/licenses/>.

.PHONY: all

all: font.bin grutto_small.bin

#banner.bin: banner.txt
#	printf "\x00" | cat $< - > $@

#font-grid.pbm: font-grid.png
#	convert $< $@

font.bin: font-grid.pbm
	mkdir -p tmp
	convert $< -crop 8x16 +repage tmp/char%02x.pbm
	montage -mode concatenate -tile 1x tmp/char*.pbm font-rows.pbm
	# FIXME: Fragile.
	tail -c 4096 font-rows.pbm > $@

grutto_small.bin: grutto2-small.png
	convert $< -color-matrix "0 0 1, 0 1 0, 1 0 0" rgba:$@
