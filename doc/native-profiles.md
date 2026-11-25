# Native profiles

Configure defaults to `--with-native-profile=compatibility`. This profile
uses the established system and workspace SDK libraries and searches the
existing effect directories for frei0r and FreeFrame.

`--with-native-profile=static` selects the Linux archive build. It links
the engine, CLI and Qt/XCB frontend with their selected static libraries.
The early static build starts with an empty effect registry. Built-in effects
will use the same host when the frei0r integration lands.

Configure prints the enabled selections and exports them through the installed
`freej.pc`. The engine reports the same selections at startup. Qfreej uses
the installed package's flags and prints its selections at configuration time.
These are build selections. The historical movie factory and other runtime
limitations are recorded in the [capability ledger](capabilities.md).

## Build the SDKs

Use Debian 13 amd64 and a scratch path without spaces. The compiler, development
tools and compatibility packages are listed in the [reference build](legacy-reference.md).
The pinned source versions, archive dependencies and licenses are in the
[dependency map](native-dependencies.md).

```bash
work=$(mktemp -d)
legacy=$work/reference
archives=$work/archives
native=$work/native
python3 tests/legacy-linux-sdk.py "$legacy"
python3 tests/native-linux-archives.py "$archives" --reference-sdk "$legacy"
python3 tests/native-static-sdk.py "$native" --archive-sdk "$archives" --reference-sdk "$legacy"
```

The static SDK adds the SDL2, Qt/XCB and keyboard dependencies from
[`native-static-sdk.lock`](../tests/native-static-sdk.lock).
Its patches bind SDL 1.2.68 to archived SDL2 2.32.4, preserve the SDL 1.2
exports, and give SDL2's symbols a private prefix. SDL2's dynamic API and
shared platform loaders are disabled in this build. It uses X11, dummy
video, ALSA and Linux joystick support. FreeJ's JACK client uses the
server's socket directory directly. Set `JACK_TMPDIR` to the existing
server's directory as printed by `jackd -l`. The archived default is
`/dev/shm`. Run the JACK server separately from FreeJ.

The Qt 5.15.15 SDK selects static XCB and includes the XCB, xkbcommon,
X session, UUID and font archives. Debian's xcb-image pkg-config metadata
omits its static dependency on xcb-util, which the recipe adds before
configuring Qt. Retain source licenses and Debian package copyright files.
The SDK retains source notices, lock inputs and build trees in its working
directory. Qt's license files are in the Qt source directory.

The static configure step validates the archive paths and verifies a
static link using SDL, JACK and samplerate. It refuses the SDL wrapper that
loads shared SDL2 and the JACK client that shells out to locate its runtime
directory. Explicit shared-object loading fails in the static application.
Supported optional selections such as AAlib and CWiid require their corresponding
archives. OpenGL, libunicap and OpenCV still have compatibility paths and
need verified archive closures before the static profile can select them.

## Build and install with assertions disabled

Use the same SDK inputs for both builds. The following commands use Bash.
`tests/freej-build-test.sh` prints the installed prefix and separate DESTDIR
stage for each run and retains its build and test logs.

```bash
debian=$legacy/debian/usr
debian_lib=$debian/lib/x86_64-linux-gnu
export CC="$legacy/gcc12/usr/bin/gcc-12"
export CXX="$legacy/gcc12/usr/bin/g++-12"
if command -v ccache >/dev/null 2>&1; then
    export CC="ccache $CC" CXX="ccache $CXX"
fi
export CFLAGS="-O2 -DNDEBUG" CXXFLAGS="-O2 -DNDEBUG"
export CPPFLAGS="-I$debian/include" LDFLAGS="-L$debian_lib"
export PKG_CONFIG_PATH="$legacy/ffmpeg/lib/pkgconfig:$debian_lib/pkgconfig"
export LD_LIBRARY_PATH="$legacy/ffmpeg/lib:$debian_lib"
tests/freej-build-test.sh --enable-static-mozjs --with-native-profile=compatibility

archive_usr=$archives/debian/usr
export CPPFLAGS="$CPPFLAGS -I$archive_usr/include"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$archive_usr/lib/x86_64-linux-gnu/pkgconfig:$archive_usr/share/pkgconfig"
for modules in "$archives"/prefixes/*/lib/pkgconfig; do
    PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$modules"
done
mkdir "$work/no-host-pkgconfig"
PKG_CONFIG_LIBDIR="$work/no-host-pkgconfig" \
    tests/freej-build-test.sh --enable-static-mozjs --with-native-profile=static --with-native-sdk="$native"
```

