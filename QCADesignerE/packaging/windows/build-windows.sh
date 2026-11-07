#!/bin/sh
# Builds the Windows version of QCADesigner-E in the current directory:
#   QCADesignerE-<version>-windows-x86_64.zip    portable, extract and run bin\QCADesignerE.exe
#   QCADesignerE-<version>-windows-x86_64-setup.exe    installer
#
# Run it in an MSYS2 UCRT64 shell, with these packages installed:
#   mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-gtk2
#   mingw-w64-ucrt-x86_64-gettext-runtime mingw-w64-ucrt-x86_64-nsis make git zip
#
# To cross-compile from Linux instead, set HOST=x86_64-w64-mingw32, set CC and PKG_CONFIG to a
# compiler and pkg-config for UCRT, set MINGW_PREFIX to the unpacked MSYS2 ucrt64 directory, and set
# CPPFLAGS and LDFLAGS to its include and lib directories.
set -eu

HERE="$(cd "$(dirname "$0")" && pwd)"
SOURCE="$(cd "$HERE/../.." && pwd)"
OUTPUT="$PWD"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

: "${MINGW_PREFIX:?set MINGW_PREFIX to the MSYS2 ucrt64 directory}"
HOST="${HOST:-}"
OBJDUMP="${OBJDUMP:-${HOST:+$HOST-}objdump}"
WINDRES="${WINDRES:-${HOST:+$HOST-}windres}"
VERSION="$(sed -n 's/^AM_INIT_AUTOMAKE(QCADesigner, *\([^)]*\)).*/\1/p' "$SOURCE/configure.in")"
NAME="QCADesignerE-$VERSION"
APP="$WORK/$NAME"

# Build from a copy, because the source tree may be configured for another system.
# Uses the tracked files as they are now, including uncommitted changes.
mkdir "$WORK/source"
cd "$SOURCE"
git -c safe.directory='*' ls-files -z \
  | xargs -0 sh -c 'for file; do [ -e "$file" ] && printf "%s\0" "$file"; done; true' _ \
  | tar --null -T - -cf - | tar -xf - -C "$WORK/source"
cd "$WORK/source"

# The program finds its data files relative to itself, so the prefix only sets the layout.
./configure --prefix=/ ${HOST:+--host=$HOST} --without-svgpix >/dev/null
# The program's icon and version details, which Windows reads from the .exe itself
VERSION_COMMA="$(echo "$VERSION.0.0.0" | cut -d. -f1-4 | tr . ,)"
"$WINDRES" -I "$HERE" -DQCAD_VERSION_COMMA="$VERSION_COMMA" -DQCAD_VERSION_STR="\\\"$VERSION\\\"" \
  -i "$HERE/QCADesignerE.rc" -O coff -o "$WORK/resources.o"
# -mwindows makes it a GUI program, so Windows does not open a console window for it
make -C src QCADesigner.exe LDFLAGS="${LDFLAGS:-} -mwindows $WORK/resources.o" >/dev/null
make install DESTDIR="$APP" >/dev/null

# Copy every DLL the program needs from MSYS2, following their own imports. DLLs that are
# not in MSYS2, such as kernel32.dll, come with Windows.
copy_dlls () {
  for dll in $("$OBJDUMP" -p "$1" | awk '/DLL Name:/ {print $3}'); do
    if [ ! -f "$APP/bin/$dll" ] && [ -f "$MINGW_PREFIX/bin/$dll" ]; then
      cp "$MINGW_PREFIX/bin/$dll" "$APP/bin/"
      copy_dlls "$APP/bin/$dll"
    fi
  done
}

# Use GTK's theme engine for the native Windows look
mkdir -p "$APP/lib/gtk-2.0/2.10.0/engines" "$APP/etc/gtk-2.0" "$APP/share/themes"
cp "$MINGW_PREFIX/lib/gtk-2.0/2.10.0/engines/libwimp.dll" "$APP/lib/gtk-2.0/2.10.0/engines/"
cp -r "$MINGW_PREFIX/share/themes/MS-Windows" "$APP/share/themes/"
printf 'gtk-theme-name = "MS-Windows"\n' > "$APP/etc/gtk-2.0/gtkrc"

copy_dlls "$APP/bin/QCADesignerE.exe"
copy_dlls "$APP/lib/gtk-2.0/2.10.0/engines/libwimp.dll"

cd "$WORK"
rm -f "$OUTPUT/$NAME-windows-x86_64.zip"
zip -qr "$OUTPUT/$NAME-windows-x86_64.zip" "$NAME"

makensis -V2 -DVERSION="$VERSION" -DAPPDIR="$APP" \
  -DOUTFILE="$OUTPUT/$NAME-windows-x86_64-setup.exe" "$HERE/QCADesignerE.nsi"
