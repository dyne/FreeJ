# Native profiles

## Moving from FreeJ 1.x to 2.0

Choose the Linux amd64 compatibility build if a set uses external frei0r
or FreeFrame modules. It searches the configured library directory,
`~/.freej/plugins`, and the system effect directories.
Keep your existing show scripts, assets, and modules together, and replay a
copy of the show with 2.0 before switching your performance setup.

Choose the Linux amd64 static build for sets that use the built-in frei0r
effects Brightness, Invert0r, and onecol0r. It loads those effects from its
own executable and does not scan plugin directories. It uses fonts, display
and audio services, and show assets supplied by your system. Launch shows
from the command line using `freej -j show.js`.

Both Linux profiles connect to the user’s running PipeWire daemon for
audio. The JavaScript `AudioJack(port, window, rate)` constructor keeps
its name for existing scripts. Set `port` to a PipeWire source shown by
`pw-link -o`, in `node.name:port.name` form. The FreeJ input and output
ports are visible in the PipeWire graph when in use and can be patched
with `pw-link` or a graph manager. The compatibility build uses
`libpipewire-0.3`. The static build includes the native client transport
and connects over the daemon’s Unix socket. Both use the normal PipeWire
runtime directory and remote, overridable with `PIPEWIRE_RUNTIME_DIR`
and `PIPEWIRE_REMOTE`.

Keep the 1.x installation and runtime libraries in a separate prefix from
2.0. If a third-party effect or a show fails in 2.0 compatibility, launch
the saved 1.x executable by its full path, restore its original library and
plugin environment, and open the saved 1.x copy of the show. Keep the 2.0
prefix separate while investigating the difference.

### Check a staged package

Build and stage each profile using the [Linux CI recipes](native-ci.md).
The Autotools source archive is `freej-2.0.0.tar.gz`. Label the two native
stage archives `freej-2.0.0-linux-amd64-compatibility.tar.gz` and
`freej-2.0.0-linux-amd64-static.tar.gz`.

In the corresponding build environment, set `stage` to the staged prefix.
For compatibility, retain the loader path for the build recipe's runtime
libraries. Confirm the version and selected profile before running a set:

```bash
LD_LIBRARY_PATH="$stage/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" "$stage/bin/freej" -v
PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR="$stage/lib/pkgconfig" pkg-config --modversion freej
PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR="$stage/lib/pkgconfig" pkg-config --variable=profile freej
```

The CLI in both profiles reports `freej version 2.0.0`, and pkg-config reports
`2.0.0` and `compatibility` or `static`. Use the external-effect show
replay for compatibility and the installed static stage check below for
the built-in effects.

## Build selections

Configure defaults to `--with-native-profile=compatibility`. This profile
uses the established system and workspace SDK libraries and searches the
existing effect directories for frei0r and FreeFrame.

`--with-native-profile=static` selects the Linux archive build. It links
the engine and CLI with their selected static libraries.
The frei0r bundle registers the built-in effects Brightness, Invert0r, and
onecol0r at startup. Both profiles use the same frei0r instance host and keep
the effect names used in shows.

Configure prints the enabled selections and exports them through the installed
`freej.pc`. The engine reports the same selections at startup.
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
python3 tests/legacy-linux-sdk.py "$legacy" --skip-historical-media
tests/movie-linux-sdk.sh compatibility "$work/movie-compat"
tests/movie-linux-sdk.sh static "$work/movie-static"
python3 tests/native-linux-archives.py "$archives" --reference-sdk "$legacy" --skip-sdl12
python3 tests/native-static-sdk.py "$native" --archive-sdk "$archives" --reference-sdk "$legacy"
gcc="$legacy/gcc12/usr/bin/gcc-12"
gxx="$legacy/gcc12/usr/bin/g++-12"
CC="$gcc" CXX="$gxx" tests/pipewire-linux-sdk.sh "$work/pipewire"
CC="$gcc" CXX="$gxx" tests/sdl-linux-sdk.sh compatibility "$work/sdl-compat"
archive_usr="$archives/debian/usr"
extensions_usr="$native/packages/usr"
PKG_CONFIG_PATH="$archive_usr/lib/x86_64-linux-gnu/pkgconfig:$extensions_usr/lib/x86_64-linux-gnu/pkgconfig" \
    CC="$gcc" CXX="$gxx" tests/sdl-linux-sdk.sh static "$work/sdl-static" "$archive_usr" "$extensions_usr"
