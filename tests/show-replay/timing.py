#!/usr/bin/env python3
"""Capture and summarize the Linux reference's software timing."""

import argparse
from bisect import bisect_left
import json
from math import ceil, floor
import os
from pathlib import Path
import platform
import resource
import select
import socket
import statistics
import subprocess
import sys
import time

sys.dont_write_bytecode = True
from replay import fixture, json_file, osc_string, owned_process, require


FPS = 25
AUDIO_RATE = 48000
AUDIO_FRAMES = 2048
WORKLOADS = ("idle", "filtered", "osc", "capture")


def period_ms(events_per_second, count=1):
    return 1000 * count / events_per_second


def stats(values):
    sorted_values = sorted(values)
    if not sorted_values:
        return None
    result = {"n": len(sorted_values)}
    for p in (50, 95, 99):
        result[f"p{p}"] = sorted_values[ceil(p * len(sorted_values) / 100) - 1]
    result["jitter_p95_minus_p50"] = result["p95"] - result["p50"]
    return result


def ns_ms(nanos):
    return nanos / 1_000_000


def events(path):
    rows = []
    for line in path.read_text().splitlines():
        ns, kind, value, detail = line.split()
        rows.append((int(ns), kind, int(value), int(detail)))
    return rows


def kind(rows, name):
    return [(ns, value, detail) for ns, label, value, detail in rows if label == name]


def pairs(first, second):
    """Pair the actual trace streams in wire order and verify identity."""
    require(len(first) == len(second), "event stream counts differ")
    latencies = []
    for (origin, sent_id, _), (target, received_id, _) in zip(first, second):
        require(sent_id == received_id, "event order differs")
        require(origin <= target, "event timestamps are reversed")
        latencies.append(ns_ms(target - origin))
    return latencies


def to_next_present(origins, presents):
    result = []
    for origin in origins:
        after = bisect_left(presents, origin)
        if after < len(presents):
            result.append(ns_ms(presents[after] - origin))
    return result


def frame_metrics(presents, fps):
    interval = period_ms(fps)
    values = [ns_ms(following - previous) for previous, following in zip(presents, presents[1:])]
    require(values and min(values) > 0, "missing or unordered frame submissions")
    duration = ns_ms(presents[-1] - presents[0])
    return {
        "frame_interval_ms": stats(values), "submissions": len(presents),
        "late_intervals_over_110_percent": sum(value > 1.10 * interval for value in values),
        "estimated_missed_software_slots": sum(max(0, floor(value / interval + 0.5) - 1)
                                               for value in values),
        "elapsed_ms": duration, "drift_ms": duration - len(values) * interval,
    }


def audio_metrics(sources, captures, first, last):
    captured = [(ns, sample, n) for ns, sample, n in captures if first <= ns <= last]
    require(len(captured) >= 2, "missing captured JACK pulses")
    positions = sorted(sample for _, sample, _ in captured)
    source_at = {sample: (ns, sample, n) for ns, sample, n in sources}
    require(all(sample in source_at for sample in positions), "captured JACK pulse has no source")
    require(all(n == AUDIO_FRAMES for _, _, n in sources + captured), "wrong JACK period")
    require(positions == [sample for _, sample, _ in captured] and len(set(positions)) == len(positions),
            "captured JACK sample positions are repeated or unordered")
    clocks = pairs([source_at[sample] for _, sample, _ in captured], captured)
    reference = period_ms(AUDIO_RATE, positions[-1] - positions[0])
    return {
        "pulse_source_to_capture_ms": stats(clocks),
        "capture_clock_drift_ms": ns_ms(captured[-1][0] - captured[0][0]) - reference,
        "first_last_capture_positions": [positions[0], positions[-1]],
        "captured_pulses": len(captured),
        "missing_capture_periods": (positions[-1] - positions[0]) // AUDIO_FRAMES + 1 - len(positions),
        "capture_to_visible_response_ms": None,
    }, [ns for ns, _, _ in captured]


def reserve_port():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as reserve:
        reserve.bind(("127.0.0.1", 0))
        return reserve.getsockname()[1]


