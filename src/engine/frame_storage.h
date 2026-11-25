#ifndef FREEJ_FRAME_STORAGE_H
#define FREEJ_FRAME_STORAGE_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class Layer;
class ViewPort;

namespace freej_engine {

enum class FrameFormat { rgba32, bgra32, argb32 };

struct FrameLayout {
  unsigned width;
  unsigned height;
  std::size_t stride_bytes;
  FrameFormat format;
};

struct Frame {
  FrameLayout layout{};
  std::uint64_t generation = 0;
  std::chrono::nanoseconds timestamp{};
  std::vector<std::uint32_t> pixels;
};

using FrameLease = std::shared_ptr<const Frame>;

// The producer serializes prepare, publish, and clear. Readers may hold an
// immutable generation across later publishes and destruction of the producer.
class FrameStore {
public:
  std::shared_ptr<Frame> prepare(const void *pixels, FrameLayout layout);
  void publish(std::shared_ptr<Frame> frame);
  FrameLease acquire() const;
  void clear();

private:
  std::shared_ptr<Frame> current_;
  std::shared_ptr<Frame> reusable_;
  std::uint64_t generation_ = 0;
};

struct FrameAccess {
  static FrameLease layer(const Layer &layer);
  static FrameLease viewport(const ViewPort &viewport);
  static void capture(ViewPort &viewport);
};

}

#endif
