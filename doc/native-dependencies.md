# Native Linux dependency map

The Debian 13 amd64 compatibility build uses the [historical reference SDK](legacy-reference.md).
Its feature and device observations are in the [capability ledger](capabilities.md).
The archive SDK below supplies the same dependency families to individual
link consumers. It lets link checks select explicit archive paths while the
existing Autotools build continues to select its compatibility libraries.

## Active link inputs

Exact input URLs, package revisions and SHA-256 digests are in
[`legacy-linux-sdk.lock`](../tests/legacy-linux-sdk.lock) and
[`native-linux-archives.lock`](../tests/native-linux-archives.lock).
For Debian packages, keep their `usr/share/doc/<package>/copyright` files
and the corresponding source notices. Source releases below use upstream
tarballs without code patches unless a repository change is identified.
Staging only rewrites absolute prefixes in Debian pkg-config files.

| Provider and reference pin | FreeJ consumer and ABI | Archive closure and runtime resources | Source notices |
| --- | --- | --- | --- |
| SDL 1.2 API via sdl12-compat 1.2.68, Debian SDL2 2.32.4 | `sdl_screen.cpp` and `joy_ctrl.cpp` use SDL 1.2 `SDL_SetVideoMode`, `SDL_Flip` and `SDL_JoystickOpen`. | Upstream CMake `STATICDEVEL=ON` produces `libSDL.a`. The wrapper uses `libdl` to open `libSDL2-2.0.so.0` and load the platform backends. The checks exercise dummy and X11. X11 needs a display server. Joystick input needs the device and permissions. Compatibility uses Debian's shared SDL 1.2 wrapper. | sdl12-compat `LICENSE.txt`, SDL2 package copyright |
| SDL_image 1.2.12 | `image_layer.cpp` uses `IMG_Load` and SDL 1.2 surfaces. | Debian archive needs SDL, PNG/zlib, JPEG, TIFF and WebP. TIFF adds WebP/SharpYUV, JBIG, libdeflate, xz, Lerc, Zstd, libm and the C++ runtime through Lerc. File input is a show resource. | SDL_image and codec package copyrights |
| SDL_gfx 2.0.25 | `geo_layer.cpp` uses `pixelColor` and `sdl_screen.cpp` uses `rotozoomSurface`. | Debian archive, SDL 1.2 and libm. Configure can select the bundled 2.0.13 fallback. | SDL_gfx package copyright and bundled source banners |
| SDL_ttf 2.0.11 | `text_layer.cpp` uses `TTF_OpenFont` and SDL 1.2 surfaces. | Debian archive with SDL 1.2 and FreeType. Configure can select the bundled 2.0.7 fallback. Fonts are runtime files. | SDL_ttf package copyright and bundled LGPL banner |
| S-Lang 2.3.3 and bundled slw | `console_ctrl.cpp` uses `SLangConsole`, whose implementation calls `SLang_init_tty`, `SLkp` and `SLsmg` functions. | Debian S-Lang archive with libdl/libm. The bundled slw archive also uses FreeJ's Entry, Linklist and logging objects plus the C++ runtime and pthreads. The interactive console uses a terminal and terminfo. | S-Lang package copyright and `lib/slw/COPYING` |
| Bundled CCVT | YUV conversion entry points such as `ccvt_420p_bgr32`, with a portable C path. | The Make graph builds `libccvt.a` for the video, capture and encoding paths. | `lib/ccvt/COPYING` |
| Bundled SpiderMonkey, JavaScript 1.8 | `jsparser.cpp` uses the older JSAPI `JS_NewRuntime`, `JS_NewContext` and `JS_EvaluateScript`. | `--enable-static-mozjs` builds the PIC archive in the reference with GCC 12. C++ runtime, libm, libdl and pthreads close the consumer link. Configure's system alternatives probe the old `mozilla-js >= 1.9` API and, where needed, its xul embedding metadata. No compatible system variant was selected. | `lib/javascript/README.html` and MPL/GPL/LGPL source banners |
| FFmpeg 0.8.15 | `video_layer.cpp` and `Context::init` use `AVPicture`, `AVStream::codec`, `av_open_input_file`, `avcodec_decode_video2`, `av_register_all` and swscale. | The historical recipe uses PIC, portable C and both shared/static libraries. Its four `.pc` files describe `avformat`, `avcodec`, `avutil`, `swscale`, plus sndio, JACK, ALSA, SDL, bzip2, zlib and libm. Link maps distinguish libraries named by metadata from archives that supplied objects. Movies are runtime assets. | Source `COPYING.LGPLv2.1`, other `COPYING.*` files and per-file notices |
| Ogg 1.3.5, Vorbis/Vorbisenc 1.3.7, Theora 1.2.0alpha1 | `theorautils.cpp` uses `ogg_stream_init`, `vorbis_encode_init` and the legacy `theora_encode_init` API. | Debian archives. Vorbisenc uses Vorbis, Ogg and libm. The legacy Theora wrapper uses Ogg and libm. This build of the Theora decoder includes Cairo telemetry references that are reached through libshout. | Xiph package copyrights |
| libshout 2.4.6 with Speex 1.2.1 | `shouter.cpp` and encoder paths use `shout_open` and Xiph streams. | The static link uses the Xiph libraries, Speex, OpenSSL, zlib, Zstd, pthreads, libdl and libm. It also needs the Cairo closure for Theora decoder telemetry, which is missing from the `shout.pc` closure. Streaming needs a separately configured Icecast endpoint. | libshout, Xiph, Speex, OpenSSL, zlib and Zstd package copyrights |
| JACK 1 0.126.0 | `audio_jack.cpp` and Qt use the JACK 1 client API, including `jack_client_open` and the process callback. | Autotools `--enable-static --disable-shared --enable-force-install --bindir=/usr/bin`, direct libjack target. Berkeley DB 5.3.28's `libdb.a`, libm and libdl supplement the pthreads in `jack.pc`. A JACK server with compatible shared memory and socket settings is a runtime service. The server and audio driver are separate from the archived client. | JACK `COPYING`, `COPYING.LGPL` and `COPYING.GPL`, Berkeley DB package copyright |
| FFTW 3.3.10, libsamplerate 0.2.2 | `audio_collector.cpp` uses the double-precision FFTW3 API. `theorautils.cpp` uses `src_new` and `src_process`. | Debian supplies `libfftw3.a`. CMake `BUILD_SHARED_LIBS=OFF` and PIC build `libsamplerate.a`. Both use libm for in-memory processing. | FFTW package copyright, libsamplerate `COPYING` |
| ALSA 1.2.14 and sndio 1.10.0 | `midi_ctrl.cpp` uses the ALSA sequencer API `snd_seq_open`. Sndio is a private dependency advertised by the historical FFmpeg metadata. | ALSA `--enable-static --disable-shared`, libm/pthread/rt/dl. Sndio `--enable-static`, with ALSA available for its Linux backend. ALSA configuration, sequencer nodes and audio devices, or a sndio endpoint, belong to runtime. | ALSA `COPYING`, sndio `LICENSE` |
| liblo 0.32 | `osc_ctrl.cpp` uses `lo_server_thread_new` and OSC values. | Debian archive plus pthreads and libm, which `liblo.pc` omits on this host. The listener, peer and network interface are runtime resources. | liblo package copyright |
| FreeType 2.13.3, fontconfig 2.15.0 | SDL_ttf and `text_layer.cpp` use the FreeType 2 and fontconfig APIs. | FreeType uses zlib, bzip2, libpng and Brotli. Fontconfig also uses Expat. Fontconfig rules, font files and caches belong to runtime. | FreeType, fontconfig and codec/parser package copyrights |
| X11 1.8.12 with XCB 1.17.0 | `xgrab_layer.cpp` and `xscreensaver_layer.cpp` use Xlib and `XGetImage`. | Debian X11 archive plus XCB, Xau, Xdmcp and pthreads. Capture needs a display, and the screensaver layer needs its selected child executable. The X11 factory limitation is recorded in the capability ledger. | X client package copyrights |
| Cairo 1.18.4 with Pixman 0.44.0 | `cairo_layer.cpp` uses Cairo image surfaces and `cairo_create`. | Meson static archives, with the Xlib, XCB, fontconfig, FreeType, PNG and zlib backends selected. Cairo feature detection uses the private XCB/Xau/Xdmcp metadata with `prefer_static=true`. The in-memory drawing probe has no display dependency. | Cairo `COPYING`, `COPYING-LGPL-2.1` and `COPYING-MPL-1.1`, Pixman `COPYING` |
| libgd 2.3.3 | `screen.cpp` uses `gdImageCreateTrueColor`, `gdImageSetPixel` and `gdImagePng` for screenshots. | The tested PNG call graph in the Debian archive uses libpng, zlib and libm. Full `gdlib.pc` metadata also lists JPEG, Xpm, TIFF, WebP, HEIF and AVIF support. The latter two closures are outside the staged PNG SDK and remain available via the compatibility package. | libgd and codec package copyrights |
| V4L2 Linux UAPI, `linux-libc-dev` 6.12.116-1 | `cam_layer.cpp` builds with `linux/videodev2.h` and uses the kernel's `VIDIOC_*` ioctls. | The header SHA-256 on this reference host is `81d95175596837bfd89a0645e2970dbddcb31f328cbdca4ebe88b3daecd19504`. The capture path uses the glibc ioctl interface directly. The fixture checks the ioctl ABI with `/dev/null`. Live capture uses a camera and driver. | Linux UAPI header notices and package copyright |
| frei0r and FreeFrame directory loaders | Bundled `src/include/frei0r.h` and `freeframe.h` pin the C ABI used by `frei0r.cpp` and `freeframe.cpp`. The loader resolves `f0r_init` or `plugMain` at runtime. | Compatibility uses libdl and external DSOs, including the pinned Brightness fixture in the [replay](../tests/show-replay/README.md). The selected built-in frei0r provider is a later integration. Plugin binaries, directories and per-effect dependencies are runtime resources. | ABI header notices, plugin-specific notices in their sources |