def configure(directory, workload, frames, server_port=0, peer_port=0):
    (directory / "timing-config.js").write_text(
        f'var workload = "{workload}", frameCount = {frames};\n'
        f'var serverPort = "{server_port}", peerPort = "{peer_port}";\n')


def prepare_trace(env, observer, path):
    existing = env.get("LD_PRELOAD")
    env = dict(env, FREEJ_TIMING_TRACE=str(path))
    env["LD_PRELOAD"] = str(observer) + (f":{existing}" if existing else "")
    env.pop("FREEJ_TIMING_PULSE_SOURCE", None)
    return env


def send_bursts(peer, port, trace, warmup, stress=False):
    packet, sender = peer.recvfrom(512)
    require(packet == osc_string("/timing/ready") + osc_string(",") and
            sender == ("127.0.0.1", port), "wrong OSC readiness packet")
    until = time.monotonic() + 6
    while not trace.exists() or len(kind(events(trace), "present")) < warmup + 2:
        require(time.monotonic() < until, "OSC warm-up presentations did not occur")
        time.sleep(0.001 if stress else 0.02)
    sent = []
    for _ in range(1 if stress else 8):
        for i in range(128 if stress else 4):
            id_ = i % 4
            packet = osc_string(f"/timing/{id_}") + osc_string(",")
            sent.append((time.monotonic_ns(), "osc-send", id_, 0))
            peer.sendto(packet, sender)
        if not stress:
            time.sleep(0.15)  # Reference cadence, after observed warm-up completion.
    (trace.parent / "sender.trace").write_text(
        "".join(f"{ns} {label} {id_} {detail}\n" for ns, label, id_, detail in sent))
    return sent


