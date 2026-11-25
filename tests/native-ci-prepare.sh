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
tests/movie-linux-sdk.sh "$profile" "$work/movie" 2>&1 | tee "$work/logs/movie-sdk.log"
read -r _ url < <(tail -n 1 tests/native-ci.lock)
curl --fail --silent --show-error --location --retry 3 --connect-timeout 10 --max-time 120 \
    --output "$work/frei0r.tar.gz" "$url"
tests/frei0r-linux-sdk.sh "$work/frei0r.tar.gz" "$work/effects" 2>&1 | tee "$work/logs/frei0r-sdk.log"
if [[ $profile == static ]]; then
    python3 tests/native-linux-archives.py "$work/archives" --reference-sdk "$work/legacy" \
        --jobs "$jobs" 2>&1 | tee "$work/logs/archive-sdk.log"
    python3 tests/native-static-sdk.py "$work/native" --archive-sdk "$work/archives" \
        --reference-sdk "$work/legacy" --jobs "$jobs" 2>&1 | tee "$work/logs/static-sdk.log"
fi