Debian SDL2's static metadata names ALSA, PulseAudio, samplerate, X11/Xext/
Xcursor/Xi/Xfixes/Xrandr/Xss, DRM/GBM, Wayland EGL/client/cursor, libxkbcommon,
libdecor and the system math/thread/loader libraries. Those platform libraries
also appear as ELF dependencies of the Debian SDL2 runtime. The wrapper probe
resolves `libSDL.a` at link time. Its SDL2 closure comes from the separate
runtime package and requires its own static backend integration.

The bundled source pins are from commit
`57bdc99c75e16b461ae4d094126ba386c60d99ba`. These SHA-256 values cover
`git archive <commit> <path>`, including repository patches.

| Path | SHA-256 | Alternative |
| --- | --- | --- |
| `lib/ccvt` | `a5d7b686256b96c3ef7c726406e6e74ea5ba3b88c7c45da76f69bf1558efe9bb` | No parallel system provider |
| `lib/slw` | `20b82fc2d5453d9996d9d7940282831ef61fa01c8984b0e7f5f913062c4af93a` | No system slw, uses system S-Lang |
| `lib/javascript` | `c075ed9fb3f25d2d4be7333da823d17adc6aee35900b7c505c598d01f0fb8a69` | Old JSAPI, with configure's system probes retained |
| `lib/sdl_gfx` | `dbcb7cad243c59683819cb12d82c94043dd7ffb81dc9d6531a81aa57fbf0d903` | System SDL_gfx 2.0.25 preferred |
| `lib/sdl_ttf` | `088559b857464d82e5538a0913ec278a1aabdf516f0291361d5f4df41a627276` | System SDL_ttf 2.0.11 preferred |
| `lib/flash` | `52c8ec0efcbdd68820ca5ee3ed55faa60e88f6272483d929c329f8fb3e707805` | Optional bundled library only |

