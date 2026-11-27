#include <algorithm>
#include <atomic>
#include <chrono>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>

#include <context.h>
#include <controller.h>
#include <image_layer.h>
#include <logging.h>
#include <geo_layer.h>
#include <sdl_controller.h>
#include <sdl_screen.h>
#include <soft_screen.h>

#include "frame_step.h"
#include "frame_storage.h"
#include "frame-probe-fixture.h"

namespace {

void require(bool condition, const char *message) {
  if (!condition)
    throw std::runtime_error(message);
}

class FixtureDirectory {
public:
  FixtureDirectory() {
    char name[] = "frame-step.XXXXXX";
    const char *directory = mkdtemp(name);
    require(directory != nullptr, "create fixture directory");
    path = directory;
  }

  ~FixtureDirectory() {
    std::remove((path + "/rows.tga").c_str());
    rmdir(path.c_str());
  }

  std::string path;
};

void write_image(const std::string &path, char width, char height, const void *pixels) {
  // Uncompressed 32-bit TGA with alpha and the origin at the top left.
  const char header[18] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, width, 0, height, 0, 32, 0x28};
  std::ofstream image(path, std::ios::binary);
  image.write(header, sizeof(header));
  image.write(static_cast<const char *>(pixels), width * height * 4);
  image.close();
  require(bool(image), "write fixture image");
}

class FlipGreenFilter : public Filter {
public:
  FlipGreenFilter() { set_name("FlipGreen"); }
  int type() override { return FREEFRAME; }

protected:
  void init_parameters(Linklist<Parameter> &) override {}

  void update(FilterInstance *, double, uint32_t *input, uint32_t *output) override {
    for (int i = 0; i < bytesize / 4; ++i)
      output[i] = input[i] ^ green_bitmask;
  }
};

uint32_t expected_pixel(uint8_t red, uint8_t green, uint8_t blue) {
  return alpha_bitmask | ((red_bitmask / 255) * red) |
         ((green_bitmask / 255) * green) | ((blue_bitmask / 255) * blue);
}

