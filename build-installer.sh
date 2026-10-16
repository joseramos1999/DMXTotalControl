#!/bin/bash
#
# DMX Total Control - Windows installer
#
# A clean build in dist/ (build-windows.sh) and its Inno Setup installer in
# build/installer/DMXTotalControl-Setup-<version>.exe. Run it like
# build-windows.sh (MSYS2 MINGW64), with Inno Setup 6 installed:
#   C:\msys64\usr\bin\bash.exe -lc "MSYSTEM=MINGW64 /d/path/to/build-installer.sh"
#
# Variables: BUILD_TYPE (Release), DEVEL (unset: as configured; OFF for an
# official release, without "GIT" in the version), ISCC (the Inno Setup compiler).
#
# Licensed under the Apache License, Version 2.0

set -e

SRCDIR="$(cd "$(dirname "$0")" && pwd)"
export BUILD_TYPE="${BUILD_TYPE:-Release}"

# the version of DMX Total Control (the first APPVERSION of variables.cmake)
VERSION=$(sed -n 's/^ *set(APPVERSION "\([0-9.]*\)")\r\?$/\1/p' "$SRCDIR/variables.cmake" | head -1)
if [ -z "$VERSION" ]; then
    echo "The version can't be read from variables.cmake" >&2
    exit 1
fi

# the Inno Setup compiler
if [ -z "$ISCC" ]; then
    # (installed for all users, or for the current one: %LOCALAPPDATA%\Programs)
    LOCALDIR="${LOCALAPPDATA:-${USERPROFILE:+$USERPROFILE/AppData/Local}}"
    LOCALDIR="${LOCALDIR:-/c/Users/${USERNAME:-$USER}/AppData/Local}"
    for candidate in "/c/Program Files (x86)/Inno Setup 6/ISCC.exe" "/c/Program Files/Inno Setup 6/ISCC.exe" \
                     "$(cygpath -u "$LOCALDIR")/Programs/Inno Setup 6/ISCC.exe"; do
        if [ -x "$candidate" ]; then
            ISCC="$candidate"
            break
        fi
    done
fi
if [ -z "$ISCC" ] || [ ! -x "$ISCC" ]; then
    echo "Inno Setup 6 (ISCC.exe) was not found: install it from https://jrsoftware.org/isinfo.php or set ISCC" >&2
    exit 1
fi

# a clean dist/ (DEVEL, when given, goes to build-windows.sh): nothing left from earlier builds goes into the installer
rm -rf "$SRCDIR/dist"
bash "$SRCDIR/build-windows.sh"

# (MSYS must not turn the /D options into paths)
MSYS2_ARG_CONV_EXCL='*' "$ISCC" /Q "/DAppVersion=$VERSION" "/DSourceDir=$(cygpath -w "$SRCDIR/dist")" \
    "$(cygpath -w "$SRCDIR/dtc/installer/DMXTotalControl.iss")"

INSTALLER="$SRCDIR/build/installer/DMXTotalControl-Setup-$VERSION.exe"
if [ ! -f "$INSTALLER" ]; then
    echo "The installer was not made" >&2
    exit 1
fi
echo "Installer: $(cygpath -w "$INSTALLER") ($(du -h "$INSTALLER" | cut -f1))"
