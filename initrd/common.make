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

ifndef NAME
$(error Name unset)
endif

TARGET ?= i686
export TARGET

LDFILE  := ../common.ld
MAPFILE := $(NAME).map

START     := ../common/start.asm
START_OBJ := ../common/start.o

OBJ_CXX := $(SRC_CXX:%.cc=%.o)

ELF := program.elf
BIN := program.bin

CXX := clang++ --target=$(TARGET)-elf
AS  := yasm
LD  := ld.lld

CXXWARNINGS :=             \
	-Wall                  \
	-Wextra                \
	-Wpedantic             \
	-Werror=return-type    \
	-Werror=unused-result  \
	-Wshadow               \
	-Wpointer-arith        \
	-Wcast-align           \
	-Wwrite-strings        \
	-Wmissing-declarations \
	-Wredundant-decls      \
	-Winline               \
	-Wuninitialized        \
	-Wno-unused-parameter

# CXXFLAGS :=                            \
# 	-fdiagnostics-color=auto           \
# 	-Os                                \
# 	-g3                                \
# 	-ggdb                              \
# 	-nostdlib                          \
# 	-ffreestanding                     \
# 	-fno-stack-protector               \
# 	-Wfatal-errors                     \
# 	-fno-exceptions                    \
# 	-fno-rtti                          \
# 	-fno-threadsafe-statics            \
# 	-fwrapv                            \
# 	-mno-red-zone                      \
# 	-mno-sse                           \
# 	-std=c++2a                         \
# 	-pipe                              \
# 	-Wall                              \
# 	-Wextra                            \
# 	-Wpedantic

CXXFLAGS :=                            \
	-fdiagnostics-color=auto           \
	-Os                                \
	-g3                                \
	-ggdb                              \
	-fhosted                           \
	-fno-stack-protector               \
	-Wfatal-errors                     \
	-fno-exceptions                    \
	-fno-rtti                          \
	-fno-threadsafe-statics            \
	-fwrapv                            \
	-mno-red-zone                      \
	-mno-sse                           \
	-std=c++2a                         \
	-pipe                              \
	-isystem ../common                 \
	$(CXXWARNINGS)


LDFLAGS := \
	-s                            \
	-T$(LDFILE)                   \
	-Map=$(MAPFILE)

COPYBIN := objcopy -I binary -O elf32-i386 -B i386

.PHONY: all clean

all: $(BIN)

-include Makefile.local

# Make targets {{{

$(ELF): $(START_OBJ) $(OBJ_CXX)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) -o $@ -c $< $(CXXFLAGS)

$(START_OBJ): $(START)
	$(AS) -f elf32 -g dwarf2 -o $@ $<

$(BIN): $(ELF)
	objcopy -Obinary $< $@

clean:
	rm -f $(BIN) $(ELF) $(OBJ_CXX) $(MAPFILE)

# }}}
