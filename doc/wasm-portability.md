# WebAssembly portability of one FreeJ frame

FreeJ's shared frame tick runs under a host-supplied clock. A test host runs
that tick with an in-memory source and sink and the statically linked
`Invert0r` effect on Linux and WebAssembly. The native application still
assembles the tick with the SDL-facing `Context`, `ViewPort` and SpiderMonkey.
A full application build encounters the platform selection in `configure.ac`
and then the desktop SDK requirements.

## What runs today

The CLI calls `Context::cafudda`, which pumps SDL
events when enabled, passes a `FrameHost` to `freej_engine::step_frame`, then
updates and delays the native scheduler. `src/engine/frame_tick.cpp` owns the
tick order. It dispatches input, then clears, resizes, blits and presents each
viewport, then collects garbage and samples completion time.
`src/engine/frame_step.cpp` adapts `Context` to that sequence and keeps the
native conditions for dispatch, clear, resize and JS GC. The native presentation callback calls
`FrameAccess::capture` and `ViewPort::show`. Layer workers feed, filter and
publish independently in `Layer::thread_loop`. The step consumes the published
buffer and never drives those workers. `ViewPort::blit_layers` walks active,
opened layers from back to front and holds their published frame leases through
the blit.

The memory mode of `tests/show-replay/frame-step-test.cpp` calls that same
compiled step. It decodes a 3×2 TGA through `ImageLayer`, runs
`Layer::do_filters` with the fixture's green-channel filter, and publishes the
result synchronously. It applies a position/crop change as input and checks
all 15 32-bit pixels and coordinate addresses on a 5×3 `SoftScreen`. Four
ticks check positive placement twice and negative-edge cropping twice.
The expected trace is `input → present → time` on every tick, including pause
and a slow tick. The fake completion clock returns 100 seconds plus 40,000,
80,000, 205,000 and 245,000 microseconds. The full test also exercises the
native wrapper with SDL dummy video, resize, optional controller polling and
three start/stop runs. The static bundle is linked into the static fixture
through `Plugger::refresh`. The image/filter assertion in this fixture uses
its own filter, so this run gives no per-effect pixel result for the bundle.

## Dependency ownership

