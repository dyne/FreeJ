#!/usr/bin/env python3
"""Check FreeJ's legacy CLI against a short, independently specified set."""

import argparse
from contextlib import contextmanager
from hashlib import sha256
import json
from math import isclose
import os
from pathlib import Path
import resource
import select
import shutil
import socket
import struct
import subprocess
import time
import zlib


HERE = Path(__file__).resolve().parent
WIDTH, HEIGHT = 80, 60
FRAME_BYTES = WIDTH * HEIGHT * 3
TICKS_PER_CUE = 8
BASE = (32, 96, 160)
OVERLAY = (24, 48, 72)
BRIGHT_OVERLAY = (140, 152, 164)  # x + (256 - x) / 2
CUES = [
    ("loaded", 0, "overlay", OVERLAY),
    ("parameter", 40, "overlay", BRIGHT_OVERLAY),
    ("hidden", 80, "base", None),
    ("shown", 120, "overlay", BRIGHT_OVERLAY),
    ("reordered", 160, "overlay", None),
    ("reloaded", 200, "overlay", OVERLAY),
]
EXPECTED_OSC_ARGS = "123|4.5|hello"
IMPULSE_BINS = (0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 40, 54, 74, 101, 137, 187, 255)


def require(condition, problem):
    if not condition:
        raise RuntimeError(problem)


def png(path, width, height, rgb):
    def chunk(name, data):
        return (struct.pack(">I", len(data)) + name + data +
                struct.pack(">I", zlib.crc32(name + data)))

    row = b"\0" + bytes((*rgb, 255)) * width
    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
                     chunk(b"IDAT", zlib.compress(row * height)) + chunk(b"IEND", b""))


def expected_frame(overlay=None, background=BASE):
    data = bytearray(bytes(background) * (WIDTH * HEIGHT))
    if overlay:
        for y in range(12, 28):
            start = (y * WIDTH + 16) * 3
            data[start:start + 24 * 3] = bytes(overlay) * 24
    return bytes(data)


def json_file(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


@contextmanager
def owned_process(command, directory, env, logfile, **kwargs):
    with (directory / logfile).open("wb") as log:
        process = subprocess.Popen(command, cwd=directory, env=env, stderr=log,
                                   **({"stdout": log} | kwargs))
        try:
            yield process
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=3)
            if process.stdout is not None:
                process.stdout.close()


def fixture(args, name):
    directory = args.output / name
    directory.mkdir()
    home = directory / "home"
    plugins = home / ".freej" / "plugins"
    plugins.mkdir(parents=True)
    shutil.copyfile(args.plugin, plugins / "brightness.so")
    for script in HERE.glob("*.js"):
        shutil.copyfile(script, directory / script.name)
    png(directory / "base.png", WIDTH, HEIGHT, BASE)
    png(directory / "overlay.png", 24, 16, OVERLAY)
    env = dict(os.environ, HOME=str(home), SDL_VIDEODRIVER=args.video_driver, LC_ALL="C")
    env.pop("FREEJ_REPLAY_PROBE", None)
    return directory, env


def run_freej(args, directory, env, script, debug=1, after_start=None):
    command = [str(args.freej), "-c", "-D", str(debug), "-s", f"{WIDTH}x{HEIGHT}", "-j", script]
    start = time.monotonic()
    process = None
    try:
        with owned_process(command, directory, env, "freej.log") as process:
            if after_start:
                after_start()
            process.wait(timeout=10)
    finally:
        json_file(directory / "process.json", {
            "command": command, "cwd": str(directory),
            "exit": process.returncode if process else None,
            "seconds": time.monotonic() - start,
            "environment": {k: env[k] for k in
                            ("HOME", "SDL_VIDEODRIVER", "DISPLAY", "LD_LIBRARY_PATH",
                             "LD_PRELOAD", "JACK_DEFAULT_SERVER", "FREEJ_REPLAY_PROBE") if k in env},
        })
    return process.returncode, (directory / "freej.log").read_text(errors="replace").splitlines()