def capture_run(args, workload, name, analyzed_frames):
    directory, env = fixture(args, name)
    warmup = FPS
    total_frames = warmup + analyzed_frames
    trace = directory / "engine.trace"
    env = prepare_trace(env, args.observer, trace)
    command = [str(args.freej), "-c", "-D", "0", "-s", "80x60", "-j", "timing.js"]
    started = time.monotonic_ns()
    sent = []
    source_status = None
    osc_stress = getattr(args, "osc_stress", False)
    if workload == "capture":
        env["JACK_DEFAULT_SERVER"] = args.jack_server
        pulse_env = prepare_trace(env, args.observer, directory / "pulse.trace")
        pulse_env["FREEJ_TIMING_PULSE_SOURCE"] = "1"
        with owned_process([str(args.pulse)], directory, pulse_env, "pulse.log",
                           stdout=subprocess.PIPE) as source:
            require(select.select([source.stdout], [], [], 5)[0] and
                    source.stdout.readline().strip() == b"replay_pcm:pulse",
                    "JACK 48 kHz / 2048-frame pulse startup failed")
            configure(directory, workload, total_frames)
            with owned_process(command, directory, env, "freej.log") as app:
                status = app.wait(timeout=total_frames / FPS + 15)
            require(source.poll() is None, "JACK pulse sender exited early")
        source_status = source.returncode
        require(source_status == 0, "JACK pulse sender cleanup failed")
    elif workload == "osc":
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as peer:
            peer.settimeout(6)
            peer.bind(("127.0.0.1", 0))
            port = reserve_port()
            configure(directory, workload, total_frames, port, peer.getsockname()[1])
            with owned_process(command, directory, env, "freej.log") as app:
                sent = send_bursts(peer, port, trace, warmup, osc_stress)
                status = app.wait(timeout=total_frames / FPS + 15)
    else:
        configure(directory, workload, total_frames)
        with owned_process(command, directory, env, "freej.log") as app:
            status = app.wait(timeout=total_frames / FPS + 15)
    ended = time.monotonic_ns()
    log = (directory / "freej.log").read_text(errors="replace").splitlines()
    require(status == 1 and "timing: workload complete" in log,
            f"{name}: FreeJ exit {status}, workload completion absent")
    rows = events(trace)
    raw = kind(rows, "present")
    require(len(raw) == total_frames and all(result == 0 for _, result, _ in raw),
            f"{name}: expected {total_frames} successful SDL submissions, saw {len(raw)}")
    presents = [ns for ns, _, _ in raw[warmup:]]
    metrics = frame_metrics(presents, FPS)
    if workload == "osc":
        receipt = kind(rows, "osc-receive")
        dispatch = kind(rows, "osc-dispatch")
        sent_events = kind(sent, "osc-send")
        expected_packets = 128 if osc_stress else 32
        require(len(sent_events) == expected_packets and
                all(presents[0] <= tick <= presents[-1] for tick, _, _ in dispatch),
                "OSC burst not contained in measured window")
        expected_callbacks = 0 if getattr(args, "expect_legacy_osc", False) else 1
        require(all(num == expected_callbacks for _, _, num in dispatch),
                f"expected {expected_callbacks} JS callbacks per OSC dispatch")
        transport = pairs(sent_events, receipt)
        handoff = pairs(receipt, dispatch)
        submission_indexes = [bisect_left(presents, ns) for ns, _, _ in dispatch]
        counts = [0] * len(presents)
        for index in submission_indexes:
            counts[index] += 1
        pending = peak_pending = 0
        for _, label, _, _ in sorted(rows):
            if label == "osc-receive":
                pending += 1
                peak_pending = max(pending, peak_pending)
            elif label == "osc-dispatch":
                pending -= 1
        require(pending == 0, "unpaired OSC queue events")
        metrics.update({"osc_send_to_receipt_ms": stats(transport),
                        "osc_receipt_to_dispatch_ms": stats(handoff),
                        "osc_dispatch_to_next_submission_ms":
                            stats(to_next_present([ns for ns, _, _ in dispatch], presents)),
                        "osc_send_to_next_submission_ms": stats([
                            ns_ms(presents[index] - sent[0])
                            for sent, index in zip(sent_events, submission_indexes)]),
                        "osc_peak_pending_inferred": peak_pending,
                        "osc_peak_dispatches_per_frame": max(counts),
                        "osc_to_visible_response_ms": None,
                        "osc_callbacks": sum(num for _, _, num in dispatch)})
    if workload == "capture":
        source = events(directory / "pulse.trace")
        xrun_registrations = [status for _, status, _ in kind(rows, "xrun-registration") +
                              kind(source, "xrun-registration")]
        require(xrun_registrations == [0, 0], "missing JACK xrun monitoring")
        audio, capture_times = audio_metrics(kind(source, "pulse-source"), kind(rows, "pulse-capture"),
                                             presents[0], presents[-1])
        metrics.update(audio)
        metrics["capture_to_next_submission_ms"] = stats(to_next_present(capture_times, presents))
        for owner, owner_events in (("engine", rows), ("source", source)):
            metrics[f"{owner}_xrun_notifications"] = sum(
                presents[0] <= ns <= presents[-1] for ns, _, _ in kind(owner_events, "jack-xrun"))
        require("timing: nonzero FFT windows 0" in log, "legacy FFT silence was not reproduced")
        metrics["nonzero_fft_windows"] = 0
        proc = subprocess.run(["ffprobe", "-v", "error", "-select_streams", "v",
                               "-count_frames", "-show_entries", "stream=codec_name,nb_read_frames",
                               "-of", "json", "timing.ogg"], cwd=directory, capture_output=True,
                              check=True, timeout=15)
        (directory / "recording.json").write_bytes(proc.stdout)
        recording, = json.loads(proc.stdout)["streams"]
        require(recording["codec_name"] == "theora" and int(recording["nb_read_frames"]) > 1,
                "capture workload did not record moving video")
        metrics["decoded_recording_frames"] = int(recording["nb_read_frames"])
    summary = {"workload": workload, "command": command, "cwd": str(directory), "exit": status,
               "pulse_exit": source_status, "started_ns": started, "ended_ns": ended,
               "warmup_frames": warmup, "metrics": metrics,
               "environment": {k: env[k] for k in ("SDL_VIDEODRIVER", "DISPLAY", "LD_LIBRARY_PATH",
                                                   "LD_PRELOAD", "JACK_DEFAULT_SERVER") if k in env}}
    json_file(directory / "timing.json", summary)
    return metrics


