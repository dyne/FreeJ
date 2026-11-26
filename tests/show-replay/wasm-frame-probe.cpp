#include "frame_tick.h"
#include "frame-probe-fixture.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>

#include <frei0r/bundle.h>

namespace {

using namespace frame_probe_fixture;

class MemoryFrameTick final : public freej_engine::FrameTick {
public:
  MemoryFrameTick(const f0r_plugin_descriptor_t &effect, f0r_instance_t instance)
    : effect_(effect), instance_(instance), source_(bundle_image_pixels()) {}

  void dispatch_input() override {
    report_input(tick_);
    position_ = positions.at(tick_);
  }

  bool select_first_screen() override { return true; }
  bool select_next_screen() override { return false; }
  void clear_screen() override { pixels_.fill(background); }
  void resize_screen() override {}

  void blit_screen() override {
    effect_.update(instance_, effect_time(tick_), source_.data(), filtered_.data());
    const int left = std::max(0, position_);
    const int top = std::max(0, position_);
    const int right = std::min(screen_width, position_ + bundle_width);
    const int bottom = std::min(screen_height, position_ + bundle_height);
    for (int y = top; y < bottom; ++y) {
      const auto *row = filtered_.data() + (y - position_) * bundle_width;
      std::copy_n(row + left - position_, right - left, pixels_.data() + y * screen_width + left);
    }
  }

  void present_screen() override { report_present(tick_, pixels_.data()); }
  void collect_garbage() override {}
  void sample_completion_time() override {
    report_time(tick_, times_us.at(tick_));
    ++tick_;
  }

private:
  const f0r_plugin_descriptor_t &effect_;
  const f0r_instance_t instance_;
  alignas(16) const BundleImage source_;
  alignas(16) BundleImage filtered_{};
  std::array<std::uint32_t, screen_width * screen_height> pixels_{};
  int tick_ = 0;
  int position_ = 0;
};

}

int main() {
  const auto *effect = f0r_bundle_plugin_by_id("invert0r");
  if (!effect || effect->descriptor_version != F0R_PLUGIN_DESCRIPTOR_VERSION ||
      effect->descriptor_size < F0R_PLUGIN_DESCRIPTOR_SIZE || !effect->update || !effect->init()) {
    std::fputs("cannot initialize built-in Invert0r\n", stderr);
    return 1;
  }
  auto instance = effect->construct(bundle_width, bundle_height);
  if (!instance) {
    effect->deinit();
    std::fputs("cannot construct built-in Invert0r\n", stderr);
    return 1;
  }
  MemoryFrameTick host(*effect, instance);
  for (std::size_t tick = 0; tick < times_us.size(); ++tick)
    freej_engine::run_frame_tick(host);
  effect->destruct(instance);
  effect->deinit();
}
