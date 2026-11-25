#!/usr/bin/env python3
"""Exercise compatibility fixtures and report native CI runtime coverage."""

import argparse
import json
import os
from pathlib import Path
import subprocess
import sys

from native_profile_services import jack_dummy, x11


REPLAY = Path(__file__).resolve().with_name("show-replay") / "replay.py"
OPTIONAL_BACKENDS = (
    "xgrab", "xscreensaver", "v4l2", "text", "cairo", "screenshot", "alsa-midi", "flash",
    "aalib", "cwiid", "unicap", "opencv", "opengl", "frei0r-directory", "freeframe-directory",
)


def compatibility(args):
    work = args.output.resolve()
    env = dict(os.environ, LC_ALL="C")
    command = [sys.executable, REPLAY, "--freej", args.freej.resolve(),
               "--observer", args.observer.resolve(), "--plugin", args.plugin.resolve()]
    with jack_dummy(args.jackd.resolve(), work, env) as (server, _):
        subprocess.run([*command, "--pulse", args.pulse.resolve(), "--jack-server", server,
                        "--output", work / "replay"], cwd=work, env=env, check=True)
    with x11(work, env) as display:
        subprocess.run([*command, "--case", "video", "--video-driver", "x11",
                        "--output", work / "offscreen"], cwd=work, env=dict(env, DISPLAY=display), check=True)


def passed_cases(path):
    checks = json.loads(path.read_text())
    if any(check["status"] != "passed" for check in checks):
        raise ValueError(f"unsuccessful replay case in {path}")
    return [check["case"] for check in checks]


def report(args):
    work = args.output
    capabilities = set((work / "capabilities.txt").read_text().split())
    required = {"sdl", "image", "javascript", "jack"}
    if args.profile == "compatibility":
        required.update(("osc", "fft", "frei0r-directory"))
    else:
        required.add("frei0r-builtin")
    if missing := sorted(required - capabilities):
        raise ValueError(f"replay capabilities absent: {', '.join(missing)}")
    disabled = [backend for backend in OPTIONAL_BACKENDS if backend not in capabilities]
    print("### Runtime coverage")
    if args.profile == "compatibility":
        checks = passed_cases(work / "replay/checks.json")
        offscreen = passed_cases(work / "offscreen/checks.json")
        if checks != ["video-a", "video-b", "startup-parity", "osc", "audio", "reset"] or offscreen != checks[:3]:
            raise ValueError(f"unexpected compatibility case outcomes: {checks!r}, {offscreen!r}")
        print("- Staged CLI: SDL dummy and Xvfb/X11 replay passed (six frame cues per startup).")
        print("- Loopback OSC: readiness, typed argument parsing and dispatch reached. JS callbacks still fail.")
        print("- Synthetic PCM: input observer captured the JACK impulse. All 25 × 16 FFT bands remain zero.")
        print("- Reset: reset path and return observed. The target script remains unevaluated.")
    else:
        log = (work / "show.log").read_text().splitlines()
        if [line for line in log if line.startswith("PASS ")] != ["PASS cli", "PASS cli-effects", "PASS factory", "PASS qt"]:
            raise ValueError("unexpected static runtime outcomes")
        print("- Staged CLI and Qt/XCB: built-in effects rendered on Xvfb with zero external effect lookups.")
        print("- Installed factory: JACK dummy connection at 48 kHz / 2,048 frames.")
        print("- OSC and PCM observer fixtures: compatibility job only.")
    print(f"- Disabled backends: {', '.join(disabled) if disabled else 'none'}.")
    print("- Physical display present-to-photon, display input and fullscreen, audio loopback and A/V sync, "
          "V4L2/camera and DV capture, physical MIDI/joystick/Wii/ViMo controls: rig checks pending.")
    print("- OSC-to-visible and audio-to-visible response are unmeasured with the current callback and FFT paths.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    modes = parser.add_subparsers(required=True)
    replay = modes.add_parser("compatibility")
    for name in ("freej", "observer", "plugin", "pulse", "jackd", "output"):
        replay.add_argument(f"--{name}", required=True, type=Path)
    replay.set_defaults(run=compatibility)
    summary = modes.add_parser("report")
    summary.add_argument("profile", choices=("compatibility", "static"))
    summary.add_argument("output", type=Path)
    summary.set_defaults(run=report)
    args = parser.parse_args()
    args.run(args)


if __name__ == "__main__":
    main()
