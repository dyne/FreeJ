#!/usr/bin/env python3
"""Exercise the archive dependencies used by the Linux compatibility build."""

import argparse
from dataclasses import dataclass
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
from textwrap import dedent


@dataclass(frozen=True)
class Consumer:
    name: str
    capability: str
    modules: tuple[str, ...]
    source: str
    extra: tuple[str, ...] = ()
    private_modules: tuple[str, ...] = ()
    cxx: bool = False
    sdl: bool = False


def probe(name, capability, modules, headers, body, extra=(), **kwargs):
    return Consumer(name, capability, tuple(modules.split()),
                    "".join(f"#include <{header}>\n" for header in headers.split())
                    + "int main(void) {\n" + dedent(body).strip() + "\n}\n",
                    extra, **kwargs)


CONSUMERS = (
    Consumer("sdl", "SDL 1.2 video and events", ("sdl",), dedent(r"""
        #include <SDL.h>
        #include <link.h>
        #include <stdio.h>
        #include <string.h>
        static int backend(struct dl_phdr_info *info, size_t size, void *found) {
            (void) size;
            if (strstr(info->dlpi_name, "libSDL2")) {
                puts(info->dlpi_name);
                *(int *)found = 1;
            }
            return 0;
        }
        int main(int argc, char **argv) {
            char name[64];
            int found = 0;
            if (argc != 2 || SDL_Init(SDL_INIT_VIDEO) != 0) {
                fprintf(stderr, "%s\n", SDL_GetError());
                return 1;
            }
            SDL_Surface *s = SDL_SetVideoMode(16, 16, 32, SDL_SWSURFACE);
            if (!s || SDL_FillRect(s, NULL, SDL_MapRGB(s->format, 255, 0, 0))
                || SDL_Flip(s) || !SDL_VideoDriverName(name, sizeof(name)))
                return 2;
            printf("SDL video driver: %s\n", name);
            dl_iterate_phdr(backend, &found);
            SDL_Quit();
            return strcmp(name, argv[1]) != 0 || !found;
        }
        """), sdl=True),
    probe("sdl-image", "image layers", "SDL_image", "SDL.h SDL_image.h",
          """int codecs = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
             int result = IMG_Init(codecs);
             IMG_Quit();
             return (result & codecs) != codecs;""",
          ("-lpng16", "-ljpeg", "-ltiff", "-lwebp", "-lsharpyuv", "-ljbig",
           "-ldeflate", "-llzma", "-lLerc", "-lzstd", "-lz", "-lm", "-ldl", "-lstdc++"),
          sdl=True),
    probe("sdl-gfx", "geometry and zoom", "SDL_gfx", "SDL.h SDL_gfxPrimitives.h",
          """SDL_Surface *s = SDL_CreateRGBSurface(0, 16, 16, 32, 0, 0, 0, 0);
             if (!s) return 1;
             int status = pixelColor(s, 2, 2, 0xff0000ff);
             SDL_FreeSurface(s);
             return status != 0;""", ("-lm", "-ldl"), sdl=True),
    probe("sdl-ttf", "text rasterization", "SDL_ttf", "SDL_ttf.h",
          """if (TTF_Init()) return 1;
             TTF_Quit();
             return 0;""",
          ("-lfreetype", "-lbz2", "-lpng16", "-lbrotlidec", "-lbrotlicommon",
           "-lz", "-lm", "-ldl"), sdl=True),
    probe("slang", "terminal front end", "slang", "slang.h",
          """return SLang_init_all() != 0;"""),
    probe("slw", "terminal console widgets", "slang", "slw_console.h",
          """SLangConsole console;
             return 0;""", ("@slw", "@engine"), cxx=True),
    probe("ccvt", "YUV sources and encoding", "", "ccvt.h",
          """const unsigned char yuv[6] = { 16, 16, 16, 16, 128, 128 };
             unsigned char bgr[16];
             ccvt_420p_bgr32(2, 2, yuv, bgr);
             return bgr[0] != 16 || bgr[1] != 16 || bgr[2] != 16;""", ("@ccvt",)),
    probe("mozjs", "show scripting", "", "jsapi.h",
          """JSRuntime *runtime = JS_NewRuntime(1024 * 1024);
             if (!runtime) return 1;
             JSContext *context = JS_NewContext(runtime, 8192);
             if (!context) return 2;
             JS_DestroyContext(context);
             JS_DestroyRuntime(runtime);
             JS_ShutDown();
             return 0;""", ("@mozjs", "-lm", "-ldl", "-lpthread", "-lstdc++")),
    probe("avcodec", "movie decoding", "libavcodec", "libavcodec/avcodec.h",
          """avcodec_register_all();
             return avcodec_find_decoder(CODEC_ID_MPEG4) == 0;"""),
    probe("avformat", "movie input and demux", "libavformat", "libavformat/avformat.h",
          """av_register_all();
             return av_find_input_format("avi") == 0;""",
          ("-ldb", "-ldl", "-lpthread")),
    probe("avutil", "movie frames and buffers", "libavutil", "libavutil/mem.h",
          """void *buffer = av_malloc(64);
             if (!buffer) return 1;
             av_free(buffer);
             return 0;"""),
    probe("swscale", "movie pixel conversion", "libswscale", "libswscale/swscale.h",
          """struct SwsContext *ctx = sws_getContext(16, 16, PIX_FMT_YUV420P,
                 16, 16, PIX_FMT_RGB24, SWS_BILINEAR, 0, 0, 0);
             if (!ctx) return 1;
             sws_freeContext(ctx);
             return 0;"""),
    probe("ogg", "Ogg stream mux", "ogg", "ogg/ogg.h",
          """ogg_stream_state stream;
             if (ogg_stream_init(&stream, 1)) return 1;
             return ogg_stream_clear(&stream) != 0;"""),
    probe("vorbis", "Vorbis audio codec", "vorbis", "vorbis/codec.h",
          """vorbis_info info;
             vorbis_info_init(&info);
             vorbis_info_clear(&info);
             return 0;"""),
    probe("vorbisenc", "Vorbis recording", "vorbisenc", "vorbis/vorbisenc.h",
          """vorbis_info info;
             vorbis_info_init(&info);
             int status = vorbis_encode_init_vbr(&info, 1, 48000, 0.3f);
             vorbis_info_clear(&info);
             return status != 0;"""),
    probe("theora", "Theora video recording", "theora", "theora/theora.h",
          """theora_info info;
             theora_info_init(&info);
             info.width = info.height = info.frame_width = info.frame_height = 16;
             info.fps_numerator = 25;
             info.fps_denominator = info.aspect_numerator = info.aspect_denominator = 1;
             info.quality = 24;
             theora_state state;
             int status = theora_encode_init(&state, &info);
             if (!status) theora_clear(&state);
             theora_info_clear(&info);
             return status != 0;""", ("-lm",)),
    probe("shout", "Icecast streaming", "shout", "shout/shout.h",
          """shout_t *stream = shout_new();
             if (!stream) return 1;
             int status = shout_set_host(stream, "127.0.0.1");
             shout_free(stream);
             return status != SHOUTERR_SUCCESS;""", private_modules=("cairo",)),
    probe("jack", "JACK capture and playback", "jack",
          "jack/jack.h jack/ringbuffer.h stdio.h unistd.h",
          """jack_ringbuffer_t *ring = jack_ringbuffer_create(32);
             if (!ring) return 1;
             size_t written = jack_ringbuffer_write(ring, "ok", 2);
             jack_ringbuffer_free(ring);
             char server[64];
             snprintf(server, sizeof(server), "freej-archive-probe-%ld", (long)getpid());
             jack_status_t status = 0;
             jack_client_t *client = jack_client_open("freej-archive-probe",
                 JackNoStartServer | JackServerName, &status, server);
             printf("JACK client status: 0x%x\\n", (unsigned)status);
             if (client) {
                 jack_client_close(client);
                 return 2;
             }
             return written != 2 || !(status & JackFailure);""",
          ("-ldb", "-lm", "-ldl")),
    probe("fftw", "audio spectrum FFT", "fftw3", "fftw3.h",
          """double input[8] = { 1 };
             fftw_complex output[5];
             fftw_plan plan = fftw_plan_dft_r2c_1d(8, input, output, FFTW_ESTIMATE);
             if (!plan) return 1;
             fftw_execute(plan);
             fftw_destroy_plan(plan);
             return output[0][0] != 1;"""),
    probe("samplerate", "audio resampling", "samplerate", "samplerate.h",
          """float input[64] = { 0.5f };
             float output[64] = { 0 };
             SRC_DATA data = { .data_in = input, .data_out = output, .input_frames = 64,
                 .output_frames = 64, .src_ratio = 1, .end_of_input = 1 };
             return src_simple(&data, SRC_LINEAR, 1) || data.output_frames_gen != 64;"""),
    probe("alsa", "ALSA sequencer MIDI", "alsa", "alsa/asoundlib.h",
          """snd_seq_t *seq = 0;
             int status = snd_seq_open(&seq, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
             if (!status) snd_seq_close(seq);
             return snd_asoundlib_version() == 0;""", ("-ldl",)),
    probe("sndio", "historical FFmpeg audio-device SDK edge", "sndio alsa", "sndio.h",
          """struct sio_par params;
             sio_initpar(&params);
             return params.sig != (unsigned)-1;""", ("-ldl",)),
    probe("liblo", "OSC controller transport", "liblo", "lo/lo.h",
          """lo_address address = lo_address_new("127.0.0.1", "12345");
             if (!address) return 1;
             lo_address_free(address);
             return 0;""", ("-lm", "-lpthread")),
    probe("freetype", "font rasterization", "freetype2", "ft2build.h freetype/freetype.h",
          """FT_Library library;
             if (FT_Init_FreeType(&library)) return 1;
             return FT_Done_FreeType(library) != 0;"""),
    probe("fontconfig", "font discovery", "fontconfig", "fontconfig/fontconfig.h",
          """FcPattern *pattern = FcPatternCreate();
             if (!pattern) return 1;
             FcPatternDestroy(pattern);
             return 0;"""),
    probe("x11", "X11 screen capture", "x11", "X11/Xlib.h",
          """Display *display = XOpenDisplay(0);
             if (!display) return 1;
             XCloseDisplay(display);
             return 0;"""),
    probe("cairo", "vector drawing", "cairo", "cairo.h",
          """cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2, 2);
             cairo_t *ctx = cairo_create(surface);
             cairo_set_source_rgb(ctx, 1, 0, 0);
             cairo_paint(ctx);
             int result = cairo_status(ctx);
             cairo_destroy(ctx);
             cairo_surface_destroy(surface);
             return result != CAIRO_STATUS_SUCCESS;"""),
    probe("gd", "PNG screenshots", "gdlib", "gd.h",
          """gdImagePtr image = gdImageCreateTrueColor(2, 2);
             if (!image) return 1;
             gdImageSetPixel(image, 0, 0, 0xff0000);
             int length = 0;
             unsigned char *png = gdImagePngPtr(image, &length);
             int result = length < 8 || !png || png[0] != 137;
             gdFree(png);
             gdImageDestroy(image);
             return result;""", ("-lgd", "-lpng16", "-lz", "-lm")),
    probe("v4l", "kernel video-capture ABI", "",
          "errno.h fcntl.h linux/videodev2.h sys/ioctl.h unistd.h",
          """struct v4l2_capability cap;
             int fd = open("/dev/null", O_RDONLY);
             if (fd < 0) return 1;
             int status = ioctl(fd, VIDIOC_QUERYCAP, &cap);
             int error = errno;
             close(fd);
             return status != -1 || error != ENOTTY;"""),
)


