#include <context.h>
#include <generator_layer.h>
#include <frei0r_freej.h>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#ifdef FREEJ_STRICT_STATIC
#include <frei0r/bundle.h>
extern "C" const f0r_plugin_descriptor_t freej_fixture_descriptor;
extern "C" int freej_fixture_kind;
#endif
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

struct PluggerTestAccess {
#ifdef FREEJ_STRICT_STATIC
  static bool bind(Context &env, const f0r_plugin_descriptor_t *descriptor) {
    return env.plugger.register_builtin(&env, descriptor);
  }
#endif
  static void add(Plugger &plugger, const char *dir) { plugger.addsearchdir(dir); }
};

namespace {
void require(bool ok, const char *message) {
  if (!ok) throw std::runtime_error(message);
}

struct Input : Layer {
  void *feed() override { return nullptr; }
};

// The public FilterInstance entry point takes float. Check the exact value
// that the host promotes to the frei0r callback's double.
constexpr float fractional_time = 0.123456789f;

void expect_timestamp(const uint32_t *frame, double expected) {
  double timestamp;
  std::memcpy(&timestamp, frame + 2, sizeof(timestamp));
  require(std::memcmp(&timestamp, &expected, sizeof(timestamp)) == 0,
          "timestamp changed before the effect callback");
}

void parameters(FilterInstance &instance, GeneratorLayer *layer = nullptr) {
  require(instance.parameters.len() == 5, "parameter count");
  auto *toggle = instance.parameters.pick(1);
  require(instance.get_parameter(1) && *static_cast<bool *>(toggle->get()), "ABI boolean threshold");
  for (int step = 0; step < 64; ++step) {
    bool enabled = step % 2;
    double n = step / 64.0;
    double color[] = {n, 0.25, 0.875};
    double point[] = {0.5, n};
    void *values[] = {&enabled, &n, color, point};
    const size_t sizes[] = {sizeof(enabled), sizeof(n), sizeof(color), sizeof(point)};
    for (int idx = 1; idx <= 4; ++idx) {
      auto *param = instance.parameters.pick(idx);
      if (layer) {
        require(param->set(values[idx-1]) && layer->set_parameter(idx), "layer setter");
        std::memset(param->value, 0xff, sizes[idx-1]);
        require(param->layer_get_f != nullptr, "layer getter registration");
        param->layer_get_f(layer, param, idx);
      } else {
        require(instance.set_parameter_value(idx, values[idx-1]), "filter setter");
        std::memset(param->value, 0xff, sizes[idx-1]);
        require(instance.get_parameter(idx), "filter getter");
      }
      require(std::memcmp(param->get(), values[idx-1], sizes[idx-1]) == 0, "round trip");
    }
  }
  char text[] = "unchanged";
  require(!instance.set_parameter_value(5, text) && !instance.get_parameter(5), "unsupported string");
  require(!instance.get_parameter(0) && !instance.set_parameter(6), "one-based index bounds");
  if (layer) require(!layer->set_parameter(5), "unsupported generator string");
}

void real_effects(Context &context, Input &input) {
  GeneratorLayer source;
  source.init(16, 8, 32);
  source.register_generators(&context.generators);
  require(source.open("onecol0r"), "real onecol0r discovery");
  double c[] = {1.0, 0.5, 0.25};
  require(source.parameters->pick(1)->set(c) && source.set_parameter(1), "onecol0r color");
  auto *generated = static_cast<uint8_t *>(source.feed());
  const uint8_t expected[] = {63, 127, 255, 255};
  require(std::memcmp(generated + 4 * 127, expected, 4) == 0, "real source BGRA sentinel");
  auto *invert = context.filters.search("Invert0r");
  require(invert != nullptr, "real Invert0r discovery");
  std::unique_ptr<FilterInstance> filter(invert->apply(&input));
  require(filter != nullptr, "Invert0r construct");
  generated[3] = 91;
  auto *out = static_cast<uint8_t *>(input.do_filters(generated));
  const uint8_t inverted[] = {192, 128, 0, 91};
  require(std::memcmp(out, inverted, 4) == 0, "Invert0r RGB and preserved alpha");
}

// Emit the same three full frames at each cue for artifact comparison across profiles.
void write_frames(const char *path) {
  Context env;
  Input input;
  require(input.init(16, 8, 32), "parity input");
  real_effects(env, input);
  GeneratorLayer source;
  source.register_generators(&env.generators);
  require(source.init(16, 8, 32) && source.open("onecol0r"), "onecol0r by show name");
  auto *inv = env.filters.search("Invert0r");
  auto *br = env.filters.search("Brightness");
  require(inv && br, "filter show names");
  std::unique_ptr<FilterInstance> invert(inv->apply(&input)), bright(br->apply(&input));
  require(invert && bright, "parity filter construct");
  auto *color_param = source.parameters->pick(1);
  auto *brightness_param = bright->parameters.pick(1);
  require(source.parameters->len() == 1 && color_param &&
          std::string(color_param->name) == "Color" && color_param->type == Parameter::COLOR,
          "onecol0r Color parameter metadata");
  require(invert->parameters.len() == 0 && bright->parameters.len() == 1 && brightness_param &&
          std::string(brightness_param->name) == "Brightness" && brightness_param->type == Parameter::NUMBER,
          "filter parameter metadata");
  require(!env.filters.search("absent-plugin") && !source.open("absent-plugin"), "unknown show name");
  std::ofstream out(path, std::ios::binary);
  out.exceptions(std::ios::badbit | std::ios::failbit);
  for (int tick = 0; tick < 8; ++tick) {
    double time = tick * 0.125;
    double color[] = {tick / 8.0, 1.0 - tick / 8.0, 0.25};
    double amount = tick / 8.0;
    require(color_param->set(color) && source.set_parameter(1) &&
            bright->set_parameter_value(1, &amount), "parity automation");
    require(source.generator->get_parameter(1) && bright->get_parameter(1) &&
            std::memcmp(color_param->get(), color, sizeof(color)) == 0 &&
            *static_cast<double *>(brightness_param->get()) == amount, "parity parameter readback");
    auto *generated = source.generator->process(time, nullptr);
    auto *inverted = invert->process(time, generated);
    auto *adjusted = bright->process(time, inverted);
    for (auto *frame : {generated, inverted, adjusted})
      out.write(reinterpret_cast<const char *>(frame), 16 * 8 * 4);
  }
  out.close();
  std::puts("onecol0r Invert0r Brightness: 8 timestamped cues, 24 full frames");
}

#ifdef FREEJ_STRICT_STATIC
void check_builtin() {
  char trace[] = "frei0r-static-trace-XXXXXX";
  int fd = mkstemp(trace);
  require(fd >= 0, "static test trace");
  close(fd);
  setenv("FREEJ_EFFECT_TRACE", trace, 1);
  const auto &fixture = freej_fixture_descriptor;
  require(!f0r_bundle_plugin_by_id("absent-plugin") && !f0r_bundle_plugin_by_index(f0r_bundle_plugin_count()),
          "out-of-range registry lookup");
  {
    Context env;
    require(env.filters.len() == 2 && env.generators.len() == 1 &&
            env.plugger.refresh(&env) == 0 && env.filters.len() == 2 && env.generators.len() == 1,
            "shipping registry inventory or idempotence");
    auto bind = [&](const f0r_plugin_descriptor_t *d) { return PluggerTestAccess::bind(env, d); };
    auto bad = fixture;
    require(!bind(nullptr), "null descriptor");
    bad.descriptor_size--;
    require(!bind(&bad), "short descriptor");
    bad = fixture; ++bad.descriptor_version;
    require(!bind(&bad), "unknown descriptor version");
    bad = fixture; bad.id = nullptr;
    require(!bind(&bad), "missing ID");
    bad = fixture; bad.id = "";
    require(!bind(&bad), "empty ID");
    bad = fixture; bad.get_param_value = nullptr;
    require(!bind(&bad), "missing callback");
    bad = fixture; bad.init = []() { return 0; };
    require(!bind(&bad), "failed init");
    bad = fixture; bad.update = nullptr;
    require(!bind(&bad), "missing single-input update");
    freej_fixture_kind = F0R_PLUGIN_TYPE_MIXER3;
    require(!bind(&fixture) && !env.filters.search("Embedded clock"), "mixer routed");
    freej_fixture_kind = F0R_PLUGIN_TYPE_FILTER;
  }
  {
    std::unique_ptr<Context> env(new Context);
    require(PluggerTestAccess::bind(*env, &fixture) && !PluggerTestAccess::bind(*env, &fixture),
            "register once by canonical ID");
    auto duplicate = fixture;
    duplicate.init = []() -> int { std::abort(); };
    require(!PluggerTestAccess::bind(*env, &duplicate), "duplicate descriptor initialized");
    {
      Context mirror;
      require(PluggerTestAccess::bind(mirror, &fixture), "second context registered static effect");
    }
    Filter *f = env->filters.search("Embedded clock");
    require(f && !env->filters.search("test-clock"), "separate stable ID and display name");
    Input layer;
    layer.init(88, 8, 32);
    require(!f->apply(&layer), "failed static construct");
    layer.init(16, 8, 32);
    std::unique_ptr<FilterInstance> a(f->apply(&layer)), b(f->apply(&layer));
    require(a && b, "two static instances");
    parameters(*a);
    alignas(16) std::array<uint32_t, 128> pixels{};
    layer.fps.set(37);
    expect_timestamp(static_cast<uint32_t *>(layer.do_filters(pixels.data())), 37.0);
    env.reset();
    a.reset();
    require(b->get_parameter(1), "static provider released early");
    expect_timestamp(b->process(fractional_time, pixels.data()), fractional_time);
  }
  {
    freej_fixture_kind = F0R_PLUGIN_TYPE_SOURCE;
    Context env;
    require(PluggerTestAccess::bind(env, &fixture), "static source registration");
    GeneratorLayer source;
    source.init(16, 8, 32);
    source.register_generators(&env.generators);
    require(source.open("Embedded clock"), "embedded source");
    parameters(*source.generator, &source);
    source.fps.set(37);
    auto *generated = static_cast<uint32_t *>(source.feed());
    expect_timestamp(generated, 0.0);
    require(generated[127] == 0xef563412, "static source frame");
    source.close();
    require(!source.parameters && source.open("Embedded clock"), "static source parameter alias");
  }
  std::ifstream events(trace);
  std::string row;
  unsigned init = 0, deinit = 0, construct = 0, destruct = 0;
  while (std::getline(events, row)) {
    init += row == "bound init"; deinit += row == "bound deinit";
    construct += row == "bound construct"; destruct += row == "bound destruct";
  }
  require(init == 4 && deinit == init && construct == 4 && destruct == construct, "static lifecycle counts");
  unlink(trace);
  std::puts("embedded descriptors: malformed, failed init/construct, mixer, 512 parameter round trips, 4 instance pairs");
}
#else
void check(bool real) {
  std::unique_ptr<Context> context(new Context);
  Input input;
  require(input.init(16, 8, 32), "input initialization");
  input.fps.set(37);
  auto &plugger = context->plugger;
  PluggerTestAccess::add(plugger, "other");
  require(plugger.refresh(context.get()) == 0, "two-directory refresh");
  auto *prototype = context->filters.search("Clock filter");
  require(prototype && std::string(prototype->description()) == "filter", "user-directory name precedence");
  require(!context->filters.search("Clock mixer") && !context->generators.search("Clock mixer"), "mixer routed");
  require(!context->generators.search("Packed source"), "undefined source byte order admitted");
  require(!context->filters.search("Failed") && !context->filters.search("Missing"), "invalid plugin admitted");
  auto *ff = context->filters.search("FF fixture");
  require(ff && ff->type() == Filter::FREEFRAME, "FreeFrame discovery fallback");
  {
    std::unique_ptr<FilterInstance> fallback(ff->apply(&input));
    alignas(16) uint32_t pixels[128]{};
    require(fallback && fallback->process(0, pixels)[0] == 0xaa113344, "FreeFrame processing");
  }
  auto *native = static_cast<Freior *>(prototype);
  require(native->param_infos.size() == 5 && native->f0r_construct &&
          native->f0r_set_param_value && native->f0r_get_param_value, "native facade callbacks");
  if (real) real_effects(*context, input);
  Input rejected;
  rejected.init(88, 8, 32);
  require(!prototype->apply(&rejected), "failed filter construction");
  std::unique_ptr<FilterInstance> first(prototype->apply(&input));
  require(first != nullptr, "first filter instance");
  parameters(*first);
  alignas(16) std::array<uint32_t, 129> pixels;
  // Supply 4-byte aligned input beginning four bytes after a 16-byte boundary.
  // All bytes including the last pixel and its alpha must survive processing.
  for (size_t i = 0; i < pixels.size(); ++i) pixels[i] = 0xa4911230 + i;
  auto *out = static_cast<uint32_t *>(input.do_filters(pixels.data() + 1));
  expect_timestamp(out, 37.0);
  require(out[0] == 3700 && std::equal(out+4, out+128, pixels.begin()+5), "filter time/alignment/pixels");
  std::unique_ptr<FilterInstance> second(prototype->apply(&input));
  require(second != nullptr, "second filter instance");
  GeneratorLayer source;
  source.register_generators(&context->generators);
  source.init(16, 8, 32);
  require(source.open("Clock source"), "source discovery");
  parameters(*source.generator, &source);
  source.fps.set(37);
  auto *generated = static_cast<uint32_t *>(source.feed());
  require(generated && generated[0] == 0 && generated[127] == 0xef563412, "source clock or sentinel");
  expect_timestamp(generated, 0.0);
  // Historical FreeJ passes the BGRA source's physical byte lanes to the RGBA filter.
  const uint8_t *source_lanes = reinterpret_cast<uint8_t *>(generated + 1);
  const uint8_t *filtered_lanes = reinterpret_cast<uint8_t *>(first->process(37, generated) + 1);
  require(filtered_lanes[0] == (source_lanes[0] ^ 0xff) &&
          std::equal(source_lanes+1, source_lanes+4, filtered_lanes+1), "historic byte lane mapping");
  expect_timestamp(first->process(fractional_time, generated), fractional_time);
  source.close();
  require(!source.parameters && !source.set_parameter(1), "closed parameter alias");
  source.init(88, 8, 32);
  require(!source.open("Clock source") && !source.parameters, "failed reopen retained alias");
  source.init(16, 8, 32);
  require(source.open("Clock source") && source.parameters == &source.generator->parameters, "source reopen");
  require(std::rename("home/.freej/plugins/filter.so", "parked") == 0, "remove module");
  require(plugger.refresh(context.get()) == 0, "refresh after removal");
  auto *alternative = context->filters.search("Clock filter");
  require(alternative && std::string(alternative->description()) == "alternative", "second-directory precedence");
  require(std::rename("parked", "home/.freej/plugins/filter.so") == 0, "restore module");
  require(plugger.refresh(context.get()) == 0 &&
          std::string(context->filters.search("Clock filter")->description()) == "filter", "reload original name");
  context.reset();
  require(first->get_parameter(1) && second->get_parameter(1), "prototype retired before live filters");
  source.close();
  first.reset();
  require(second->get_parameter(1), "provider ended with another filter alive");
  second.reset();
}
#endif
}

int main(int argc, char **argv) {
  try {
    if (argc == 3 && std::string(argv[1]) == "--frames") {
      write_frames(argv[2]);
      return 0;
    }
#ifdef FREEJ_STRICT_STATIC
    check_builtin();
#else
    check(argc == 2);
    std::puts("verified directory effects, 512 parameter round trips, clocks, reload and provider lifetime");
#endif
  } catch (const std::exception &error) {
    std::fprintf(stderr, "%s\n", error.what());
    return 1;
  }
}
