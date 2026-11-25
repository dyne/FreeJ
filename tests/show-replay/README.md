# Legacy show replay

The fixture uses the assertion-enabled Linux [reference build](../../doc/legacy-reference.md).
It uses an 80×60 SDL viewport, two generated opaque PNGs and the Brightness
frei0r filter. The input scripts are based on `filter_image.js`, `osc_test.js`
and `freej_equalizer.js`. Python 3.9 or newer runs the observer. The staged
FreeJ binary, observer and plugins must all use the same target ABI.

## Build the test artifacts

From the FreeJ source root, set `build` to the configured out-of-tree build,
`stage` to the staged install prefix, `sdk` to the reference SDK, `effects`
to a frei0r checkout at `8bd7873af265493b69b39f5274851bfca59ebefd`, and
`scratch` to a writable directory. These compiler commands use the GCC 12 SDK
and ccache from the reference recipe. Remove the ccache prefix if unavailable.

```sh
ccache "$sdk/gcc12/usr/bin/g++-12" -std=c++11 -Wall -Wextra -Werror -fPIC -shared \
    -isystem src/include -isystem "$build" -isystem lib/javascript \
    -isystem "$build/lib/javascript" -isystem "$sdk/debian/usr/include" \
    -DJS_GCMETER $(sdl-config --cflags) tests/show-replay/observe.cpp \
    -o "$scratch/observe.so" -ldl
ccache "$sdk/gcc12/usr/bin/gcc-12" -O2 -shared -fPIC -I"$effects/include" \
    "$effects/src/filter/brightness/brightness.c" -o "$scratch/brightness.so"
ccache "$sdk/gcc12/usr/bin/g++-12" -std=c++11 -Wall -Wextra -Werror -pthread \
    tests/show-replay/pulse.cpp -o "$scratch/pulse" -ljack
export LD_LIBRARY_PATH="$stage/lib:$sdk/ffmpeg/lib:$sdk/debian/usr/lib/x86_64-linux-gnu"
```

Start a private JACK dummy server at 48 kHz with 2048 frames per period.
For JACK 1, use `jackd -r -n freej-replay -d dummy -r 48000 -p 2048`. If
using a scratch JACK installation, put its `bin` first on `PATH` and its
`lib` first on `LD_LIBRARY_PATH` for the server, pulse client and FreeJ. Set
`JACK_DRIVER_DIR` to its driver directory. JACK 1 needs an existing writable
socket directory with a path short enough for a Unix socket. Set it at build
time with `--with-default-tmpdir` when the packaged default is unavailable.

The collector contract has a separate diagnostic replay. Build
`tests/show-replay/equalizer-observer.cpp` as a shared library with the same
compiler and header paths as `observe.cpp` above. Use the current pulse client
and a shared compatibility build of FreeJ. The fixture starts and stops JACK
dummy servers at 44.1 kHz/2048 and 48 kHz/256.

```sh
python3 tests/show-replay/equalizer.py --freej "$stage/bin/freej" \
    --jackd "$jack/bin/jackd" --observer "$scratch/equalizer-observer.so" \
    --pulse "$scratch/pulse" --output "$scratch/equalizer"
```

Each script call observes a known JACK input period. The `live` and
`live-recording` runs consume the production capture handoff. The `injected`
run withholds the observed period from the live port and submits it on the
script thread. It holds silence for the first seven 256-frame periods, then
checks all 16 bands, harmonic wrapping, and the shipped equalizer frame
handler's `PhaseIncrement` and `ZoomRate` arguments. The recording variant
decodes the Theora/Vorbis streams and measures the known tone in the PCM.
The filter receiver records those arguments. The fixture measures neither
rendering nor callback timing. The device-free `audio-collector-test` in
`make check` covers both constructors, odd and short FFTs, phased tones, gain,
and smoothing across successive windows.

Run the six historical replay checks in a fresh evidence directory.

```sh
python3 tests/show-replay/replay.py --freej "$stage/bin/freej" \
    --observer "$scratch/observe.so" --plugin "$scratch/brightness.so" \
    --pulse "$scratch/pulse" --jack-server freej-replay --output "$scratch/show"
```

The SDL dummy driver is the default. With a running Xvfb and its `DISPLAY`
set, `--video-driver x11` exercises the X11 SDL path. You can select a
single check family with `--case video|osc|audio|reset`. Use
`--case video --expect-reversed-order` as a negative control. It must fail
at the reordered cue. Audio arguments are needed for `all` and `audio`.
`--case osc-example` invokes the unedited `osc_test.js` example's existing
`released_m` handler. It needs UDP port 9696, the port used by that example.

## Expected observations