## Other frontend and opt-in dependencies

Candidate source checksums identify the corresponding Debian source archives
from the host's package index. Each optional backend needs an explicit build
selection and an archive check using its existing options and probes.

| Consumer and selected ABI | Candidate pin or availability | Link and runtime requirements |
| --- | --- | --- |
| Qt frontend, `qt/Qfreej.pro`, Qt Core/Gui/Widgets with qmake and moc | Debian Qt 5.15.15+dfsg-6+deb13u1. The source `qtbase-opensource-src_5.15.15+dfsg.orig.tar.xz` is `0710566cfa9d5d3b14f889cd60e02debd92bae2b831144460b1f952b096b7878`. Licenses in Qt source `LICENSE.*` and package copyrights. | The project names local FreeJ/SpiderMonkey archives and a separate native link list, including CWiid. Debian provides shared Core/Gui/Widgets and platform plugins. The archive equivalents and statically registered platform integration are open for the Linux GUI. XCB/offscreen plugins, display availability, fonts and Qt's own codec/font/system closure belong to the chosen Qt configuration. |
| CWiid controller, `cwiid_open` | 0.6.91, `a0d872536d4a63f3b80771cd4ba7df0be334fe634fb7aadda2b70b1c559221ec`. | Optional pkg-config or library/header detection. Archive selection is open. Runtime uses libbluetooth, the Bluetooth stack and a paired Wii remote. |
| AAlib `aa_autoinit`/console display | 1.4p5, `fbddda9230cf6ee2a4f5706b4b11e2190ae45f5eda1f0409dc4f99b35e0a70ee`. | Optional header/library detection. Archive selection is open. Runtime uses a terminal or X11 driver and its associated libraries. The default display uses SDL. |
| GL/GLU experimental screens, `gluNewQuadric` via `--enable-opengl` | GLU 9.0.2, `24effdfb952453cc00e275e1c82ca9787506aba0282145fff054498e60e19a65`. | Archive selection is open. Runtime needs an OpenGL ABI provider and compatible display/driver. The default display uses SDL software rendering. |
| Alternate cameras, libunicap `unicap_open` | `--enable-unicap` selects a compatible SDK. The host's source index has no candidate. | Static flags and archive pin are open. Capture device and drivers match the selected unicap backend. The default Linux camera implementation is V4L2. |
| Alternate cameras, legacy OpenCV C API `cvCaptureFromCAM` | `--enable-opencv` probes `opencv`. Debian's 4.10.0+dfsg source `4ca619efbad524c550771ab110332fc81293fb720bd03980fe58bc15f9ca3be8` is an inventory candidate. Compatibility with the legacy package name and capture API is unverified. | No static flags or archive pin established. Requires the selected OpenCV video I/O backend and a device. Linux V4L2 remains selectable. |
| SWF v3, `FlashNew` | `--enable-flash` selects the bundled 0.4.12 pin. GPL source notices are in `lib/flash`. | Conditional `libflash.la` plus its C++/math inputs. Show supplies the SWF file. |
| Dmalloc instrumentation | `--with-dmalloc`, candidate 5.5.2 source `d3be5c6eec24950cb3bd67dbfbcdf036f1278fae5fd78655ef8cdf9e911e428a`. | `dmalloc.h` and `dmallocthcxx` when selected. No archive selected in the reference. |
| Cocoa/QuickTime and historical bindings | Xcode project and sources in the pinned checkout. | No Linux link inputs. The Xcode route needs a matching macOS SDK, frameworks and runtime. The capability ledger records retired build targets and retained examples. |