def envelopes(runs):
    """Same-configuration repeatability envelope, external to CI assertions."""
    numeric = {}
    for key, first in runs[0].items():
        if isinstance(first, dict) and "p95" in first:
            for statistic in ("p50", "p95", "p99", "jitter_p95_minus_p50"):
                numeric[f"{key}.{statistic}"] = [run[key][statistic] for run in runs]
        elif key == "drift_ms" or key == "capture_clock_drift_ms":
            numeric[f"absolute_{key}"] = [abs(run[key]) for run in runs]
        elif key in ("late_intervals_over_110_percent", "estimated_missed_software_slots",
                     "missing_capture_periods", "engine_xrun_notifications", "source_xrun_notifications"):
            numeric[key] = [run[key] for run in runs]
    result = {}
    for key, values in numeric.items():
        spread = max(values) - min(values)
        milliseconds = "_ms" in key
        margin = max(2 * spread, period_ms(FPS) * 0.05 if milliseconds else 0)
        result[key] = {"min": min(values), "median": statistics.median(values), "max": max(values),
                       "software_upper_limit": ceil(max(values) + margin)}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("freej", "plugin", "observer", "output"):
        parser.add_argument(f"--{name}", required=True, type=lambda value: Path(value).resolve())
    parser.add_argument("--pulse", type=lambda value: Path(value).resolve())
    parser.add_argument("--jack-server")
    parser.add_argument("--video-driver", choices=("dummy", "x11"), default="x11")
    parser.add_argument("--workload", choices=("all", *WORKLOADS), default="all")
    parser.add_argument("--osc-stress", action="store_true",
                        help="send one burst of 128 packets for the selected OSC workload")
    parser.add_argument("--expect-legacy-osc", action="store_true",
                        help="expect the historical OSC dispatch attempts to produce no JS calls")
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--seconds", type=int, default=3)
    parser.add_argument("--drift-seconds", type=int, default=60)
    args = parser.parse_args()
    if args.repeats < 5 or args.seconds < 3 or args.drift_seconds < 60:
        parser.error("minimums are five repetitions, three seconds and a 60-second drift run")
    if args.osc_stress and args.workload != "osc":
        parser.error("--osc-stress requires --workload osc")
    if args.workload in ("all", "capture") and not (args.pulse and args.jack_server):
        parser.error("capture requires --pulse and --jack-server")
    require(time.get_clock_info("monotonic").implementation == "clock_gettime(CLOCK_MONOTONIC)",
            "controller and native observer must share Linux CLOCK_MONOTONIC")
    args.output.mkdir()
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    result = {"platform": platform.uname()._asdict(), "clock": "CLOCK_MONOTONIC",
              "fps": FPS, "frame_ms": period_ms(FPS), "dimensions": [80, 60],
              "sample_rate": AUDIO_RATE, "samples_per_period": AUDIO_FRAMES,
              "audio_period_ms": period_ms(AUDIO_RATE, AUDIO_FRAMES), "display": args.video_driver,
              "plugin_set": ["Brightness"], "runs_per_workload": args.repeats,
              "software_envelopes": {}, "physical_measurements": None,
              "osc_pattern": "128 packet stress burst" if args.osc_stress else "eight bursts of four"}
    workloads = WORKLOADS if args.workload == "all" else (args.workload,)
    for workload in workloads:
        runs = []
        for index in range(1, args.repeats + 1):
            name = f"{workload}-{index}"
            runs.append(capture_run(args, workload, name, args.seconds * FPS))
            print(f"PASS {name}", flush=True)
        result["software_envelopes"][workload] = envelopes(runs)
    executed = args.repeats * len(workloads)
    if "capture" in workloads:
        result["long_run"] = capture_run(args, "capture", "capture-drift", args.drift_seconds * FPS)
        print("PASS capture-drift", flush=True)
        executed += 1
    json_file(args.output / "summary.json", result)
    print(f"{executed} executed, 0 failed, 0 skipped", flush=True)


if __name__ == "__main__":
    main()
