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

Run the six checks. Use a fresh evidence directory for each invocation.

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
- OSC sends a ready datagram from the listening port. The peer sends a
  no-argument packet and then `/test` with integer 123, float 4.5 and string
  “hello”. The reference logs these parsed arguments and a dispatch failure
  for each route. A working route must invoke both JS callbacks in order
  with the original arguments. Use `--case osc --require-working-routes` to
  reproduce the missing callbacks as a failing test.
- The JACK source sends a mono window with sample zero at 0.5 and all other
  samples zero. The read-only test observer confirms this input at FreeJ’s
  actual JACK buffer. The legacy collector still reports 25 silent windows
  of 16 bands. Its capture callback setup and input copy are commented out
  in `src/audio_collector.cpp` and `src/audio_jack.cpp`. Use
  `--case audio --require-working-routes` to reproduce the silence as a
  failing test. The evidence includes the expected first impulse bands.
- The example’s `reset(filename)` takes the engine reset path and returns
  from the call, but it never evaluates the target file. The isolated reset
  fixture checks for the reset log, return marker and absent target marker.

The preload observes RGB at SDL submission and one real JACK input block.
Its file I/O and arbitrary SDL or JACK scheduling make these captures
unsuitable for wall-clock latency measurements.

The [software timing fixture and stage checklist](timing-budget.md) use a
separate observer for clocked workloads.
