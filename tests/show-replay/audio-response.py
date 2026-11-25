#!/usr/bin/env python3
"""Pair sparse PCM pulses with actual submitted pixels under recording + OSC."""
import argparse
from bisect import bisect_left
import json
from pathlib import Path
import select
import socket
import subprocess

from replay import BASE, OVERLAY, fixture, owned_process, require
from timing import (FPS, configure, events, frame_metrics, kind, ns_ms, pairs, prepare_trace,
                    reserve_port, send_bursts, stats)


def rgb(color):
    r, g, b = color
    return (r << 16) | (g << 8) | b


def match_responses(source, capture, fft, presents, begin, end):
    on = rgb(OVERLAY)
    edges = []
    prior = False
    for ns, result, pixel in presents:
        marked = pixel == on
        if begin <= ns <= end:
            require(result == 0 and pixel in (rgb(BASE), on), f"unexpected submitted pixel {pixel:x}")
        if marked and not prior:
            edges.append(ns)
        prior = marked
    sources = {frame: ns for ns, frame, _ in source}
    windows = {}
    for ns, frame, marked in fft:
        if marked:
            windows.setdefault(frame, ns)
    matches = []
    # Exclude only the trailing two frames, where the run can end before a
    # pulse is displayed. A missing earlier response is an explicit failure.
    used = set()
    for observed, frame, _ in capture:
        if not begin <= observed <= end - 80_000_000:
            continue
        require(frame in sources and frame in windows, f"pulse {frame} has no source or responsive FFT")
        origin, analyzed = sources[frame], windows[frame]
        index = bisect_left(edges, analyzed)
        require(index < len(edges) and index not in used and origin <= observed <= analyzed
                and edges[index] - observed < 400_000_000, f"pulse {frame} has no distinct marker onset")
        used.add(index)
        matches.append(dict(frame=frame, source_ns=origin, capture_ns=observed,
                            fft_ns=analyzed, pixel_ns=edges[index]))
    require(len(matches) >= 4, "too few measured pulse responses")
    return matches


def run(args, name, frames):
    directory, env = fixture(args, name)
    trace = directory / "engine.trace"
    env = dict(prepare_trace(env, args.observer, trace), JACK_DEFAULT_SERVER=args.jack_server,
               FREEJ_TIMING_VISIBLE_AUDIO="1")
    pulse_env = dict(prepare_trace(env, args.observer, directory / "pulse.trace"),
                     FREEJ_TIMING_PULSE_SOURCE="1", LD_PRELOAD=str(args.observer))
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as peer:
        peer.settimeout(6)
        peer.bind(("127.0.0.1", 0))
        port = reserve_port()
        configure(directory, "audio-response", frames + FPS, port, peer.getsockname()[1])
        with owned_process([args.pulse, "--sparse"], directory, pulse_env, "pulse.log",
                           stdout=subprocess.PIPE) as source:
            require(select.select([source.stdout], [], [], 5)[0] and
                    source.stdout.readline().strip() == b"replay_pcm:pulse", "pulse sender startup")
            with owned_process([args.freej, "-c", "-D", "0", "-s", "80x60", "-j", "audio-response.js"],
                               directory, env, "freej.log") as engine:
                sent = send_bursts(peer, port, trace, FPS)
                status = engine.wait(timeout=frames/FPS + 15)
            require(status == 1 and "response: recorded and finished" in (directory/"freej.log").read_text(),
                    "engine did not finish the recorded script")
            require(source.poll() is None, "pulse client exited early")
        require(source.returncode == 0, "pulse shutdown")
    rows = events(trace)
    source_rows = events(directory / "pulse.trace")
    presents = kind(rows, "present")
    require(len(presents) == frames + FPS, "unexpected number of submissions")
    stamps = [ns for ns, _, _ in presents[FPS:]]
    begin, end = stamps[0], stamps[-1]
    matches = match_responses(kind(source_rows, "pulse-source"), kind(rows, "pulse-capture"),
                              kind(rows, "fft-window"), presents, begin, end)
    (directory / "matched.json").write_text(json.dumps(matches, indent=2))
    sent_events = [(ns, seq, 0) for ns, _, seq, _ in sent]
    pairs(sent_events, kind(rows, "osc-receive"))
    dispatch = kind(rows, "osc-dispatch")
    pairs(kind(rows, "osc-receive"), dispatch)
    require(len(sent) == 32 and all(callbacks == 1 for _,_,callbacks in dispatch), "lost OSC dispatch")
    code = json.loads(subprocess.check_output(["ffprobe", "-v", "error", "-count_frames",
        "-show_entries", "stream=codec_name,nb_read_frames,channels,sample_rate", "-of", "json", "response.ogg"],
        cwd=directory))
    (directory / "streams.json").write_text(json.dumps(code))
    for codec in ("vorbis", "theora"):
        require(any(s.get("codec_name") == codec and int(s["nb_read_frames"]) > 2
                    for s in code["streams"]), f"missing decoded {codec} frames")
    positions = [(ns, frame) for ns, frame, _ in kind(rows, "input-period") if begin <= ns <= end]
    require(len(positions) > 1, "no captured clock span")
    missing = sum(((b-a) & 0xffffffff)//2048 - 1 for (_, a), (_, b) in zip(positions, positions[1:]))
    drift = ns_ms(positions[-1][0]-positions[0][0]) - ((positions[-1][1]-positions[0][1]) & 0xffffffff)/48
    require([s for _,s,_ in kind(rows, "xrun-registration") + kind(source_rows, "xrun-registration")] == [0,0],
            "xrun counters unavailable")
    xruns = sum(begin <= ns <= end for ns,_,_ in kind(rows, "jack-xrun") + kind(source_rows, "jack-xrun"))
    worst = max(ns_ms(duration) for ns,_,duration in kind(rows, "process-duration") if begin <= ns <= end)
    result = dict(frame_metrics(stamps, FPS), responses=len(matches), osc_callbacks=len(dispatch),
        xruns=xruns, missing_periods=missing, audio_drift_ms=drift, callback_worst_ms=worst,
        capture_to_pixel_ms=stats([ns_ms(m["pixel_ns"]-m["capture_ns"]) for m in matches]),
        source_to_pixel_ms=stats([ns_ms(m["pixel_ns"]-m["source_ns"]) for m in matches]))
    (directory / "response.json").write_text(json.dumps(result, indent=2))
    require(xruns == missing == 0 and worst < 2048/48, f"capture discontinuity or deadline: {result}")
    print(f"PASS {name}: {len(matches)} pulse/FFT/pixel matches; {len(dispatch)} OSC callbacks", flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ("freej","pulse","observer","plugin","output"):
        parser.add_argument("--"+option, type=lambda p:Path(p).resolve(), required=True)
    parser.add_argument("--jack-server",required=True)
    parser.add_argument("--video-driver",default="x11")
    args = parser.parse_args()
    args.output.mkdir()
    short = [run(args, f"short-{i}", 75) for i in range(1,6)]
    sustained = run(args, "sustained", 1500)
    (args.output/"summary.json").write_text(json.dumps({"short":short,"sustained":sustained},indent=2))


if __name__ == "__main__":
    main()
