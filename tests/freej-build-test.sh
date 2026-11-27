#!/bin/sh
set -eu

# Build a disposable copy of this checkout, including unignored local files.
repo=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd -P)
if test -n "${FREEJ_BUILD_WORK:-}"; then
    # An explicit, new path lets cache trials use identical build inputs.
    mkdir -- "$FREEJ_BUILD_WORK"
    work=$(CDPATH='' cd -- "$FREEJ_BUILD_WORK" && pwd -P)
else
    work=$(mktemp -d "${TMPDIR:-/tmp}/freejbuild.XXXXXXXXXX")
fi
printf 'Build artifacts and logs: %s\n' "$work"

source=$work/source
build=$work/build
prefix=$work/install
stage=$work/stage
dist=$work/dist
mkdir "$source" "$build" "$prefix" "$stage" "$dist"
git -C "$repo" archive --format=tar HEAD > "$work/source.tar"
git -C "$repo" diff --binary HEAD -- > "$work/source.diff"
git -C "$repo" ls-files --others --exclude-standard -z > "$work/source-untracked"
tar -xf "$work/source.tar" -C "$source"
if test -s "$work/source.diff"; then
    (cd "$source" && git apply "$work/source.diff")
fi
if test -s "$work/source-untracked"; then
    tar -C "$repo" --null -T "$work/source-untracked" -cf "$work/source-untracked.tar"
    tar -xf "$work/source-untracked.tar" -C "$source"
fi

"$source/autogen.sh"
cmp "$repo/INSTALL" "$source/INSTALL"
cd "$build"
if "$source/configure" --enable-debug --disable-cpuflags "$@" --prefix="$prefix"; then
    :
else
    status=$?
    printf 'Configure log: %s\n' "$build/config.log" >&2
    if test -f "$build/lib/javascript/config.log"; then
        printf 'SpiderMonkey configure log: %s\n' "$build/lib/javascript/config.log" >&2
    fi
    exit "$status"
fi

make
make check
make install
make install DESTDIR="$stage"

make dist-gzip
set -- ./*.tar.gz
test "$#" -eq 1
test -f "$1"
tar -xzf "$1" -C "$dist"
archive=${1#./}
cmp "$repo/INSTALL" "$dist/${archive%.tar.gz}/INSTALL"
printf 'Installed prefix: %s\nStaged prefix: %s\nDistribution: %s\n' \
    "$prefix" "$stage$prefix" "$dist"
