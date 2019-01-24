# Copyright (c) 2018, 2019, Chris Smeele
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

# Parameters {{{

VERBOSE := 1

ifdef VERBOSE
Q :=
E := @true 
else
Q := @
E := @echo 
endif

MAKEFLAGS += --no-print-directory

TARGET ?= i686
export TARGET

# }}}
# Package metadata {{{

NAME    := grutto32
VERSION := $(shell git describe --always --dirty)

ifndef PACKAGE_VERSION
PACKAGE_VERSION := 9999
endif

NAME_VERSION := $(NAME)-$(VERSION)

# }}}
# Directories {{{

KERNEL_DIR := kernel
INITRD_DIR := initrd

# }}}
# Source and intermediate files {{{


# }}}
# Output files {{{

BINFILE := $(KERNEL_DIR)/bin/$(NAME).elf
INITRD  := $(INITRD_DIR)/initrd.bin

# }}}

.PHONY: all doc clean disk kernel initrd

all: kernel

-include Makefile.local

# Make targets {{{

#$(BINFILE): kernel

# NB: Currently the initrd is embedded into the kernel image.
#     Ideally, the bootloader would load it at some page-aligned address for us
#     so that we can reclaim that space.
kernel: initrd
	$(MAKE) -C $(KERNEL_DIR)

initrd:
	$(MAKE) -C $(INITRD_DIR)

clean:
	$(E) "  CLEAN"
	$(MAKE) -C $(KERNEL_DIR) clean
	$(MAKE) -C $(INITRD_DIR) clean
	rm -f $(DISK_FILE)


DISK_SIZE_M  ?= 72
DISK_DIR     ?= ./disk
DISK_FILE    ?= /tmp/$(NAME)-boot.img
DISK_FS      ?= $(DISK_FILE)@@1M
DISK_FSID    ?= 42424242
DISK_FSLABEL ?= STOOMLDR
DISK_BOOTDIR ?= boot

LOADER_RCFILE ?= ./loader.rc

STAGE1_MBR_BIN ?= stoomboot-stage1-mbr.bin
STAGE2_BIN     ?= stoomboot-stage2.bin

# If you uncomment these lines, we will automatically build stoomboot and use a
# newly built stage1&2.
#STAGE1_MBR_BIN := stoomboot/stage1/bin/stoomboot-stage1-mbr.bin
#STAGE2_BIN     := stoomboot/stage2/bin/stoomboot-stage2.bin

#QEMU    ?= qemu-system-i386
QEMU    ?= qemu-system-x86_64
GDB     ?= gdb

QEMUMEM ?= 512

#QEMUFLAGS       := -m $(QEMUMEM)M -name "osdev" -net none -serial none -vga std \
#	-drive file=$(DISK_FILE),if=ide,media=disk,format=raw -d guest_errors \

QEMUFLAGS       := -m $(QEMUMEM)M -name "osdev" -net none -serial none -vga std \
	-drive file=$(DISK_FILE),if=ide,media=disk,format=raw -d guest_errors \
	-enable-kvm
# -display sdl

QEMUFLAGS_DEBUG := -m $(QEMUMEM)M -name "osdev" -net none -serial none -vga std \
	-drive file=$(DISK_FILE),if=ide,media=disk,format=raw -d guest_errors \
	-s -S -monitor stdio

QEMUFLAGS_FAST  := -m $(QEMUMEM)M -name "osdev" -net none -serial stdio -vga std \
	-drive file=$(DISK_FILE),if=ide,media=disk,format=raw -d guest_errors \
	-enable-kvm -display none -kernel $(BINFILE) -append "log-debug serial"

ifdef SERIAL_IO
QEMUFLAGS += -serial stdio
endif
ifdef NOVGA
QEMUFLAGS += -display none
endif

GDB := gdb

GDBFLAGS := -q -n -x gdbrc32

run: disk
	$(E) "  QEMU     $<"
	$(Q)$(QEMU) $(QEMUFLAGS)

fast: disk
	$(E) "  QEMU     $<"
	$(Q)$(QEMU) $(QEMUFLAGS_FAST)

run-debug: disk
	$(E) "  QEMU     $<"
	$(Q)$(QEMU) $(QEMUFLAGS_DEBUG)

disk: kernel
	$(Q)$(MAKE) $(DISK_FILE)

$(DISK_FILE): $(STAGE1_MBR_BIN) $(STAGE2_BIN) $(BINFILE) $(LOADER_RCFILE)
	$(E) ""
	$(E) "Disk Image"
	$(E) "=========="
	$(Q)mkdir -p $(@D)
	rm -f $(DISK_FILE)
	truncate -s$(DISK_SIZE_M)M $(DISK_FILE)
	parted -s $(DISK_FILE) unit MiB mklabel msdos mkpart primary fat32 1 '100%'
	mformat -i $(DISK_FS) -FN $(DISK_FSID) -v $(DISK_FSLABEL)
	mmd     -i $(DISK_FS) /$(DISK_BOOTDIR)
	mcopy   -i $(DISK_FS) $(LOADER_RCFILE) ::/$(DISK_BOOTDIR)
	mcopy   -i $(DISK_FS) $(BINFILE)       ::/$(DISK_BOOTDIR)
	mcopy   -i $(DISK_FS) -s $(DISK_DIR)/* ::/
	stoomboot/tools/loader-install.pl \
	    --mbr \
	    --stage1-mbr   $(STAGE1_MBR_BIN) \
	    --stage2       $(STAGE2_BIN) \
	    --stage2-lba   1 \
	    --loader-fs-id $(DISK_FSID) \
	    --img          $(DISK_FILE)

stoomboot/stage1/bin/stoomboot-stage1-mbr.bin:
	$(MAKE) -C stoomboot stage1-mbr-bin
stoomboot/stage2/bin/stoomboot-stage2.bin:
	$(MAKE) -C stoomboot stage2-bin

# }}}
