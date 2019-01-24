#!/usr/bin/env perl

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

use 5.12.0;
use warnings;
use autodie;

# Boring utilities {{{

sub basename { shift =~ s@.*/@@r }
sub pad_size { my ($i, $align) = @_; $i % $align && $align - $i % $align }

sub slurp {
    # preferably we would read/write files in chunks.
    my $fn = shift;
    open my $fh, '<', $fn or die "$0: could not open file '$fn' for reading: $!\n";
    binmode $fh;
    local $/ = undef;
    my $content = <$fh>;
    return $content;
}

# }}}

# File offset alignment, should not be higher than 32.
my $align = 8;

# the initrd purpose:
#
#   To make accessible to the kernel, drivers that are required in early boot
#   stages, and drivers (e.g. disk and fs drivers) that enable the loading of
#   other drivers.
#
# the initrd format:
#
# - all numbers are unsigned 32-bit little-endian
# - all strings are UTF-8, null-terminated, and null-padded to fixed sizes
# - file data are guaranteed to be aligned to 8-byte boundaries
# - file order is unspecified
#
#        HEADER
# ┌───────────────────┐ 0
# │ magic "INITRD..." │               ─ the first 6 bytes must match "INITRD"
# ├───────────────────┤ 24
# │ table_entry_count │
# ├───────────────────┤ 28
# │ table_entry_size  │               ─ must be 32 (amount of bytes per entry)
# ├───────────────────┤ 32
#
#      FILE TABLE
# ├───────────────────┤ n*32+0      ─╮
# │ entry_name        │              │
# ├───────────────────┤ n*32+24      │
# │ entry_offset      │              ├─ for n = 1..table_entry_count
# ├───────────────────┤ n*32+28      │
# │ entry_size        │              │
# ├───────────────────┤ n*32+32     ─╯
#          ⋮
#
#       FILE DATA
# ├───────────────────┤ x+off       ─╮
# │ file_data         │              ├─ for each table entry,
# ├───────────────────┤ x+off+size  ─╯  where x = 32 + table_entry_size * table_entry_count
#          ⋮
# └───────────────────┘

# * Gather input data

my $i = 0;
my @files = map { my ($name, $path) = /([^=]+)=(.+)/
                      or die "usage: $0 [NAME=PATH]...\n";
                  my $s = -s $path
                      or die "could not get size of file \"$path\": $!";
                  my $j = $i;
                  my $pad = pad_size($j, $align);
                  $i += $pad + $s;

                  die "file name <$name> exceeds max name length" if length $name > 23;

                  { path   => $path,
                    name   => $name,
                    size   => $s,
                    offset => $pad + $j, }
            } @ARGV;

my $header_size = 32; # 24 magic, 4 entry size, 4 entry count.
my $entry_size  = 32; # 24 file name, 4 offset, 4 size.
my $table_size  = @files * $entry_size;

binmode(STDOUT);

# * Print header

print pack "a24VV",
           "INITRD kitaa!",
           scalar(@files),
           $entry_size;

say STDERR sprintf("initrd entries (data starts at %08x):", $header_size + $table_size);
say STDERR sprintf("  %-24s %-8s %-8s", qw(NAME OFFSET SIZE));

# * Print file table

for (@files) {
    print pack "a24VV",                     @{$_}{qw(name offset size)};
    say STDERR sprintf("  %-24s %08x %08x", @{$_}{qw(name offset size)});
}

# * Print file data

$i = 0;
for (@files) {
    my $pad = pad_size($i, $align);
    print "\x00" x $pad;
    $i += $pad + $_->{size};

    print slurp $_->{path};
}
