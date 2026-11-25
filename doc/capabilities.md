# Historical Linux capability ledger

This ledger covers the assertion-enabled FreeJ 1.0 reference described in
[the Linux build recipe](legacy-reference.md). The observations below were
made on 2026-11-25 on Debian 13 amd64, using the pinned GCC 12.4 and FFmpeg
0.8.15 SDK and the bundled SpiderMonkey. Status describes this build and its
test environment. The source tree's `doc/architecture.md` identifies the
engine, adapter and frontend seams.

`README` and `USAGE` span older FreeJ releases. To reconcile their advertised
features with this checkout, follow `configure.ac`, the source lists in
`src/Makefile.am`, the registrations in `src/context.cpp` and
`src/jsparser.cpp`, and the staged artifacts.

## Engine, sources and display

| Capability | Frontend and invocation | Build guard and dependency | Reference status and fixture |
| --- | --- | --- | --- |
| Composition, ordered layers, per-layer effects and blits | CLI `freej -c -s 80x64 -j show.js`; JS `add_layer`, `Layer.add_filter`, `set_position`, `set_fps`, `run`; C++ `Context::cafudda` and `ViewPort` | Core, SDL 1.x, built-in color conversion; `context.cpp`, `screen.cpp`, `layer.cpp`, `blitter.cpp` | Xvfb presented the expected pixels from a local PNG. The startup script ran 24 ticks at 20 FPS. An image, geometry, text and vector fixture also composed into an 80×64 screenshot. |
| SDL and memory screens | CLI `-S sdl` (default), `-S soft`, `-s WxH`, `-F`, `-n`, `-f FPS`; JS `new Screen(type)` | `sdl_screen.cpp`, `soft_screen.cpp`; factory `Screen::sdl` and `Screen::soft` | SDL rendered on Xvfb and ran headlessly with the dummy driver. `-S soft` created an 80×64 memory viewport and ran the registry script. Fullscreen and SDL event behavior on a real display remain rig checks. |
| OpenGL and ASCII screens | CLI `-S sdlgl`, `-S gl`; AA factory ID `ascii` (help advertises `aa`) | `WITH_OPENGL`, `--enable-opengl`, GL/GLU; `WITH_AALIB` if AAlib probes succeed | Both disabled in the reference. Draw a local image through each screen on a compatible visual or terminal. In `freej.cpp`, the experimental `-g` flag only sets `opengl`; `-S` selects the screen. |
| Still images | CLI image filename or JS `new ImageLayer(); layer.open(path)`; `scripts/javascript/tests/image.js` | SDL_image and SDL 1.x, both required for this target. `IS_IMAGE_EXTENSION` lists CLI extensions. | Local RGBA PNG loaded by JS and by positional CLI argument. Other SDL_image formats remain untested. |
| File movies, URLs and FireWire/DV | Historical CLI paths and JS `new MovieLayer`; `video_layer.cpp` and `video_layer_js.cpp` | FFmpeg and swscale are linked under `WITH_FFMPEG` / `WITH_SWSCALE`. Commit `e8e2eb5f3447f569505e9317d119894d112d24c4` removed both movie translation units from the Linux target and commented out JS registration. | CLI help still advertises movies. A valid local MPEG-4 clip independently decoded, but FreeJ exited on SIGSEGV after looking for the absent `MovieLayer::ffmpeg` factory entry. `typeof MovieLayer` was `undefined`. Playback, seeking, URL and DV checks require a working decoder. |
| V4L2 capture | CLI `/dev/video0%160x120`; JS `new CamLayer()` and camera methods; see `cam_layer_js.cpp` | `WITH_V4L` detects `linux/videodev2.h`. Factory `CamLayer::v4l2` takes precedence when multiple implementations are enabled. | CamLayer registered. V4L2 camera, tuner/input selection, and capture-size negotiation need compatible hardware. |
| Alternate cameras | JS `CamLayer` via alternate factory implementations; historical CLI `layer_opencv_cam` case | `WITH_UNICAP` via `--enable-unicap` and libunicap; `WITH_OPENCV` via `--enable-opencv` and the historical OpenCV SDK | Neither selected. Exercise each implementation with its own build and camera. |
| X11 window capture | JS `new XGrabLayer(); layer.open(windowId)` | `WITH_XGRAB` and X11, `xgrab_layer.cpp` | Class registered, but the constructor probe reported `cannot create XGrabLayer`. It uses the generic factory constructor without a corresponding XGrab factory registration. Window capture requires that path and an X11 window. |
| XScreenSaver source | CLI executable path containing `xscreensaver`; adapter passes `-window-id` to the selected program | `WITH_XSCREENSAVER` and X11, `xscreensaver_layer.cpp` | Built. Requires an X server and a compatible external saver executable. Child start, capture and shutdown remain rig checks. |
| Text | CLI text file, S-Lang text command, or JS `TextLayer.print`, `font`, `size`; `text_scroller.js` | `WITH_TEXTLAYER` via fontconfig/FreeType; SDL_ttf and runtime fonts | Registered. The draw script instantiated and placed a text layer using the reference font setup. Font packages and exact font appearance are show dependencies. |
| Geometry and Cairo drawing | JS `new GeometryLayer(w,h)`, `rectangle_fill`; `new VectorLayer(w,h)`, `fillRect`; examples `acid_geo.js` and `vector_test.js` | Geometry uses SDL_gfx; vectors require Cairo and `WITH_CAIRO` | Both constructed and participated in the local draw script. Per-primitive rendering parity remains unverified. |
| Generators | JS `new GeneratorLayer()`, S-Lang generator command, provider generator list | Factory `GeneratorLayer::ff_f0r`; external FreeFrame/frei0r sources | JS class registered. The reference plugin fixture supplies an effect, so generator output remains a plugin rig check. |
| Screenshot | JS `new Screen().save_frame("draw.png")`; current screen via factory | `WITH_GD`, libgd; `screen_js.cpp` and `screen.cpp` | Wrote an 80×64 PNG that decoded successfully. |
| Flash animation | Historical CLI `.swf` and JS `FlashLayer` | `WITH_FLASH`, `--enable-flash`, bundled `lib/flash`; SWF v3 | Disabled. The class was absent from the runtime registry. Build with the flag and use a local compatible SWF fixture to test it. |