def lines_at(lines, prefix):
    return [line[len(prefix):] for line in lines if line.startswith(prefix)]


def observe(env, args, mode):
    env["FREEJ_REPLAY_PROBE"] = mode
    inherited = env.get("LD_PRELOAD")
    env["LD_PRELOAD"] = str(args.observer) + (f":{inherited}" if inherited else "")


def video(args, name):
    directory, env = fixture(args, name)
    observe(env, args, "video")
    status, lines = run_freej(args, directory, env, "video.js")
    require(status == 1 and "replay: quit" in lines, f"{name}: CLI did not reach scripted quit")
    require(lines_at(lines, "replay: cue ") == [f"{n} {t}" for n, t, _, _ in CUES],
            f"{name}: unexpected cue order or logical time")
    pixels = (directory / "frames.rgb").read_bytes()
    metadata = (directory / "presentations.txt").read_text().splitlines()
    count = TICKS_PER_CUE * len(CUES)
    require(len(pixels) == count * FRAME_BYTES and len(metadata) == count,
            f"{name}: expected {count} complete presentations")
    frames = [pixels[i:i + FRAME_BYTES] for i in range(0, len(pixels), FRAME_BYTES)]
    canonical, observations = [], []
    old = b"\0" * FRAME_BYTES
    for index, (cue, logical_ms, selected, color) in enumerate(CUES):
        start = index * TICKS_PER_CUE
        end = start + TICKS_PER_CUE
        new = expected_frame(color)
        if cue == "reordered" and args.expect_reversed_order:
            new = expected_frame(BRIGHT_OVERLAY)
        phase = frames[start:end]
        require(phase[-1] == new, f"{name}: {cue}: settled RGB bytes differ")
        first = phase.index(new)
        require(all(frame == new for frame in phase[first:]),
                f"{name}: {cue}: expected frame did not stay visible")
        before = {old}
        if cue in ("loaded", "reloaded"):
            # The two image workers can publish in either order at startup.
            before.update((expected_frame(), expected_frame(OVERLAY, (0, 0, 0)),
                           expected_frame(background=(0, 0, 0))))
        require(all(frame in before for frame in phase[:first]),
                f"{name}: {cue}: unexpected pixels before change")
        require(cue in ("loaded", "parameter", "reloaded") or first == 0,
                f"{name}: {cue}: did not take effect on the next presentation")
        require(all(item == f"{WIDTH} {HEIGHT} {selected}" for item in metadata[start:end]),
                f"{name}: {cue}: wrong frame size or selected layer")
        canonical.append((cue, logical_ms, selected, sha256(new).hexdigest()))
        observations.append({"cue": cue, "first_visible_tick": first,
                             "first_visible_presentation_index": start + first,
                             "canonical_presentation_index": end - 1})
        old = new
    json_file(directory / "observations.json", {
        "canonical": canonical, "observed_ticks": observations,
        "clock": "logical milliseconds; startup and layer-worker tick counts remain observations",
    })
    return canonical


def osc_string(value):
    data = value.encode("ascii") + b"\0"
    return data + bytes((-len(data)) % 4)


def osc_packet(path, signature="", body=b""):
    return osc_string(path) + osc_string("," + signature) + body