| Boundary and owner | Classification and first blocker | Minimum path for the probe |
| --- | --- | --- |
| Frame order and composition, `engine/frame_tick.cpp`, `engine/frame_step.cpp`, `screen.cpp`, `layer.cpp` | Portable tick, native adapters. `frame_step.cpp` includes `context.h` and `jsparser.h`. `context.h` includes SDL and reaches `screen.h` through `layer.h`. The object directly requires `ViewPort::handle_resize`, `ViewPort::blit_layers` and `JsParser::gc`. | Compile the common tick with the test-only memory source, scalar blit and sink. The native wrapper supplies the same operations through `Context`. |
| Frame layout and leases, `engine/frame_storage.cpp`, `surface_lock.h` | Geometry, row copies and ownership are portable operations. The current object also implements `FrameAccess` and pulls in native layer and screen headers. The store publishes using atomic shared-pointer operations and samples a steady clock. | Use a single-thread owner and explicit dimensions, byte stride and byte layout. Keep input and output live through the synchronous tick. Exclude native display capture from the probe. |
| Workers and locking, `layer.cpp`, `jsync.cpp`, `linklist.cpp`, `closure.cpp`, `context.cpp` | Native worker admission and lifecycle. `Layer` inherits `JSyncThread`; `JSyncThread::start` and `Context::start_threaded` require `pthread_create`, while worker teardown uses `pthread_join`. Link lists and the viewport take locks even in memory mode. | One owner performs the fixture's feed, effect and publish synchronously. No create/join, blocking waits or shared-memory requirement in the test host. |
| x86 copy and conversion, `fastmemcpy.cpp`, `cpu_accel.cpp`, `linear_blits.cpp`, `lib/ccvt` | Portable byte copy with native acceleration. `--disable-cpuflags` clears the optional MMX/SSE macros, but native `ARCH_X86` still selects inline assembly and `rdtsc` in copy selection. `libccvt` carries guarded 32-bit MMX conversion assembly alongside portable C. | Use the default standard byte copy used by the RGB blit, omit CPU probing and timing calibration, and omit the conversion library for already uploaded 32-bit pixels. |
| Pixel bytes, `screen.h`, `soft_screen.cpp`, `frei0r.cpp` | Portable byte payload with native conventions. Channel masks and lane constants depend on `SDL_BYTEORDER` and the Cocoa branch. frei0r descriptors distinguish BGRA, RGBA and PACKED32. The existing filter path passes physical byte lanes through, which the frei0r host replay checks. | Pin the byte layout of both input and output, keep the alpha lane, honor stride, and hash bytes in row order. Choose an effect that works on the fixture's declared layout and preserve the native mapping. |
| Effect discovery and loading, `plugger.cpp`, `frei0r.cpp`, `freeframe.cpp`, `plugin.cpp`, `static_loader.cpp` | Static descriptor registry is portable in principle after recompilation. The compatibility provider reaches `scandir`, `dlopen` and `dlsym`. The native static link includes legacy loader objects under whole-archive and uses GNU ELF `--wrap` to guard loading. | Recompile the selected frei0r kernels and descriptor registry for the probe target. Resolve descriptors and callbacks statically from the same bundle pin. Leave plugin-directory compatibility and Linux loader guards in their native targets. |
| Directories and show assets, `Plugger`, `jutils.cpp`, `ImageLayer`, console and script bindings | Host I/O. `dircheck` uses `opendir`. Compatibility resolves installed and home plugin directories. Script and console discovery also use `scandir`. `ImageLayer::open` calls `IMG_Load` on a file path even for the current memory sink. | Inject decoded fixture bytes directly. Any future host asset loading would be an explicit host operation with known paths or preloaded data. |
| OSC and network output, `osc_ctrl.cpp`, liblo, `shouter.cpp`, libshout | Native transport is outside the probe. `lo_server_thread_new` creates the liblo receiver and `lo_server_thread_start` runs it. The receiver uses sockets, while streaming uses its own network client. Native OSC copies typed values and dispatches the FIFO on the frame owner. | The host queues ordered, owned control values and drains a bounded batch during input dispatch. A probe can inject the fixture's position changes directly. |
| Embedded JS, `jsparser.cpp` and `lib/javascript` | Legacy interpreter and bindings outside the probe. The native adapter calls `JsParser::gc`, which invokes `JS_GC` on the frame owner. `Context::init` creates the runtime with native bindings when configured. | The test host's cleanup operation is a no-op. The common tick keeps its position after presentation. |
| Clocks and pacing, `context.cpp`, `fps.cpp`, `FrameStore`, `Layer` | Host clock and scheduler. The wrappers use `gettimeofday` and `nanosleep`. `FrameStore::publish` separately uses `steady_clock`; the filter chain currently passes the worker's `fps.fps` into `FilterInstance::process`. The existing `FrameHost::now` is a completion sample. | Supply a deterministic monotonic time at the test boundary and use an explicit unit for any effect time. Advance one step and return. Keep acquisition time, completion sample and pacing as distinct operations. |
| JACK and audio analysis, `audio_jack.cpp`, `audio_collector.cpp`, FFTW | JACK is native. `jack_client_open` requires a reachable daemon, and the callback writes sample position, rate and PCM into the capture handoff. The existing `AudioCollector::SubmitSamples` supports synchronous injected float PCM; the same translation unit also contains the worker path and FFTW calls. | Give the host a bounded PCM block with count, sample rate and start-frame position. For this video fixture pass zero audio blocks. A future audio probe would use synchronous analysis behind its own dependency boundary. |
| SDL, other devices and media, `sdl_screen.cpp`, `MEDIA_SOURCES` | Native adapters. SDL headers, window/event calls, image decoding, capture, FFmpeg, fonts and recording stay in the existing profile assembly. | Use a memory sink and an in-memory source. Leave frontend input/presentation, devices and codecs with their existing owners. |

The host owns the tick time, admitted control queue and sample blocks. For an
asynchronous host, finish reading borrowed inputs or copy them before returning
from the tick. Pass rendered bytes back with their dimensions, stride, layout
and validity interval.

