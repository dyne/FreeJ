#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || ! $1 =~ ^(compatibility|static)$ || ($1 == static && $# != 4) || ($1 == compatibility && $# != 2) ]]; then
    echo 'Usage: sdl-linux-sdk.sh compatibility WORK | static WORK ARCHIVE_USR EXTENSIONS_USR' >&2
    exit 2
fi
profile=$1
work=$(realpath -m -- "$2")
recipe=$(cd -- "$(dirname -- "$0")" && pwd -P)
prefix=$work/sdk
jobs=${FREEJ_CI_JOBS:-4}
mkdir -- "$work"
mkdir -- "$work/sources"

sources=()
while read -r digest url; do
    [[ -n $digest && $digest != '#'* ]] || continue
    name=${url##*/}
    archive=$work/$name
    if [[ -n ${FREEJ_SDL_SDK_CACHE:-} && -f $FREEJ_SDL_SDK_CACHE/$name ]]; then
        archive=$FREEJ_SDL_SDK_CACHE/$name
    else
        curl --fail --silent --show-error --location --retry 3 --connect-timeout 10 --max-time 120 \
            --output "$archive" "$url"
    fi
    printf '%s  %s\n' "$digest" "$archive" | sha256sum -c -
    tar -xzf "$archive" -C "$work/sources"
    sources+=("$work/sources/${name%.tar.gz}")
done < "$recipe/sdl-linux-sdk.lock"

preset=()
shared=ON
static=OFF
x11_shared=ON
if [[ $profile == static ]]; then
    shared=OFF
    static=ON
    x11_shared=OFF
    preset+=("-DCMAKE_PREFIX_PATH=$prefix;$3;$4"
             "-DCMAKE_FIND_LIBRARY_SUFFIXES=.a"
             "-DFREETYPE_LIBRARY=$3/lib/x86_64-linux-gnu/libfreetype.a"
             "-DZLIB_USE_STATIC_LIBS=ON" "-DJPEG_LIBRARY_RELEASE=$3/lib/x86_64-linux-gnu/libjpeg.a"
             "-DPNG_LIBRARY_RELEASE=$3/lib/x86_64-linux-gnu/libpng16.a"
             "-DTIFF_LIBRARY_RELEASE=$3/lib/x86_64-linux-gnu/libtiff.a")
    # An archived build should never consult SDL_DYNAMIC_API for a replacement library.
    sed -i 's/^#define SDL_DYNAMIC_API 1$/#define SDL_DYNAMIC_API 0/' \
        "${sources[0]}/src/dynapi/SDL_dynapi.h"
else
    preset+=("-DCMAKE_PREFIX_PATH=$prefix")
fi

flags=(-DCMAKE_BUILD_TYPE=Release "-DCMAKE_INSTALL_PREFIX=$prefix" -DCMAKE_INSTALL_LIBDIR=lib
       "-DBUILD_SHARED_LIBS=$shared" -DCMAKE_POSITION_INDEPENDENT_CODE=ON "${preset[@]}")
if command -v ccache >/dev/null; then
    flags+=(-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
fi

cmake -S "${sources[0]}" -B "$work/core" "${flags[@]}" "-DSDL_STATIC=$static" "-DSDL_SHARED=$shared" \
    -DSDL_TESTS=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_X11=ON "-DSDL_X11_SHARED=$x11_shared" \
    -DSDL_WAYLAND=OFF -DSDL_DUMMYVIDEO=ON -DSDL_AUDIO=OFF -DSDL_KMSDRM=OFF -DSDL_VULKAN=OFF \
    "-DSDL_OPENGL=$shared" "-DSDL_OPENGLES=$shared" -DSDL_DBUS=OFF -DSDL_IBUS=OFF -DSDL_LIBUDEV=OFF \
    -DSDL_HIDAPI=OFF -DSDL_X11_XTEST=OFF -DSDL_FRIBIDI=OFF -DSDL_LIBTHAI=OFF
cmake --build "$work/core" --parallel "$jobs"
cmake --install "$work/core"

cmake -S "${sources[1]}" -B "$work/image" "${flags[@]}" -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=OFF \
    -DSDLIMAGE_VENDORED=OFF -DSDLIMAGE_STRICT=ON \
    -DSDLIMAGE_DEPS_SHARED=OFF -DSDLIMAGE_AVIF=OFF -DSDLIMAGE_JXL=OFF \
    -DSDLIMAGE_PNG=ON -DSDLIMAGE_PNG_LIBPNG=ON -DSDLIMAGE_JPG=ON -DSDLIMAGE_TIF=ON \
    -DSDLIMAGE_WEBP=ON -DSDLIMAGE_TESTS=OFF -DSDLIMAGE_SAMPLES=OFF
cmake --build "$work/image" --parallel "$jobs"
cmake --install "$work/image"

cmake -S "${sources[2]}" -B "$work/ttf" "${flags[@]}" -DSDLTTF_VENDORED=OFF \
    -DSDLTTF_HARFBUZZ=OFF -DSDLTTF_SAMPLES=OFF
cmake --build "$work/ttf" --parallel "$jobs"
cmake --install "$work/ttf"