void memory_frames(const FixtureDirectory &fixture) {
  using namespace frame_probe_fixture;
  constexpr int width = screen_width, height = screen_height;
  const uint32_t a = expected_pixel(0x30, 0xdf, 0x10);
  const uint32_t b = expected_pixel(0x31, 0xde, 0x11);
  const uint32_t c = expected_pixel(0x32, 0xdd, 0x12);
  const uint32_t d = expected_pixel(0x60, 0xaf, 0x40);
  const uint32_t e = expected_pixel(0x61, 0xae, 0x41);
  const uint32_t f = expected_pixel(0x62, 0xad, 0x42);
  const std::array<uint32_t, width * height> offset{{
    background, background, background, background, background,
    background, a, b, c, background,
    background, d, e, f, background
  }};
  const std::array<uint32_t, width * height> cropped{{
    e, f, background, background, background,
    background, background, background, background, background,
    background, background, background, background, background
  }};

  Context context;
  FlipGreenFilter filter;
  std::unique_ptr<Layer> layer(Factory<Layer>::new_instance("ImageLayer", "sdl"));
  auto *image = dynamic_cast<ImageLayer *>(layer.get());
  std::unique_ptr<ViewPort> viewport(Factory<ViewPort>::new_instance("Screen", "soft"));
  auto *screen = dynamic_cast<SoftScreen *>(viewport.get());
  require(image && screen, "registered SDL image and memory screen factories");
  require(screen->init(width, height, 32), "initialize memory screen");
  std::unique_ptr<void, decltype(&std::free)> pixels(std::malloc(screen->geo.bytesize), std::free);
  require(bool(pixels), "allocate memory screen replacement");
  void *replacement = pixels.get();
  screen->set_buffer(pixels.release());
  require(screen->get_surface() == replacement, "screen adopted the allocated buffer");
  require(context.add_screen(screen), "attach memory screen");
  viewport.release();  // Context will destroy the screen and its buffer.

  const std::string source = fixture.path + "/rows.tga";
  write_image(source, image_width, image_height, image_pixels.data());
  require(image->open(source.c_str()), "load the 3x2 image through ImageLayer");
  require(image->geo.w == 3 && image->geo.h == 2, "decoded image size");
  require(screen->add_layer(image), "attach the image to the memory screen");
  require(filter.apply(image) != nullptr, "attach filter instance");
  void *decoded = image->feed();
  require(decoded != nullptr, "read decoded image");
  // Publish through the same filter path as the worker, with the fixture
  // controlling when publication happens.
  image->buffer = image->do_filters(decoded);
  require(image->buffer != nullptr && image->buffer != decoded, "filtered frame did not publish");

  auto *const surface = static_cast<uint32_t *>(screen->get_surface());
  std::vector<std::string> trace;
  std::vector<long> completion_times;
  const auto &requested_times = times_us;
  int tick = 0;
  int presentations = 0;
  const freej_engine::FrameHost host{
    [&] {
      trace.push_back("input");
      image->geo.x = positions.at(tick);
      image->geo.y = positions.at(tick);
      image->need_crop = true;
      std::fill_n(surface, width * height, background);
    },
    [&](ViewPort &presented) {
      trace.push_back("present");
      require(&presented == screen, "unexpected presented screen");
      const auto &expected = tick < 2 ? offset : cropped;
      require(std::equal(expected.begin(), expected.end(), surface), "raw memory frame differs");
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          const auto *pixel = static_cast<uint32_t *>(context.coords(x, y));
          require(pixel == surface + y * width + x, "coordinate address uses the wrong row");
          require(*pixel == expected[y * width + x], "coordinate pixel differs");
        }
      }
      ++presentations;
    },
    [&] {
      trace.push_back("time");
      return timeval{100, requested_times.at(tick)};
    }
  };

  // The pause flag historically leaves presentation active. The supplied
  // clock includes a slow tick, with no extra presentation to catch up.
  for (; tick < static_cast<int>(requested_times.size()); ++tick) {
    context.pause = tick == 1;
    const timeval done = freej_engine::step_frame(context, host);
    require(done.tv_sec == 100, "frame completion did not use the host clock");
    completion_times.push_back(done.tv_usec);
  }
  require(std::equal(requested_times.begin(), requested_times.end(), completion_times.begin()),
          "completion times differ from the fake clock");
  require(presentations == tick, "one presentation per requested tick");
  require(trace.size() == 3 * requested_times.size(), "unexpected number of frame events");
  for (size_t i = 0; i < trace.size(); i += 3)
    require(trace[i] == "input" && trace[i + 1] == "present" && trace[i + 2] == "time",
            "controller, presentation or clock order changed");
  screen->rem_layer(image);
}

#ifdef FREEJ_STRICT_STATIC
void bundle_reference(const FixtureDirectory &fixture) {
  using namespace frame_probe_fixture;
  Context context;
  ImageLayer image;
  std::unique_ptr<SoftScreen> screen(new SoftScreen);
  require(screen->init(screen_width, screen_height, 32), "initialize memory screen");
  require(context.add_screen(screen.get()), "attach memory screen");
  auto *viewport = screen.release(); // Context owns the screen and its buffer.
  const std::string source = fixture.path + "/rows.tga";
  const auto source_pixels = bundle_image_pixels();
  write_image(source, bundle_width, bundle_height, source_pixels.data());
  require(image.open(source.c_str()), "open bundle reference image");
  require(viewport->add_layer(&image), "attach bundle reference image");

  auto *invert = context.filters.search("Invert0r");
  require(invert != nullptr, "discover built-in Invert0r");
  std::unique_ptr<FilterInstance> instance(invert->apply(&image));
  require(instance != nullptr, "construct built-in Invert0r");
  void *decoded = image.feed();
  require(decoded && std::memcmp(decoded, source_pixels.data(), sizeof(source_pixels)) == 0,
          "native source bytes match the portable source");

  auto *surface = static_cast<uint32_t *>(viewport->get_surface());
  int tick = 0;
  const freej_engine::FrameHost host{
    [&] {
      report_input(tick);
      image.geo.x = image.geo.y = positions.at(tick);
      image.need_crop = true;
      image.fps.fps = effect_time(tick);
      std::fill_n(surface, screen_width * screen_height, background);
      image.buffer = image.do_filters(decoded);
      require(image.buffer != nullptr && image.buffer != decoded, "publish Invert0r frame");
    },
    [&](ViewPort &presented) {
      require(&presented == viewport, "unexpected presented screen");
      report_present(tick, surface);
    },
    [&] {
      report_time(tick, times_us.at(tick));
      return timeval{100, times_us.at(tick)};
    }
  };
  for (; tick < static_cast<int>(times_us.size()); ++tick)
    require(freej_engine::step_frame(context, host).tv_usec == times_us.at(tick),
            "unexpected frame completion time");
  viewport->rem_layer(&image);
}
#endif