read -r _ frei0r_url < <(tail -n 1 tests/native-ci.lock)
curl -fL "$frei0r_url" -o "$work/frei0r.tar.gz"
tests/frei0r-linux-sdk.sh "$work/frei0r.tar.gz" "$work/effects"
```

Both profiles use [FFmpeg 7.1.5](https://ffmpeg.org/releases/ffmpeg-7.1.5.tar.xz)
for movie decoding. The [movie SDK lock](../tests/movie-linux-sdk.lock)
pins the release source and the recipe builds the decoder and native
demuxer libraries. The historical FFmpeg 0.8.15 SDK remains available for
reproducing the reference observations.

Both Linux profiles use SDL 3.4.20, SDL3_image 3.4.8 and SDL3_ttf 3.2.2
from the [SDL SDK lock](../tests/sdl-linux-sdk.lock). The compatibility
profile uses shared SDL3 libraries. The static profile links the SDL3
archives and the X11 extensions from the
[static SDK lock](../tests/native-static-sdk.lock). The static SDL3 build
disables the dynamic API and shared X11 loading. The archived Xlib also
builds with loadable cursor and locale modules disabled. FreeJ uses X11 or the
dummy display driver and polls SDL3 for input. Both profiles build against the PipeWire 1.4.2 source from the
[PipeWire SDK lock](../tests/pipewire-linux-sdk.lock). The recipe also builds
the daemon used by the private software test graph.

The SDK retains source licenses, Debian package copyright files, lock inputs
and build trees in its working directory.

The static configure step validates the archive paths and verifies a
static link using SDL3, SDL3_image, SDL3_ttf, FFmpeg, frei0r, PipeWire and
samplerate. It requires the built-in PipeWire native transport and SPA
support archive from the pinned SDK. Explicit shared-object loading fails in the static application.
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
export PKG_CONFIG_PATH="$work/sdl-compat/sdk/lib/pkgconfig:$work/movie-compat/sdk/lib/pkgconfig:$work/pipewire/sdk/lib/pkgconfig:$debian_lib/pkgconfig"
export LD_LIBRARY_PATH="$work/sdl-compat/sdk/lib:$work/movie-compat/sdk/lib:$work/pipewire/sdk/lib:$debian_lib"
tests/freej-build-test.sh --enable-static-mozjs --with-native-profile=compatibility

archive_usr=$archives/debian/usr
export CPPFLAGS="$CPPFLAGS -I$archive_usr/include"
export PKG_CONFIG_PATH="$work/sdl-static/sdk/lib/pkgconfig:$work/movie-static/sdk/lib/pkgconfig:$work/pipewire/sdk/lib/pkgconfig:$debian_lib/pkgconfig:$archive_usr/lib/x86_64-linux-gnu/pkgconfig:$archive_usr/share/pkgconfig:$extensions_usr/lib/x86_64-linux-gnu/pkgconfig"
for modules in "$archives"/prefixes/*/lib/pkgconfig; do
    PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$modules"
done
mkdir "$work/no-host-pkgconfig"
PKG_CONFIG_LIBDIR="$work/no-host-pkgconfig" \
    tests/freej-build-test.sh --enable-static-mozjs --with-native-profile=static \
        --with-native-sdk="$native" --with-frei0r-bundle="$work/effects"
```

Set `compat_prefix` and `static_prefix` to the respective installed prefixes.
For compatibility startup, include `$compat_prefix/lib` in
`LD_LIBRARY_PATH`. Use the external-effect [show replay](../tests/show-replay/README.md)
on that build. Retain the normal list of directories and its external
effect modules.

## Check the installed static stage

Run `tests/native-ci-run.sh static PREPARED_WORK NEW_EVIDENCE_DIR` with
the pinned SDKs prepared by `tests/native-ci-prepare.sh`. It builds the
factory consumer with the installed package flags and checks the CLI and
factory with Xvfb and a private PipeWire daemon. Linux Landlock limits
the staged processes to runtime resources and `strace` audits file
accesses. The checks verify image and built-in effect rendering, capture
from the native PipeWire graph, and absence of external library and
effect lookups.

The CLI linker map is `src/freej.map` in the build directory. The
evidence also includes the installed-consumer linker map,
`readelf` output and file traces. Physical MIDI, joystick, capture and
audio devices need their respective rigs. glibc’s NSS configuration and
any external ALSA or OpenSSL providers remain runtime inputs.
