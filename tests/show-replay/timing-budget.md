# Linux software timing and stage checks

Use the [legacy show replay](README.md) to obtain the FreeJ reference, the
Brightness plugin and the pulse client. Build a separate timing observer
against the same out-of-tree build:

```sh
ccache "$sdk/gcc12/usr/bin/g++-12" -std=c++11 -O2 -Wall -Wextra -Werror -fPIC -shared \
    -isystem src/include -isystem "$build" -isystem lib/javascript \
    -isystem "$build/lib/javascript" -isystem "$sdk/debian/usr/include" \
    -DJS_GCMETER $(pkg-config --cflags sdl3) tests/show-replay/timing-observer.cpp \
    -o "$scratch/timing-observer.so" -ldl
```

Start the private 48 kHz / 2048-frame JACK dummy server from the replay recipe.
Use a fresh Xvfb server with `-screen 0 160x120x24 -nolisten tcp` and set
`DISPLAY` to its reported display. Run the software reference from the source
root with a new output directory:

```sh
python3 tests/show-replay/timing.py --freej "$stage/bin/freej" \
    --plugin "$scratch/brightness.so" --observer "$scratch/timing-observer.so" \
    --pulse "$scratch/pulse" --jack-server freej-replay --output "$scratch/timing"
```

This run uses an 80×60 SDL viewport at 25 FPS. A frame is 40 ms and a
2048-sample period at 48 kHz is 42.667 ms. Each short run starts a new FreeJ
process, discards 25 warm-up presentations and measures the following 75.
The fixture runs each workload five times with one run active at a time:

- Idle shows a cleared viewport.
- Filtered plays two image layers and changes the overlay position and
  Brightness setting.
- OSC runs the same filtered show and sends eight bursts of four loopback
  packets after warm-up.
- Capture and record runs the filtered show with a JACK PCM pulse in every
  period and active Theora recording. `ffprobe` verifies the resulting video.
  A separate 60-second capture and record run measures clock drift.

Use `--workload osc` to run just the OSC workload. The current receiver
should run one JS callback per packet. Add `--expect-legacy-osc` when measuring
the historical binary, which attempted dispatch with no callback. Add
`--osc-stress` for one 128-packet burst per run. The stress workload has a
different offered load and its measurements use their own observed range.

## Measurement definitions

All trace timestamps use Linux `CLOCK_MONOTONIC`. The OSC sender, receiver and
controller events must agree in order. The latter two correspond to entry
into liblo's FreeJ handler and entry into the JavaScript dispatch call.
The dispatch record includes the number of callbacks that actually ran.
The sender-to-submission observation pairs each send with the first software
submission following its dispatch, including any time queued across frames.
JACK source and capture events pair by `jack_last_frame_time` with the pulse
at sample offset zero. The observer records JACK xrun notifications for each
client and preserves missing sample periods separately. The SDL trace dates
each successful `SDL_UpdateWindowSurface` call at entry, without using `FPS::get()`.

`summary.json` holds cross-run ranges and upper limits. Each run directory
keeps its raw trace, settings, log and metrics. Percentiles use nearest rank.
Latency jitter is p95 minus p50. An interval is late after 44 ms (110% of a
frame). Rounded frame intervals estimate missed software slots. Physical
dropped frames require observation of display output. Frame drift compares
elapsed submission time with the nominal span for the same count. Audio
clock drift compares the elapsed monotonic time between first and last input
pulses with the 48 kHz span of their JACK sample positions.

The fixed software limits use the worst observed run plus twice the
run-to-run range, with a minimum 2 ms margin on time metrics. Limits round up
to whole milliseconds or counts. Five replicates characterize observed
variance on this reference and do not establish a population confidence bound.
Apply the limits only to repeated runs with the same configuration.
`make check` exercises event order and clock calculations without SDL, JACK
or wall-clock thresholds.

## Recorded software reference

The reference ran on a Ryzen 7 PRO 8700GE host with Debian Linux
6.12.116+deb13-amd64. The assertion-enabled build used GCC/G++ 12.4.0-5 and
`-O2 -g`. SDL 1.2.68 submitted to Xvfb 21.1.16 at 160×120×24.
JACK1 0.126.0 ran its dummy backend at 48 kHz with 2048
frames per period. The pinned Brightness plugin was the only loaded plugin.