def osc(args):
    directory, env = fixture(args, "osc")
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as peer:
        peer.settimeout(5)
        peer.bind(("127.0.0.1", 0))
        # The CLI requires an explicit listening port. Release our reservation
        # before launch, then confirm the received datagram's bound source.
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as reserve:
            reserve.bind(("127.0.0.1", 0))
            port = reserve.getsockname()[1]
        script = directory / "osc.js"
        script.write_text(script.read_text().replace("@SERVER@", str(port))
                          .replace("@PEER@", str(peer.getsockname()[1])))
        wire = [osc_packet("/test/noargs"),
                osc_packet("/test", "ifs", struct.pack(">if", 123, 4.5) + osc_string("hello"))]
        if not args.expect_legacy_osc:
            wire += [osc_packet("/test", "i", struct.pack(">i", 999)),
                     osc_packet("/unknown"),
                     osc_packet("/REORDERED", "sfi", osc_string("abcde") +
                                struct.pack(">fi", -1.25, -2147483648)),
                     osc_packet("/test", "ifs", struct.pack(">if", 2147483647, 0.5) +
                                osc_string(""))]

        def send():
            packet, address = peer.recvfrom(512)
            require(packet == osc_packet("/replay/ready") and
                    address == ("127.0.0.1", port), "OSC readiness datagram differs")
            for data in wire:
                peer.sendto(data, address)
            json_file(directory / "wire.json", {
                "destination": address, "ready": packet.hex(),
                "sent": [item.hex() for item in wire], "expected_typed": EXPECTED_OSC_ARGS,
            })

        status, lines = run_freej(args, directory, env, "osc.js", debug=3, after_start=send)
    require(status == 1 and "replay: OSC quit" in lines, "OSC CLI did not reach scripted quit")
    callbacks = lines_at(lines, "replay: OSC callback ")
    if args.expect_legacy_osc:
        require(not callbacks and all(any(f"OSC failed JSCall to replay_{name}" in line
                                         for line in lines) for name in ("noargs", "typed")),
                f"changed historical inbound OSC outcome: {callbacks!r}")
        require(all(any(marker in line for line in lines) for marker in
                    ("OSC arg 0 is int: 123", "OSC arg 1 is float: 4.50",
                     "OSC arg 2 is string: hello")), "OSC dispatch did not parse all sent arguments")
    else:
        require(callbacks == ["noargs", EXPECTED_OSC_ARGS, "abcde|-1.25|-2147483648",
                              "2147483647|0.5|"], f"OSC callbacks missing or wrong: {callbacks!r}")
        for diagnostic in ('OSC path /test called with wrong types: "i" instead of "ifs"',
                           "OSC path /unknown called, but no method is handling it"):
            require(sum(diagnostic in line for line in lines) == 1,
                    f"expected one OSC diagnostic: {diagnostic}")
        osc_lifecycle(args)


