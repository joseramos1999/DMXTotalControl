#!/bin/bash
#
# DMX Total Control - Windows developer build
#
# Run from an MSYS2 MINGW64 shell, or from anywhere with:
#   C:\msys64\usr\bin\bash.exe -lc "MSYSTEM=MINGW64 /d/path/to/build-windows.sh"
#
# Required MSYS2 packages (pacman -S --needed):
#   mingw-w64-x86_64-{gcc,cmake,ninja,pkgconf,libusb,libftdi,libmad,libsndfile,fftw}
#   mingw-w64-x86_64-qt6-{base,declarative,3d,multimedia,serialport,websockets,imageformats,svg,tools,shadertools}
#
# Output: build/ (objects) and dist/ (runnable installation)

set -e

export PATH="/mingw64/bin:$PATH"
SRCDIR="$(cd "$(dirname "$0")" && pwd)"
BUILDDIR="$SRCDIR/build"
DISTDIR="$SRCDIR/dist"
BUILD_TYPE="${BUILD_TYPE:-Debug}"

# (DEVEL=OFF: an official release, without "GIT" in the version)
cmake -S "$SRCDIR" -B "$BUILDDIR" -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ${DEVEL:+-DDEVEL="$DEVEL"} \
    -DCMAKE_PREFIX_PATH=/mingw64 \
    -DWIN_INSTALLROOT="$(cygpath -m "$DISTDIR")"

cmake --build "$BUILDDIR" -j "$(nproc)"
cmake --install "$BUILDDIR"

# Make dist/ self contained: Qt and MinGW runtime DLLs + QML modules
if [ "$1" != "--no-deploy" ]; then
    windeployqt6 --qmldir "$SRCDIR/dtc/qml" --no-translations --no-compiler-runtime \
        "$DISTDIR/dmxtotalcontrol.exe" > /dev/null
    # windeployqt does not follow MinGW library dependencies, ldd does
    find "$DISTDIR" -iname '*.exe' -o -iname '*.dll' | while read -r f; do
        ldd "$f" 2>/dev/null | awk '/\/mingw64\/bin\// { print $3 }'
    done | sort -u | while read -r dll; do
        cp -u "$dll" "$DISTDIR/"
    done
fi

echo "Done: $(cygpath -w "$DISTDIR")\\dmxtotalcontrol.exe"
