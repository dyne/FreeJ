#!/usr/bin/env python3
"""Build the pinned Linux SDL/X11 stage SDK in a fresh directory."""

import argparse
from hashlib import file_digest
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tarfile
from urllib.parse import unquote, urlsplit


MULTIARCH = "x86_64-linux-gnu"


def run(args, cwd, env=None):
    print(f"$ cd {shlex.quote(str(cwd))} && {shlex.join(map(str, args))}", flush=True)
    subprocess.run(args, cwd=cwd, env=env, check=True)


def fetch(work, caches):
    sources = work / "sources"
    downloads = work / "downloads"
    sources.mkdir()
    downloads.mkdir()
    for line in Path(__file__).with_suffix(".lock").read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        expected, url = line.split()
        filename = Path(unquote(urlsplit(url).path)).name
        target = downloads / filename
        cached = next((cache / filename for cache in caches if (cache / filename).is_file()), None)
        if cached is None:
            run(["curl", "--fail", "--silent", "--show-error", "--location",
                 "--connect-timeout", "10", "--max-time", "180", "--output", target, url], work)
        else:
            shutil.copyfile(cached, target)
        with target.open("rb") as source:
            actual = file_digest(source, "sha256").hexdigest()
        if actual != expected:
            raise ValueError(f"{filename}: expected SHA256 {expected}, got {actual}")
        print(f"SHA256 {filename} {actual}", flush=True)
        if target.suffix == ".deb":
            run(["dpkg-deb", "-x", target, work / "packages"], work)
        else:
            with tarfile.open(target) as source:
                source.extractall(sources, filter="data")


def stage(work, archives, legacy, jobs):
    gcc = legacy / "gcc12/usr/bin/gcc-12"
    gxx = gcc.with_name("g++-12")
    if not gcc.is_file() or not gxx.is_file():
        raise ValueError(f"GCC/G++ 12 is missing in {gcc.parent}")
    ccache = shutil.which("ccache")
    cc = f"{ccache} {gcc}" if ccache else str(gcc)
    main_usr = archives / "debian/usr"
    private_usr = work / "packages/usr"
    common_lib = main_usr / "lib" / MULTIARCH
    private_lib = private_usr / "lib" / MULTIARCH
    pcdirs = [private_lib / "pkgconfig", common_lib / "pkgconfig", main_usr / "share/pkgconfig"]

    for module in pcdirs[0].glob("*.pc"):
        if module.is_symlink():
            continue
        source = module.read_text().replace("/usr/", f"{private_usr}/")
        source = source.replace("=/usr\n", f"={private_usr}\n")
        module.write_text(source)

    sources = work / "sources"
    env = dict(os.environ, CC=cc, CXX=str(gxx), LC_ALL="C", SOURCE_DATE_EPOCH="1695750228",
               PKG_CONFIG_PATH=os.pathsep.join(map(str, pcdirs)))
    for variable in ("CFLAGS", "CXXFLAGS", "CPPFLAGS", "LDFLAGS", "DESTDIR", "CPATH",
                     "LIBRARY_PATH", "PKG_CONFIG", "PKG_CONFIG_LIBDIR", "PKG_CONFIG_SYSROOT_DIR"):
        env.pop(variable, None)

    x11_build = work / "x11-build"
    x11_build.mkdir()
    # SDL3 asks Xlib for the default cursor. Keep that call inside the static closure.
    run([sources / "libX11-1.8.12/configure", f"--prefix={work / 'x11'}", "--datadir=/usr/share",
         "--enable-static", "--disable-shared", "--disable-loadable-xcursor",
         "--disable-loadable-i18n", "--disable-specs", "--disable-devel-docs"], x11_build, env)
    run(["make", f"-j{jobs}"], x11_build, env)

    libraries = work / "lib"
    libraries.mkdir()
    for directory in (*sorted((archives / "prefixes").glob("*/lib")),
                      common_lib, legacy / f"debian/usr/lib/{MULTIARCH}", legacy / "ffmpeg/lib",
                      private_lib):
        for archive in directory.glob("lib*.a"):
            shutil.copyfile(archive, libraries / archive.name)
    shutil.copyfile(x11_build / "src/.libs/libX11.a", libraries / "libX11.a")



def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("work", type=Path)
    parser.add_argument("--archive-sdk", type=Path, required=True)
    parser.add_argument("--reference-sdk", type=Path, required=True)
    parser.add_argument("--archive-cache", action="append", type=Path, default=[])
    parser.add_argument("--jobs", type=int, default=6)
    args = parser.parse_args()
    paths = tuple(path.resolve() for path in (args.work, args.archive_sdk, args.reference_sdk))
    if args.jobs <= 0 or any(not re.fullmatch(r"[a-zA-Z0-9_./-]+", str(path)) for path in paths):
        parser.error("use positive jobs and SDK paths without spaces or shell metacharacters")
    work, archives, legacy = paths
    work.mkdir()
    fetch(work, args.archive_cache)
    stage(work, archives, legacy, args.jobs)


if __name__ == "__main__":
    main()
