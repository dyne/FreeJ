#!/usr/bin/env python3
"""Check installed static artifacts with external X11 and PipeWire services."""

import argparse
import ctypes as C
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import zlib

from native_profile_services import pipewire_dummy, owned, until, x11


COLOR = 0x2060A0
EFFECT_COLOR = 0x7F7F7F
EFFECT_SCRIPT = """
if (list_filters().sort().join(",") != "Brightness,Invert0r")
    throw "unexpected built-in filter inventory";
var source = new GeneratorLayer(64, 48, "onecol0r");
if (!add_layer(source)) throw "built-in source setup failed";
var invert = new Filter("Invert0r");
var brightness = new Filter("Brightness");
source.add_filter(invert);
source.add_filter(brightness);
brightness.set_parameter("Brightness", 0.25);
source.start();
set_clear_all();
echo("built-in effects show loaded");
"""


class WindowAttributes(C.Structure):
    _fields_ = [
        *[(name, C.c_int) for name in ("x", "y", "width", "height", "border_width", "depth")],
        ("visual", C.c_void_p), ("root", C.c_ulong),
        *[(name, C.c_int) for name in ("window_class", "bit_gravity", "win_gravity", "backing_store")],
        ("backing_planes", C.c_ulong), ("backing_pixel", C.c_ulong), ("save_under", C.c_int),
        ("colormap", C.c_ulong), ("map_installed", C.c_int), ("map_state", C.c_int),
        *[(name, C.c_long) for name in ("all_event_masks", "your_event_mask",
                                        "do_not_propagate_mask")],
        ("override_redirect", C.c_int), ("screen", C.c_void_p),
    ]


class Display:
    def __init__(self, display):
        self.x = C.CDLL("libX11.so.6")
        pointer, ulong, uint, integer = C.c_void_p, C.c_ulong, C.c_uint, C.c_int
        for name, result, args in (
            ("XOpenDisplay", pointer, (C.c_char_p,)),
            ("XDefaultRootWindow", ulong, (pointer,)),
            ("XGetImage", pointer, (pointer, ulong, integer, integer, uint, uint, ulong, integer)),
            ("XGetPixel", ulong, (pointer, integer, integer)),
            ("XDestroyImage", integer, (pointer,)),
            ("XQueryTree", integer, (pointer, ulong, C.POINTER(ulong), C.POINTER(ulong),
                                     C.POINTER(C.POINTER(ulong)), C.POINTER(uint))),
            ("XGetWindowAttributes", integer, (pointer, ulong, C.POINTER(WindowAttributes))),
            ("XFetchName", integer, (pointer, ulong, C.POINTER(pointer))),
            ("XInternAtom", ulong, (pointer, C.c_char_p, integer)),
            ("XGetWindowProperty", integer, (pointer, ulong, ulong, C.c_long, C.c_long, integer, ulong,
                                              C.POINTER(ulong), C.POINTER(integer), C.POINTER(ulong),
                                              C.POINTER(ulong), C.POINTER(pointer))),
            ("XFree", integer, (pointer,)),
            ("XCloseDisplay", integer, (pointer,)),
        ):
            method = getattr(self.x, name)
            method.restype, method.argtypes = result, args
        self.ptr = self.x.XOpenDisplay(display.encode())
        if not self.ptr:
            raise ValueError(f"could not open {display}")
        self.root = self.x.XDefaultRootWindow(self.ptr)

    def windows(self):
        root, parent, children, count = C.c_ulong(), C.c_ulong(), C.POINTER(C.c_ulong)(), C.c_uint()
        ok = self.x.XQueryTree(self.ptr, self.root, C.byref(root), C.byref(parent),
                               C.byref(children), C.byref(count))
        visible = []
        try:
            if ok:
                for window in children[:count.value] if count.value else ():
                    attributes = WindowAttributes()
                    if (self.x.XGetWindowAttributes(self.ptr, window, C.byref(attributes))
                            and attributes.map_state == 2):
                        visible.append(window)
        finally:
            if children:
                self.x.XFree(children)
        return visible

    def window_title(self, window):
        title, encoding, field_width = C.c_void_p(), C.c_ulong(), C.c_int()
        items, remaining = C.c_ulong(), C.c_ulong()
        utf8_name = self.x.XInternAtom(self.ptr, b"_NET_WM_NAME", 1)
        if utf8_name and not self.x.XGetWindowProperty(
                self.ptr, window, utf8_name, 0, 256, 0, 0, C.byref(encoding), C.byref(field_width),
                C.byref(items), C.byref(remaining), C.byref(title)) and title:
            try:
                if field_width.value == 8:
                    return C.string_at(title, items.value).decode(errors="replace")
            finally:
                self.x.XFree(title)
        if not self.x.XFetchName(self.ptr, window, C.byref(title)):
            return ""
        try:
            return C.string_at(title).decode(errors="replace")
        finally:
            self.x.XFree(title)

    def contains_image(self, color=COLOR):
        viewport = next((window for window in self.windows()
                         if self.window_title(window).startswith("freej ")), None)
        if viewport is None:
            return False
        geometry = WindowAttributes()
        if not self.x.XGetWindowAttributes(self.ptr, viewport, C.byref(geometry)):
            return False
        image = self.x.XGetImage(self.ptr, viewport, 0, 0, geometry.width, geometry.height,
                                 C.c_ulong(-1).value, 2)
        if not image:
            raise ValueError("could not read X11 pixels")
        try:
            return sum(self.x.XGetPixel(image, x, y) == color
                       for y in range(0, geometry.height, 12)
                       for x in range(0, geometry.width, 12)) >= 4
        finally:
            self.x.XDestroyImage(image)


