#!/usr/bin/env bash
# Prepare the exact native files consumed by FreeJ from the checksum-locked source.
set -euo pipefail
if [[ $# != 2 ]]; then
    echo "usage: $0 PINNED_SOURCE_ARCHIVE NEW_PREFIX" >&2
    exit 2
fi
repo=$(cd "$(dirname "$0")/.." && pwd)
archive=$(realpath "$1")
prefix=$(realpath -m "$2")
read -r checksum url < <(tail -n 1 "$repo/tests/native-ci.lock")
printf '%s  %s\n' "$checksum" "$archive" | sha256sum -c -
mkdir "$prefix"
mkdir "$prefix/source" "$prefix/lib" "$prefix/modules" -p "$prefix/include/frei0r"
tar xzf "$archive" --strip-components=1 -C "$prefix/source"
cache=()
if command -v ccache >/dev/null; then
    cache=(-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
fi
effects=(brightness invert0r onecol0r)
selection=$(IFS=';'; echo "${effects[*]}")
cmake -S "$prefix/source" -B "$prefix/build" -DCMAKE_BUILD_TYPE=Release \
    -DFREI0R_BUILD_BUNDLE=ON "-DFREI0R_BUNDLE_PLUGINS=$selection" \
    -DWITHOUT_GAVL=ON -DWITHOUT_OPENCV=ON -DBUILD_TESTING=OFF "${cache[@]}"
cmake --build "$prefix/build" --parallel "${FREEJ_CI_JOBS:-4}" --target frei0r-bundle "${effects[@]}"
install -m644 "$prefix/build/libfrei0r-bundle-static.a" "$prefix/lib/"
install -m644 "$prefix/source/include/frei0r/bundle.h" "$prefix/include/frei0r/"
install -m644 "$prefix/source/include/frei0r.h" "$prefix/include/"
for id in "${effects[@]}"; do
    case "$id" in onecol0r) kind=generator ;; *) kind=filter ;; esac
    install -m755 "$prefix/build/src/$kind/$id/$id.so" "$prefix/modules/"
done
