#!/usr/bin/env bash
set -euo pipefail

# Prepare SDKs from the same locks used by the local Linux profile recipes.
if [[ $# != 2 || ! $1 =~ ^(compatibility|static)$ ]]; then
    echo 'Usage: native-ci-prepare.sh compatibility|static WORK' >&2
    exit 2
fi
profile=$1
work=$(realpath -m -- "$2")
repo=$(cd -- "$(dirname -- "$0")/.." && pwd -P)
cd -- "$repo"
mkdir -- "$work"
mkdir -- "$work/logs"
jobs=${FREEJ_CI_JOBS:-4}
export LC_ALL=C

python3 tests/legacy-linux-sdk.py "$work/legacy" --skip-historical-media --jobs "$jobs" 2>&1 | tee "$work/logs/legacy-sdk.log"
tests/movie-linux-sdk.sh "$profile" "$work/movie" 2>&1 | tee "$work/logs/movie-sdk.log"
read -r _ url < <(tail -n 1 tests/native-ci.lock)
curl --fail --silent --show-error --location --retry 3 --connect-timeout 10 --max-time 120 \
    --output "$work/frei0r.tar.gz" "$url"
tests/frei0r-linux-sdk.sh "$work/frei0r.tar.gz" "$work/effects" 2>&1 | tee "$work/logs/frei0r-sdk.log"
if [[ $profile == static ]]; then
    python3 tests/native-linux-archives.py "$work/archives" --reference-sdk "$work/legacy" \
        --skip-sdl12 --jobs "$jobs" 2>&1 | tee "$work/logs/archive-sdk.log"
    python3 tests/native-static-sdk.py "$work/native" --archive-sdk "$work/archives" \
        --reference-sdk "$work/legacy" --jobs "$jobs" 2>&1 | tee "$work/logs/static-sdk.log"
fi

gcc="$work/legacy/gcc12/usr/bin/gcc-12"
gxx="$work/legacy/gcc12/usr/bin/g++-12"
sdl_args=()
if [[ $profile == static ]]; then
    archive_usr=$work/archives/debian/usr
    extensions_usr=$work/native/packages/usr
    sdl_args+=("$archive_usr" "$extensions_usr")
    export PKG_CONFIG_PATH="$archive_usr/lib/x86_64-linux-gnu/pkgconfig:$extensions_usr/lib/x86_64-linux-gnu/pkgconfig"
fi
CC="$gcc" CXX="$gxx" tests/sdl-linux-sdk.sh "$profile" "$work/sdl" "${sdl_args[@]}" \
    2>&1 | tee "$work/logs/sdl-sdk.log"
CC="$gcc" CXX="$gxx" tests/pipewire-linux-sdk.sh "$work/pipewire" \
    2>&1 | tee "$work/logs/pipewire-sdk.log"