def osc_lifecycle(args):
    for scenario in ("stop-restart", "self-removal", "reset", "quit"):
        directory, env = fixture(args, f"osc-{scenario}")
        if scenario != "reset":
            observe(env, args, "video")
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as peer:
            peer.settimeout(5)
            peer.bind(("127.0.0.1", 0))
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as reserve:
                reserve.bind(("127.0.0.1", 0))
                port = reserve.getsockname()[1]
            json_file(directory / "osc-input.json", {"scenario": scenario, "burst": 128,
                      "server_port": port, "peer_port": peer.getsockname()[1]})
            (directory / "osc-config.js").write_text(
                f'var scenario = "{scenario}", server = "{port}", peer = "{peer.getsockname()[1]}";\n')

            def ready(route):
                packet, address = peer.recvfrom(512)
                require((packet, address) == (osc_packet(route), ("127.0.0.1", port)),
                        f"{scenario}: unexpected readiness response")
                return address

            def exercise():
                address = ready("/replay/ready")
                for sequence in range(128):
                    peer.sendto(osc_packet("/packet", "i", struct.pack(">i", sequence)), address)
                # Gate the first poll on admissions, so the stop/removal
                # checks start with known pending work, not a UDP sleep.
                deadline = time.monotonic() + 3
                marker = b"OSC queued /packet (i)"
                while (directory / "freej.log").read_bytes().count(marker) < 128:
                    require(time.monotonic() < deadline, f"{scenario}: 128 messages were not admitted")
                    time.sleep(0.005)
                (directory / "osc-admitted").touch()
                if scenario != "quit":
                    route = "/replay/restarted" if scenario == "stop-restart" else "/replay/rebound"
                    address = ready(route)
                    peer.sendto(osc_packet("/packet", "i", struct.pack(">i", 128)), address)

            status, lines = run_freej(args, directory, env, "osc-lifecycle.js", debug=3,
                                       after_start=exercise)

        require(status == 1 and f"replay: OSC {scenario} complete" in lines,
                f"{scenario}: FreeJ did not complete (exit {status})")
        sequences = lines_at(lines, "replay: sequence ")
        expected = list(range(129)) if scenario == "stop-restart" else [0]
        require(sequences == list(map(str, expected)), f"{scenario}: wrong callback order: {sequences}")
        require(lines_at(lines, "replay: callback completed ") == [scenario],
                f"{scenario}: first callback did not finish once")
        if scenario == "stop-restart":
            counts = lines_at(lines, "replay: batch ")
            require(counts == ["32", "64"], f"{scenario}: wrong per-frame or stopped count: {counts}")
        if scenario in ("self-removal", "reset"):
            require(lines_at(lines, "replay: replacement ") == ["128"],
                    f"{scenario}: receiver did not release its port and run again")
        frames = None
        if scenario != "reset":
            frames = len((directory / "presentations.txt").read_text().splitlines())
            require(frames >= 6, f"{scenario}: rendering did not advance after the first callback")
        json_file(directory / "osc-check.json", {"scenario": scenario, "first_admitted": 128,
                  "sequences": expected, "replacement_callbacks": lines_at(lines, "replay: replacement "),
                  "submissions": frames})


def osc_example(args):
    directory, env = fixture(args, "osc-example")
    example = HERE.parents[1] / "scripts/javascript/examples/osc_test.js"
    shutil.copyfile(example, directory / example.name)
    json_file(directory / "osc-input.json", {"source": str(example),
              "sha256": sha256(example.read_bytes()).hexdigest(), "port": 9696})
    (directory / "osc-example.js").write_text(
        'include("osc_test.js");\n'
        'kbd.released_m();\n'
        'for (var frame = 0; frame < 20; ++frame) run(0);\n'
        'osc.stop();\n'
        'echo("replay: instrument loopback complete");\n'
        'quit();\n')
    status, lines = run_freej(args, directory, env, "osc-example.js", debug=3)
    require(status == 1 and "replay: instrument loopback complete" in lines,
            "the unedited instrument example did not complete")
    expected = ["OSC remote call to my test function", "int argument is: 123",
                "float argument is: 4.5", "string argument is: hello"]
    require([line for line in lines if line in expected] == expected,
            "the unedited instrument example did not deliver one /test ifs callback")


def audio(args):
    directory, env = fixture(args, "audio")
    env["JACK_DEFAULT_SERVER"] = args.jack_server
    with owned_process([str(args.pulse)], directory, env, "pulse.log",
                       stdout=subprocess.PIPE) as source:
        require(select.select([source.stdout], [], [], 5)[0] and
                source.stdout.readline().strip() == b"replay_pcm:pulse",
                "JACK pulse sender did not activate at 48000 Hz / 2048 frames")
        observe(env, args, "audio")
        status, lines = run_freej(args, directory, env, "audio.js")
        require(source.poll() is None, "JACK pulse sender exited early")
    require(source.returncode == 0, f"JACK pulse sender cleanup exit {source.returncode}")
    require(status == 1 and "replay: audio quit" in lines, "audio CLI did not reach scripted quit")
    capture = (directory / "capture.txt").read_text().split()
    require(len(capture) == 4 and capture[0].startswith("freej:") and
            capture[1:] == ["2048", "0.500", "1"], f"unexpected captured pulse: {capture!r}")
    bands = [list(map(float, row.split(","))) for row in lines_at(lines, "replay: bands ")]
    require(len(bands) == 25 and all(len(window) == 16 for window in bands),
            "expected 25 FFT windows of 16 bands")
    # An impulse at sample index zero gives real FFT value 0.5 in every bin.
    # These are the first-window bands for zero prior state, in JS order 1..16.
    impulse = [((end - start) * 0.5) ** 2 * 0.025 / 2.4
               for start, end in zip(IMPULSE_BINS, IMPULSE_BINS[1:])]
    expected = impulse[1:] + impulse[:1]
    json_file(directory / "bands.json", {"actual": bands, "first_impulse": expected})
    if not args.expect_legacy_audio:
        first = next((window for window in bands if any(window)), None)
        require(first is not None, "capture-to-FFT is silent")
        require(all(isclose(actual, target, rel_tol=1e-5, abs_tol=1e-7)
                    for actual, target in zip(first, expected)), f"first pulse bands differ: {first}")
    else:
        require(all(value == 0 for window in bands for value in window),
                "changed historical capture-to-FFT outcome")


