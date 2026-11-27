#!/usr/bin/env python3
"""Exercise the shipped JS equalizer frame handler with native AudioJack."""
import argparse
import array
import json
import math
import os
from pathlib import Path
import select
import subprocess
import sys

from replay import lines_at, owned_process, require, run_freej

here = Path(__file__).resolve().parent
sys.path.insert(0, str(here.parent))
from native_profile_services import pipewire_dummy


def script(rate, calls, recording):
    example = (here.parents[1] / "scripts/javascript/examples/freej_equalizer.js").read_text()
    start = example.index("bang.frame = function()")
    end = example.index("register_controller(bang)", start)
    # Use the actual frame handler with the fixture port and a filter that
    # records its arguments. No image, plugin, render frame or scheduler here.
    return f'''
var audio = new AudioJack("replay_pcm:pulse", 2048, {rate});
set_fps(25);
var recording = {str(recording).lower()};
var encoder;
if (recording) {{
    encoder = new VideoEncoder(50, 64000, 75, 0);
    encoder.add_audio(audio);
    register_encoder(encoder);
    encoder.start_filesave("tone.ogg");
}}
var bang = {{}}, params = {{}};
var vtg = {{set_parameter: function(name, value) {{ params[name] = value; }} }};
{example[start:end]}
for (var i = 0; i < {calls}; ++i) {{
    bang.frame();
    var values = [];
    for (var b = 0; b < 16; ++b) values.push(audio.get_harmonic(b));
    values.push(audio.get_harmonic(-1), audio.get_harmonic(16));
    values.push(params.PhaseIncrement, params.ZoomRate);
    echo("equalizer: values " + values.join(","));
}}
if (recording) {{
    for (var frame = 0; frame < 35; ++frame) run(0);
    encoder.stop_filesave();
}}
echo("equalizer: quit");
quit();
'''


def measure(args, output, env, rate, period, route):
    ticks = 2048 // period + 3
    recording = route == "live-recording"
    (output / "equalizer.js").write_text(script(rate, ticks, recording))
    status, lines = run_freej(args, output, env, "equalizer.js")
    require(status == 1 and "equalizer: quit" in lines, "script did not reach quit")
    captures = [s.split() for s in lines_at(lines, "equalizer: capture ")]
    require(len(captures) == ticks and all(
        int(n) == period and float(first) == 0.5 and
        math.isclose(float(second), 0.25 + 0.25 * math.cos(6 * math.pi / 256), abs_tol=1e-6)
        for n, first, second in captures), "missing or unexpected known input at PipeWire port")
    rows = [list(map(float, row.split(","))) for row in lines_at(lines, "equalizer: values ")]
    windows = [list(map(int, s.split())) for s in lines_at(lines, "equalizer: window ")]
    require(len(windows) == ticks, "missing native window positions")
    require(len(rows) == ticks, "incomplete script trace")
    (output / "trace.json").write_text(json.dumps(rows) + "\n")
    previous = [0.] * 16
    for tick, row in enumerate(rows, 1):
        bins = [0.] * 16
        complete, first, native_period, native_rate = windows[tick - 1]
        if route != "injected":
            require(complete or (tick-1) * period < 2048, "analysis worker did not publish a complete window")
            if complete:
                require(native_period == period and native_rate == rate, "mismatched device clock")
        if (route == "injected" and tick * period >= 2048) or complete:
            bins[0], bins[8] = 512., 256.
        bands = [(past * 1.2 + real**2 * 0.025 / 1.2) / 2 for past, real in zip(previous, bins)]
        phase = (bands[8] + bands[1]) * 3 / 256
        want = bands + [bands[15], bands[0], phase if phase >= 1 else 0., max(1., bands[0] * 3 / 64)]
        require(len(row) == len(want) and all(math.isclose(a, b, abs_tol=1e-5, rel_tol=1e-5)
                for a, b in zip(row, want)), f"{rate}/{period} {route} frame {tick}: {row} != {want}")
        previous = bands
    if recording:
        verify_recording(output, rate)
    print(f"PASS {rate}/{period} {route}: {len(rows)} PipeWire periods, {len(rows) * 20} values")


def verify_recording(output, rate):
    codecs = subprocess.check_output(["ffprobe", "-v", "error", "-show_entries",
        "stream=codec_name,sample_rate,channels", "-of", "json", "tone.ogg"], cwd=output)
    (output / "codecs.json").write_bytes(codecs)
    streams = json.loads(codecs)["streams"]
    require(any(s.get("codec_name") == "theora" for s in streams) and any(
        s.get("codec_name") == "vorbis" and int(s["sample_rate"]) == rate and s["channels"] == 1
        for s in streams), "recording lacks matching Theora/Vorbis streams")
    pcm = array.array("f")
    pcm.frombytes(subprocess.check_output(["ffmpeg", "-v", "error", "-i", "tone.ogg", "-map", "0:a:0",
                                          "-f", "f32le", "-"], cwd=output))
    require(len(pcm) >= rate // 2, "recording has less than half a second of decoded audio")
    samples = pcm[len(pcm)//2:len(pcm)//2+2048]
    def amplitude(bin_number):
        angle = 2 * math.pi * bin_number / 2048
        return math.hypot(sum(x * math.cos(i*angle) for i,x in enumerate(samples)),
                          sum(x * math.sin(i*angle) for i,x in enumerate(samples))) / 1024
    tone = amplitude(24)
    require(0.17 <= tone <= 0.33 and max(amplitude(23), amplitude(25)) < 0.04,
            f"decoded Vorbis tone amplitude {tone}")
    (output / "decoded.json").write_text(json.dumps({"samples": len(pcm), "tone_amplitude": tone}))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("freej", "pipewire", "observer", "pulse", "output"):
        parser.add_argument("--" + name, required=True, type=lambda p: Path(p).resolve())
    args = parser.parse_args()
    args.output.mkdir()
    env = dict(os.environ, LC_ALL="C", SDL_VIDEODRIVER="dummy")
    for rate, period in ((44100, 2048), (48000, 256)):
        directory = args.output / f"{rate}-{period}"
        directory.mkdir()
        with pipewire_dummy(args.pipewire, directory, env, rate, period) as run_env:
            with owned_process([args.pulse, "--equalizer", str(rate), str(period)], directory,
                               run_env, "pulse.log", stdout=subprocess.PIPE) as source:
                require(select.select([source.stdout], [], [], 5)[0] and
                        source.stdout.readline().strip() == b"replay_pcm:pulse", "pulse startup failed")
                for route in ("live", "injected", "live-recording"):
                    output = directory / route
                    output.mkdir()
                    home = output / "home"
                    home.mkdir()
                    measure(args, output, dict(run_env, FREEJ_EQUALIZER_ROUTE=route,
                            HOME=str(home), LD_PRELOAD=str(args.observer)), rate, period, route)
                require(source.poll() is None, "pulse sender exited during trace")
            require(source.returncode == 0, "pulse cleanup failed")


if __name__ == "__main__":
    main()