class CountedScreen : public SdlScreen {
public:
  int presentations = 0;

  void show() override {
    SdlScreen::show();
    ++presentations;
  }
};

class StopController : public Controller {
public:
  int polls = 0;
  int stop_on = 0;

  int poll() override {
    if (++polls == stop_on)
      env->quit = true;
    return 0;
  }

  int dispatch() override { return 0; }
};

class KeyboardEvents : public SdlController {
public:
  std::vector<Uint32> events;

  int poll() override {
    poll_sdlevents(SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);
    return 0;
  }

  int dispatch() override {
    events.push_back(event.type);
    return event.type == SDL_EVENT_KEY_DOWN;
  }
};

class CapturedLog : public Logger {
public:
  CapturedLog() : previous_(GlobalLogger::get_loglevel()) {
    require(GlobalLogger::register_logger(this), "capture rendering diagnostics");
    set_debug(1);
  }

  ~CapturedLog() {
    GlobalLogger::unregister_logger(this);
    GlobalLogger::set_loglevel(previous_);
  }

  int vprintlog(LogLevel level, const char *format, va_list args) override {
    char text[1024];
    const int length = std::vsnprintf(text, sizeof(text), format, args);
    entries.emplace_back(level, text);
    return length;
  }

  unsigned count(LogLevel level, const char *text) const {
    return std::count_if(entries.begin(), entries.end(), [&](const std::pair<LogLevel, std::string> &entry) {
      return entry.first == level && entry.second.find(text) != std::string::npos;
    });
  }

private:
  LogLevel previous_;
  std::vector<std::pair<LogLevel, std::string>> entries;
};

void check_rendering_diagnostics(SdlScreen &screen, Layer &layer) {
  CapturedLog log;
  warning("warning at normal verbosity");
  func("trace at normal verbosity");
  require(log.count(WARNING, "warning at normal verbosity") == 1 &&
          log.count(DEBUG, "trace at normal verbosity") == 0, "normal logging includes warnings");

  auto *surface = screen.sdl_screen;
  screen.sdl_screen = nullptr;
  for (int frame = 0; frame < 4; ++frame) {
    screen.blit(&layer);
    screen.show();
  }
  screen.sdl_screen = surface;
  screen.blit(&layer);
  screen.show();

  require(log.count(ERROR, "blit layer pixels failed:") == 1 &&
          log.count(ERROR, "copy frame to window failed:") == 1,
          "each rendering operation reports only its first error");
  require(log.count(NOTICE, "recovered after 4 failed attempt(s)") == 2,
          "each rendering operation reports recovery and the failed-attempt count");
}

