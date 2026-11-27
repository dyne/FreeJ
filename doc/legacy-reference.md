# Linux reference build

This recipe reproduces the assertion-enabled FreeJ 1.0 reference on Debian 13
(trixie), amd64. Use it with local show fixtures. The FFmpeg 0.8.15 SDK is
from 2013 and is intended for historical comparison.
See the [capability ledger](capabilities.md) for working paths, runtime
resources and historical limitations.
See the [native dependency map](native-dependencies.md) for exact archive
inputs and individual link checks.

The recorded environment used Autoconf 2.72, Automake 1.17, Libtool 2.5.4,
binutils 2.44, Make 4.4.1, pkgconf 1.8.1, Python 3.13, Perl 5.40 and glibc 2.41.
Install or provision standard Debian development tools and these host
development packages before running the recipe:

```text
libslang2-dev libsdl1.2-dev libogg-dev libvorbis-dev liblo-dev
libjack-dev libfftw3-dev libsamplerate0-dev libasound2-dev libfontconfig-dev
libfreetype-dev libcairo2-dev libx11-dev libssl-dev libbz2-dev zlib1g-dev
```

The SDK recipe extracts the GCC and G++ 12.4.0-5 packages and the remaining
Debian libraries into a new directory. The archive URLs and SHA256 digests
are in `tests/legacy-linux-sdk.lock`. The FFmpeg tarball was verified against
the upstream release key `FCF986EA15E6E293A5644F10B4322F04D67658D8` before
pinning. The recipe builds its shared and static libraries with PIC and the
portable C implementations. Current GNU binutils reject an old x86 shift
operand in the assembler implementations. The compiler also accepts the old
bundled SpiderMonkey configure probes, so the FreeJ build needs no global
diagnostic overrides.

From the FreeJ checkout, use a scratch path without spaces:

```sh
reference=$(mktemp -d)
sdk=$reference/sdk
python3 tests/legacy-linux-sdk.py "$sdk" --jobs 4

debian_usr=$sdk/debian/usr
debian_lib=$debian_usr/lib/x86_64-linux-gnu
gcc=$sdk/gcc12/usr/bin/gcc-12
gxx=$sdk/gcc12/usr/bin/g++-12
if command -v ccache >/dev/null 2>&1; then
    gcc="ccache $gcc"
    gxx="ccache $gxx"
fi
PKG_CONFIG_PATH="$sdk/ffmpeg/lib/pkgconfig:$debian_lib/pkgconfig" \
CPPFLAGS="-I$debian_usr/include" LDFLAGS="-L$debian_lib" \
LD_LIBRARY_PATH="$sdk/ffmpeg/lib:$debian_lib" \
CC="$gcc" CXX="$gxx" MAKEFLAGS=-j4 TMPDIR="$reference" \
    tests/freej-build-test.sh --enable-static-mozjs
```

The test exports the local files to a disposable source, bootstraps there, and
uses a separate build directory. It runs `make check`, installs to an empty
prefix, installs separately using `DESTDIR`, and unpacks the source archive.
It compares FreeJ's `INSTALL` with the copies left by bootstrap and packaged
in the archive.
Use a fresh scratch directory to repeat the whole run. Save the top-level and
SpiderMonkey `config.log` files if configuration fails.

To launch the staged binary with a local PNG, substitute the staged prefix
printed by the script and use an available X11 display:

```sh
LD_LIBRARY_PATH="$staged_prefix/lib:$sdk/ffmpeg/lib:$debian_lib" \
    "$staged_prefix/bin/freej" -c -s 80x60 /absolute/path/to/local.png
```

For a bounded Xvfb show, pass `-j` a script that loads the PNG, discovers the
Invert0r effect, runs 24 frames at 20 FPS, and calls `quit()`. The CLI exits
with status 1 when the startup script sets `freej->quit`, as handled in
`main` after `open_script` in `src/freej.cpp`. The reference observer exits
0 after confirming that child status, script and effect markers, the
expected pixels, and 0.85–2.50 seconds of frame pacing without a timeout.

Use assertions when reproducing the original reference commit. In that
revision, `-DNDEBUG` removed the constructor's call to `Context::init()`
and JavaScript startup failed. The current native profiles exercise startup
with `-DNDEBUG`. See [native profiles](native-profiles.md) for those checks.
Test actual JACK, MIDI and capture hardware separately from the Xvfb reference.
