#!/usr/bin/env python3
"""Prepare the pinned historical Linux amd64 SDK in a fresh directory."""

import argparse
import hashlib
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tarfile
from urllib.parse import unquote, urlsplit
from urllib.request import urlopen


def run(command, cwd):
    print(f"$ {shlex.join(str(arg) for arg in command)}", flush=True)
    subprocess.run(command, cwd=cwd, check=True)


def download(url, expected, directory):
    destination = directory / Path(unquote(urlsplit(url).path)).name
    temporary = destination.with_suffix(destination.suffix + ".part")
    print(f"Fetching {url}", flush=True)
    with urlopen(url, timeout=60) as response, temporary.open("wb") as output:
        shutil.copyfileobj(response, output)
    with temporary.open("rb") as archive:
        actual = hashlib.file_digest(archive, "sha256").hexdigest()
    if actual != expected:
        raise ValueError(f"{destination.name}: expected SHA256 {expected}, got {actual}")
    temporary.rename(destination)
    return destination


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, help="new SDK directory")
    parser.add_argument("--jobs", type=int, default=4, help="parallel FFmpeg build jobs")
    args = parser.parse_args()
    work = args.directory.resolve()
    if not re.fullmatch(r"[a-zA-Z0-9_./-]+", str(work)):
        parser.error("the historical build tools require a directory without spaces or shell metacharacters")
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    work.mkdir()
    archives = work / "archives"
    archives.mkdir()
    lock = Path(__file__).with_suffix(".lock")
    for line in lock.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        expected, url = line.split()
        archive = download(url, expected, archives)
        if archive.suffix == ".deb":
            target = work / ("gcc12" if "/gcc-12/" in url else "debian")
            target.mkdir(exist_ok=True)
            run(["dpkg-deb", "-x", archive, target], work)
        else:
            with tarfile.open(archive) as ffmpeg_archive:
                ffmpeg_archive.extractall(work, filter="data")

    debian_usr = work / "debian/usr"
    for module in (debian_usr / "lib/x86_64-linux-gnu/pkgconfig").glob("*.pc"):
        contents = module.read_text()
        module.write_text(contents.replace("prefix=/usr\n", f"prefix={debian_usr}\n", 1))

    gcc = work / "gcc12/usr/bin/gcc-12"
    ccache = shutil.which("ccache")
    compiler = f"{ccache} {gcc}" if ccache else str(gcc)
    build = work / "ffmpeg-build"
    build.mkdir()
    run([
        work / "ffmpeg-0.8.15/configure", f"--prefix={work / 'ffmpeg'}",
        f"--cc={compiler}", f"--ld={gcc}", "--enable-shared", "--enable-static",
        "--enable-pic", "--disable-asm", "--disable-yasm", "--disable-ffmpeg",
        "--disable-ffplay", "--disable-ffprobe", "--disable-ffserver", "--disable-doc",
    ], build)
    run(["make", f"-j{args.jobs}", "V=1"], build)
    run(["make", "install"], build)


if __name__ == "__main__":
    main()