## Recording, streaming and controls

| Capability | Frontend and invocation | Build guard and dependency | Reference status and fixture |
| --- | --- | --- | --- |
| Ogg/Theora recording | JS `new VideoEncoder(50, 64000, 0, 0)`, `register_encoder`, `start_filesave`, `stop_filesave`; Qt encoder controls in `qt/qEncoder.cpp` | `WITH_OGGTHEORA`, Ogg, Theora, Vorbis/Vorbisenc, color conversion | Video-only recording from a moving PNG decoded strictly as 44 Theora frames at 80×64. All 44 decoded frame hashes differed. Live JACK audio recording and sync remain rig checks. |
| Icecast streaming | JS `VideoEncoder.stream_*`, `register_encoder`, `start_stream`, `stop_stream`; see the shipped `stream.js` example for API usage | Same Ogg/Theora path plus libshout and an Icecast server | Isolated Icecast 2.4.4 on loopback served HTTP 200 for `/reference.ogg`. The listener body and server dump matched and decoded strictly, with 95 Theora frames at 80×64. Audio, network loss and reconnection remain unverified. |
| Alternate encoder code | Native `FFmpegEncoder` in `src/ffmpeg_encoder.cpp`; `theora11_encoder.cpp` is a separate prototype | FFmpeg encoder translation unit is in the target; the active JS binding constructs `OggTheoraEncoder`. Theora 1.1 prototype is outside the target. | Source retained. The reference recording and streaming checks cover the Ogg/Theora binding. No active performer route was found for the alternate encoders. |
| JACK input, FFT and audio on encoded output | JS `new AudioJack(...)`, `fft()`, `VideoEncoder.add_audio`; `audio_test.js`, `freej_equalizer.js`; Qt `qJackClient` | `WITH_AUDIO`; JACK, FFTW, libsamplerate. CLI still parses `-a` into `audiomode`, which has no initialization consumer in `freej.cpp`. | AudioJack registered. Live JACK input, DSP routing, stereo buffer delivery and muxed audio need a JACK rig. The architecture map records the commented collector registration/copy path. |
| S-Lang console | CLI attached to a terminal, unless `-c`; help key `?` or Ctrl-H; source `console_readline_ctrl.cpp` | S-Lang required, bundled `lib/slw` | PTY drew the UI and responded to `?` with help. The interrupt probe exited without observing the confirmation prompt described in `USAGE`. Use current in-app help for key bindings. |
| SDL keyboard, mouse and joystick | JS `KeyboardController`, `MouseController`, `JoystickController`; register callbacks via `register_controller`. Examples include `keyboard.js`, `mouse_layerctl.js` and `pan_joy.js`. | SDL events, `kbd_ctrl.cpp`, `mouse_ctrl.cpp`, `joy_ctrl.cpp` | Classes registered. A real display, keyboard, pointing device and joystick are rig checks for event delivery and mapping. |
| ViMo video mouse | JS `new ViMoController("/dev/ttyUSB0")`, `register_controller`; example `vimo.js` | Serial device, `vimo_ctrl.cpp` | Class registered. A ViMo controller and usable serial endpoint are needed to verify wheel, button and transport events. |
| Per-frame scripted control | JS `new TriggerController()`, `trigger.frame = function () { … }` and `register_controller(trigger)` | Core trigger factory and bundled JS | Registered. The paced show tested normal JS frame steps. Trigger callback ordering remains unverified. |
| MIDI | JS `MidiController`, `scripts/javascript/examples/midi.js` | `WITH_MIDI` if ALSA found | Class registered. An ALSA sequencer endpoint and MIDI device are needed to test mapping and delivery. |
| WiiMote | JS `WiiController`, `scripts/javascript/examples/wiimote.js` | `WITH_CWIID` if libcwiid probes succeed; Bluetooth hardware | Probe did not find libcwiid. Class absent. Pairing and input require the compatible SDK and a WiiMote rig. |
| OSC | JS `OscController`, `add_method`, `send_to`, `send`; `osc_test.js` | liblo required, `osc_ctrl.cpp` | Loopback received the expected datagram from the bound source port. Incoming test packet reached dispatch, but the JS callback failed. Argument marshalling and routing remain separate checks. |