def fixture(work):
    def chunk(name, data):
        return struct.pack(">I", len(data)) + name + data + struct.pack(">I", zlib.crc32(name + data))

    row = b"\0" + COLOR.to_bytes(3, "big") * 64
    image = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 64, 48, 8, 2, 0, 0, 0))
    (work / "image.png").write_bytes(image + chunk(b"IDAT", zlib.compress(row * 48)) + chunk(b"IEND", b""))
    (work / "show.js").write_text("""
var image = new ImageLayer();
image.open("image.png");
image.start();
if (!add_layer(image)) throw "image setup failed";
set_clear_all();
set_fps(20);
for (var tick = 0; tick < 24; ++tick) run();
echo("static profile presented 24 frames");
quit();
""".lstrip())


def check_static(program, args, opts, work, env, name, inspect, exit_code):
    instance = work / name
    instance.mkdir()
    staged = shutil.copy2(program, instance / Path(program).name)
    home = instance / "home"
    home.mkdir()
    if name == "cli":
        fixture(instance)
    elif name == "cli-effects":
        (instance / "show.js").write_text(EFFECT_SCRIPT + """
set_fps(20);
for (var tick = 0; tick < 24; ++tick) run();
quit();
""")
    elf = subprocess.check_output(["readelf", "-dW", "-lW", staged], text=True, cwd=instance)
    (instance / "elf.txt").write_text(elf)
    if "(NEEDED)" in elf or re.search(r"^\s*INTERP\b", elf, re.M):
        raise ValueError(f"{name}: dynamic program dependency")
    trace = instance / f"{name}.trace"
    command = [str(args.strace), "--kill-on-exit", "-f", "-s", "4096", "-e", "trace=%file",
               "-o", str(trace), str(args.sandbox), str(instance), str(staged), *opts]
    with owned(command, instance, dict(env, HOME=str(home)), name) as process:
        if inspect is not None:
            inspect(process)
        if exit_code is not None and process.wait(timeout=12) != exit_code:
            raise ValueError(f"{name}: unexpected exit {process.returncode}")
    output = (instance / f"{name}.log").read_text(errors="replace")
    if output.count("Initializing the FreeJ engine") != 1:
        raise ValueError(f"{name}: expected one engine initialization")
    if name == "cli" and "static profile presented 24 frames" not in output:
        raise ValueError("static CLI did not complete the image fixture")
    if name == "cli-effects" and "built-in effects show loaded" not in output:
        raise ValueError(f"{name}: could not load the built-in effects show")
    if name == "factory" and "Installed PipeWire client captured at 48000 Hz / 2048 frames" not in output:
        raise ValueError("installed static client did not connect to the PipeWire endpoint")
    accesses = trace.read_text(errors="replace")
    effect_path = (r'"[^"]+\.(?:so(?:\.[^"/]*)?|frf|dylib)"|'
                   r'"(?:[^"]*/)?(?:frei0r-1|FreeFrame|\.freej/plugins)(?:/[^"]*)?"')
    if re.search(effect_path, accesses):
        raise ValueError(f"{name}: external library or effect lookup in {trace}")
    print(f"PASS {name}", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    for option in ("sandbox", "strace", "freej", "factory", "pipewire", "pulse"):
        parser.add_argument(f"--{option}", type=Path, required=True)
    args = parser.parse_args()
    for option, path in vars(args).items():
        setattr(args, option, path.resolve())
    work = args.output
    work.mkdir()
    env = dict(os.environ, LC_ALL="C", SDL_VIDEODRIVER="x11")
    with x11(work, env) as name, pipewire_dummy(args.pipewire, work, env) as env, \
            owned([args.pulse], work, env, "pulse") as pulse:
        until(lambda: "replay_pcm:pulse" in (work / "pulse.log").read_text(), pulse)
        for variable in ("LD_LIBRARY_PATH", "LD_PRELOAD"):
            env.pop(variable, None)
        env["DISPLAY"] = name
        display = Display(name)
        try:
            cli = ["-c", "-D", "2", "-s", "80x60", "-j", "show.js"]
            check_static(args.freej, args, cli, work, env, "cli",
                         lambda process: until(display.contains_image, process), 1)
            check_static(args.freej, args, cli, work, env, "cli-effects",
                         lambda process: until(lambda: display.contains_image(EFFECT_COLOR), process), 1)
            check_static(args.factory, args, ["--pipewire"], work, env, "factory", None, 0)
        finally:
            display.x.XCloseDisplay(display.ptr)
    print("3 executed, 0 failed, 0 skipped")


if __name__ == "__main__":
    main()
