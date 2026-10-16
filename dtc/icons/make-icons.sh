#!/bin/bash
#
# DMX Total Control - makes the icon files from the two SVGs
#
#   dtc-icon.svg        the icon, 24 px and up
#   dtc-icon-small.svg  16 and 20 px: the D alone (the three letters can't be read)
#
# Needs rsvg-convert (MSYS2: mingw-w64-x86_64-librsvg) and perl. The files it
# makes are in the repository: the build doesn't need these tools.
#
# Licensed under the Apache License, Version 2.0

set -e
cd "$(dirname "$0")"

for size in 16 20; do
    rsvg-convert -w $size -h $size dtc-icon-small.svg -o dtc-$size.png
done
for size in 24 32 40 48 64 96 128 180 192 256 512; do
    rsvg-convert -w $size -h $size dtc-icon.svg -o dtc-$size.png
done

# Windows: the executable, the installer
perl make-ico.pl dtc.ico dtc-16.png dtc-20.png dtc-24.png dtc-32.png dtc-40.png \
    dtc-48.png dtc-64.png dtc-96.png dtc-128.png dtc-256.png

echo "Icons made in $(pwd)"