Each cell gives the median of five per-run percentiles followed by the
fixed upper limit, in ms. The drift column uses the greatest absolute
drift observed followed by its upper limit over the 74 measured intervals.

| Workload | Interval p50 | Interval p95 | Interval p99 | Absolute drift |
| --- | ---: | ---: | ---: | ---: |
| Idle | 39.946 / 42 | 40.389 / 43 | 41.048 / 45 | 2.876 / 9 |
| Filtered playback | 39.940 / 43 | 40.349 / 43 | 40.607 / 43 | 4.479 / 11 |
| OSC bursts | 39.977 / 43 | 40.325 / 43 | 40.574 / 43 | 2.039 / 6 |
| Capture and record | 39.997 / 43 | 40.318 / 43 | 40.536 / 43 | 1.907 / 6 |

| Software segment | p50 | p95 | p99 | Jitter |
| --- | ---: | ---: | ---: | ---: |
| OSC send to receipt | 0.106 / 3 | 0.177 / 3 | 0.221 / 3 | 0.071 / 3 |
| OSC receipt to dispatch | 17.015 / 24 | 37.173 / 50 | 37.176 / 50 | 20.369 / 26 |
| OSC dispatch to next submission | 0.039 / 3 | 0.094 / 3 | 0.099 / 3 | 0.055 / 3 |
| JACK pulse source to capture | 0.070 / 3 | 0.099 / 3 | 0.201 / 21 | 0.028 / 3 |
| JACK capture to next submission | 19.355 / 27 | 37.786 / 44 | 39.795 / 42 | 18.566 / 26 |

Across all short runs, there were zero late intervals, zero estimated
missed software slots, zero missing capture periods and zero reported JACK
xruns from either client. The software limits for these counts are zero for
the same run length. Each OSC run paired 32 packets. Each capture run saw
69–70 pulses and decoded 53 recorded video frames. Maximum absolute
short-run JACK drift was 0.287 ms with a 3 ms upper limit.

The long capture run submitted 1,500 frames and saw 1,406 pulses. Frame
interval p50/p95/p99 was 40.001/40.420/40.705 ms. Submission drift was
+7.843 ms over 59.968 seconds and the JACK sample clock drift was
−0.673 ms. The run reported zero late intervals, missed software slots,
missing sample periods or xruns and decoded 1,490 video frames. Use the
short-run limits at their defined duration. One long run gives a drift
observation without establishing its repeat variance.

## Outstanding stage measurements

This host has no accessible physical display session or DRM device, ALSA
input, camera or optical sensor. Xvfb and software JACK establish software
submission and transport timing. Present-to-photon, physical audio
loopback, end-to-end jitter and physical frame drops remain unmeasured.

On the stage rig, record the monitor's mode and refresh rate, audio
interface, sample rate and period, loopback wiring, camera or sensor
sampling rate and the timestamp uncertainty. Record the electrical input
pulse and a photodiode at the flash region on a common acquisition clock,
or film the flash and an input-triggered reference light together. Retain
the raw capture and the common synchronization marker alongside the
software trace for at least five warmed runs per workload. Use the
alternating overlay as an independent visibility check.

The historical OSC receiver reached dispatch with no JS callback. Its JACK
input received the known sample while all FFT bands stayed silent. Use
`--expect-legacy-audio` to replay that outcome. The capture workload still
measures the next submission and preserves its original comparison limits.
With the current observer, `--require-callback-timing` also checks the
longest measured JACK process callback against its period.

For the software response test, supply the same JACK server, Xvfb display,
current timing observer, plugin and current pulse binary:

```sh
python3 tests/show-replay/audio-response.py --freej "$stage/bin/freej" \
    --plugin "$scratch/brightness.so" --observer "$scratch/timing-observer.so" \
    --pulse "$scratch/pulse" --jack-server freej-replay --output "$scratch/response"
```

Every twelfth period carries a unit-position pulse of amplitude 0.5. Native
window sample positions pair the input pulse with the 16th band's response.
The script shows an overlay while that band exceeds 5. The observer reads
the on/off pixel at (20,16) before SDL submits the surface. Each matched
row records source, capture, FFT and first distinct marker onset timestamps.
The fixture records Theora/Vorbis and dispatches 32 OSC packets in each
of five short runs and one 60-second run. It reports capture-to-pixel and
source-to-pixel p50/p95/p99, callback worst time, xruns, clock drift and
missing periods. These pixels date software submission. Optical measurement
requires the stage equipment above.
