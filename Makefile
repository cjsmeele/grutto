# Copyright (c) 2014 - 2018, Chris Smeele
# All rights reserved.
#
# Please consult LICENSE for license information.

# Parameters {{{

ifdef VERBOSE
Q :=
E := @true 
else
Q := @
E := @echo 
MAKEFLAGS += --no-print-directory
endif

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

KERNELDIR := kernel

# }}}
# Source and intermediate files {{{


# }}}
# Output files {{{

BINFILE := $(KERNELDIR)/bin/$(NAME).elf

# }}}

.PHONY: all doc clean disk kernel

all: kernel

-include Makefile.local

# Make targets {{{

$(BINFILE): kernel

kernel:
	$(MAKE) -C $(KERNELDIR)

clean:
	$(E) "  CLEAN"
	$(MAKE) -C $(KERNELDIR) clean
	rm -f $(DISK_FILE)


DISK_SIZE_M  ?= 72
DISK_DIR     ?= ./disk
DISK_FILE    ?= /tmp/$(NAME)-boot.img
DISK_FS      ?= $(DISK_FILE)@@1M
DISK_FSID    ?= 42424242
DISK_FSLABEL ?= STOOMLDR
DISK_BOOTDIR ?= boot

LOADER_RCFILE ?= ./loader.rc

STAGE1_MBR_BIN ?= stoomboot/stage1/bin/stoomboot-stage1-mbr.bin
STAGE2_BIN     ?= stoomboot/stage2/bin/stoomboot-stage2.bin

#QEMU    ?= qemu-system-i386
QEMU    ?= qemu-system-x86_64
GDB     ?= gdb

QEMUMEM ?= 1024

#QEMUFLAGS       := -m $(QEMUMEM)M -name "osdev" -net none -serial none -vga std \
#	-drive file=$(DISK_FILE),if=ide,media=disk,format=raw -d guest_errors \

QEMUFLAGS       := -m $(QEMUMEM)M -name "osdev" -net none -serial none -vga std \
	-drive file=$(DISK_FILE),if=ide,media=disk,format=raw -d guest_errors \
	-enable-kvm

QEMUFLAGS_DEBUG := -m $(QEMUMEM)M -name "osdev" -net none -serial none -vga std \
	-drive file=$(DISK_FILE),if=scsi,media=disk,format=raw -d guest_errors \
	-s -S -monitor stdio

ifdef SERIAL_IO
QEMUFLAGS += -serial stdio
endif
ifdef NOVGA
QEMUFLAGS += -display none
endif

GDB := gdb

GDBFLAGS := -q -n -x gdbrc32

# nb: dep on kernel needed to avoid 2-step build.
run: kernel $(DISK_FILE)
	$(E) "  QEMU     $<"
	$(Q)$(QEMU) $(QEMUFLAGS)

run-debug: $(DISK_FILE)
	$(E) "  QEMU     $<"
	$(Q)$(QEMU) $(QEMUFLAGS_DEBUG)

disk: $(DISK_FILE)

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
	    --stage1-mbr   stoomboot/stage1/bin/stoomboot-stage1-mbr.bin \
	    --stage2       stoomboot/stage2/bin/stoomboot-stage2.bin \
	    --stage2-lba   1 \
	    --loader-fs-id $(DISK_FSID) \
	    --img          $(DISK_FILE)

$(STAGE1_MBR_BIN):
	$(MAKE) -C stoomboot stage1-mbr-bin
$(STAGE2_BIN):
	$(MAKE) -C stoomboot stage2-bin

# }}}
