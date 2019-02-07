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

#VERBOSE:=1

ifdef VERBOSE
Q :=
E := @true 
else
Q := @
E := @echo 
MAKEFLAGS += --no-print-directory
endif

# One of i686, amd64.
# (currently only supports i686)
TARGET ?= i686

# }}}
# Package metadata {{{

NAME     := grutto32
VERSION  := $(shell git describe --always)
VERSIOND := $(shell git describe --always --dirty)

ifndef VERSION
VERSION  := 9999
VERSIOND := 9999
endif

ifneq (,$(findstring dirty,$(VERSIOND)))
	DIRTY_FLAG := -DKERNEL_VDIRTY
endif

NAME_VERSION := $(NAME)-$(VERSION)

# }}}
# Directories {{{

SRCDIR := src
RESDIR := res
OBJDIR := obj
BINDIR := bin
DEPDIR := dep

# }}}
# Source files {{{

ASMFILES := $(shell find src -name '*.asm')

CXXFILES := $(shell find src -name '*.cc')
HFILES   := $(shell find src -name '*.hh')
PCHDEPS  := $(shell find src/common src/std -name '*.hh')

include $(RESDIR)/resource-names.make
RESFILES := $(RESOURCE_NAMES:%=$(RESDIR)/%.bin)

LDFILE := kernel.ld

INFILES := $(ASMFILES) $(CXXFILES)

# }}}
# Output files {{{

BINFILE := $(BINDIR)/$(NAME).elf
MAPFILE := $(BINDIR)/$(NAME).map

# }}}
# Toolkit {{{

#AS  := yasm
#CXX := x86_64-elf-g++
#LD  := x86_64-elf-ld

AS      := yasm
CXX     := clang++ --target=i686-elf
COPYBIN := objcopy -I binary -O elf32-i386 -B i386
LD      := ld.lld

#CXX := i686-elf-g++
#LD  := i686-elf-ld

# }}}
# Toolkit flags {{{

ASFLAGS  := -f elf32 -g dwarf2

CXXWARNINGS :=              \
	-Wall                   \
	-Wextra                 \
	-Wpedantic              \
	-Werror=return-type     \
	-Werror=unused-result   \
	-Werror=return-std-move \
	-Wshadow                \
	-Wpointer-arith         \
	-Wcast-align            \
	-Wwrite-strings         \
	-Wmissing-declarations  \
	-Wredundant-decls       \
	-Winline                \
	-Wuninitialized         \
	-Wno-unused-parameter

CXXFLAGS :=                            \
	-fdiagnostics-color=auto           \
	-Os                                \
	-g3                                \
	-ggdb                              \
	-nostdlib                          \
	-ffreestanding                     \
	-fno-stack-protector               \
	-Wfatal-errors                     \
	-fno-exceptions                    \
	-fno-rtti                          \
	-fno-threadsafe-statics            \
	-fwrapv                            \
	-mno-red-zone                      \
	-std=c++2a                         \
	-pipe                              \
	-flto                              \
	-I$(SRCDIR)                        \
	-I../common/include                \
	$(CXXWARNINGS)                     \
	-DKERNEL_TARGET=\"$(TARGET)\"      \
	-DKERNEL_TARGET_$(TARGET)          \
	-DKERNEL_VERSION=\"$(VERSION)\"    \
	-DKERNEL_NAME=\"$(NAME)\"          \
	-DKERNEL_VNAME=\"$(NAME_VERSION)\" \
	$(DIRTY_FLAG)                      \
	-mno-sse
#-mgeneral-regs-only        \
#-masm=intel                \

LDFILE := kernel.ld
LDLIBPATH := . /usr/lib/clang/7.0.1/lib/linux
LDLIBS    := clang_rt.builtins-i386
#LDLIBS += gcc_i686
LDFLAGS    =                      \
	-s                            \
	-lto-O3                       \
	$(addprefix -L, $(LDLIBPATH)) \
	$(addprefix -l, $(LDLIBS))    \
	-T$(LDFILE)                   \
	-Map=$(MAPFILE)

# }}}

.PHONY: all dep doc res clean bin

all: bin

#include target-$(TARGET).mk

-include Makefile.local

# Intermediate files {{{

OBJNAMES := $(ASMFILES:$(SRCDIR)/%.asm=%) \
			$(CXXFILES:$(SRCDIR)/%.cc=%)  \
			$(RESFILES:$(RESDIR)/%.bin=res-%)
OBJFILES := $(OBJNAMES:%=$(OBJDIR)/%.o)
DEPFILES := $(CXXFILES:$(SRCDIR)/%.cc=$(DEPDIR)/%.d)
DEPFILE  := deps.make
PCHFILE  := $(SRCDIR)/common.hh.pch

# }}}

OUTFILES = $(OBJFILES) $(PCHFILE) $(DEPFILES) $(DEPFILE) $(BINFILE) $(MAPFILE)

# Make targets {{{

dep: $(DEPFILES)

$(DEPFILE): $(DEPFILES)
	cat $^ > $@

# Include generated dependencies if we're compiling or linking.
ifeq (,$(filter $(MAKECMDGOALS),dep doc res clean))
ifeq (,$(findstring B,$(MAKEFLAGS)))
-include $(DEPFILE)
endif
endif

bin: res $(BINFILE)

res:
	$(MAKE) -C res

$(RESFILES): res

$(BINFILE): $(OBJFILES) $(LDFILE)
	$(E) "  LD   $@"
	$(Q)mkdir -p $(@D) && $(LD) -o $@ $(OBJFILES) $(LDFLAGS)

$(DEPDIR)/%.d: $(SRCDIR)/%.cc
	$(E)"  DEP  $@"
	$(Q)mkdir -p $(@D) && $(CXX) $(CXXFLAGS) -MM -MT $(<:$(SRCDIR)/%.cc=$(OBJDIR)/%.o) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	$(E) "  AS   $<"
	$(Q)mkdir -p $(@D) && $(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cc $(PCHFILE)
	$(E)"  CXX  $<"
	$(Q)mkdir -p $(@D) && $(CXX) -include-pch $(PCHFILE) -o $@ -c $< $(CXXFLAGS)

$(PCHFILE): src/common.hh $(PCHDEPS)
	$(E)"  PCH  $<"
	$(Q)$(CXX) -x c++-header -Xpreprocessor -emit-pch -o $@ $< $(CXXFLAGS)

$(OBJDIR)/res-%.o: $(RESDIR)/%.bin
	$(E) "  CPY  $<"
	$(Q)mkdir -p $(@D) && $(COPYBIN) $< $@

clean:
	$(E) "  CLEAN    $(OUTFILES)"
	$(Q)$(MAKE) -C res clean
	$(Q)rm -f $(OUTFILES)

# }}}