TOOLCHAIN_LIBS = ("-ldl", "-lm", "-lpthread", "-lrt", "-lstdc++")
MISSING_CONTROL = {"sdl": "SDL", "jack": "jack", "samplerate": "samplerate"}


def utc():
    return datetime.now(timezone.utc).isoformat()


def evidence(path):
    with path.open("rb") as source:
        return {"path": str(path), "sha256": hashlib.file_digest(source, "sha256").hexdigest(),
                "bytes": path.stat().st_size}


def run(command, work, steps, env, expected=0):
    command = list(map(str, command))
    log = work / f"{len(steps):02}-{Path(command[0]).name}.log"
    observed = utc()
    with log.open("wb") as output:
        result = subprocess.run(command, cwd=work, env=env, stdout=output,
                                stderr=output, timeout=45)
    step = {"command": command, "cwd": str(work), "exit": result.returncode,
            "expectedExit": expected, "startedAt": observed, "observedAt": utc(),
            "reference": evidence(log)}
    if "SDL_VIDEODRIVER" in env:
        step["SDL_VIDEODRIVER"] = env["SDL_VIDEODRIVER"]
    steps.append(step)
    contents = log.read_text(errors="replace")
    if expected is not None and result.returncode != expected:
        raise ValueError(f"{shlex.join(command)} exited {result.returncode}: {log}")
    return result.returncode, contents


