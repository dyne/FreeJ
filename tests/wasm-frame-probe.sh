#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 || $# -gt 3 ]]; then
    echo "Usage: $0 PINNED_FREI0R_ARCHIVE NEW_WORK_DIRECTORY [STATIC_FRAME_STEP_TEST]" >&2
    exit 2
fi

repo=$(cd -- "$(dirname -- "$0")/.." && pwd -P)
archive=$(realpath -- "$1")
work=$(realpath -m -- "$2")
context_reference=
if [[ $# == 3 ]]; then
    context_reference=$(realpath -- "$3")
    test -x "$context_reference"
fi
if [[ $(uname -sm) != 'Linux x86_64' ]]; then
    echo 'This probe uses the Linux x86-64 emsdk package and native BGRA replay.' >&2
    exit 2
fi
mkdir -- "$work"
cd -- "$work"

read -r effects_checksum _ < <(tail -n 1 "$repo/tests/native-ci.lock")
printf '%s  %s\n' "$effects_checksum" "$archive" | sha256sum -c -
mkdir -- effects-source
tar -xzf "$archive" --strip-components=1 -C effects-source

# Verify the SDK tool source, release mapping and downloaded binaries before use.
read -r sdk_version sdk_commit sdk_release node_version < "$repo/tests/wasm-emsdk.lock"
git clone --depth 1 --branch "$sdk_version" https://github.com/emscripten-core/emsdk.git emsdk
test "$(git -C emsdk rev-parse HEAD)" = "$sdk_commit"
grep -Fq "\"$sdk_version\": \"$sdk_release\"" emsdk/emscripten-releases-tags.json
mkdir -- emsdk/downloads
while read -r checksum url filename; do
    path="$work/emsdk/downloads/$filename"
    if [[ -n ${FREEJ_WASM_SDK_ARCHIVES:-} ]]; then
        cp -- "$FREEJ_WASM_SDK_ARCHIVES/$filename" "$path"
    else
        curl --fail --silent --show-error --location --max-time 240 --retry 2 "$url" -o "$path"
    fi
    printf '%s  %s\n' "$checksum" "$path" | sha256sum -c -
done < <(tail -n +2 "$repo/tests/wasm-emsdk.lock")
EMSDK_KEEP_DOWNLOADS=1 timeout 360 ./emsdk/emsdk install "$sdk_version"
timeout 60 ./emsdk/emsdk activate "$sdk_version"
source "$work/emsdk/emsdk_env.sh"
export EM_CACHE="$work/em-cache"
node_bin=$EMSDK_NODE
em++ --version | tee emscripten-version.txt
grep -Fq "$sdk_version" emscripten-version.txt
test "$("$node_bin" --version)" = "v$node_version"
"$node_bin" --version > node-version.txt

cache=()
cmake_cache=()
if command -v ccache > /dev/null; then
    cache=("$(command -v ccache)")
    cmake_cache=(-DCMAKE_C_COMPILER_LAUNCHER="${cache[0]}" -DCMAKE_CXX_COMPILER_LAUNCHER="${cache[0]}")
    export EM_COMPILER_WRAPPER="${cache[0]}"
fi
bundle_options=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DFREI0R_BUILD_BUNDLE=ON '-DFREI0R_BUNDLE_PLUGINS=brightness;invert0r;onecol0r'
    -DWITHOUT_GAVL=ON -DWITHOUT_OPENCV=ON -DWITHOUT_CAIRO=ON -DBUILD_TESTING=OFF)
cmake -S effects-source -B native-effects "${bundle_options[@]}" "${cmake_cache[@]}" \
    -DCMAKE_C_FLAGS=-fno-tree-vectorize -DCMAKE_CXX_FLAGS=-fno-tree-vectorize
cmake --build native-effects --parallel 4 --target frei0r-bundle
emcmake cmake -S effects-source -B wasm-effects "${bundle_options[@]}" \
    -DFREI0R_BUNDLE_WASM_BASELINE=ON -DFREI0R_BUNDLE_WASM_SIMD=OFF -DFREI0R_BUNDLE_WASM_PTHREADS=OFF
cmake --build wasm-effects --parallel 4 --target frei0r-bundle

includes=(-I"$repo/src/engine" -I"$work/effects-source/include")
for source in "$repo/src/engine/frame_tick.cpp" "$repo/tests/show-replay/wasm-frame-probe.cpp"; do
    name=$(basename -- "$source" .cpp)
    "${cache[@]}" c++ -std=c++11 -O2 -Wall -Wextra -Werror -fno-tree-vectorize "${includes[@]}" \
        -c "$source" -o "$name.native.o"
    em++ -std=c++11 -O2 -Wall -Wextra -Werror -mno-simd128 -mno-atomics \
        -fno-vectorize -fno-slp-vectorize "${includes[@]}" -c "$source" -o "$name.wasm.o"
done
bundle=libfrei0r-bundle-static.a
c++ -Wl,-Map,native.map frame_tick.native.o wasm-frame-probe.native.o "native-effects/$bundle" -o native-frame-probe
# Debug names keep imports readable and available to the audit.
em++ -O2 -g3 -sENVIRONMENT=node -sFILESYSTEM=0 -Wl,-Map,wasm.map \
    frame_tick.wasm.o wasm-frame-probe.wasm.o "wasm-effects/$bundle" -o wasm-frame-probe.js

llvm_bin=$work/emsdk/upstream/bin
"$llvm_bin/llvm-readobj" --file-headers wasm-frame-probe.wasm > wasm-headers.txt
"$llvm_bin/llvm-readobj" --file-headers "wasm-effects/$bundle" > wasm-bundle-headers.txt
"$llvm_bin/llvm-readobj" --file-headers "native-effects/$bundle" > native-bundle-headers.txt
"$llvm_bin/llvm-ar" t "wasm-effects/$bundle" > wasm-bundle-members.txt
"$llvm_bin/llvm-ar" t "native-effects/$bundle" > native-bundle-members.txt
"$llvm_bin/llvm-nm" -u frame_tick.wasm.o > wasm-tick-undefined.txt
# The linker fills the table used by virtual calls in the common tick.
test "$(awk '{print $NF}' wasm-tick-undefined.txt)" = __indirect_function_table
grep -Fxq 'Arch: wasm32' wasm-headers.txt
cmp native-bundle-members.txt wasm-bundle-members.txt
test "$(grep -c '^Arch: wasm32$' wasm-bundle-headers.txt)" = "$(wc -l < wasm-bundle-members.txt)"
test "$(grep -c '^Arch: x86_64$' native-bundle-headers.txt)" = "$(wc -l < native-bundle-members.txt)"
for map in native.map wasm.map; do
    grep -Fq 'freej_engine::run_frame_tick' "$map"
    grep -Fq "$bundle(frei0r-bundle-registry.c.o)" "$map"
    grep -Fq "$bundle(invert0r.c.o)" "$map"
done

# Validate with a bounded feature set. SIMD, atomics and shared memory are disabled.
"$llvm_bin/wasm-opt" wasm-frame-probe.wasm --mvp-features --enable-mutable-globals \
    --enable-bulk-memory --enable-bulk-memory-opt --enable-sign-ext --enable-nontrapping-float-to-int \
    --print-features > wasm-validation-features.txt

timeout 30 ./native-frame-probe > native.trace
timeout 30 "$node_bin" wasm-frame-probe.js > wasm.trace
reference_trace=()
if [[ -n $context_reference ]]; then
    timeout 30 "$context_reference" --bundle-reference > context.log 2>&1
    grep '^frame-probe ' context.log > context.trace
    reference_trace=(context.trace)
fi
"$node_bin" "$repo/tests/wasm-frame-audit.mjs" wasm-frame-probe.wasm wasm.trace native.trace "${reference_trace[@]}" \
    | tee results.txt
sha256sum wasm-frame-probe.wasm "native-effects/$bundle" "wasm-effects/$bundle" | tee artifacts.sha256