- The six logical cue times are 0 through 200 ms in 40 ms steps. Eight actual
  presentations are recorded per cue, including the first visible transition.
  The eighth frame is compared byte-for-byte as packed RGB and its dimensions
  and selection are checked. The fresh startups must agree on the six frame
  hashes and selections. A worker can publish its image or parameter update
  between presentations, so the runner stores the actual transition tick as
  an observation. Hide, show and reorder must take effect on the very next
  presentation. Reload includes the same script and recreates the layers
  after stopping their predecessors. Startup-script `quit()` returns CLI
  status 1 in this version.
- OSC sends a ready datagram from the listening port. The peer sends empty and
  mixed-signature messages, including `/test` with integer 123, float 4.5 and
  string “hello”. It checks callback order, values, case-insensitive paths and
  diagnostics for unknown paths and wrong signatures. Lifecycle exercises
  start with 128 admitted packets and check rendering, FIFO drain across
  stop/restart, self-removal, reset and quit. Use `--expect-legacy-osc` with the
  historical binary to reproduce the old dispatch failures.
- The JACK source sends a mono window with sample zero at 0.5 and all other
  samples zero. The observer confirms the input at FreeJ’s JACK port. The
  replay compares the first responsive window with the analytical impulse
  bands. Use `--expect-legacy-audio` to check the historical build's 25
  silent windows.
- The example’s `reset(filename)` takes the engine reset path and returns
  from the call, but it never evaluates the target file. The isolated reset
  fixture checks for the reset log, return marker and absent target marker.

The preload observes RGB at SDL submission and one real JACK input block.
Its file I/O and arbitrary SDL or JACK scheduling make these captures
unsuitable for wall-clock latency measurements.

The [software timing fixture and stage checklist](timing-budget.md) use a
separate observer for clocked workloads.

`make check` runs `movie-replay-test` on `movie-av.mov`. The synthetic clip
has 12 MPEG-4 frames at 20 fps, B-frames, and 0.5 seconds of 48 kHz stereo
PCM starting 100 ms after video. Regenerate it with FFmpeg 7.1.5:

```sh
ffmpeg -hide_banner -loglevel error \
  -f lavfi -i 'testsrc2=size=80x64:rate=20:duration=0.6' \
  -itsoffset 0.1 -f lavfi -i 'aevalsrc=0.125*sin(2*PI*400*t)|0.25*sin(2*PI*600*t):s=48000:d=0.5' \
  -c:v mpeg4 -bf 2 -g 4 -flags +bitexact -c:a pcm_s16le -map_metadata -1 movie-av.mov
```

### frei0r effect contract

The compatibility host looks up user-facing names in directory order and in reverse alphabetical filename order within each directory. `make check` runs `show-replay/test-effects.py` with instrumented frei0r and FreeFrame modules in a temporary user directory. To include the pinned native effects, run the following from the configured tests build directory with its compiler and library environment:

```sh
python3 "$src/tests/show-replay/test-effects.py" --driver ./frei0r-host-test --cli ../src/freej \
    --real "$modules/invert0r.so" "$modules/onecol0r.so"
```

- FILTER and SOURCE share one parameter adapter. Native parameter indices start at one; frei0r indices start at zero. Boolean, number, color and position values are copied during the callback. The legacy host exposes a string parameter's metadata, but its filter and generator interfaces do not implement string transfer. The test plugin aborts if its string callback is invoked.
- A frei0r library initializes before metadata access and deinitializes after its last registration and instance are released. Refresh may retire a registration while its instances continue rendering. The CLI fixture also changes 96 parameters through JavaScript and quits with a live source and filter. MIXER2 and MIXER3 are recognized and excluded from the single-input lists. A PACKED32 source has no declared channel order and is rejected.
- The host supplies tightly packed four-byte pixels and the layer's dimensions to the plugin. It preserves the historical physical byte order, including when a filter declares RGBA and receives a BGRA layer. There is no implicit red/blue swap. Buffers passed to the plugin are 16-byte aligned. Construction failure propagates to the filter or generator caller.
- The historical software probe set a 64×48 generator layer to 37 fps. Its attached filter received `37.0` and its source received `0.0` for all 29 observed updates. These values come from `fps.fps` and `fps.get()` respectively. The fixture changes a pixel as a function of the supplied time.

The static profile runs the same instance host with callbacks from the pinned
descriptor bundle. Its registered host test checks malformed descriptors,
failed initialization and construction, unknown effects, and teardown with
live instances. The bundle IDs are internal. Shows use the registered names
`Brightness`, `Invert0r`, and `onecol0r`.

The native CI driver puts the pinned modules in a private compatibility home,
uses an empty home for the static host, and writes `frei0r-frames.bin` from each
profile. The file contains 24 full frames from eight automation cues, in BGRA
order. Each driver checks parameter names, types, and readback. Compare the
artifacts with `cmp`. Actions checks the SHA-256 digests.
