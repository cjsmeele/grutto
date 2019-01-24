# Copyright (c) 2019, Chris Smeele
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

MAKEFLAGS += --no-print-directory

SOURCE_DIRS := \
	hello      \
	hello2

SOURCE_BINS := $(SOURCE_DIRS:%=%/program.elf)

INITRD := initrd.bin

.PHONY: all clean initrd $(SOURCE_DIRS)


all: initrd

initrd: $(SOURCE_DIRS)
	$(MAKE) $(INITRD)

$(INITRD): mkinitrd.pl $(SOURCE_BINS)
	./mkinitrd.pl $(foreach DIR,$(SOURCE_DIRS),$(DIR).elf=$(DIR)/program.elf) > $(INITRD)

#$(SOURCE_BINS): %/program.elf: %

$(SOURCE_DIRS):
	$(MAKE) -C $@

clean:
	rm -f $(INITRD)
	for i in $(SOURCE_DIRS); do $(MAKE) -C "$$i" clean; done
