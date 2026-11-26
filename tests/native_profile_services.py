"""Own the local Xvfb and PipeWire processes for native profile fixtures."""

from contextlib import contextmanager
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import select
import subprocess
import time
from tempfile import TemporaryDirectory


def utc():
    return datetime.now(timezone.utc).isoformat()


@contextmanager
def owned(command, work, env, name, **kwargs):
    with (work / f"{name}.log").open("wb") as log:
        process = subprocess.Popen(command, cwd=work, env=env, stdout=log, stderr=log, **kwargs)
        start = utc()
        try:
            yield process
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=5)
            (work / f"{name}.json").write_text(json.dumps({
                "command": list(map(str, command)), "cwd": str(work), "exit": process.returncode,
                "startedAt": start, "observedAt": utc(), "environment": {
                    name: env[name] for name in ("DISPLAY", "SDL_VIDEODRIVER", "HOME", "PATH",
                                                 "PIPEWIRE_RUNTIME_DIR", "PIPEWIRE_REMOTE",
                                                 "PIPEWIRE_MODULE_DIR", "SPA_PLUGIN_DIR",
                                                 "LD_LIBRARY_PATH")
                    if name in env},
            }, indent=2) + "\n")


@contextmanager
def x11(work, env):
    reader, writer = os.pipe()
    command = ["Xvfb", "-displayfd", str(writer), "-screen", "0",
               "640x480x24", "-nolisten", "tcp"]
    try:
        with owned(command, work, env, "xvfb", pass_fds=(writer,)):
            os.close(writer)
            writer = None
            if not select.select([reader], [], [], 10)[0]:
                raise ValueError("Xvfb did not signal readiness")
            display = os.read(reader, 16).decode().strip()
            if not display.isdecimal():
                raise ValueError("Xvfb returned an invalid display")
            yield f":{display}"
    finally:
        if writer is not None:
            os.close(writer)
        os.close(reader)


def until(predicate, process):
    end = time.monotonic() + 5
    while process.poll() is None and time.monotonic() < end:
        if predicate():
            return
        time.sleep(0.04)
    raise ValueError(f"runtime observation unavailable; child status {process.poll()}")


@contextmanager
def pipewire_dummy(daemon, work, env, rate=48000, period=2048):
    sdk = daemon.parent.parent
    config = work / "dummy.conf"
    config.write_text(f"""
context.properties = {{
    core.daemon = true
    core.name = pipewire-0
    support.dbus = false
    default.clock.rate = {rate}
    default.clock.quantum = {period}
    default.clock.min-quantum = {period}
    default.clock.max-quantum = {period}
}}
context.spa-libs = {{ support.* = support/libspa-support }}
context.modules = [
    {{ name = libpipewire-module-protocol-native }}
    {{ name = libpipewire-module-access }}
    {{ name = libpipewire-module-spa-node-factory }}
    {{ name = libpipewire-module-client-node }}
    {{ name = libpipewire-module-link-factory }}
]
context.objects = [
    {{ factory = spa-node-factory
       args = {{ factory.name = support.node.driver node.name = FreeJ-Dummy
                 node.group = pipewire.dummy node.always-process = true
                 priority.driver = 20000 }}
    }}
]
""")
    # Native Unix socket paths have a small length limit, independent of the
    # worktree and build-prefix lengths.
    with TemporaryDirectory(prefix="freej-pw-", dir="/tmp") as runtime:
        client_env = dict(env, PIPEWIRE_RUNTIME_DIR=runtime, PIPEWIRE_REMOTE="pipewire-0")
        daemon_env = dict(client_env, LD_LIBRARY_PATH=str(sdk / "lib"),
                          PIPEWIRE_MODULE_DIR=str(sdk / "lib/pipewire-0.3"),
                          SPA_PLUGIN_DIR=str(sdk / "lib/spa-0.2"))
        with owned([daemon, "-c", config], work, daemon_env, "pipewire") as process:
            until(lambda: (Path(runtime) / "pipewire-0").is_socket(), process)
            yield client_env
            if process.poll() is not None:
                raise ValueError(f"PipeWire dummy exited during the fixture: {process.returncode}")