def find_archive(lib, directories, consumer):
    archive = f"lib{lib}.a"
    for directory in directories:
        path = directory / archive
        if path.is_file():
            return path
    raise ValueError(f"{consumer.name}: {consumer.capability} requires {archive} in the staged SDK")


def archive_flags(case, metadata, libdirs, bundled):
    flags = []
    for flag in dict.fromkeys([*metadata, *case.extra]):
        if flag.startswith("-L"):
            continue
        if flag.startswith("@"):
            flags.append(bundled[flag[1:]])
        elif flag in TOOLCHAIN_LIBS or flag == "-pthread":
            flags.append(flag)
        elif flag.startswith("-l"):
            flags.append(find_archive(flag[2:], libdirs, case))
        else:
            raise ValueError(f"{case.name}: unsupported pkg-config linker flag {flag}")
    return flags


def check_shared_fallback(case, lib, work, steps, env, compiler, obj, flags, libdirs):
    candidate = Path(f"/usr/lib/x86_64-linux-gnu/lib{lib}.so")
    if not candidate.is_file():
        raise ValueError(f"missing test prerequisite {candidate}")
    prefix = work / "shared-only"
    prefix.mkdir()
    (prefix / candidate.name).symlink_to(candidate)
    archive = find_archive(lib, libdirs, case)
    fallback_flags = [arg for arg in flags if arg != archive]
    # Put the real system library ahead of the closure so -l resolves lazily as
    # it does in a default native link. The same directory has no archive.
    build = [compiler, obj, f"-L{prefix}", f"-l{lib}",
             "-Wl,--start-group", *fallback_flags, "-Wl,--end-group"]
    shared = work / "shared-control"
    run([*build, "-o", shared], work, steps, env)
    _, dynamic = run(["readelf", "-dW", shared], work, steps, env)
    if not re.search(rf"NEEDED.*\[lib{re.escape(lib)}(?:[-.]|\])", dynamic):
        raise ValueError(f"{case.name}: shared control did not select the packaged library")
    exit_code, output = run([*build, "-static", "-o", work / "missing-control"],
                            work, steps, env, expected=None)
    if exit_code == 0 or f"cannot find -l{lib}" not in output:
        raise ValueError(f"{case.name}: expected the native linker to reject the absent archive")
    try:
        find_archive(lib, [prefix], case)
    except ValueError as error:
        missing = str(error)
    else:
        raise ValueError(f"{case.name}: archive check accepted a shared library")
    (prefix / archive.name).symlink_to(archive)
    restored = [arg if arg != archive else find_archive(lib, [prefix], case) for arg in flags]
    run([compiler, obj, "-static", "-Wl,--start-group", *restored, "-Wl,--end-group",
         "-o", work / "restored"], work, steps, env)
    return {"missing": missing, "sharedArtifact": evidence(candidate.resolve()),
            "restoredArchive": evidence(archive), "restored": evidence(work / "restored")}


