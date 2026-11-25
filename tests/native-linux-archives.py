#!/usr/bin/env python3
"""Stage pinned Linux amd64 archives alongside the historical FreeJ SDK."""

import argparse
import hashlib
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import subprocess
import tarfile
from urllib.parse import unquote, urlsplit


def run(command, cwd, env=None):
    print(f"$ cd {shlex.quote(str(cwd))} && {shlex.join(map(str, command))}", flush=True)
    subprocess.run(command, cwd=cwd, env=env, check=True)


def unpack_inputs(work, cache):
    downloads = work / "downloads"
    downloads.mkdir()
    lock = Path(__file__).with_suffix(".lock")
    for line in lock.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        digest, url = line.split()
        name = Path(unquote(urlsplit(url).path)).name
        archive = downloads / name
        if cache is not None and (cache / name).is_file():
            shutil.copyfile(cache / name, archive)
        else:
            run(["curl", "--fail", "--silent", "--show-error", "--location",
                 "--connect-timeout", "10", "--max-time", "180", "-o", archive, url], work)
        with archive.open("rb") as source:
            actual = hashlib.file_digest(source, "sha256").hexdigest()
        if actual != digest:
            raise ValueError(f"{name}: expected SHA256 {digest}, got {actual}")
        print(f"SHA256 {name} {actual}", flush=True)
        if archive.suffix == ".deb":
            run(["dpkg-deb", "-x", archive, work / "debian"], work)
        else:
            with tarfile.open(archive) as source:
                source.extractall(work / "sources", filter="data")


def configure(source, build, prefix, env, *options):
    build.mkdir()
    run([source / "configure", f"--prefix={prefix}", f"--libdir={prefix / 'lib'}",
         *options], build, env)


def cmake_archive(source, build, prefix, compiler, launcher, env, jobs, *options):
    cmake_env = dict(env, CFLAGS="", CXXFLAGS="")
    run(["cmake", "-S", source, "-B", build,
         f"-DCMAKE_INSTALL_PREFIX={prefix}", "-DCMAKE_INSTALL_LIBDIR=lib",
         "-DCMAKE_BUILD_TYPE=Release", f"-DCMAKE_C_COMPILER={compiler}",
         f"-DCMAKE_C_COMPILER_LAUNCHER={launcher or ''}", *options], source, cmake_env)
    run(["cmake", "--build", build, "--parallel", str(jobs), "--verbose"], source, cmake_env)
    run(["cmake", "--install", build], source, cmake_env)