def reset(args):
    directory, env = fixture(args, "reset")
    status, lines = run_freej(args, directory, env, "reset.js")
    require(any("FreeJ engine reset" in line for line in lines), "reset API was not reached")
    require(status == 1 and "replay: reset returned" in lines and
            "replay: reset target loaded" not in lines,
            f"changed historical reset outcome: CLI exit {status}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ("freej", "plugin", "observer", "output"):
        parser.add_argument(f"--{option}", type=Path, required=True)
    parser.add_argument("--video-driver", default="dummy")
    parser.add_argument("--pulse", type=Path)
    parser.add_argument("--jack-server", help="name of the running private JACK dummy server")
    parser.add_argument("--case", choices=("all", "video", "osc", "osc-example", "audio", "reset"),
                        default="all")
    parser.add_argument("--expect-reversed-order", action="store_true")
    parser.add_argument("--require-working-routes", action="store_true")
    parser.add_argument("--expect-legacy-audio", action="store_true",
                        help="reproduce the historical silent collector")
    parser.add_argument("--expect-legacy-osc", action="store_true",
                        help="check the historical build's parsed inbound OSC without JS callbacks")
    args = parser.parse_args()
    if args.case in ("all", "audio") and not (args.pulse and args.jack_server):
        parser.error("audio requires --pulse and --jack-server")
    if args.expect_reversed_order and args.case != "video":
        parser.error("--expect-reversed-order requires --case video")
    if args.expect_legacy_osc and args.require_working_routes:
        parser.error("historical OSC and working-route expectations conflict")
    for field in ("freej", "plugin", "observer", "output", "pulse"):
        value = getattr(args, field)
        if value:
            setattr(args, field, value.resolve())
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    args.output.mkdir()
    runs = []

    def check(name, function):
        try:
            result = function()
        except (OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
            runs.append({"case": name, "status": "failed", "detail": str(error)})
            print(f"FAIL {name} | {error}", flush=True)
            return None
        runs.append({"case": name, "status": "passed"})
        print(f"PASS {name}", flush=True)
        return result

    if args.case in ("all", "video"):
        first = check("video-a", lambda: video(args, "video-a"))
        if not args.expect_reversed_order:
            second = check("video-b", lambda: video(args, "video-b"))
            check("startup-parity", lambda: require(first is not None and first == second,
                                                   "startup replays differ or are incomplete"))
    for name, function in (("osc", osc), ("audio", audio), ("reset", reset)):
        if args.case in ("all", name):
            check(name, lambda: function(args))
    if args.case == "osc-example":
        check("osc-example", lambda: osc_example(args))
    json_file(args.output / "checks.json", runs)
    failed = sum(run["status"] == "failed" for run in runs)
    print(f"{len(runs)} executed, {failed} failed, 0 skipped", flush=True)
    return int(failed > 0)


if __name__ == "__main__":
    raise SystemExit(main())
