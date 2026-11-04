#!/bin/sh
# Builds QCADesignerE-x86_64.AppImage in the current directory.
#
# Run this on the oldest system the AppImage should support, because it only runs on
# systems with the same or a newer glibc. Use Ubuntu 22.04: newer versions of gdk-pixbuf
# load images through separate glycin programs, which cannot be bundled this way.
#
# Requires: build-essential pkg-config libgtk2.0-dev gettext shared-mime-info git file curl
# Set APPIMAGETOOL to use a copy of appimagetool instead of downloading one.
# Set APPDIR_ONLY=1 to stop at QCADesignerE.AppDir, which does not need appimagetool.
set -eu

HERE="$(cd "$(dirname "$0")" && pwd)"
SOURCE="$(cd "$HERE/../.." && pwd)"
OUTPUT="$PWD"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
APPDIR="$WORK/AppDir"

# Build from a copy, because the source tree may be configured for another prefix.
# Uses the tracked files as they are now, including uncommitted changes.
mkdir "$WORK/source"
cd "$SOURCE"
git -c safe.directory='*' ls-files -z \
  | xargs -0 sh -c 'for file; do [ -e "$file" ] && printf "%s\0" "$file"; done; true' _ \
  | tar --null -T - -cf - | tar -xf - -C "$WORK/source"
cd "$WORK/source"

# QCADesignerE finds its data files relative to the program, so this prefix only sets
# the layout inside the AppImage.
./configure --prefix=/usr --without-svgpix >/dev/null
make -j"$(nproc)" >/dev/null
make install DESTDIR="$APPDIR" >/dev/null

# Libraries to leave to the host. glibc must match the host, and the others are on every
# desktop and need to match its graphics drivers and font configuration.
is_host_library () {
  case "$1" in
    ld-linux*|libc.so*|libm.so*|libdl.so*|librt.so*|libpthread.so*|libresolv.so*|libnsl.so*|libutil.so*|\
    libgcc_s.so*|libstdc++.so*|libX11.so*|libxcb.so*|libGL*.so*|libEGL.so*|libdrm.so*|\
    libfontconfig.so*|libfreetype.so*|libharfbuzz.so*|libfribidi.so*|libexpat.so*|libz.so*) return 0 ;;
  esac
  return 1
}

mkdir -p "$APPDIR/usr/lib"
ldd "$APPDIR/usr/bin/QCADesignerE" | awk '$2 == "=>" && $3 ~ /^\// { print $3 }' | sort -u | while read -r library; do
  name="$(basename "$library")"
  if is_host_library "$name"; then
    continue
  fi
  cp -L "$library" "$APPDIR/usr/lib/$name"
done

mkdir -p "$APPDIR/usr/share/qcadesignere"
: > "$APPDIR/usr/share/qcadesignere/gtkrc"
: > "$APPDIR/usr/share/qcadesignere/empty"

# GTK needs a MIME database to recognize its PNG icons, and a minimal system may not have one.
mkdir -p "$APPDIR/usr/share/mime"
find /usr/share/mime -maxdepth 1 -type f -exec cp {} "$APPDIR/usr/share/mime/" \;

cp "$HERE/AppRun" "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"
cp "$HERE/QCADesignerE.desktop" "$APPDIR/QCADesignerE.desktop"
cp "$WORK/source/pixmaps/QCADesignerE_0_48x48x24a.png" "$APPDIR/QCADesignerE.png"

# Stop here if only the unpacked application is wanted, for example to test it.
if [ -n "${APPDIR_ONLY:-}" ]; then
  cp -a "$APPDIR" "$OUTPUT/QCADesignerE.AppDir"
  exit 0
fi

if [ -z "${APPIMAGETOOL:-}" ]; then
  APPIMAGETOOL="$WORK/appimagetool"
  curl -fsSL -o "$APPIMAGETOOL" \
    https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
  chmod +x "$APPIMAGETOOL"
fi

ARCH=x86_64 "$APPIMAGETOOL" --appimage-extract-and-run "$APPDIR" "$OUTPUT/QCADesignerE-x86_64.AppImage"
