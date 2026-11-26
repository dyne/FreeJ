#!/usr/bin/env bash
set -euo pipefail

if [[ $# != 1 ]]; then
    echo 'Usage: pipewire-linux-sdk.sh NEW_WORK_DIRECTORY' >&2
    exit 2
fi
work=$(realpath -m -- "$1")
recipe=$(cd -- "$(dirname -- "$0")" && pwd -P)
mkdir -- "$work"
cd -- "$work"
read -r digest url < "$recipe/pipewire-linux-sdk.lock"
curl --fail --silent --show-error --location --retry 3 --connect-timeout 10 --max-time 120 \
    --output pipewire.tar.bz2 "$url"
printf '%s  pipewire.tar.bz2\n' "$digest" | sha256sum --check --strict
mkdir -- source
tar -xjf pipewire.tar.bz2 --strip-components=1 --directory source
patch --directory source -p1 < "$recipe/pipewire-static-sdk.patch"

# The daemon and the compatibility SDK use upstream shared libraries. The
# static client embeds only the native transport, client-node, and SPA support.
meson setup build source --prefix="$work/sdk" --libdir=lib --buildtype=release --wrap-mode=nodownload \
    -Dauto_features=disabled -Dspa-plugins=enabled -Dsupport=enabled \
    -Dpipewire-jack=disabled -Djack=disabled -Dexamples=disabled -Dtests=disabled \
    -Dsession-managers=
meson compile -C build -j "${FREEJ_CI_JOBS:-4}"
meson install -C build --no-rebuild
test -x sdk/bin/pipewire
test -s sdk/lib/libpipewire-0.3.a