Use the optional SDK's source notices and Debian package copyright records for
any selection. The optional entries have no local archive recipe, patches or
redistribution bundle unless noted above.

## Reproduce the archive and compatibility checks

Build-only tools on the recorded host were GCC/G++ 12.4.0-5 from the historical
SDK, Autoconf 2.72, Automake 1.17, Libtool 2.5.4, binutils 2.44, Make 4.4.1,
CMake 3.31.6, Meson 1.7.0, Ninja 1.12.1, pkgconf 1.8.1, Python 3.13,
Perl 5.40, dpkg-deb and curl. Ccache is used when available. qmake and moc
are build tools when Qt is selected. Executables still link target archives
for the GCC, C++ and glibc 2.41 runtimes and glibc's math, thread, rt and dl
interfaces. Development headers and pkg-config metadata are build inputs.
The X server, SDL2 runtime library, JACK server, device drivers, plugins,
fonts and show assets are separate.

Run these commands from the checkout with a fresh scratch path that contains
no spaces. The tools and compatibility development packages are listed in the
[reference recipe](legacy-reference.md).

```sh
work=$(mktemp -d)
python3 tests/legacy-linux-sdk.py "$work/reference"
python3 tests/native-linux-archives.py "$work/archives" --reference-sdk "$work/reference"
```

Use the reference recipe's `tests/freej-build-test.sh --enable-static-mozjs`
to build, run `make check`, and stage the compatibility binary. Take
`compat_build` from the printed artifact directory's `build` subdirectory.
Use the running X11 or Xvfb display for the archive checks.

```sh
python3 tests/check-native-linux-archives.py "$work/checks" \
    --reference-sdk "$work/reference" --archive-sdk "$work/archives" \
    --compat-build "$compat_build" --x11-display "$DISPLAY"
```

`checks.json` retains the pkg-config results, exact compile/link commands,
ELF checks, linker maps, contributing archive identities and runtime results.
`environment.json` records the selected metadata directories and display.
The three missing-archive controls make a packaged `.so` available, confirm
that a normal link selects it, exercise the static link's failure, and then
restore the pinned archive and relink. SDL's runtime consumer uses the
archived wrapper with the normal glibc loader and identifies the SDL2 file
opened for dummy and X11. The ELF archive check and runtime loader check
measure different parts of that dependency.

For a dependency upgrade, build twice in fresh prefixes, changing only that
family's source and checksum. Run the same consumer on both, compare maps
and runtime results, then run the affected show fixture against both
compatible builds. Keep the preceding lock and build evidence for rollback.
For a pin restoration, `--archive-cache` accepts previously retained downloads
and verifies every byte against the current lock before extraction. A second
consumer run can limit execution to one family with `--only`.
The recipe uses SDL 1.2.68's release timestamp for `SOURCE_DATE_EPOCH` so
its embedded build banner is repeatable.