void check_clipping(Context &context, ViewPort &screen, bool native) {
  const auto red = expected_pixel(240, 20, 10);
  const auto blue = expected_pixel(10, 30, 210);
  const auto background = expected_pixel(15, 25, 35);
  const auto rgb = red_bitmask | green_bitmask | blue_bitmask;
  GeoLayer layer;
  require(layer.init(3, 2, 32), "initialize 3x2 drawing");
  layer.set_color(240, 20, 10);
  require(layer.clear() == 0 && layer.pixel(1, 1, 0x0a1ed2ff) == 0, "paint the fixture");
  require(screen.add_layer(&layer), "attach the drawing");
  layer.buffer = layer.feed();
  const auto solid = {std::array<int, 2>{1, 1}, {-2, 1}, {4, 1}, {1, -1}, {1, 3},
                      {-3, 1}, {5, 1}, {1, -2}, {1, 4}, {1, 1}};
  for (const auto &point : solid) {
    layer.geo.x = point[0];
    layer.geo.y = point[1];
    layer.need_crop = true;
    const int width = screen.geo.w, height = screen.geo.h;
    auto *pixels = static_cast<uint32_t *>(screen.get_surface());
    std::fill_n(pixels, width * height, background);
    screen.blit_layers();
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        const int sx = x - point[0], sy = y - point[1];
        const auto expected = sx >= 0 && sx < 3 && sy >= 0 && sy < 2
            ? (sx == 1 && sy == 1 ? blue : red) : background;
        require(screen.coords(x, y) == pixels + y * width + x, "viewport row address");
        require((pixels[y * width + x] & rgb) == (expected & rgb), "clipped drawing pixel");
      }
    }
  }
  if (native) {
    require(!screen.coords(-1, 0) && !screen.coords(0, screen.geo.h), "out-of-range SDL coordinates");
    require(layer.set_blit("RGB"), "select native linear blit");
    layer.rotating = true;
    layer.rotate = 33;
    layer.geo.x = -9;
    layer.need_crop = true;
    context.resize(7, 5);
    screen.handle_resize();
    require(screen.geo.w == 7 && screen.geo.h == 5, "resize SDL drawing target");
    auto *pixels = static_cast<uint32_t *>(screen.get_surface());
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    require(std::all_of(pixels, pixels + 35, [&](uint32_t p) { return p == background; }),
            "rotated layer wholly outside resized viewport");
    layer.geo.x = 0;
    layer.need_crop = true;
    screen.blit_layers();
    require(std::any_of(pixels, pixels + 35, [&](uint32_t p) { return p == red; }),
            "rotated layer returns into resized viewport");

    // Stop the producer after each publish so two draws reuse one generation.
    auto publish = [&] {
      const auto initial = freej_engine::FrameAccess::layer(layer);
      require(layer.start() == 0, "start the geometry producer");
      const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
      auto current = freej_engine::FrameAccess::layer(layer);
      while (current == initial && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        current = freej_engine::FrameAccess::layer(layer);
      }
      layer.stop();
      require(current && current != initial, "publish a new drawing frame");
    };
    publish();
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    const std::vector<uint32_t> first(pixels, pixels + 35);
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    require(std::equal(first.begin(), first.end(), pixels), "cached transform retains its pixels");
    layer.rotate = 90;
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    require(!std::equal(first.begin(), first.end(), pixels), "transform settings invalidate the cache");
    layer.set_color(0, 255, 0);
    require(layer.clear() == 0, "paint a new generation");
    publish();
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    const auto green = expected_pixel(0, 255, 0);
    require(std::any_of(pixels, pixels + 35, [&](uint32_t p) { return p == green; }),
            "new frame invalidates the cached transform");

    layer.rotating = false;
    layer.geo.x = layer.geo.y = 0;
    require(layer.set_blit("SRCALPHA"), "select source alpha");
    layer.set_color(255, 0, 0, 0);
    require(layer.clear() == 0 && layer.pixel(1, 1, 0x0a1ed2ff) == 0,
            "render over transparent clear");
    layer.buffer = layer.feed();
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    require((pixels[0] & rgb) == (background & rgb) && (pixels[8] & rgb) == (blue & rgb),
            "transparent pixels preserve the background and opaque pixels cover it");

    layer.set_color(200, 100, 50, 128);
    require(layer.clear() == 0, "paint translucent pixels");
    layer.buffer = layer.feed();
    std::fill_n(pixels, 35, background);
    screen.blit_layers();
    uint8_t r, g, b;
    auto *surface = static_cast<SdlScreen &>(screen).sdl_screen;
    require(SDL_ReadSurfacePixel(surface, 0, 0, &r, &g, &b, nullptr) &&
            std::abs(int(r) - 108) <= 1 && std::abs(int(g) - 63) <= 1 &&
            std::abs(int(b) - 43) <= 1, "half-alpha drawing blends with the viewport");
    check_rendering_diagnostics(static_cast<SdlScreen &>(screen), layer);
  }
  screen.rem_layer(&layer);
}

