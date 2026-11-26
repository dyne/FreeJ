#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

#include <context.h>
#include <controller.h>
#include <image_layer.h>
#include <sdl_screen.h>
#include <soft_screen.h>

#include "frame_step.h"
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

void native_wrapper() {
  require(setenv("SDL_VIDEODRIVER", "dummy", 1) == 0, "select SDL dummy video");
  Context context;
  std::unique_ptr<CountedScreen> screen(new CountedScreen);
  require(screen->init(10, 6, 32), "initialize SDL dummy screen");
  require(context.add_screen(screen.get()), "attach SDL screen");
  auto *presented = screen.release();
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
    }
  } catch (const std::exception &error) {
    std::fprintf(stderr, "FAIL %s: %s\n", test.c_str(), error.what());
    return 1;
  }
  return 0;
}
