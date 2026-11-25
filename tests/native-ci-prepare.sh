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

python3 tests/legacy-linux-sdk.py "$work/legacy" --jobs "$jobs" 2>&1 | tee "$work/logs/legacy-sdk.log"
if [[ $profile == static ]]; then
    python3 tests/native-linux-archives.py "$work/archives" --reference-sdk "$work/legacy" \
        --jobs "$jobs" 2>&1 | tee "$work/logs/archive-sdk.log"
    python3 tests/native-static-sdk.py "$work/native" --archive-sdk "$work/archives" \
        --reference-sdk "$work/legacy" --jobs "$jobs" 2>&1 | tee "$work/logs/static-sdk.log"
else
    read -r checksum url < <(tail -n 1 tests/native-ci.lock)
    curl --fail --silent --show-error --location --retry 3 --connect-timeout 10 --max-time 120 \
        --output "$work/frei0r.tar.gz" "$url"
    printf '%s  %s\n' "$checksum" "$work/frei0r.tar.gz" | sha256sum --check
    mkdir -- "$work/frei0r"
    tar -xzf "$work/frei0r.tar.gz" --strip-components=1 -C "$work/frei0r"
fi