Emscripten documents how a host callback yields between iterations and how
preloaded or virtual files can supply assets. The test itself can run with a
direct function call and injected bytes.
[Browser main loop](https://emscripten.org/docs/porting/emscripten-runtime-environment.html#browser-main-loop)
[File systems](https://emscripten.org/docs/porting/emscripten-runtime-environment.html#file-systems)

## Native build and audit

`src/Makefile.am` compiles the engine, controller, display, media and JS source
groups into the same `libfreej`. The movie decoder is a convenience library,
while ccvt and the S-Lang widgets are folded into the native library. In the
static profile, `m4/freej-static.m4` links the entire `libfreej.a` plus bundled
SpiderMonkey and SDK archives. The link map attributes SDL and its X11 stack to
display/input, SDL_image/SDL_ttf/SDL_gfx and their codec/font dependencies to
image/geometry/text, FFmpeg to movies and conversion, S-Lang to the console,
JACK/ALSA/sndio/FFTW/libsamplerate to audio, liblo to OSC, and the
Ogg/Vorbis/Theora/libshout/TLS family to recording and streaming. The static
effect archive contributes its registry and the descriptors and implementation
objects for `brightness`, `invert0r` and `onecol0r`. The SDK's ELF archives
are Linux inputs. The test probe rebuilds the selected effects for each target
ABI. The compatibility fixture depends directly on `libfreej.so.0` and the
native runtime, with adapter dependencies supplied through the shared library.

From a clean out-of-tree build, use the dependency environment for the chosen
profile from `tests/native-ci-run.sh` and the archived SDK inputs described in
[native-dependencies.md](native-dependencies.md). Bootstrap a fresh source
export with `./autogen.sh`, then build the test and save its actual link map.

```sh
profile_options=(--with-native-profile=compatibility "--prefix=$prefix")
export CFLAGS='-O2 -DNDEBUG -fno-tree-vectorize'
export CXXFLAGS="$CFLAGS"
"$source/configure" --enable-debug --disable-cpuflags --disable-lubrication \
  --enable-static-mozjs "${profile_options[@]}"
make -j4 V=1
make -C tests V=1 "LDFLAGS=$LDFLAGS -Wl,-Map,$build/frame-step.map" frame-step-test
timeout 30 tests/frame-step-test --memory-only
timeout 30 tests/frame-step-test
nm -uC src/engine/frame_step.o
nm -uC src/engine/frame_storage.o
# Static profile executable. In compatibility, inspect tests/.libs/frame-step-test.
readelf -hW -lW -dW tests/frame-step-test
nm -uC tests/frame-step-test
```

For the static run, select `--with-native-profile=static` and add the
`--with-native-sdk` and `--with-frei0r-bundle` paths.

At the source-audit baseline (`933762a78fda`), the compatibility and static
fixture builds both passed with these flags.
Their memory mode executed four ticks with the same asserted trace and pixels,
and their full mode passed both the memory and wrapper cases. The static
executable has no ELF interpreter, shared-library dependency or undefined
symbols. Its link closure still contains the native owners listed above.
The frame-step compiler dependencies include SDL, pthread and SpiderMonkey
headers. The undefined references in `frame_step.o` are the three native
engine/JS references and the standard library's `std::__throw_bad_function_call`.

A second static build disabled `ARCH_X86` in its private generated `config.h`.
The FreeJ memory fixture retained the same trace and pixels, selected standard
`memcpy`, and its `fastmemcpy.o` disassembly showed no `rdtsc` or `cpuid`.
This isolates the FreeJ copy path on a native x86-64 host. The linked native
SDKs and the compiler's default target ABI are still x86-64.

The first native adapter blocker can be reproduced independently of any runtime. With the
configured build directory available for generated headers and the SDL SDK's
include flags omitted, compiling `src/engine/frame_step.cpp` fails at
`src/include/screen.h` with `SDL.h: No such file or directory` on the
verification host. A native link taking the address of the actual
`step_frame` from `frame_step.o` and using only the C++ runtime fails on
`ViewPort::handle_resize`, `ViewPort::blit_layers` and `JsParser::gc`.
The extracted WebAssembly `frame_tick.o` requires only the linker-resolved
`__indirect_function_table` for virtual calls.

## Executed WebAssembly probe

On Linux x86-64, obtain the frei0r source archive from `tests/native-ci.lock`
and run the following. The script checks the frei0r checksum, the emsdk commit
and release mapping, and the SHA-256 of SDK and Node archives in
`tests/wasm-emsdk.lock`. It builds from a fresh private directory. An optional
third argument is the static profile's built `tests/frame-step-test` binary.

```sh
bash tests/wasm-frame-probe.sh "$frei0r_archive" "$new_work_directory"
bash tests/wasm-frame-probe.sh "$frei0r_archive" "$another_new_work_directory" \
  "$static_build/tests/frame-step-test"
```

The full native reference uses `ImageLayer`, the existing static effect
provider and `Layer::do_filters`, then composites through `SoftScreen`.
The native and WebAssembly test hosts call the same compiled FreeJ tick and
resolve `invert0r` in the pinned static descriptor bundle. They inject the same
8×8 BGRA source and apply the same position controls, 5×3 viewport and clock
samples as the existing replay. The 8×8 size and 16-byte-aligned effect
buffers satisfy the frei0r ABI. Each host feeds the same float seconds value
to the effect and records the completion clock separately. The native reference
also verifies that its decoded source bytes match the in-memory input.

Emscripten 4.0.23 and Node 22.16.0 produced four matching frames with 12
ordered events. All 240 BGRA bytes matched the native probe, the native
`Context` reference and the expected bytes. The frame-byte SHA-256 was
`83465dc2b297269a2e7ccc7b343136f6228239af0e9e7106b34e8070e01b7707`.
The link maps identify the shared FreeJ tick object and the target's
frei0r registry and Invert0r objects. The native bundle contains x86-64
objects and the WebAssembly bundle contains wasm32 objects. The module
validated with mutable globals, nontrapping conversions, sign extension and
bulk-memory operations enabled. SIMD and threads were disabled. Its six
function imports serve stdio and the Emscripten C/C++ runtime.

The bounded Actions job runs the native and WebAssembly comparison on
`ubuntu-24.04`. The probe uses the memory host for its render operations.
The tested scope ends at one ordered frame tick and this deterministic
static effect. Native layer lifetimes, codecs, sockets, devices, scripts and
frontends remain with their native adapters.
