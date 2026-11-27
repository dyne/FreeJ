#!/usr/bin/env bash
set -euo pipefail

if [[ $# != 2 || ! $1 =~ ^(compatibility|static)$ ]]; then
    echo 'Usage: movie-linux-sdk.sh compatibility|static WORK' >&2
    exit 2
fi
work=$(realpath -m -- "$2")
recipe=$(cd -- "$(dirname -- "$0")" && pwd -P)
mkdir -- "$work"
cd -- "$work"
read -r checksum url < <(tail -n 1 "$recipe/movie-linux-sdk.lock")
archive=${FREEJ_MOVIE_SDK_TARBALL:-"$work/ffmpeg.tar.xz"}
if [[ ! -f $archive ]]; then
    curl --fail --silent --show-error --location --retry 3 --connect-timeout 10 --max-time 120 \
        --output "$archive" "$url"
fi
printf '%s  %s\n' "$checksum" "$archive" | sha256sum --check
mkdir source build
tar -xJf "$archive" --strip-components=1 -C source

linkage=(--enable-shared --disable-static)
if [[ $1 == static ]]; then
    linkage=(--enable-static --disable-shared)
fi
cd build
../source/configure --prefix="$work/sdk" --enable-pic "${linkage[@]}" \
    --disable-autodetect --disable-programs --disable-doc --disable-debug --disable-x86asm \
    --disable-encoders --disable-muxers --disable-avdevice --disable-avfilter --disable-postproc
make -j"${FREEJ_CI_JOBS:-4}"
make install
