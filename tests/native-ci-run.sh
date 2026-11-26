#!/usr/bin/env bash
set -euo pipefail

if [[ $# != 3 || ! $1 =~ ^(compatibility|static)$ ]]; then
    echo 'Usage: native-ci-run.sh compatibility|static PREPARED_WORK NEW_EVIDENCE_DIR' >&2
    exit 2
fi
profile=$1
work=$(realpath -- "$2")
evidence=$(realpath -m -- "$3")
repo=$(cd -- "$(dirname -- "$0")/.." && pwd -P)
cd -- "$repo"
mkdir -- "$evidence"

legacy=$work/legacy
multiarch=x86_64-linux-gnu
debian=$legacy/debian/usr
debian_lib=$debian/lib/$multiarch
native=$work/native
archives=$work/archives
effects=$work/effects
build_work=$work/trial
ccache_bin=$(command -v ccache)
gcc=$legacy/gcc12/usr/bin/gcc-12
gxx=$legacy/gcc12/usr/bin/g++-12
jobs=${FREEJ_CI_JOBS:-4}
export LC_ALL=C
export CC="$ccache_bin $gcc" CXX="$ccache_bin $gxx" CFLAGS='-O2 -DNDEBUG' CXXFLAGS='-O2 -DNDEBUG'
export CPPFLAGS="-I$debian/include" LDFLAGS="-L$debian_lib"
export PKG_CONFIG_PATH="$work/movie/sdk/lib/pkgconfig:$debian_lib/pkgconfig"
export LD_LIBRARY_PATH="$work/movie/sdk/lib:$debian_lib"
export MAKEFLAGS="-j$jobs" FREEJ_BUILD_WORK="$build_work"
configure=(--enable-static-mozjs "--with-native-profile=$profile")
if [[ $profile == static ]]; then
    archive_usr=$archives/debian/usr
    CPPFLAGS="$CPPFLAGS -I$archive_usr/include"
    PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$archive_usr/lib/$multiarch/pkgconfig:$archive_usr/share/pkgconfig"
    for modules in "$archives"/prefixes/*/lib/pkgconfig; do
        PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$modules"
    done
    export PKG_CONFIG_LIBDIR="$work/no-host-pkgconfig"
    mkdir -p -- "$PKG_CONFIG_LIBDIR"
    configure+=("--with-native-sdk=$native" "--with-frei0r-bundle=$effects")
fi

{
    printf 'profile=%s\n' "$profile"
    printf 'ref=%s\n' "${GITHUB_REF:-local}"
    printf 'checkout_sha=%s\n' "$(git rev-parse HEAD)"
    printf 'event_sha=%s\n' "${GITHUB_SHA:-local}"
    printf 'run_id=%s\nrun_attempt=%s\n' "${GITHUB_RUN_ID:-local}" "${GITHUB_RUN_ATTEMPT:-local}"
    printf 'configured_prefix=%s\n' "$build_work/install"
    printf 'started_at_utc=%s\n' "$(date -u +%FT%TZ)"
    "$gcc" --version
    uname -sm
} | tee "$evidence/identity.txt"

compatibility_show() (
    read -r -a sdl_flags <<< "$(sdl-config --cflags)"
    "$ccache_bin" "$gxx" -std=c++11 -Wall -Wextra -Werror -fPIC -isystem src/include \
        -isystem "$build_work/build" -isystem lib/javascript \
        -isystem "$build_work/build/lib/javascript" -isystem "$debian/include" \
        -DJS_GCMETER "${sdl_flags[@]}" -c tests/show-replay/observe.cpp -o "$build_work/observe.o"
    "$gxx" -shared "$build_work/observe.o" -o "$build_work/observe.so" -ldl
    "$ccache_bin" "$gxx" -std=c++11 -Wall -Wextra -Werror -pthread \
        -c tests/show-replay/pulse.cpp -o "$build_work/pulse.o"
    "$gxx" -pthread "$build_work/pulse.o" -ljack -o "$build_work/pulse"
    LD_LIBRARY_PATH="$stage/lib:$LD_LIBRARY_PATH" python3 tests/native-ci-coverage.py compatibility \
        --freej "$stage/bin/freej" --observer "$build_work/observe.so" --plugin "$effects/modules/brightness.so" \
        --pulse "$build_work/pulse" --jackd "$jackd" --output "$out"
    python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["canonical"])' \
        "$out/replay/video-a/observations.json" > "$out/canonical-frames.txt"
    if [[ $pass == cold ]]; then
        replay=(env "LD_LIBRARY_PATH=$stage/lib:$LD_LIBRARY_PATH"
                python3 tests/show-replay/replay.py --freej "$stage/bin/freej"
                --observer "$build_work/observe.so" --plugin "$effects/modules/brightness.so" --case video)
        status=0
        "${replay[@]}" --output "$evidence/reordered" --expect-reversed-order \
            > "$evidence/reordered.log" 2>&1 || status=$?
        cat -- "$evidence/reordered.log"
        # A startup or setup failure cannot satisfy a pixel-mismatch control.
        test "$status" -eq 1
        grep -Fx 'FAIL video-a | video-a: reordered: settled RGB bytes differ' "$evidence/reordered.log"
        grep -Fx '1 executed, 1 failed, 0 skipped' "$evidence/reordered.log"
        printf 'PASS deliberate reorder mismatch reported with exit %s\n' "$status"
    fi
)

static_show() (
    read -r -a engine_cflags <<< "$(PKG_CONFIG_PATH=$prefix/lib/pkgconfig pkg-config --cflags freej)"
    read -r -a engine_libs <<< "$(PKG_CONFIG_PATH=$prefix/lib/pkgconfig pkg-config --libs freej)"
    "$ccache_bin" "$gxx" -DNDEBUG "${engine_cflags[@]}" -c tests/native-profile-factory.cpp \
        -o "$build_work/factory.o"
    "$gxx" "$build_work/factory.o" "-Wl,-Map=$out/factory.map" \
        "${engine_libs[@]}" -o "$build_work/factory"
    "$gcc" -static tests/native-profile-sandbox.c -o "$build_work/sandbox"
    python3 tests/native-profile-runtime.py "$out/runtime" \
        --sandbox "$build_work/sandbox" --strace "$native/packages/usr/bin/strace" \
        --jackd "$jackd" --freej "$stage/bin/freej" \
        --factory "$build_work/factory"
)

effect_parity() (
    home="$out/effect-home"
    mkdir -- "$home"
    if [[ $profile == compatibility ]]; then
        mkdir -p -- "$home/.freej/plugins"
        cp -- "$effects"/modules/*.so "$home/.freej/plugins/"
    fi
    HOME="$home" SDL_VIDEODRIVER=dummy "$build/tests/frei0r-host-test" --frames "$out/frei0r-frames.bin"
    test "$(wc -c < "$out/frei0r-frames.bin")" -eq "$((24 * 16 * 8 * 4))"
    sha256sum "$out/frei0r-frames.bin" | cut -d ' ' -f 1 > "$out/frei0r-frames.sha256"
)

# Both runtime suites use an external JACK 1 daemon with its dummy driver.
jackd=${FREEJ_CI_JACKD:-/usr/bin/jackd}
if [[ $jackd == /usr/bin/jackd ]]; then
    export JACK_DRIVER_DIR=${JACK_DRIVER_DIR:-/usr/lib/$multiarch/jack}
fi
test -x "$jackd"
cold_cache=$work/cold-ccache
mkdir -- "$cold_cache"
# SDK compilation can use the remote cache. The cold FreeJ build always uses
# a fresh object cache, and the warm build uses the objects it just populated.
export CCACHE_DIR="$cold_cache" CCACHE_COMPILERCHECK=content
for pass in cold warm; do
    out=$evidence/$pass
    mkdir -- "$out"
    ccache --zero-stats
    printf 'Building %s with %s cache\n' "$profile" "$pass"
    status=0
    tests/freej-build-test.sh "${configure[@]}" 2>&1 | tee "$out/build.log" || status=$?
    prefix=$build_work/install
    stage=$build_work/stage$prefix
    build=$build_work/build
    for log in config.log tests/test-suite.log; do
        if [[ -f "$build/$log" ]]; then
            cp -- "$build/$log" "$out/"
        fi
    done
    if [[ -f "$build/lib/javascript/config.log" ]]; then
        cp -- "$build/lib/javascript/config.log" "$out/spidermonkey-config.log"
    fi
    if [[ $status != 0 ]]; then
        exit "$status"
    fi
    grep -E '^# (TOTAL|PASS|SKIP|XFAIL|FAIL|XPASS|ERROR):' "$out/test-suite.log" | tee "$out/checks.txt"
    test -x "$stage/bin/freej"
    test "$(PKG_CONFIG_PATH=$prefix/lib/pkgconfig pkg-config --variable=profile freej)" = "$profile"
    PKG_CONFIG_PATH=$prefix/lib/pkgconfig pkg-config --variable=capabilities freej | tee "$out/capabilities.txt"
    test -s "$out/capabilities.txt"
    (cd -- "$stage" && sha256sum bin/freej) | tee "$out/binaries.sha256"
    readelf -lW -dW "$stage/bin/freej" > "$out/freej.elf.txt"
    if [[ $profile == static ]]; then
        if grep -Eq '^[[:space:]]+INTERP[[:space:]]|\(NEEDED\)' "$out/freej.elf.txt"; then
            echo 'freej unexpectedly has dynamic linkage' >&2
            exit 1
        fi
    else
        grep -q '(NEEDED)' "$out/freej.elf.txt"
        LD_LIBRARY_PATH="$stage/lib:$LD_LIBRARY_PATH" ldd "$stage/bin/freej" > "$out/freej.ldd.txt"
        if grep -Fq 'not found' "$out/freej.ldd.txt"; then
            cat -- "$out/freej.ldd.txt" >&2
            exit 1
        fi
    fi
    echo 'CLI built, staged and passed link audits.' > "$out/compile-outcome.txt"
    effect_parity 2>&1 | tee "$out/frei0r-parity.log"
    if [[ $profile == static ]]; then
        cp -- "$build_work/build/src/freej.map" "$out/freej.map"
        static_show 2>&1 | tee "$out/show.log"
    else
        compatibility_show 2>&1 | tee "$out/show.log"
    fi
    grep -E '^[0-9]+ executed, 0 failed, 0 skipped$' "$out/show.log" | tee -a "$out/checks.txt"
    python3 tests/native-ci-coverage.py report "$profile" "$out" | tee "$out/coverage.md"
    ccache --print-stats > "$out/ccache.tsv"
    ccache --show-stats | tee "$out/ccache.txt"
    if [[ $pass == cold ]]; then
        # Preserve the first trial, then rebuild from an empty install prefix at the same path.
        mv -- "$build_work" "$work/cold-build"
    fi
done

cmp "$evidence/cold/capabilities.txt" "$evidence/warm/capabilities.txt"
cmp "$evidence/cold/checks.txt" "$evidence/warm/checks.txt"
cmp "$evidence/cold/coverage.md" "$evidence/warm/coverage.md"
cmp "$evidence/cold/frei0r-frames.bin" "$evidence/warm/frei0r-frames.bin"
if [[ $profile == compatibility ]]; then
    cmp "$evidence/cold/canonical-frames.txt" "$evidence/warm/canonical-frames.txt"
fi
awk -F '\t' '
    $1 == "direct_cache_hit" || $1 == "preprocessed_cache_hit" {
        if (FILENAME == ARGV[1]) cold += $2
        else warm += $2
    }
    END {
        printf "Compiler cache hits: %d cold, %d warm\n", cold, warm
        exit !(warm > cold)
    }
' "$evidence/cold/ccache.tsv" "$evidence/warm/ccache.tsv" | tee "$evidence/cache-comparison.txt"
if cmp -s "$evidence/cold/binaries.sha256" "$evidence/warm/binaries.sha256"; then
    echo 'Staged CLI SHA-256 digests match'
else
    echo 'Staged CLI recorded different SHA-256 digests'
fi | tee -a "$evidence/cache-comparison.txt"
echo 'PASS cold/warm capabilities and test outcomes match' | tee -a "$evidence/cache-comparison.txt"

tar -C "$stage" -czf "$evidence/stage.tar.gz" .
cp -- tests/*sdk.lock tests/native-linux-archives.lock tests/native-ci.lock "$evidence/"
(
    cd -- "$evidence"
    find . -type f ! -path './SHA256SUMS' -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS
)
