#!/bin/sh

set -e
set -x

# Freej build-system test example

REPO=ssh://git@code.dyne.org/freej.git
# avoid remote cloning
#REPO=/path/to/local/repo

CONFIGURE_FLAGS="--enable-debug --disable-cpuflags --enable-python --enable-ruby"

BASE=`mktemp --tmpdir=/tmp -d freejbuild.XXXXXXXXX`

GIT=`mktemp --tmpdir=$BASE -d git.XXXXX`
BUILD=`mktemp --tmpdir=$BASE -d build.XXXXX`
INST=`mktemp --tmpdir=$BASE -d inst.XXXXX`
DIST=`mktemp --tmpdir=$BASE -d dist.XXXXX`
BUILD_DIST=`mktemp --tmpdir=$BASE -d build_dist.XXXXX`
INST_DIST=`mktemp --tmpdir=$BASE -d inst_dist.XXXXX`
DESTDIR=`mktemp --tmpdir=$BASE -d destdir.XXXXX`

# Start from scratch
git clone $REPO $GIT

# Build in a directory different from sources
cd $BUILD
srcdir=$GIT $GIT/autogen.sh
$GIT/configure $CONFIGURE_FLAGS --prefix=$INST
make

# Install into an empty directory
make install

# Install into an arbitrary directory
make install DESTDIR=$DESTDIR

# Create distribution tarball and check it
make dist
PACKAGE=`grep '^PACKAGE\>' Makefile | cut -d ' ' -f 3`
VERSION=`grep '^VERSION\>' Makefile | cut -d ' ' -f 3`
tar xzf ${PACKAGE}-${VERSION}.tar.gz -C $DIST

# XXX(godog): make distcheck
# XXX(godog): diff -urN $INST $DESTDIR should return nothing!

# XXX(godog) compare file lists, does this have any sense instead of diff -Naur | lsdiff ?
# tar tzf ${PACKAGE}-${VERSION}.tar.gz | cut -d/ -f2- | sort | uniq | sed 's|/$||' > /tmp/dist.freej
# make distclean
# cd $GIT && make distclean && find . -path ./.git -prune -o -print | cut -d/ -f2- | sort | uniq > /tmp/git.freej
# meld /tmp/dist.freej /tmp/git.freej

cd $BASE
echo Directories are:
echo git      = $GIT
echo build    = $BUILD
echo inst     = $INST
echo dist     = $DIST
echo destdir  = $DESTDIR
echo
echo Please check everything has been installed and included in the tarball
