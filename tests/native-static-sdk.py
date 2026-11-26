#!/usr/bin/env python3
"""Build the pinned Linux SDL/X11 and JACK stage SDK in a fresh directory."""

import argparse
from hashlib import file_digest
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tarfile


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
        filename = url.rsplit("/", 1)[1]
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
    for name in ("sdl12-compat-release-1.2.68", "jack1-0.126.0"):
        shutil.copytree(archives / "sources" / name, sources / name, symlinks=True)
    run(["patch", "--batch", "--forward", "-p1", "-i", Path(__file__).with_suffix(".patch")], sources)
    alsa = archives / "prefixes/alsa"
    env = dict(os.environ, CC=cc, CXX=str(gxx), LC_ALL="C", SOURCE_DATE_EPOCH="1695750228",
               PKG_CONFIG_PATH=os.pathsep.join(map(str, pcdirs)))
    for variable in ("CFLAGS", "CXXFLAGS", "CPPFLAGS", "LDFLAGS", "DESTDIR", "CPATH",
                     "LIBRARY_PATH", "PKG_CONFIG", "PKG_CONFIG_LIBDIR", "PKG_CONFIG_SYSROOT_DIR"):
        env.pop(variable, None)

    sdl2 = work / "sdl2"
    sb = work / "sdl2-build"
    x11_private = ";".join(str(common_lib / f"lib{name}.a") for name in ("xcb", "Xau", "Xdmcp"))
    sdl_env = dict(env, PKG_CONFIG_PATH=f"{alsa}/lib/pkgconfig:{env['PKG_CONFIG_PATH']}")
    sdl_flags = [f"-DCMAKE_C_COMPILER={gcc}", f"-DCMAKE_C_COMPILER_LAUNCHER={ccache or ''}",
                 "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
                 "-DCMAKE_C_FLAGS=-DFREEJ_STATIC_SDL2"]
    run(["cmake", "-S", sources / "SDL2-2.32.4", "-B", sb,
         f"-DCMAKE_INSTALL_PREFIX={sdl2}", "-DCMAKE_INSTALL_LIBDIR=lib", *sdl_flags,
         f"-DFREEJ_X11_PRIVATE_LIBS={x11_private}", f"-DCMAKE_PREFIX_PATH={alsa};{main_usr};{private_usr}",
         "-DSDL_SHARED=OFF", "-DSDL_STATIC=ON", "-DSDL_TESTS=OFF", "-DSDL_TEST_LIBRARY=OFF",
         "-DSDL_LOADSO=OFF", "-DSDL_ALSA=ON", "-DSDL_ALSA_SHARED=OFF", "-DSDL_JACK=OFF",
         "-DSDL_PULSEAUDIO=OFF", "-DSDL_PIPEWIRE=OFF", "-DSDL_SNDIO=OFF",
         "-DSDL_X11=ON", "-DSDL_X11_SHARED=OFF", "-DSDL_WAYLAND=OFF", "-DSDL_KMSDRM=OFF",
         "-DSDL_RPI=OFF", "-DSDL_VULKAN=OFF", "-DSDL_OPENGL=OFF", "-DSDL_OPENGLES=OFF",
         "-DSDL_LIBUDEV=OFF"], work, sdl_env)
    run(["cmake", "--build", sb, "--parallel", str(jobs)], work, sdl_env)
    run(["cmake", "--install", sb], work, sdl_env)
    sdl2_archive = sdl2 / "lib/libSDL2.a"
    names = subprocess.check_output(["nm", "-g", "--defined-only", sdl2_archive], text=True)
    symbols = sorted(set(re.findall(r"^\w+ \w (SDL_\w+)$", names, re.M)))
    if "SDL_Init" not in symbols:
        raise ValueError("the SDL2 archive has no SDL_Init")
    mapping = work / "sdl2.names"
    mapping.write_text("".join(f"{name} FreejSDL2_{name[4:]}\n" for name in symbols))

    wrapper = work / "sdl12-build"
    run(["cmake", "-S", sources / "sdl12-compat-release-1.2.68", "-B", wrapper, *sdl_flags,
         f"-DSDL2_INCLUDE_DIR={sdl2 / 'include/SDL2'}", f"-DSDL2_LIBRARY={sdl2_archive}",
         "-DSTATICDEVEL=ON", "-DSDL12TESTS=OFF"], work, env)
    run(["cmake", "--build", wrapper, "--parallel", str(jobs), "--target", "SDL-static"], work, env)

    jb = work / "jack-build"
    jb.mkdir()
    jack_env = dict(env, CFLAGS=f"-O2 -fPIC -ffile-prefix-map={work}=/freej-stage",
                    CPPFLAGS=f"-I{main_usr / 'include'}", LDFLAGS=f"-L{common_lib}")
    run([sources / "jack1-0.126.0/configure", f"--prefix={work / 'jack'}", "--bindir=/usr/bin",
         "--enable-static", "--disable-shared", "--enable-force-install"], jb, jack_env)
    run(["make", "-C", "libjack", f"-j{jobs}", "libjack.la"], jb, jack_env)

    libraries = work / "lib"
    libraries.mkdir()
    for directory in (*sorted((archives / "prefixes").glob("*/lib")),
                      common_lib, legacy / f"debian/usr/lib/{MULTIARCH}", legacy / "ffmpeg/lib",
                      private_lib):
        for archive in directory.glob("lib*.a"):
            shutil.copyfile(archive, libraries / archive.name)
    shutil.copyfile(jb / "libjack/.libs/libjack.a", libraries / "libjack.a")
    shutil.copyfile(wrapper / "libSDL.a", libraries / "libSDL.a")
    run(["objcopy", f"--redefine-syms={mapping}", sdl2_archive, libraries / "libFreejSDL2.a"], work)


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