void drawing_frames() {
  require(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE), "select SDL dummy video");
  for (bool native : {false, true}) {
    Context context;
    std::unique_ptr<ViewPort> screen(native ? static_cast<ViewPort *>(new SdlScreen) : new SoftScreen);
    require(screen->init(5, 4, 32) && context.add_screen(screen.get()), "attach drawing viewport");
    check_clipping(context, *screen.release(), native);
  }
  GeoLayer layer;
  require(layer.init(96, 64, 32), "initialize the concurrent drawing fixture");
  const uint32_t red = expected_pixel(255, 0, 0);
  const uint32_t blue = expected_pixel(0, 0, 255);
  layer.set_color(255, 0, 0);
  require(layer.clear() == 0, "paint the canvas red");
  auto *published = static_cast<uint32_t *>(layer.feed());
  layer.set_color(0, 0, 255);
  require(layer.clear() == 0 && published[0] == red &&
          *static_cast<uint32_t *>(layer.feed()) == blue, "drawing does not modify the worker snapshot");
  std::atomic<bool> done{false};
  std::atomic<unsigned> draws{0};
  std::thread writer([&] {
    while (!done) {
      layer.set_color(draws.load() % 2 ? 0xff0000ff : 0x0000ffff);
      layer.clear();
      ++draws;
    }
  });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!draws && std::chrono::steady_clock::now() < deadline)
    std::this_thread::yield();
  bool coherent = draws > 0;
  for (int sample = 0; coherent && sample < 128; ++sample) {
    const auto *pixels = static_cast<const uint32_t *>(layer.feed());
    const auto color = pixels[0];
    coherent &= (color == red || color == blue) &&
        std::all_of(pixels, pixels + 96 * 64, [&](uint32_t p) { return p == color; });
    std::this_thread::yield();
  }
  done = true;
  writer.join();
  require(coherent, "a concurrent clear must publish a complete drawing snapshot");
}

void native_wrapper() {
  require(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "dummy", SDL_HINT_OVERRIDE), "select SDL dummy video");
  Context context;
  std::unique_ptr<CountedScreen> screen(new CountedScreen);
  require(screen->init(10, 6, 32), "initialize SDL dummy screen");
  require(context.add_screen(screen.get()), "attach SDL screen");
  auto *presented = screen.release();

  {
    KeyboardEvents keyboard;
    SDL_Event key = {};
    key.type = SDL_EVENT_KEY_DOWN;
    key.key.key = SDLK_A;
    require(SDL_PushEvent(&key), "enqueue key down");
    key.type = SDL_EVENT_KEY_UP;
    require(SDL_PushEvent(&key), "enqueue key up");
    keyboard.poll();
    require(keyboard.events == std::vector<Uint32>{SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP},
            "SDL keyboard events did not dispatch in order");
    require(!SDL_HasEvent(SDL_EVENT_KEY_DOWN), "handled key event was queued again");
    require(SDL_PeepEvents(&key, 1, SDL_GETEVENT, SDL_EVENT_KEY_UP, SDL_EVENT_KEY_UP) == 1
                && key.key.key == SDLK_A,
            "unhandled key event did not retain its payload");
  }

  std::unique_ptr<StopController> controller(new StopController);
  require(context.register_controller(controller.get()), "register stop controller");
  auto *stop = controller.release();

  context.pause = true;
  context.resize(6, 4);
  context.cafudda(20.0);
  require(presented->geo.w == 6 && presented->geo.h == 4 && !presented->resizing,
          "pending resize was not applied by the tick");
  require(presented->presentations == 1 && stop->polls == 1,
          "duration or pause changed the one-frame wrapper");
  context.pause = false;
  context.poll_events = false;
  context.cafudda(0.0);
  require(presented->presentations == 2 && stop->polls == 1,
          "controller polling should be optional");
  context.poll_events = true;

  for (int run = 0; run < 3; ++run) {
    stop->stop_on = stop->polls + 3;
    const int previous = presented->presentations;
    context.start();
    require(context.quit && stop->polls == stop->stop_on &&
                presented->presentations == previous + 3,
            "synchronous start/stop did not present precisely the requested frames");
  }
}

}  // namespace

int main(int argc, char **argv) {
  const std::string mode = argc == 2 ? argv[1] : "";
  if (argc > 2 ||
      (argc == 2 && mode != "--memory-only"
#ifdef FREEJ_STRICT_STATIC
       && mode != "--bundle-reference"
#endif
      ))
    return 2;
  const std::string test = argc == 2 ? mode : "memory frames and native wrapper";
  try {
    const FixtureDirectory fixture;
    require(setenv("HOME", fixture.path.c_str(), 1) == 0, "isolate fixture home");
#ifdef FREEJ_STRICT_STATIC
    if (mode == "--bundle-reference") {
      bundle_reference(fixture);
      return 0;
    }
#endif
    memory_frames(fixture);
    std::puts("PASS memory frames");
    if (argc == 1) {
      native_wrapper();
      std::puts("PASS native wrapper");
      drawing_frames();
      std::puts("PASS drawing bounds and snapshots");
    }
  } catch (const std::exception &error) {
    std::fprintf(stderr, "FAIL %s: %s\n", test.c_str(), error.what());
    return 1;
  }
  return 0;
}
