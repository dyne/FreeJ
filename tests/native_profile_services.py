"""Own the local Xvfb and JACK processes for native profile fixtures."""

from contextlib import contextmanager
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import select
import subprocess
import time


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
                                                 "JACK_TMPDIR", "JACK_DEFAULT_SERVER",
                                                 "JACK_DRIVER_DIR", "LD_LIBRARY_PATH")
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
def jack_dummy(jackd, work, env):
    # The daemon uses its own shared library and driver lookup paths.
    jack = jackd.parent.parent
    server = f"fjstage{os.getpid()}"
    # JACK 1 metadata uses the default server name even with an explicit -n.
    jack_env = dict(env, LD_LIBRARY_PATH=str(jack / "lib"), JACK_DEFAULT_SERVER=server,
                    JACK_DRIVER_DIR=env.get("JACK_DRIVER_DIR", str(jack / "lib/jack")),
                    PATH=f"{jack / 'bin'}{os.pathsep}{env['PATH']}")
    tmpdir = subprocess.check_output([jackd, "-l"], env=jack_env, text=True).strip()
    socket = Path(tmpdir) / f"jack-{os.getuid()}" / server
    command = [str(jackd), "-r", "-n", server, "-d", "dummy", "-r", "48000", "-p", "2048"]
    with owned(command, work, jack_env, "jackd") as daemon:
        until(lambda: any(path.is_socket() for path in socket.glob("*")), daemon)
        yield server, tmpdir
        if daemon.poll() is not None:
            raise ValueError(f"JACK dummy exited during the fixture: {daemon.returncode}")