Set `compat_prefix` and `static_prefix` to the respective installed prefixes.
Use the system qmake for compatibility and the native SDK qmake for the static
profile. Qfreej defaults to its historical debug build. Select
`CONFIG+=freej_release` to build Qfreej with `NDEBUG` for either profile.

```bash
source_root=$PWD
mkdir "$work/gui-compat" "$work/gui-static"
(
    cd "$work/gui-compat"
    export PKG_CONFIG_PATH="$compat_prefix/lib/pkgconfig"
    qmake "$source_root/qt/Qfreej.pro" "CONFIG+=freej_release"
    make -j4
    make install
)
(
    cd "$work/gui-static"
    export PKG_CONFIG_PATH="$static_prefix/lib/pkgconfig"
    "$native/qt/bin/qmake" "$source_root/qt/Qfreej.pro" "CONFIG+=freej_release" \
        "QMAKE_LFLAGS+=-Wl,-Map=$work/qfreej.map"
    make -j4
    make install
)
```

For compatibility startup, include `$compat_prefix/lib` in
`LD_LIBRARY_PATH`. Use the external-effect [show replay](../tests/show-replay/README.md)
on that build. Retain the normal list of directories and its external
effect modules.

## Check the installed static stage

Build the factory consumer using the installed package flags. It exercises
registration from the archive and attaches to a running JACK server. The
runtime check starts Xvfb and a separate JACK 1 dummy server, observes
the rendered image and both mapped Qt/SDL windows, and checks that each
host initializes its engine once. It restricts the static processes to
the runtime resources, with no library tree or plugin directories. Linux
Landlock and `strace` provide the file-access check.

Set `jackd` to a JACK 1 installation with the dummy driver and a writable
socket directory with a short Unix socket path. Set `tracer` to a `strace`
with `--kill-on-exit` support, such as the one in the SDK packages.

```bash
export PKG_CONFIG_PATH="$static_prefix/lib/pkgconfig"
pkg-config --variable=capabilities freej
read -r -a engine_cflags <<< "$(pkg-config --cflags freej)"
read -r -a engine_libs <<< "$(pkg-config --libs freej)"
"$legacy/gcc12/usr/bin/g++-12" -DNDEBUG "${engine_cflags[@]}" \
    tests/native-profile-factory.cpp "-Wl,-Map=$work/factory.map" \
    "${engine_libs[@]}" -o "$work/factory"
"$legacy/gcc12/usr/bin/gcc-12" -static tests/native-profile-sandbox.c -o "$work/sandbox"
python3 tests/native-profile-runtime.py "$work/static-runtime" \
    --sandbox "$work/sandbox" --strace "$tracer" --jackd "$jackd" \
    --freej "$static_prefix/bin/freej" --qfreej "$static_prefix/bin/qfreej" \
    --factory "$work/factory"
```

The CLI linker map is `src/freej.map` in the build directory. Review it
alongside the Qt and installed-consumer maps and the runtime driver's
`readelf` output and file traces. The checks use X11 and JACK software
endpoints. Physical MIDI, joystick, capture and audio devices need their
respective rigs. glibc's NSS configuration and any external ALSA or OpenSSL
providers remain runtime inputs. The static image and GUI fixture covers
their behavior during the observed startup, image and JACK paths.