def meson_archive(source, build, prefix, env, jobs, *options):
    run(["meson", "setup", build, source, f"--prefix={prefix}", "--libdir=lib",
         "--buildtype=release", "--default-library=static", "--wrap-mode=nodownload",
         *options], source, env)
    run(["meson", "compile", "-C", build, "-j", str(jobs), "-v"], source, env)
    run(["meson", "install", "-C", build, "--no-rebuild"], source, env)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, help="new writable archive SDK directory")
    parser.add_argument("--reference-sdk", type=Path, required=True,
                        help="historical SDK from legacy-linux-sdk.py")
    parser.add_argument("--archive-cache", type=Path,
                        help="directory with cached lockfile inputs, verified as downloads")
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    work = args.directory.resolve()
    if platform.system() != "Linux" or platform.machine() != "x86_64":
        parser.error("the pinned packages and archive recipes target Linux amd64")
    if not re.fullmatch(r"[a-zA-Z0-9_./-]+", str(work)):
        parser.error("the native build tools require a directory without spaces or shell metacharacters")
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    gcc = args.reference_sdk.resolve() / "gcc12/usr/bin/gcc-12"
    gxx = gcc.with_name("g++-12")
    if not gcc.is_file() or not gxx.is_file():
        parser.error(f"historical GCC/G++ 12 missing in {gcc.parent}")
    ccache = shutil.which("ccache")
    # The SDL wrapper's debug banner includes the compiler's date and time.
    # Use the pinned SDL 1.2.68 tarball's release directory timestamp.
    env = dict(os.environ, CC=f"{ccache} {gcc}" if ccache else str(gcc),
               CXX=f"{ccache} {gxx}" if ccache else str(gxx),
               CFLAGS=f"-O2 -fPIC -ffile-prefix-map={work}=/freej-archives",
               CXXFLAGS=f"-O2 -fPIC -ffile-prefix-map={work}=/freej-archives",
               CPPFLAGS="", LDFLAGS="", PKG_CONFIG_PATH="",
               LC_ALL="C", SOURCE_DATE_EPOCH="1695750228")
    for variable in ("DESTDIR", "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH",
                     "PKG_CONFIG_LIBDIR", "PKG_CONFIG_SYSROOT_DIR"):
        env.pop(variable, None)
    work.mkdir()
    unpack_inputs(work, args.archive_cache)
    source = work / "sources"
    build = work / "build"
    build.mkdir()
    prefixes = work / "prefixes"
    prefixes.mkdir()
    debian = work / "debian/usr"
    debian_lib = debian / "lib/x86_64-linux-gnu"
    for directory in (debian_lib / "pkgconfig", debian / "share/pkgconfig"):
        for module in directory.glob("*.pc"):
            if module.is_symlink():
                continue
            contents = module.read_text()
            module.write_text(contents.replace("/usr/", f"{debian}/")
                               .replace("=/usr\n", f"={debian}\n"))

    jack_env = dict(env, CPPFLAGS=f"-I{debian / 'include'}", LDFLAGS=f"-L{debian_lib}")
    jack_build = build / "jack"
    configure(source / "jack1-0.126.0", jack_build, prefixes / "jack", jack_env,
              "--enable-static", "--disable-shared", "--enable-force-install",
              "--bindir=/usr/bin")
    run(["make", "-C", "libjack", f"-j{args.jobs}", "V=1", "libjack.la"], jack_build, jack_env)
    run(["make", "-C", "libjack", "install-libLTLIBRARIES"], jack_build, jack_env)
    run(["make", "install-libjackincludeHEADERS", "install-pkgconfigDATA"], jack_build, jack_env)
    cmake_archive(source / "sdl12-compat-release-1.2.68", build / "sdl", prefixes / "sdl",
                  gcc, ccache, env, args.jobs, "-DSTATICDEVEL=ON", "-DSDL12TESTS=OFF",
                  "-DSDL12DEVEL=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
                  f"-DSDL2_INCLUDE_DIR={debian / 'include/SDL2'}",
                  f"-DSDL2_LIBRARY={debian_lib / 'libSDL2.a'}",
                  f"-DCMAKE_C_FLAGS=-I{debian / 'include/x86_64-linux-gnu'}")
    cmake_archive(source / "libsamplerate-0.2.2", build / "samplerate", prefixes / "samplerate",
                  gcc, ccache, env, args.jobs, "-DBUILD_SHARED_LIBS=OFF",
                  "-DBUILD_TESTING=OFF", "-DLIBSAMPLERATE_EXAMPLES=OFF",
                  "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")
    alsa_build = build / "alsa"
    configure(source / "alsa-lib-1.2.14", alsa_build, prefixes / "alsa", env,
              "--enable-static", "--disable-shared")
    run(["make", f"-j{args.jobs}", "V=1"], alsa_build, env)
    run(["make", "install"], alsa_build, env)
    sndio = source / "sndio-1.10.0"
    sndio_prefix = prefixes / "sndio"
    run([sndio / "configure", "--enable-static", f"--prefix={sndio_prefix}",
         f"--libdir={sndio_prefix / 'lib'}"], sndio, env)
    run(["make", "-C", "libsndio", f"-j{args.jobs}", f"CC={env['CC']}"], sndio, env)
    run(["make", "-C", "libsndio", "install"], sndio, env)
    pixman = prefixes / "pixman"
    meson_archive(source / "pixman-0.44.0", build / "pixman", pixman, env, args.jobs,
                  "-Dtests=disabled", "-Ddemos=disabled")
    cairo_env = dict(env, PKG_CONFIG_PATH=os.pathsep.join(map(str, (
        pixman / "lib/pkgconfig", debian_lib / "pkgconfig", debian / "share/pkgconfig"))))
    meson_archive(source / "cairo-1.18.4", build / "cairo", prefixes / "cairo",
                  cairo_env, args.jobs, "-Dprefer_static=true", "-Dtests=disabled", "-Dxlib=enabled",
                  "-Dxcb=enabled", "-Dfontconfig=enabled", "-Dfreetype=enabled",
                  "-Dpng=enabled", "-Dzlib=enabled")


if __name__ == "__main__":
    main()