## Plugins, frontends and embedding

| Capability | Frontend and invocation | Build guard and dependency | Reference status and fixture |
| --- | --- | --- | --- |
| frei0r dynamic effects | Put a shared plugin in the configured prefix's `lib/freej`, `~/.freej/plugins`, or a documented system path; JS `list_filters()`, `new Filter(name)` | `WITH_FREI0R`, dlopen and bundled header; `plugger.cpp`, `frei0r.cpp` | A pinned sibling Invert0r shared object was discovered in an isolated home. No effect binaries were installed by FreeJ. Provider rejects mixer2/mixer3 in this reference; generator and effect parity need their own fixtures. |
| FreeFrame | Legacy `plugMain` plugin, e.g. a Linux `.so` in the same plugin directories | `WITH_FREEFRAME`, dlopen, bundled FreeFrame 1 ABI; macOS bundle path uses CoreFoundation | Compiled and selected as fallback. An isolated metadata-only ABI fixture triggered SIGSEGV during startup discovery. `Freeframe::Freeframe` reads `info` and calls `plugmain` before `open` loads them. Plugin discovery and processing are unresolved for this reference. |
| Embedded JavaScript | `freej -j script.js`, positional `.js`, `include` / `use`, console script commands; startup `keyboard.js` lookup in user, site and data paths | `WITH_JAVASCRIPT`, bundled `lib/javascript` here; configuration also probes compatible system SpiderMonkey SDKs | Startup, registry, local show, recording and streaming passed with assertions. `-DNDEBUG` suppresses Context initialization and JS startup. Installed examples are under `share/doc/freej/scripts/javascript`. |
| Experimental Processing | `freej -p sketch.pde`; engine includes `processing.js` and invokes `Processing(script)`; retained `scripts/processing` and `scripts/javascript/lib/processing.js` | Embedded JS and compatibility library | CLI path retained. Processing conversion/rendering was untested. The Linux install copies the JS library as documentation and leaves the separate Processing source tree in the checkout. |
| Native C++ library | Link `pkg-config --cflags --libs freej` from a configured prefix; use source interfaces for `Context`, `Layer`, `ViewPort`, `Controller` and `Filter` (see `API`) | `libfreej.la`, resulting shared and static archives, pinned native dependencies | Staged metadata exposed version 1.0 and the dynamic library exported `Context::cafudda(double)`. Install ships only `freej.h` (Doxygen introduction) and historical `freej_plugin.h`; engine headers such as `context.h` remain source-tree interfaces. |
| Qt frontend | `qmake qt/Qfreej.pro`, target `qfreej`; Qt drives frames from `qfreej.cpp` and `FakeWindow.cpp`, with its own encoder/JACK controls | Qt project references local `libfreej.a`, `libmozjs.a` and a separate SDL, Cairo, CWiid, codec, shout, JACK, FFTW, samplerate and liblo link list | Qt 5.15.15 generated a Makefile. CWiid lookup failed for the project's link list. Qt is outside the tested Autotools install. A compatible Qt build, frame display, file/stream and JACK controls remain platform checks. |
| macOS/Xcode frontend | `xcode/freej.xcodeproj`, target `freej`; Cocoa layers/controllers, QuickTime capture/playback and Core Image paths | macOS SDK, Cocoa, Carbon, QuickTime, QTKit and the Xcode project’s native dependency configuration | Source/project retained. No macOS toolchain or runtime was exercised on Linux. Verify build, window composition, capture, playback and effects on a compatible Mac rig. |
| Python and Ruby samples, GTK/Pygame/PyGLUT | Historical `scripts/python` and `scripts/ruby` imports of FreeJ, including `scripts/python/gtkgui` | Historical SWIG bindings removed in `3fb0744b`; current `configure.ac` has no Python/Ruby binding targets | Script samples are installed as documentation. No built Python/Ruby extension was present. GTK, Pygame and PyGLUT samples need their historical bindings/runtimes; the separate old C++ GTK controller is in `src/obsolete`. |

