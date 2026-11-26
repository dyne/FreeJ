#ifndef FREEJ_FRAME_PROBE_FIXTURE_H
#define FREEJ_FRAME_PROBE_FIXTURE_H

#include <array>
#include <cstdint>
#include <cstdio>

namespace frame_probe_fixture {

constexpr int image_width = 3;
constexpr int image_height = 2;
constexpr int bundle_width = 8;
constexpr int bundle_height = 8;
constexpr int screen_width = 5;
constexpr int screen_height = 3;
constexpr std::uint32_t background = 0x12345678;
constexpr std::array<long, 4> times_us{{40000, 80000, 205000, 245000}};
constexpr std::array<int, 4> positions{{1, 1, -1, -1}};

// BGRA bytes vary with both row and column to expose a wrong crop origin.
constexpr std::array<std::uint8_t, image_width * image_height * 4> image_pixels{{
  0x10, 0x20, 0x30, 0xff, 0x11, 0x21, 0x31, 0xff, 0x12, 0x22, 0x32, 0xff,
  0x40, 0x50, 0x60, 0xff, 0x41, 0x51, 0x61, 0xff, 0x42, 0x52, 0x62, 0xff
}};

using BundleImage = std::array<std::uint32_t, bundle_width * bundle_height>;

inline BundleImage bundle_image_pixels() {
  BundleImage pixels{};
  auto *bytes = reinterpret_cast<unsigned char *>(pixels.data());
  for (int i = 0; i < bundle_width * bundle_height; ++i) {
    bytes[4 * i] = 0x10 + i;
    bytes[4 * i + 1] = 0x40 + i;
    bytes[4 * i + 2] = 0x70 + i;
    bytes[4 * i + 3] = 0x90 + i;
  }
  return pixels;
}

// Layer::do_filters passes a float to FilterInstance::process.
inline float effect_time(int tick) {
  return static_cast<float>(times_us.at(tick) / 1000000.0);
}

inline void report_input(int tick) {
  std::printf("frame-probe input %d %d\n", tick, positions.at(tick));
}

inline void report_present(int tick, const void *pixels) {
  auto *bytes = static_cast<const unsigned char *>(pixels);
  std::printf("frame-probe present %d ", tick);
  for (int i = 0; i < screen_width * screen_height * 4; ++i)
    std::printf("%02x", static_cast<unsigned>(bytes[i]));
  std::putchar('\n');
}

inline void report_time(int tick, long micros) {
  std::printf("frame-probe time %d %ld\n", tick, micros);
}

}

#endif