def check(case, work, args, steps, env, prefixes, libdirs, bundled):
    gcc = args.reference_sdk / "gcc12/usr/bin/gcc-12"
    compiler = gcc.with_name("g++-12") if case.cxx else gcc
    ccache = shutil.which("ccache")
    cflags, libs, metadata = [], [], {}
    if case.modules:
        # The archive for the actual GD PNG call graph needs only libpng/zlib.
        # Capture the whole .pc closure diagnostic for the unused HEIF/AVIF edges.
        options = ("--modversion", "--cflags", "--libs")
        for option in options:
            exit_code, out = run(["pkg-config", "--static", option, *case.modules],
                                 work, steps, env, expected=None if case.name == "gd" else 0)
            metadata[option] = out.strip()
            if exit_code and (case.name != "gd" or "libheif" not in out or "libavif" not in out):
                raise ValueError(f"unexpected metadata closure failure for {case.name}")
            if exit_code == 0:
                if option == "--cflags":
                    cflags = shlex.split(out)
                elif option == "--libs":
                    libs = shlex.split(out)
    if case.private_modules:
        _, out = run(["pkg-config", "--static", "--libs", *case.private_modules], work, steps, env)
        metadata["privateModules"] = {"names": case.private_modules, "linkFlags": out.strip()}
        libs.extend(shlex.split(out))

    repo = Path(__file__).resolve().parent.parent
    includes = [prefix / "include" for prefix in prefixes]
    includes += [args.archive_sdk / "debian/usr/include/x86_64-linux-gnu",
                 args.reference_sdk / "debian/usr/include/SDL",
                 repo / "lib/slw", repo / "src/include", repo / "lib/ccvt",
                 repo / "lib/javascript", args.compat_build / "lib/javascript"]
    source = work / ("probe.cc" if case.cxx else "probe.c")
    source.write_text(case.source)
    obj = work / "probe.o"
    run([*([ccache] if ccache else []), compiler, "-O2", "-fPIC", *cflags,
         *(f"-I{path}" for path in includes), "-c", source, "-o", obj], work, steps, env)

    flags = archive_flags(case, libs, libdirs, bundled)
    executable = work / "static-probe"
    linkmap = work / "link.map"
    run([compiler, "-static", obj, f"-Wl,-Map={linkmap}", "-Wl,--start-group",
         *flags, "-Wl,--end-group", "-o", executable], work, steps, env)
    _, dynamic = run(["readelf", "-dW", executable], work, steps, env)
    _, segments = run(["readelf", "-lW", executable], work, steps, env)
    if "(NEEDED)" in dynamic or re.search(r"^\s*INTERP\b", segments, re.M):
        raise ValueError(f"{case.name}: static link has a shared-library dependency or interpreter")
    archives = [evidence(flag) for flag in flags if isinstance(flag, Path)]
    # A GNU ld map names an archive member when it contributed to the link.
    linkmap_text = linkmap.read_text(errors="replace")
    for archive in archives:
        archive["contributed"] = archive["path"] + "(" in linkmap_text
    if archives and not archives[0]["contributed"]:
        raise ValueError(f"{case.name}: the first provider archive supplied no objects")
    result = {"metadata": metadata, "supplementalFlags": case.extra, "archives": archives,
              "map": evidence(linkmap), "binary": evidence(executable)}
    if case.sdl:
        # SDL 1.2.68's archive still dlopens SDL2. Use the host loader for this
        # runtime check and retain the separate fully archived link above.
        executable = work / "runtime-probe"
        run([compiler, obj, "-Wl,--start-group", *flags, "-Wl,--end-group",
             "-o", executable], work, steps, env)
        _, dynamic = run(["readelf", "-dW", executable], work, steps, env)
        if re.search(r"NEEDED.*libSDL", dynamic):
            raise ValueError(f"{case.name}: runtime probe directly linked shared SDL")
        if case.name == "sdl":
            backends = {}
            for driver in ("dummy", "x11"):
                _, output = run([executable, driver], work, steps, dict(env, SDL_VIDEODRIVER=driver))
                loaded = next(line for line in output.splitlines() if "libSDL2" in line)
                backends[driver] = evidence(Path(loaded).resolve(strict=True))
            result["sdl2Backends"] = backends
        else:
            run([executable], work, steps, dict(env, SDL_VIDEODRIVER="dummy"))
        result["runtimeBinary"] = evidence(executable)
    else:
        run([executable], work, steps, env)
    if case.name in MISSING_CONTROL:
        result["missingControl"] = check_shared_fallback(case, MISSING_CONTROL[case.name],
                                                         work, steps, env, compiler, obj,
                                                         flags, libdirs)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="new directory for probes and evidence")
    parser.add_argument("--reference-sdk", type=Path, required=True)
    parser.add_argument("--archive-sdk", type=Path, required=True)
    parser.add_argument("--compat-build", type=Path, required=True)
    parser.add_argument("--x11-display", required=True)
    parser.add_argument("--only", nargs="+", choices=[case.name for case in CONSUMERS])
    args = parser.parse_args()
    for path in ("output", "reference_sdk", "archive_sdk", "compat_build"):
        setattr(args, path, getattr(args, path).resolve())
    args.output.mkdir()
    prefixes = [*sorted((args.archive_sdk / "prefixes").iterdir()), args.reference_sdk / "ffmpeg",
                args.archive_sdk / "debian/usr", args.reference_sdk / "debian/usr"]
    libdirs = [prefix / "lib" for prefix in prefixes] + [
        prefix / "lib/x86_64-linux-gnu" for prefix in prefixes]
    pcdirs = [path / "pkgconfig" for path in libdirs] + [
        prefix / "share/pkgconfig" for prefix in prefixes]
    env = dict(os.environ, PKG_CONFIG_PATH="", PKG_CONFIG_SYSROOT_DIR="",
               PKG_CONFIG_LIBDIR=os.pathsep.join(map(str, pcdirs)), LC_ALL="C",
               DISPLAY=args.x11_display)
    for variable in ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH",
                     "LD_LIBRARY_PATH", "LD_PRELOAD", "SDL_VIDEODRIVER",
                     "PKG_CONFIG_ALLOW_SYSTEM_CFLAGS", "PKG_CONFIG_ALLOW_SYSTEM_LIBS"):
        env.pop(variable, None)
    environment = {key: env[key] for key in
                   ("LC_ALL", "DISPLAY", "PKG_CONFIG_LIBDIR", "PKG_CONFIG_PATH",
                    "PKG_CONFIG_SYSROOT_DIR")}
    (args.output / "environment.json").write_text(json.dumps(environment, indent=2) + "\n")
    bundled = {key: args.compat_build / relative for key, relative in (
        ("slw", "lib/slw/.libs/libslw.a"), ("engine", "src/.libs/libfreej.a"),
        ("ccvt", "lib/ccvt/.libs/libccvt.a"), ("mozjs", "lib/javascript/libmozjs.a"),
    )}
    manifest = []
    for case in CONSUMERS:
        if args.only and case.name not in args.only:
            continue
        work = args.output / case.name
        work.mkdir()
        steps = []
        row = {"consumer": case.name, "capability": case.capability, "steps": steps}
        try:
            row.update(check(case, work, args, steps, env, prefixes, libdirs, bundled))
        except (ValueError, OSError, subprocess.TimeoutExpired) as error:
            row["failure"] = str(error)
            print(f"FAIL {case.name}: {error}", flush=True)
        else:
            print(f"PASS {case.name}", flush=True)
        manifest.append(row)
        (args.output / "checks.json").write_text(json.dumps(manifest, indent=2) + "\n")
    failed = sum("failure" in row for row in manifest)
    print(f"{len(manifest)} executed, {failed} failed, 0 skipped")
    return int(bool(failed))


if __name__ == "__main__":
    raise SystemExit(main())