## Installed components and retained source

The reference stages contained `bin/freej`, `lib/libfreej.so*`,
`lib/libfreej.a`, `lib/libfreej.la`, `lib/pkgconfig/freej.pc`, the two
installed headers, the man page, and documentation including JavaScript,
Python and Ruby examples. Pkg-config selected the isolated SDK's SDL_gfx
and SDL_ttf packages. Their `lib/` trees are fallback implementations
when system probes fail. `lib/ccvt`, `lib/slw` and static SpiderMonkey
participate in the build. `lib/flash` is conditional. The optional dmalloc
developer allocator (`--with-dmalloc`, `WITH_DMALLOC`) was disabled.
Plugins, cameras, controllers, fonts, displays, JACK and Icecast are runtime
resources. FreeBSD host detection remains in configuration and was untested.

Retained translation units outside the Linux Automake target include the
deactivated movie pair, `ffdec.c`, `ffenc.c`, `oggenc.c`, `impl_layers.cpp`,
`theora11_encoder.cpp`, `audio_input.cpp`, `console_keys_ctrl.cpp`,
`shouter.cpp`, `past_blits.cpp`, `osd.cpp`, `sdlxv_screen.cpp` and
`yuv_screen.cpp`. The built `AudioLayer` also lacks an active selection
path in the CLI or JS registry. `src/obsolete/` retains Avifile, GOOM,
old GTK, OSD, scrolling/text, V4L1 and old font/scale sources.
`USAGE` references old Avifile, GTK, OSD and encoder key routes. Its
VJoE (VeeJay over Ethernet) instructions describe an SSH terminal session
for the console. Direct SSH console behavior was untested. Those
references survive as historical documentation alongside the current paths
above.

## Media checks

Use a fresh directory with a local PNG and the staged executable and SDK
loader paths from [the build recipe](legacy-reference.md). For example, on an
SDL display or with the dummy video driver:

```javascript
set_clear_all();
set_fps(20);
var image = new ImageLayer();
image.open("fixture.png");
image.start();
add_layer(image);
run();
run();
var encoder = new VideoEncoder(50, 64000, 0, 0);
register_encoder(encoder);
encoder.start_filesave("record.ogg");
for (var frame = 0; frame < 72; ++frame) {
    image.set_position(frame % 15, 4);
    run();
}
encoder.stop_filesave();
quit();
```

Run that script with `freej -c -s 80x64 -j record.js`. The startup script's
`quit()` exits 1 on this historical CLI. Inspect the recorded frames with
`ffprobe -v error -count_frames -show_streams record.ogg` and decode with
`ffmpeg -v error -xerror -i record.ogg -f null -` using an independent decoder.
For streaming, configure an isolated Icecast source and mount on loopback,
set matching `stream_host`, `stream_port`, `stream_mountpoint`,
`stream_username` and `stream_password`, then replace the file methods with
`start_stream` and `stop_stream`. Capture the local listener body and
independently decode it. The examples in the checkout can contain old remote
endpoint settings. The reference test used a loopback-only sink.

For the historical movie probe, generate an 80×64 MPEG-4 clip with
`ffmpeg -f lavfi -i testsrc2=size=80x64:rate=20 -t 0.5 -c:v mpeg4 fixture.mp4`,
verify its frames with an independent decoder, then supply the local MP4 to
`freej -c -D 3 -s 80x64 fixture.mp4`. The affected Linux reference exits after
the missing movie factory lookup.
